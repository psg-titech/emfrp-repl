project( emfrp-repl )
cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)

#if (UNIX)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ledit")
#endif (UNIX)

set(SOURCES
    src/main.c
    src/string_t.c
    src/hal/console.unix.c
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
