cmake_minimum_required(VERSION 3.16)
include(pico_sdk_import.cmake)

project( emfrp-repl C CXX ASM )
set(CMAKE_C_STNDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
add_compile_definitions(RPI_PICO=1)
set(PROJ_DIR "${PROJECT_SOURCE_DIR}/../..")
include(${PROJ_DIR}/src/CMakeLists.txt)
set_source(${PROJ_DIR})
if (UNIX)
    if (EXISTS "${PROJ_DIR}/packcc/build/clang/release/bin/packcc")
        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/clang/release)
    else ()
        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/gcc/release)
    endif ()
    add_custom_command(
        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJ_DIR}/src/parser.peg
        COMMAND ${PROJ_DIR}/project/common/execPackcc.sh ${PACKCC_BUILD_DIR}/bin/packcc  ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
    )
elseif (WIN32)
    set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/msvc/x64/Release)
    add_custom_command(
        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJ_DIR}/src/parser.peg
        COMMAND ${PROJ_DIR}/project/common/execPackcc.bat ${PACKCC_BUILD_DIR}/packcc.exe ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
    )
endif ()
include_directories(
    PRIVATE ${PROJ_DIR}/include
)

add_executable(emfrp-repl
  ${SOURCES}
  ${PROJ_DIR}/src/hal/console.rpi.c
  ${PROJ_DIR}/src/main/rpi_main.c
)

pico_enable_stdio_usb(emfrp-repl 0)
pico_enable_stdio_uart(emfrp-repl 1)

pico_add_extra_outputs(emfrp-repl)

target_link_libraries(emfrp-repl pico_stdlib)
