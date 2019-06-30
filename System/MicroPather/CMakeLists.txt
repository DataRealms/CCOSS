set(SOURCES micropather.cpp
micropather.h)

complete_path(${SOURCES})
set(MicroPather_SRC ${RESULT})
source_group(MicroPather FILES ${RESULT})

add_library(MicroPather ${MicroPather_SRC})
