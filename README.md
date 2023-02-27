# A-problem-about-libevent
[This code is associated with this issue](https://github.com/libevent/libevent/issues/1420).

# How to build
First, you must install libevent-2.1.12 first, or you can modify CMakeLists.txt to assign link dictionary.

## Build in a reply way that asynchronous and may cause coredump
```
mkdir -p build
cd build
cmake ..
make
./a.out # excute
```
## Build in same reply way but is sync and may not cause coredump
Maybe you need rm last build dictionary first.
```
mkdir -p build
cd build
cmake .. -DASYNC_TYPE=SYNC
make
./a.out # excute in sync way
```
## Build in other reply way that asynchronous and won't cause coredump
Maybe you need rm last build dictionary first or you must redefine ASYNC_TYPE
```
mkdir -p build
cd build
cmake .. -DREPLY_TYPE=GOOD_TYPE
make
./a.out # excute in async way and won't cause coredump
```
## Build with TSAN
Maybe you need rm last build dictionary first or you must redefine ASYNC_TYPE and REPLY_TYPE
```
mkdir -p build
cd build
cmake .. -DBUILD_TYPE=TSAN
make
TSAN_OPTIONS=log_path=$(cd $(dirname $0); pwd)/tsan.log ./a.out
```

# Test
Run my script must install node first, the tool will send a get request every 2 ms
```
node test.js
```
