project( emfrp-repl )
cmake_minimum_required(VERSION 3.1)
set(PROJ_DIR "${PROJECT_SOURCE_DIR}/../..")
include(${PROJ_DIR}/src/CMakeLists.txt)
set_source(${PROJ_DIR})
set(CMAKE_C_STANDARD 11)

add_custom_command(
    OUTPUT ${PROJ_DIR}/src/emfrp_parser.c
    DEPENDS ${PROJ_DIR}/src/parser.peg
    COMMAND ${PROJ_DIR}/script/execPackcc.bat ${PROJ_DIR}/packcc/build/msvc/ ${PROJ_DIR}/src/parser.peg ${PROJ_DIR}/src/emfrp_parser.c ${PROJ_DIR}/include/emfrp_parser.h
)

add_executable(emfrp-repl ${SOURCES} ${PROJECT_SOURCE_DIR}/src/main.c)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(libemfrp-repl SHARED ${SOURCES} ${PROJ_DIR}/src/emfrp.c)

add_compile_definitions(EMFRP_EXPORTING=1)
include_directories(PRIVATE ${PROJ_DIR}/include)
