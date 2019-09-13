Cortex Command Community Project - open source under GNU AGPL v3

This is a community-driven effort to continue the development of Cortex Command

# CONTACT #

https://discord.gg/junGCWe


# Known problems and limitations #

Tested to build only with Visual Studio 2017 for Win32 platform with Visual Studio 2013 toolset installed (You may need to install full VS 2013).

Almost builds with modern toolchains in VS 2017 and VS 2019, but there's some incompatibility in Luabind which fails to compile. Other than that should be fine.

Included source code does not match the current B32 functionality. This is a work-in-progress snapshot of what's internally called "B33", so it won't start with data from B32.

Game data isn't included, you should get it from the Cortex Command beta branch on Steam. (See BUILD PROCESS section below for details).

Commercial fmod library was replaced with free SDL Mixer which lacks pitching functionality hence some sound effects will sound differently or incorrectly.

All source and binary dependencies required to build the project are included.

This version uses a slightly modified version of Allegro. The only difference from the original is a custom window borderless mode implemented to fix fullscreen issues. If you're gonna rebuild Allegro you'l probably need to disable custom calls in FrameMan.


# BUILD PROCESS #

## Windows ##

Use "Debug Open Source" configuration to debug (be prepared, it's very slow).

Use "Final Open Source" configuration to build release .exe.

After the build has finished, in order to run it:
* Copy required dlls from external\lib\win\ next to .exe
* Copy latest game data (.rte modules) to the same place. The data can be found under the "ccoss" beta branch of Cortex Command on Steam, if you own a license for the game.
* Patch game data by copying modules from https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data. This will overwrite some of the module files.

Binaries go to _Bin. Note: you can't run the debug build from there outside of the IDE, it won't find required stuff.
