## General explanation

Note: If you run in any issues, please just file an issue on github.

The purpose of the project is to leverage C++17's to guarantee type abstraction for server-client communication<br/>
using message_queues (in this case boost's message queues (since they work on linux and windows)).<br/>
The server runs in one termninal, the client in another. The server waits for incoming client connections.<br/>
The server can:<br/>
- receive any type of data (view code to see implementation) and process it in it's own memory space.<br/>
- store any type of data.<br/>
- retrieve stored data.<br/>
<br/>
The client can:<br/>
- send data to server for processing.<br/>
- send data to server for storage.<br/>
- query data to be retrieved from server.<br/>

## Setup

First you are required to [install cmake](https://cmake.org/install/).


Then, download boost:<br/>
https://www.boost.org/users/download/<br/>
If you're on Windows, download the windows version, on for unix based
download the unix version.

On Unix based systems:<br/>
Decompress boost.
```
tar -xzf boost_1_68_0.tar.gz # Decompressed boost.
```
Change directory to boost's.
```
cd boost_1_68_0
```
Run boostrapping.
```
./bootstrap.sh
```
Compile boost.
```
./b2 toolset=clang threading=multi runtime-link=static  link=static cxxflags="-stdlib=libc++ -DBOOST_DISABLE_ASSERTS" linkflags="-stdlib=libc++" address-model=64
```
This will generate the appropriate dependencies for boost.

You're all set!

## Running it

First off, run `cmake`:
```
cmake .
```
Then, run `make`:
```
make
```
From there, two executables: `server` and `client` were generated.

You'll need to first run the server in one terminal:
```
./server
```

Always run the `server` first as he is the one creating the message queues.

Then to test out the `client`:
```
./client
```

To ensure it worked, this should be the output:
```
THIS IS ME: 50
THIS IS ME: 30
THIS IS ME: 22220
THIS IS ME: 40
THIS IS ME: 50
```

Which is essentially the client registering data to the server, retrieving it and calling the associated object.

If you run in any issues, please just file an issue on github.


