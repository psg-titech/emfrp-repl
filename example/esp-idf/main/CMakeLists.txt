include(../../../src/CMakeLists.txt)
set_source(../../../)

LIST(APPEND SOURCES
    ../../../src/emfrp.c
    app_main.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "../../../include")
