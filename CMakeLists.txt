project( emfrp-repl )
cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(PACKCC_BUILD_DIR ${PROJECT_SOURCE_DIR}/packcc/build/gcc)

#if (UNIX)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ledit")
#endif (UNIX)

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/src/parser.c
  DEPENDS ${PROJECT_SOURCE_DIR}/src/parser.peg
  COMMAND ${PROJECT_SOURCE_DIR}/execPackcc.sh ${PACKCC_BUILD_DIR}/release/bin/packcc  ${PROJECT_SOURCE_DIR}/src/parser.peg ${PROJECT_SOURCE_DIR}/src/parser.c ${PROJECT_SOURCE_DIR}/include/parser.h
)

set(SOURCES
    src/main.c
    src/string_t.c
    src/hal/console.unix.c
    src/parser.c
    src/ast.c
)

add_executable(emfrp-repl ${SOURCES})

if (UNIX)
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
