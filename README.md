## Setup

Download boost:
https://www.boost.org/users/download/
If you're on Windows, download the windows version, on for unix based
download the unix version.

On Unix based systems:
For boost run:
tar -xzf boost_1_68_0.tar.gz # Decompressed boost.
cd boost_1_68_0 # Go into boost directory.
./bootstrap.sh # Run bootstrapping
./b2 toolset=clang threading=multi runtime-link=static  link=static cxxflags="-stdlib=libc++ -DBOOST_DISABLE_ASSERTS" linkflags="-stdlib=libc++" address-model=64

This will generate the appropriate dependencies for boost.

Then for serialization purposes download Cereal:
https://github.com/USCiLab/cereal/archive/v1.2.2.zip

It's an open source header only serialization library leveraging C++11's basic reflexion (type_traits) for binary serialization
of classes.

