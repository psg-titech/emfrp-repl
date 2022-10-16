include(../../../src/CMakeLists.txt)
set_source(../../../)

LIST(APPEND SOURCES
    ../../../src/hal/console.esp32.c
    ../../../src/main/app_main.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "../../../include")
