set(SOURCES
DebugMessage.cpp
DebugMessage.h
DebugMessageFactory.cpp
DebugMessageFactory.h
DebugMessageListener.h
DebugTool.cpp
DebugTool.h)

complete_path(${SOURCES})
set(DebugTool_SRC ${RESULT} PARENT_SCOPE)
source_group(DebugTool FILES ${RESULT})
