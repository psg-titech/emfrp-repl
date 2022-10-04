# emfrp-repl
A REPL implementation of Emfrp, running on low power devices.

# Requirements
## Windows
This is the tested environment.

 * Visual Studio 2019, 2022
 * Windows 10, 11

## DragonFly
This is the tested environment.

 * DragonFly 6.2.2
 * gcc 8.3

## macOS
This is the tested environment.

 * macOS 12.6 Monterey
 * MacBook Air (M1, 2020)

# How to build

## macOS
1. Execute `./setup-macos-brew.sh` as privilaged user.
2. Execute the Makefile in `./packcc/build/clang`.
3. `$ mkdir build && cd build && cmake ..`
4. `$ make`
5. `$ ./emfrp-repl`

## FreeBSD/DragonFlyBSD
1. Execute `./setup-bsd.sh` as privilaged user.
2. Execute the Makefile in `./packcc/build/clang` or `./packcc/build/gcc`.
3. `$ cmake build`
4. `$ cd build`
5. `$ make`
6. `$ ./emfrp-repl`

## Windows
0. Install Visual Studio 2019 or newer(with C++ Desktop Development workload) and CMake.
1. Open `./packcc/build/msvc/msvc.sln`.
2. Build the project packcc with the configuration `Release` and `x64`.
3. `> cmake -B build`.
4. Open `./build/emfrp-repl.sln`.
5. Build and debug!
