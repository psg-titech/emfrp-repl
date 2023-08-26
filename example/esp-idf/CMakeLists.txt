# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
add_compile_definitions(__ESP_IDF__=1)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(emfrp-repl-esp32)

# ESP-IDF SDK cannot use generated .c files.

#set(PROJ_DIR "${PROJECT_SOURCE_DIR}/../..")

#if (UNIX)
#    if (EXISTS "${PROJ_DIR}/packcc/build/clang/release/bin/packcc")
#        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/clang/release)
#    else ()
#        set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/gcc/release)
#    endif ()
#    add_custom_command(
#        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
#        DEPENDS ${PROJ_DIR}/src/parser.peg
#        COMMAND ${PROJ_DIR}/project/common/execPackcc.sh ${PACKCC_BUILD_DIR}/bin/packcc  ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
#    )
#elseif (WIN32)
#    set(PACKCC_BUILD_DIR ${PROJ_DIR}/packcc/build/msvc/x64/Release)
#    add_custom_command(
#        OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
#        DEPENDS ${PROJ_DIR}/src/parser.peg
#        COMMAND ${PROJ_DIR}/project/common/execPackcc.bat ${PACKCC_BUILD_DIR}/packcc.exe ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
#    )
#endif ()
