set(SOURCES 
AchievementMan.cpp
AchievementMan.h
ActivityMan.cpp
ActivityMan.h
AudioMan.cpp
AudioMan.h
ConsoleMan.cpp
ConsoleMan.h
EntityMan.cpp
EntityMan.h
FrameMan.cpp
FrameMan.h
LicenseMan.cpp
LicenseMan.h
LuaMan.cpp
LuaMan.h
MetaMan.cpp
MetaMan.h
MovableMan.cpp
MovableMan.h
PresetMan.cpp
PresetMan.h
RTEManagers.h
SceneMan.cpp
SceneMan.h
SettingsMan.cpp
SettingsMan.h
TimerMan.cpp
TimerMan.h
MetaMan.cpp
MetaMan.h
UInputMan.cpp
UInputMan.h
SteamUGCMan.h
SteamUGCMan.cpp)

complete_path(${SOURCES})
set(Managers_SRC ${RESULT} PARENT_SCOPE)
source_group(Managers FILES ${RESULT})

