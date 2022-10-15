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

## HaikuOS
This is the tested environment.

 * Haiku R1/beta hrev55181+67
 * gcc 11.2

## macOS
This is the tested environment.

 * macOS 12.6 Monterey
 * MacBook Air (M1, 2020)

# How to build
## How to build for own devices
### FreeBSD/DragonFlyBSD
1. Execute `project/common/setup-bsd.sh` as privilaged user.
2. Execute the Makefile in `./packcc/build/clang` or `./packcc/build/gcc`.
3. Open `project/pc/`.
4. `$ cmake -B build`
5. `$ cd build`
6. `$ make`
7. `$ ./emfrp-repl`

### Windows
0. Install Visual Studio 2019 or newer(with C++ Desktop Development workload) and CMake.
1. Open `packcc/build/msvc/msvc.sln`.
2. Build the project packcc with the configuration `Release` and `x64`.
3. Open `project/pc/`.
3. `> cmake -B build`.
4. Open `project/pc/build/emfrp-repl.sln`.
5. Build and debug!

### Haiku OS
1. Execute `project/common/setup-haiku.sh`.
2. Execute the Makefile in `./packcc/build/gcc`.
3. Open `project/pc/`.
4. `$ cmake -B build`
5. `$ cd build`
6. `$ make`
7. `$ ./emfrp-repl`

### macOS
1. Execute `project/common/setup-macos-brew.sh` as privilaged user.
2. Execute the Makefile in `packcc/build/clang`.
3. Open `project/pc/`.
4. `$ mkdir build`
5. `$ cd build`
6. `$ cmake ..`
7. `$ make`
8. `$ ./emfrp-repl`

### GUI Emfrp-REPL
We recommend not to use on macOS.  
You had better use Linux, or Windows on virtual machines if you use macOS.  

1. Execute `gui/emfrp-repl-gui.py` by Python 3.x.
