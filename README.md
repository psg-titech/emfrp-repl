# emfrp-repl
A REPL implementation of Emfrp, running on low power devices.

# Requirements
This is the tested environment.
 * Windows 10 or 11
   * Visual Studio 2019 or 2022 (Maybe compatible for 2013, 2015, and 2017)
 * DragonFly 6.2.2
   * gcc 8.3 (also clang)
 * Haiku R1/beta hrev55181+67
   * gcc 11.2
 * macOS 13 Ventura
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
6. `$ cmake -B build`
7. `$ make` in the build directory.
8. `$ ./emfrp-repl`

### GUI Emfrp-REPL
We recommend not to use on macOS.  
You had better use Linux, or Windows on virtual machines if you use macOS.  

1. Execute `gui/emfrp-repl-gui.py` by Python 3.x.

## Compilation for ESP32
You need esp-idf SDK. available on: https://github.com/espressif/esp-idf  

1. Execute `install.ps1`, or `install.sh` in the esp-idf repository.
2. Execute `export.ps1`, or `export.ps1` in the esp-idf repository.
3. Execute `project/esp-idf/gen_parser.bat`, or `project/esp-idf/gen_parser.sh`. You need to execute these files if you rewrite `parser.peg`. This comes from esp-idf CMake Project System's limitations.
4. Open `project/esp-idf`
6. Execute `idf.py set-target esp32` in your first time.
5. Execute `idf.py build` and flash `idf.py flash -p <PORT>`, and then monitor by `idf.py monitr -p <PORT>`.
