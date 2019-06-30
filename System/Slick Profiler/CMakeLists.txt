set(SOURCES
BinaryReader.h
BinaryReaderEndianSwapped.cpp
BinaryReaderEndianSwapped.h
BinaryReaderNormal.cpp
BinaryReaderNormal.h
BinaryWriter.h
BinaryWriterEndianSwapped.cpp
BinaryWriterEndianSwapped.h
BinaryWriterNormal.cpp
BinaryWriterNormal.h
debug.cpp
debug.h)

include_directories(pch)

complete_path(${SOURCES})
set(Slick_Profiler_SRC ${RESULT})
source_group("Slick Profiler" FILES ${RESULT})

add_subdirectory(DebugTool)
add_subdirectory(Profiler)

add_library(SlickProfiler 
	${Slick_Profiler_SRC} 
	${Profiler_SRC} 
	${Deb_ugTool_SRC}
	${PCH_SRC})

