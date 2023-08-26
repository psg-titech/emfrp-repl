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
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/clang)
    else ()
        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/gcc)
    endif ()
    add_custom_command(
        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJ_DIR}/src/parser.peg
        COMMAND ${PROJ_DIR}/script/execPackcc.sh ${PACKCC_BUILD_DIR}  ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
    )
elseif (WIN32)
    add_custom_command(
        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJ_DIR}/src/parser.peg
        COMMAND ${PROJ_DIR}/script/execPackcc.bat ${PROJ_DIR}/packcc/build/msvc/ ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
    )
endif ()
include_directories(
    PRIVATE ${PROJ_DIR}/include
)

add_executable(emfrp-repl
  ${SOURCES}
  main.c
)

pico_enable_stdio_usb(emfrp-repl 0)
pico_enable_stdio_uart(emfrp-repl 1)

pico_add_extra_outputs(emfrp-repl)

target_link_libraries(emfrp-repl pico_stdlib)
