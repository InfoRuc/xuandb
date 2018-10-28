# Overview

A simple database implemented by C++ which has some basic functions.

# Building

This project is built by **CMake**(v2.8 or higher) on **Linux**. You should have these dependencies installed:

- gcc/g++
- git
- cmake

You can install them by this command(only for centos):

```
# yum install -y gcc gcc-c++ git cmake
```

If you have your environment ready, you can build this project.

```
$ cd MiniDB
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Now you can get the binary file in MiniDB\/bin.

**Node:** If you can get a binary file named 'demo' in MiniDB\/bin\/demo, it indicates your environment is normal.

# License

![gplv3-logo](https://www.gnu.org/graphics/gplv3-127x51.png)

This project is under GPLv3.0 license. You can get more details from [here](https://www.gnu.org/licenses/gpl-3.0.en.html).
