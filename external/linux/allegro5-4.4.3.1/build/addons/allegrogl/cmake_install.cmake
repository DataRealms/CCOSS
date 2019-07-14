# Install script for directory: /home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so.4.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so.4.4"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeggl.so.4.4.3"
    "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeggl.so.4.4"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so.4.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so.4.4"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeggl.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so"
         OLD_RPATH "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeggl.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/alleggl.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/gl_ext.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/gl_header_detect.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/addons/allegrogl/include/allegrogl/alleggl_config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/glx_ext_alias.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/glx_ext_api.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/glx_ext_defs.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/glx_ext_list.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/gl_ext_alias.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/gl_ext_api.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/gl_ext_defs.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/gl_ext_list.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/wgl_ext_alias.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/wgl_ext_api.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/wgl_ext_defs.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegrogl/GLext" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/addons/allegrogl/include/allegrogl/GLext/wgl_ext_list.h")
endif()

