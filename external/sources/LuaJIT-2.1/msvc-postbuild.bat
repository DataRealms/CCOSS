cd %~dp0\src\

@setlocal
@set LJCOMPILE=cl /nologo /c /O2 /W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_STDIO_INLINE=__declspec(dllexport)__inline
@set LJLINK=link /nologo
@set LJMT=mt /nologo
@set LJLIBNAMESTEM=..\_Bin\luajit-
@set LJLIBNAMEEXT=.lib
@set BUILDTYPE=release
@set LJARCH=x64

@minilua >nul 2>&1
@if errorlevel 8 goto :X64
@echo x86
@set LJARCH=x86
@if "%1"=="debug" (
  @set BUILDTYPE=debug
)
@if "%1"=="debug-release" (
  @set BUILDTYPE=debug-release
)

:X64
@echo x64
@if "%1"=="" (
  @set BUILDTYPE=release-64
)
@if "%1"=="debug" (
  @set BUILDTYPE=debug-64
)
@if "%1"=="debug-release" (
  @set BUILDTYPE=debug-release-64
)

@set LJLIBNAME=%LJLIBNAMESTEM%%BUILDTYPE%%LJLIBNAMEEXT%

%LJCOMPILE% luajit.c
@if errorlevel 1 goto :BAD
%LJLINK% /out:luajit.exe luajit.obj %LJLIBNAME%
@if errorlevel 1 goto :BAD
if exist luajit.exe.manifest^
  %LJMT% -manifest luajit.exe.manifest -outputresource:luajit.exe

@move /y %LJLIBNAME% ..\_Bin\%LJLIBNAME%

@del *.lib *.obj *.exp *.manifest *.exe
@del host\buildvm_arch.h
@del lj_bcdef.h lj_ffdef.h lj_libdef.h lj_recdef.h lj_folddef.h

@goto :END
:BAD
@echo.
@echo *******************************************************
@echo *** Build FAILED -- Please check the error messages ***
@echo *******************************************************
@goto :END
:FAIL
@echo You must open a "Visual Studio Command Prompt" to run this script
:END