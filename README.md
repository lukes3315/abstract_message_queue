## Setup

Download boost:
https://www.boost.org/users/download/
If you're on Windows, download the windows version, on for unix based
download the unix version.

For boost run:
mv ~/Downloads/boost_1_68_0.tar.gz .
tar -xzf boost_1_68_0.tar.gz
cd boost_1_68_0
./bootstrap.sh
./b2

Then for serialization purposes download Cereal:
https://github.com/USCiLab/cereal/archive/v1.2.2.zip

It's an open source header only serialization library leveraging C++11's basic reflexion (type_traits) for binary serialization
of classes.
