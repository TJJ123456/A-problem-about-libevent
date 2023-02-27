#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/thread.h>
#include <event2/bufferevent.h>
#include <thread>
#include <string>
#include <iostream>
#include <condition_variable>
#include <functional>
#include <queue>

using namespace std;

void deal_request(evhttp_request *request)
{
    const struct evhttp_uri *evhttp_uri = evhttp_request_get_evhttp_uri(request);
    char url[8192];
    evhttp_uri_join(const_cast<struct evhttp_uri *>(evhttp_uri), url, 8192);

    evhttp_add_header(evhttp_request_get_output_headers(request),
        "Content-Type", "text/html");
    struct evbuffer *evbuf = evbuffer_new();
    evbuffer_add_printf(evbuf, "Server response. Your request url is %s", url);

#ifdef GOOD_TYPE
    evhttp_send_reply(request, 200, "OK", evbuf);
#else
    evhttp_send_reply_start(request, HTTP_OK, "OK");
    evhttp_send_reply_chunk(request, evbuf);
    evhttp_send_reply_end(request);
#endif
}

class ThreadPool {
public:
    ThreadPool(int size)
    {
        for (int i = 0; i < size; i++) {
            m_ts.emplace_back(thread([this]() {
                for (;;) {
                    std::function<void(evhttp_request *request)> task;
                    evhttp_request *request = nullptr;
                    {
                        unique_lock<mutex> lock(m_mutex);
                        m_cv.wait(lock, [this]() {
                            return m_stop || (!m_tasks.empty() && !m_reqs.empty());
                            });
                        if (m_stop && m_tasks.empty() && m_reqs.empty()) {
                            return;
                        }
                        task = m_tasks.front();
                        m_tasks.pop();
                        request = m_reqs.front();
                        m_reqs.pop();
                    }
                    task(request);
                }
                }));
        }
    }

    ~ThreadPool()
    {
        stop();
        m_cv.notify_all();
        for (auto &t : m_ts) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void add(std::function<void(evhttp_request *request)> func, evhttp_request *request)
    {
        {
            lock_guard<mutex> lock(m_mutex);
            if (m_stop) {
                return;
            }
            m_tasks.emplace(func);
            m_reqs.emplace(request);
        }
        m_cv.notify_all();
    }

    void stop()
    {
        {
            lock_guard<mutex> lock(m_mutex);
            m_stop = true;
        }
    }

private:
    condition_variable m_cv;
    queue<std::function<void(evhttp_request *request)>> m_tasks;
    queue<evhttp_request *> m_reqs;
    mutex m_mutex;
    bool m_stop = false;
    vector<thread> m_ts;
};

ThreadPool threadpool(10);

void OnRequestCallback(struct evhttp_request *req, void *arg)
{
#ifdef SYNC
    deal_request(req);
#else
    threadpool.add(deal_request, req);
#endif
}

bufferevent *bufferevent_callback(struct event_base *base, void *arg)
{
    return bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
}

int main(int argc, char *argv[])
{
    evthread_use_pthreads();
    event_base *base = event_base_new();
    evhttp *http = evhttp_new(base);
    evhttp_set_bevcb(http, bufferevent_callback, nullptr);
    evhttp_bind_socket_with_handle(http, "0.0.0.0", 3333);
    evhttp_set_cb(http, "/test", OnRequestCallback, nullptr);

    event_base_dispatch(base);
    return 0;
}
