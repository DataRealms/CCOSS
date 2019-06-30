set(SOURCES 
ProfileControlMessage.cpp
ProfileControlMessage.h
ProfileReportMessage.cpp
ProfileReportMessage.h
ProfileStatusMessage.cpp
ProfileStatusMessage.h
Profiler.cpp
Profiler.h)

complete_path(Profiler ${SOURCES})
set(Profiler_SRC ${RESULT} PARENT_SCOPE)
source_group(Profiler FILES ${RESULT})
