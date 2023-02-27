const http = require('http')
const fs = require('fs');

function batchGet() {
    let option = {
        host: "127.0.0.1",
        port: 3333,
        path: "/test",
        method: 'GET',
        headers: {
            'Content-Type': 'application/json;charset=UTF-8',
        },
    };
    const request = http.request(option, (res) => {
        if (res.statusCode != 200) {
            console.log(`get failure: ${res.statusCode}`);
        }
    });
    request.end();
    request.on('error', (err) => {
        console.log(`err :${err.message}`);
    })
}

function go() {
  setInterval(() => {
      batchGet();
  }, 2);
}

go();
