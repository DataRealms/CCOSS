@rem Copy Managers source
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Managers\*.*" Managers\ /E /Q /Y

@rem Remove Steam stuff
del Managers\SteamUGCMan.h /Q
del Managers\SteamUGCMan.cpp /Q

@rem Copy GUI subsystem
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\GUI\*.*" GUI\ /E /Q /Y

@rem Copy UI source
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Menus\*.*" Menus\ /E /Q /Y

@rem Copy Entities sources
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Entities\*.*" Entities\ /E /Q /Y

@rem Copy Debug tool
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Slick DebugTool\*.*" "Slick DebugTool\" /E /Q /Y



@rem Copy submodules licences
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Licences\*.*" Licences\ /E /Q /Y



@rem Copy system source
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\System\*.*" "System\" /E /Q /Y

@rem Remove proprietary and unneeded stuff
rmdir System\EXECryptor /S /Q

@rem Copy External binary libs and includes
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\external\*.*" external\ /E /Q /Y

@rem Remove proprietary and unneeded stuff
rmdir external\include\boost-1_34_1 /S /Q
rmdir external\include\boost-1_36 /S /Q

rmdir external\sources\luabind-0.7.2-beta /S /Q

rmdir external\include\steam /S /Q

rmdir external\include\_oolua /S /Q
rmdir external\include\_mathfu /S /Q
rmdir external\include\_vectorial /S /Q

rmdir external\include\fmod /S /Q
rmdir external\sources\fmodshim /S /Q

del external\lib\win\*.* /Q

@rem Copy debug libs
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\alld_s_c.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\dinput.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_chrono-vc120-mt-gd-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_date_time-vc120-mt-gd-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_system-vc120-mt-gd-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_thread-vc120-mt-gd-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libcurl.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\lua5.1.x86.debug.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\luabind.x86.debug.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2.x86.debug.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2_mixer.x86.debug.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\zlibwapi.lib" external\lib\win\

@rem Copy release libs
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\alleg_s_c.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\ddraw.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\dinput.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\dxguid.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_chrono-vc120-mt-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_date_time-vc120-mt-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_system-vc120-mt-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libboost_thread-vc120-mt-1_55.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\libcurl.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\lua.x86.release.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\luabind.x86.release.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2.x86.release.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2_mixer.x86.release.lib" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\zlibwapi.lib" external\lib\win\

@rem Copy SDL Mixer dlls

copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2.dll" external\lib\win\
copy "..\..\Cortex Command Source Multiplayer\Trunk\external\lib\win\SDL2_mixer.dll" external\lib\win\


@rem Copy NAT punch through server subproject
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\NATPunchServer\*.*" NATPunchServer\ /E /Q /Y

@rem Clean NAT server build stuff
rmdir NATPunchServer\Server\NATCompleteServer\Release /S /Q
rmdir NATPunchServer\Server\NATCompleteServer\Debug /S /Q
rmdir NATPunchServer\Server\NATCompleteServer\.vs /S /Q


@rem Copy Documentation folder
xcopy "..\..\Cortex Command Source Multiplayer\Trunk\Documentation\*.*" Documentation\ /E /Q /Y

@rem Remove wiki related stuff from documentation
del "Documentation\Parser\Update Wiki Lua Docs.bat"
del "Documentation\Parser\parse_lua_wiki.php"
del "Documentation\Parser\LuaDocHtml_B33.zip"



@rem Copy root stuff / Don't copy as those need manual cleanup
@rem copy "..\..\Cortex Command Source Multiplayer\Trunk\RTEA.sln"
@rem copy "..\..\Cortex Command Source Multiplayer\Trunk\RTEA.vcxproj"
@rem copy "..\..\Cortex Command Source Multiplayer\Trunk\RTEA.vcxproj.filters"

copy "..\..\Cortex Command Source Multiplayer\Trunk\Main.cpp"
copy "..\..\Cortex Command Source Multiplayer\Trunk\TargetSxS.h"
copy "..\..\Cortex Command Source Multiplayer\Trunk\RTEA.rc"
copy "..\..\Cortex Command Source Multiplayer\Trunk\resource.h"
copy "..\..\Cortex Command Source Multiplayer\Trunk\ccicon32.ico"
copy "..\..\Cortex Command Source Multiplayer\Trunk\ccicon.ico"
copy "..\..\Cortex Command Source Multiplayer\Trunk\Credits.txt"
copy "..\..\Cortex Command Source Multiplayer\Trunk\.gitignore"

@rem Those are likely obsolete, but might be helpful anyway
copy "..\..\Cortex Command Source Multiplayer\Trunk\OSX Build Instructions.txt"
