### General explanation


## Setup

Download boost:<br/>
https://www.boost.org/users/download/<br/>
If you're on Windows, download the windows version, on for unix based
download the unix version.

On Unix based systems:<br/>
For boost run:
```
tar -xzf boost_1_68_0.tar.gz # Decompressed boost.
```
```
cd boost_1_68_0 # Go into boost directory.
```
```
./bootstrap.sh # Run bootstrapping
```
```
./b2 toolset=clang threading=multi runtime-link=static  link=static cxxflags="-stdlib=libc++ -DBOOST_DISABLE_ASSERTS" linkflags="-stdlib=libc++" address-model=64
```
This will generate the appropriate dependencies for boost.

You're all set!
