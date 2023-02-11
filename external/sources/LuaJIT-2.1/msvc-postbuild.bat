@rem This will be executed as a post-build job when building the static lib in VS.

@if not defined INCLUDE goto :FAIL

cd %~dp0\src\

@setlocal
@set LJCOMPILE=cl /nologo /c /O2 /W3 /D_CRT_SECURE_NO_DEPRECATE /D_CRT_STDIO_INLINE=__declspec(dllexport)__inline
@set LJLINK=link /nologo
@set LJMT=mt /nologo
@set LJLIBNAMESTEM=..\_Bin\luajit-
@set LJLIBNAMEEXT=.lib
@set BUILDTYPE=release

@rem Call minilua to determine what the architecture is.
@minilua
@if errorlevel 8 (
  @if "%1"=="" (@set BUILDTYPE=release-64)
  @if "%1"=="debug" (@set BUILDTYPE=debug-64)
  @if "%1"=="debug-release" (@set BUILDTYPE=debug-release-64)
) else (
  @if "%1"=="debug" (@set BUILDTYPE=debug)
  @if "%1"=="debug-release" (@set BUILDTYPE=debug-release)
)

@set LJLIBNAME=%LJLIBNAMESTEM%%BUILDTYPE%%LJLIBNAMEEXT%

%LJCOMPILE% luajit.c
@if errorlevel 1 goto :BAD
%LJLINK% /out:luajit.exe luajit.obj %LJLIBNAME%
@if errorlevel 1 goto :BAD
if exist luajit.exe.manifest^
  %LJMT% -manifest luajit.exe.manifest -outputresource:luajit.exe

@del *.lib *.obj *.exp *.manifest *.exe
@del host\buildvm_arch.h
@del lj_bcdef.h lj_ffdef.h lj_libdef.h lj_recdef.h lj_folddef.h

@echo.
@echo === Successfully complete post-build job for Windows/%LJARCH% ===

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