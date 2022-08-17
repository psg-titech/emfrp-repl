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

# How to build

## FreeBSD/DragonFlyBSD
1. Execute `./setup-bsd.sh` as privilaged user.
2. `$ cmake build`
3. `$ cd build`
4. `$ make`
5. `$ ./emfrp-repl`

## Windows
0. Install Visual Studio 2019 or newer(with C++ Desktop Development workload) and CMake.
1. Open `./packcc/build/msvc/msvc.sln`.
2. Build the project packcc with the configuration `Release` and `x64`.
3. `> cmake -B build`.
4. Open `./build/emfrp-repl.sln`.
5. Build and debug!
