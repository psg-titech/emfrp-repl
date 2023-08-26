# emfrp-repl
A REPL implementation of Emfrp, running on embedded devices.

# Requirements
This is the tested environment.
 * Windows 10 or 11
   * Visual Studio 2019 or 2022 (Maybe compatible with 2013, 2015, and 2017)
 * DragonFly 6.2.2
   * gcc 8.3 (also clang)
 * Haiku R1/beta hrev55181+67
   * gcc 11.2
 * Debian 12 bookworm
   * clang-13 or above
 * macOS 13 Ventura
   * MacBook Air (M1, 2020)

We recommend to use clang-format version 13 for formatting.

# How to build
## How to build for your host PC.
### clang/gcc based OSes
1. Execute `script/setup-?.sh` as a privilaged user.  
   FreeBSD and DragonFly: setup-bsd.sh  
   Debian: setup-apt.sh  
   macOS: setup-macos-brew.sh (brew required)  
   Haiku OS: setup-haiku.sh  
2. Open `example/unix/`.
3. `$ cmake -B build`
4. `$ cd build`
5. `$ make`
6. `$ ./emfrp-repl`

### Windows
#### Visual Studio 2022
1. Install Visual Studio 2022 or newer(with C++ Desktop Development workload) and CMake support.
2. Open `example/windows/CMakeLists.txt`.
3. Build and debug!

#### Visual Studio 2019, 2022
1. Install Visual Studio 2019 or newer(with C++ Desktop Development workload) and CMake.
2. Open `example/windows/`.
3. `> cmake -B build`.
4. Open `example/windows/build/emfrp-repl.sln`.
5. Build and debug!

### GUI Emfrp-REPL (for LINUX, BSD, and Windows)
1. Build Emfrp-REPL following the instructions above.
2. Run `example/python-gui/emfrp-repl-gui.py` with Python 3.x.

## Compilation for ESP32
### Install esp-idf SDK.
You need esp-idf SDK. available on: https://github.com/espressif/esp-idf  
Execute `install.ps1`, or `install.sh` in the esp-idf repository.

### Build this project.
1. Execute `export.ps1`, or `export.ps1` in the esp-idf repository.
2. Execute `example/esp-idf/gen_parser.bat`, or `example/esp-idf/gen_parser.sh`. You need to execute these files if you rewrite `parser.peg`. This comes from esp-idf CMake Project System's limitations.
3. Open `example/esp-idf`
4. Execute `idf.py set-target <target-board>` in your first time.
5. Execute `idf.py build` and flash `idf.py flash`, and then monitor by `idf.py monitor`.

## Compilation for ESP8266
DOES NOT WORK DUE TO A LINKER ISSUE.
