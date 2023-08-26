include(../../../src/CMakeLists.txt)
set_source(../../../)

LIST(APPEND SOURCES
    main.c
    ../../../src/emfrp.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "../../../include")
