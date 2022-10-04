project( emfrp-repl )
cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
if (UNIX)
    if (EXISTS "${PROJECT_SOURCE_DIR}/packcc/build/clang/release/bin/packcc")
        set(PACKCC_BUILD_DIR ${PROJECT_SOURCE_DIR}/packcc/build/gcc/release)
    else
        set(PACKCC_BUILD_DIR ${PROJECT_SOURCE_DIR}/packcc/build/clang/release)
    endif ()
    add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJECT_SOURCE_DIR}/src/parser.peg
        COMMAND ${PROJECT_SOURCE_DIR}/execPackcc.sh ${PACKCC_BUILD_DIR}/bin/packcc  ${PROJECT_SOURCE_DIR}/src/parser.peg ${PROJECT_SOURCE_DIR}/src/emfrp_parser.c ${PROJECT_SOURCE_DIR}/include/emfrp_parser.h
    )
elseif (WIN32)
    set(PACKCC_BUILD_DIR ${PROJECT_SOURCE_DIR}/packcc/build/msvc/x64/Release)

    add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/src/emfrp_parser.c
        DEPENDS ${PROJECT_SOURCE_DIR}/src/parser.peg
        COMMAND ${PROJECT_SOURCE_DIR}/execPackcc.bat ${PACKCC_BUILD_DIR}/packcc.exe ${PROJECT_SOURCE_DIR}/src/parser.peg ${PROJECT_SOURCE_DIR}/src/emfrp_parser.c ${PROJECT_SOURCE_DIR}/include/emfrp_parser.h
    )
endif ()

set(SOURCES
    src/main.c
    src/string_t.c
    src/hal/console.unix.c
    src/hal/console.win.c
    src/emfrp_parser.c
    src/ast.c
    src/vm/exec.c
    src/vm/machine.c
    src/vm/node_t.c
    src/collections/list_t.c
    src/collections/dictionary_t.c
    src/emfrp.c
)

add_executable(emfrp-repl ${SOURCES})

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    target_include_directories(emfrp-repl
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            /usr/local/include
    )
elseif (WIN32)
    target_include_directories(emfrp-repl
        PRIVATE
            ${PROJECT_SOURCE_DIR}/include
    )
endif ()

if (UNIX)
    find_library (EDITLINE_LIB edit)
    target_link_libraries(
        emfrp-repl PRIVATE "${EDITLINE_LIB}"
    )
endif ()
