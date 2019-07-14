set(SOURCES Box.cpp
Box.h
Color.cpp
Color.h
Cheeky.c
ContentFile.cpp
ContentFile.h
DDTError.cpp
DDTError.h
DDTTools.cpp
DDTTools.h
DataModule.cpp
DataModule.h
Matrix.cpp
Matrix.h
PathFinder.cpp
PathFinder.h
Reader.cpp
Reader.h
Serializable.h
Singleton.h
StdString.h
System.h
System.cpp
Timer.cpp
Timer.h
Vector.cpp
Vector.h
Writer.cpp
Writer.h)

complete_path(${SOURCES})
set(System_SRC ${RESULT} PARENT_SCOPE)
source_group(System FILES ${RESULT})

add_subdirectory("MD5")
add_subdirectory("Slick_Profiler")
add_subdirectory("MicroPather")
