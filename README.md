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
 * Debian 11
   * clang 11.0 or 10.2
 * macOS 13 Ventura
   * MacBook Air (M1, 2020)

# How to build
## How to build for your host PC.
### clang/gcc based OSes
1. Execute `project/common/setup-?.sh` as a privilaged user.  
   FreeBSD and DragonFly: setup-bsd.sh  
   Debian: setup-apt.sh  
   macOS: setup-macos-brew.sh (brew required)  
   Haiku OS: setup-haiku.sh  
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

### GUI Emfrp-REPL
1. Run `gui/emfrp-repl-gui.py` with Python 3.x.

## Compilation for ESP32
### Install esp-idf SDK.
You need esp-idf SDK. available on: https://github.com/espressif/esp-idf  
Execute `install.ps1`, or `install.sh` in the esp-idf repository.

### Build this project.
1. Execute `export.ps1`, or `export.ps1` in the esp-idf repository.
2. Execute `project/esp-idf/gen_parser.bat`, or `project/esp-idf/gen_parser.sh`. You need to execute these files if you rewrite `parser.peg`. This comes from esp-idf CMake Project System's limitations.
3. Open `project/esp-idf`
4. Execute `idf.py set-target esp32` in your first time.
5. Execute `idf.py build` and flash `idf.py flash -p <PORT>`, and then monitor by `idf.py monitr -p <PORT>`.

## Compilation for ESP8266
DOES NOT WORK DUE TO A LINKER ISSUE.
