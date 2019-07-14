set(SOURCES 
pch.cpp
pch.h)

complete_path(${SOURCES})
set(PCH_SRC ${RESULT} PARENT_SCOPE)
