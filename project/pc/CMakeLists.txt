project( emfrp-repl )
cmake_minimum_required(VERSION 3.22)
set(PROJ_DIR "${PROJECT_SOURCE_DIR}/../..")
include(${PROJ_DIR}/src/CMakeLists.txt)
set_source(${PROJ_DIR})
set(CMAKE_C_STANDARD 11)

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

add_executable(emfrp-repl
    ${SOURCES}
    ${PROJ_DIR}/src/hal/console.unix.c
    ${PROJ_DIR}/src/hal/console.win.c
    ${PROJ_DIR}/src/main/main.c)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(libemfrp-repl SHARED ${SOURCES} ${PROJ_DIR}/src/emfrp.c)

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wmissing-noreturn")
    include_directories(
        PRIVATE
            ${PROJ_DIR}/include
            /usr/local/include
    )
elseif (WIN32)
    add_compile_definitions(EMFRP_EXPORTING=1)
    include_directories(
       PRIVATE ${PROJ_DIR}/include
    )
endif ()

if (UNIX)
    find_library (EDITLINE_LIB edit)
    target_link_libraries(
        emfrp-repl PRIVATE "${EDITLINE_LIB}"
    )
endif ()
