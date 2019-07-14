# Install script for directory: /home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1

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
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/modules.lst")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/modules.lst")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3" TYPE MODULE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/alleg-alsadigi.so")
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsadigi.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3" TYPE MODULE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/alleg-alsamidi.so")
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-alsamidi.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3" TYPE MODULE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/alleg-jack.so")
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-jack.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3" TYPE MODULE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/alleg-dga2.so")
  if(EXISTS "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/install/lib/allegro/4.4.3/alleg-dga2.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so.4.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so.4.4"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeg.so.4.4.3"
    "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeg.so.4.4"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so.4.4.3"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so.4.4"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/liballeg.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/liballeg.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/include/allegro/platform/alplatf.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/include/allegro/platform/alunixac.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/3d.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/3dmaths.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/alcompat.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/alinline.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/base.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/color.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/compiled.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/datafile.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/debug.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/digi.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/draw.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/file.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/fix.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/fixed.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/fli.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/fmaths.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/font.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/gfx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/graphics.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/gui.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/joystick.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/keyboard.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/lzss.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/matrix.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/midi.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/mouse.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/palette.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/quat.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/rle.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/sound.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/stream.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/system.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/text.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/timer.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/unicode.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/3dmaths.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/asm.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/color.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/draw.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/fix.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/fmaths.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/gfx.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/matrix.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/rle.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/inline" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/inline/system.inl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/internal" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/internal/aintern.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/internal" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/internal/aintvga.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/internal" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/internal/alconfig.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintbeos.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintdos.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintlnx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintmac.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintosx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintpsp.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintqnx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintunix.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aintwin.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/al386gcc.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/al386vc.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/al386wat.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/albcc32.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/albecfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/albeos.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aldjgpp.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aldmc.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/aldos.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/almac.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/almaccfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/almngw32.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/almsvc.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alosx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alosxcfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alpsp.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alpspcfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alqnx.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alqnxcfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alucfg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alunix.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alwatcom.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/alwin.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/astdint.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/allegro/platform" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/allegro/platform/macdef.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/include/xalleg.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/allegro-config")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/pkgconfig/allegro.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/pkgconfig/allegrogl.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/pkgconfig/jpgalleg.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/pkgconfig/loadpng.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/lib/pkgconfig/logg.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/addons/allegrogl/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/addons/loadpng/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/addons/logg/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/addons/jpgalleg/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/docs/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/examples/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/demos/shooter/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/demos/skater/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/tools/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/setup/cmake_install.cmake")
  include("/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/tests/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/ian/source/CCOSS/external/linux/allegro5-4.4.3.1/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
