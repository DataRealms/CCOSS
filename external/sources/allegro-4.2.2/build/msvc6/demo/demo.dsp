# Microsoft Developer Studio Project File - Name="demo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=demo - Win32 Release

!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "demo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
BSC32=bscmake.exe
LINK32=link.exe

!IF "$(CFG)" == "demo - Win32 Release"
# PROP Output_Dir "../../../demo"
# PROP Intermediate_Dir "../../../obj/msvc/alleg"
# ADD CPP /nologo /I "." /I "../../../" /I "../../../include" /MD /O2 /Gd
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib ..\..\..\lib\msvc\alleg.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib /nologo /machine:IX86 /release /out:"..\..\..\demo\demo.exe"
!ENDIF

# Begin Target
# Name "demo - Win32 Release"
# Begin Group "Source Files"
# Begin Source File
SOURCE=..\..\..\demo\animsel.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\aster.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\bullet.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\demo.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\dirty.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\display.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\expl.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\game.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\star.c
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\title.c
# End Source File
# End Group
# Begin Group "Header Files"
# Begin Source File
SOURCE=..\..\..\demo\animsel.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\aster.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\bullet.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\data.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\demo.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\dirty.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\display.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\expl.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\game.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\star.h
# End Source File
# Begin Source File
SOURCE=..\..\..\demo\title.h
# End Source File
# End Group
# Begin Group "Resource Files"
# End Group
# End Target
# End Project
