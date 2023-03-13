include(../../../src/CMakeLists.txt)
set_source(../../../)

LIST(APPEND SOURCES
#    ../../../src/hal/console.esp.c
    ../../../src/main/app_main.esp8266.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "../../../include")
