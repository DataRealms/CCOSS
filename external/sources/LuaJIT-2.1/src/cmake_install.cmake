# Install script for directory: C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/luajit")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/luajit" TYPE FILE FILES
    "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/lauxlib.h"
    "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/lua.h"
    "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/luaconf.h"
    "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/luajit.h"
    "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/lualib.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/Debug/luajit.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/Release/luajit.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/MinSizeRel/luajit.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/RelWithDebInfo/luajit.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/Debug/luajit.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/Release/luajit.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/MinSizeRel/luajit.exe")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES "C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/RelWithDebInfo/luajit.exe")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/host/cmake/minilua/cmake_install.cmake")
  include("C:/Users/Max/Desktop/CCCP 1/Cortex-Command-Community-Project-Source/external/sources/LuaJIT-2.1/src/host/cmake/buildvm/cmake_install.cmake")

endif()

