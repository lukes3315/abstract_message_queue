### General explanation



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
