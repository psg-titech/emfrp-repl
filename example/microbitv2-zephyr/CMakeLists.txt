# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_C_STANDARD 11)
set(BOARD bbc_microbit_v2)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(microbitv2-emfrp-repl)

set(PROJ_DIR "${PROJECT_SOURCE_DIR}/../..")
include(${PROJ_DIR}/src/CMakeLists.txt)
set_source(${PROJ_DIR})


#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/clang)
#else ()
#    set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/gcc)
#endif ()
#add_custom_command(
#    OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
#    DEPENDS ${PROJ_DIR}/src/parser.peg
#    COMMAND ${PROJ_DIR}/script/execPackcc.sh ${PACKCC_BUILD_DIR}  ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
#)

include_directories(PRIVATE ${PROJ_DIR}/include /usr/local/include)

target_sources(app PRIVATE ${SOURCES} ${PROJ_DIR}/src/emfrp.c src/main.c)
