# Microsoft Developer Studio Project File - Name="Allegro DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Allegro DLL - Win32 Release

!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Allegro DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Allegro DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Allegro DLL - Win32 Profile" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF "$(CFG)" == "Allegro DLL - Win32 Release"
# PROP Output_Dir "../../../lib/msvc"
# PROP Intermediate_Dir "../../../obj/msvc/alleg"
# ADD CPP /nologo /I "." /I "../../../" /I "../../../include" /MD /O2 /Gd /D ALLEGRO_SRC /D ALLEGRO_NO_ASM
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib /nologo /dll /machine:IX86 /release /implib:"..\..\..\lib\msvc\alleg.lib" /def:"..\..\..\lib\msvc\allegro.def" /out:"..\..\..\lib\msvc\alleg42.dll"
!ELSEIF "$(CFG)" == "Allegro DLL - Win32 Debug"
# PROP Output_Dir "../../../lib/msvc"
# PROP Intermediate_Dir "../../../obj/msvc/alld"
# ADD CPP /nologo /I "." /I "../../../" /I "../../../include" /MDd /Zi /Gd /D DEBUGMODE=1 /D ALLEGRO_SRC /D ALLEGRO_NO_ASM
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib /nologo /dll /machine:IX86 /debug /debugtype:cv /implib:"..\..\..\lib\msvc\alld.lib" /def:"..\..\..\lib\msvc\allegro.def" /out:"..\..\..\lib\msvc\alld42.dll"
!ELSEIF "$(CFG)" == "Allegro DLL - Win32 Profile"
# PROP Output_Dir "../../../lib/msvc"
# PROP Intermediate_Dir "../../../obj/msvc/allp"
# ADD CPP /nologo /I "." /I "../../../" /I "../../../include" /MD /Ox /Gd /D PROFILEMODE /D ALLEGRO_SRC /D ALLEGRO_NO_ASM
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib dinput.lib ddraw.lib dxguid.lib winmm.lib dsound.lib /nologo /dll /machine:IX86 /profile /implib:"..\..\..\lib\msvc\allp.lib" /def:"..\..\..\lib\msvc\allegro.def" /out:"..\..\..\lib\msvc\allp42.dll"
!ENDIF

# Begin Target
# Name "Allegro DLL - Win32 Release"
# Name "Allegro DLL - Win32 Debug"
# Name "Allegro DLL - Win32 Profile"
# Begin Group "Source Files"
# Begin Source File
SOURCE=..\..\..\src\allegro.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\blit.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\bmp.c
# End Source File
# Begin Group "c"
# Begin Source File
SOURCE=..\..\..\src\c\cblit16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cblit24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cblit32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cblit8.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\ccpu.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\ccsprite.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cgfx15.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cgfx16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cgfx24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cgfx32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cgfx8.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cmisc.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cscan15.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cscan16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cscan24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cscan32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cscan8.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cspr15.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cspr16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cspr24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cspr32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cspr8.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\cstretch.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\czscan15.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\czscan16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\czscan24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\czscan32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\c\czscan8.c
# End Source File
# End Group
# Begin Source File
SOURCE=..\..\..\src\clip3d.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\clip3df.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\colblend.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\color.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\config.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\datafile.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\dataregi.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\digmid.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\dispsw.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\dither.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\drvlist.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\file.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fli.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\flood.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\font.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fontbios.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fontbmp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fontdat.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fontgrx.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fonttxt.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\fsel.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\gfx.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\glyph.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\graphics.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\gsprite.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\gui.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\guiproc.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\inline.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\joystick.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\keyboard.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\lbm.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\libc.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\lzss.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\math.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\math3d.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\midi.c
# End Source File
# Begin Group "misc"
# Begin Source File
SOURCE=..\..\..\src\misc\ccolconv.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\misc\colconv.c
# End Source File
# End Group
# Begin Source File
SOURCE=..\..\..\src\mixer.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\modesel.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\mouse.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\pcx.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\poly3d.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\polygon.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\quantize.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\quat.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\readbmp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\readfont.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\readsmp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\rle.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\rotate.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\scene3d.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\sound.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\spline.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\stream.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\text.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\tga.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\timer.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\unicode.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable15.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable16.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable24.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\vtable8.c
# End Source File
# Begin Group "win"
# Begin Source File
SOURCE=..\..\..\src\win\gdi.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddaccel.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddbmp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddbmpl.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddfull.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddlock.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddmode.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddovl.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddraw.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wddwin.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wdispsw.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wdsinput.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wdsndmix.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wdsound.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wdxver.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wfile.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wgdi.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wgfxdrv.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\winput.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wjoydrv.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wjoydx.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wjoyhelp.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wjoyw32.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wkeybd.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wmidi.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wmouse.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wsnddrv.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wsndwo.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wsystem.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wthread.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wtimer.c
# End Source File
# Begin Source File
SOURCE=..\..\..\src\win\wwnd.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"
# Begin Source File
SOURCE=..\..\..\include\allegro.h
# End Source File
# Begin Group "allegro"
# Begin Source File
SOURCE=..\..\..\include\allegro\3d.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\3dmaths.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\alcompat.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\alinline.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\base.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\color.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\compiled.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\config.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\datafile.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\debug.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\digi.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\draw.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\file.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\fix.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\fixed.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\fli.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\fmaths.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\font.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\gfx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\graphics.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\gui.h
# End Source File
# Begin Group "inline"
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\3dmaths.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\asm.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\color.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\draw.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\file.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\fix.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\fmaths.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\gfx.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\matrix.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\rle.inl
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\inline\system.inl
# End Source File
# End Group
# Begin Group "internal"
# Begin Source File
SOURCE=..\..\..\include\allegro\internal\aintern.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\internal\aintvga.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\internal\alconfig.h
# End Source File
# End Group
# Begin Source File
SOURCE=..\..\..\include\allegro\joystick.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\keyboard.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\lzss.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\matrix.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\midi.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\mouse.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\palette.h
# End Source File
# Begin Group "platform"
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintbeos.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintdos.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintlnx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintmac.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintosx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintqnx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintunix.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aintwin.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\al386gcc.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\al386vc.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\al386wat.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\albcc32.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\albecfg.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\albeos.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aldjgpp.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\aldos.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\almac.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\almaccfg.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\almngw32.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\almsvc.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alosx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alosxcfg.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alplatf.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alqnx.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alqnxcfg.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alucfg.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alunix.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alwatcom.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\alwin.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\astdint.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\platform\macdef.h
# End Source File
# End Group
# Begin Source File
SOURCE=..\..\..\include\allegro\quat.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\rle.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\sound.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\stream.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\system.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\text.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\timer.h
# End Source File
# Begin Source File
SOURCE=..\..\..\include\allegro\unicode.h
# End Source File
# End Group
# Begin Source File
SOURCE=..\..\..\include\winalleg.h
# End Source File
# End Group
# Begin Group "Resource Files"
# Begin Group "win"
# Begin Source File
SOURCE=..\..\..\src\win\dllver.rc
# End Source File
# End Group
# End Group
# End Target
# End Project
