# Microsoft Developer Studio Project File - Name="aldat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# TARGTYPE "Win32 (x86) Static  Library" 0x0104

CFG=aldat - Win32 Release

!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aldat - Win32 Release" (based on "Win32 (x86) Static  Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
BSC32=bscmake.exe
LIB32=link.exe -lib

!IF "$(CFG)" == "aldat - Win32 Release"
# PROP Output_Dir "../../../lib/msvc"
# PROP Intermediate_Dir "../../../obj/msvc/alleg"
# ADD CPP /nologo /I "." /I "../../../" /I "../../../include" /MD /O2 /Gd
# ADD LIB32 /nologo /out:"..\..\..\lib\msvc\aldat.lib"
!ENDIF

# Begin Target
# Name "aldat - Win32 Release"
# Begin Group "Source Files"
# Begin Source File
SOURCE=..\..\..\tools\datedit.c
# End Source File
# Begin Group "plugins"
# Begin Source File
SOURCE=..\..\..\tools\plugins\datalpha.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datfli.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datfname.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datfont.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datgrab.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datgrid.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datimage.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datitype.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datmidi.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datpal.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datsamp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\tools\plugins\datworms.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"
# End Group
# Begin Group "Resource Files"
# End Group
# End Target
# End Project
