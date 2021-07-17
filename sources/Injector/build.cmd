:: Syntax:
:: build.cmd $(InputPath) $(PlatformName) $(ConfigurationName) 
@echo off

setlocal

:get_platform
if /I "%4"=="Debug" (goto debug ) else ( goto release)
:debug
set CONF=Debug
set LINK_OPT=
goto get_arch
:release 
set LINK_OPT=/IGNORE:4089
set CONF=Release
goto get_arch

:get_arch
if /I "%3"=="x64" (goto x64) else ( goto Win32 )
:x64
set Arch=x64
set ML=ML64.exe
set LINK=LINK.exe
goto next
:Win32
set Arch=Win32
set MLBIN=ml.exe
set LINKBIN=link.exe

set MASM32_PATH=C:\setupless_progs\masm32
set INCLUDE=%INCLUDE%;%MASM32_PATH%\INCLUDE
::set LIB=%LIB%;%MASM32_PATH%\LIB
::set MLBIN=%MASM32_PATH%\bin\ML.exe
::set LINKBIN=%MASM32_PATH%\bin\LINK.exe
goto next

:next
echo Architecture:   %Arch%
echo Configuration:  %CONF%
echo.
set OBJDIR=..\obj_%Arch%_%CONF%
set OUTDIR=..\bin_%Arch%_%CONF%
set SRCFILE=%2

if /I "%1"=="compile" (goto compile) else ( goto link )


:compile
%MLBIN% /c /coff %SRCFILE%.asm
if errorlevel 1 goto errasm
move /y %SRCFILE%.obj %OBJDIR%\Injector\
goto TheEnd

:link
echo %LINKBIN% /SUBSYSTEM:WINDOWS %LINK_OPT% /DEBUG %OBJDIR%\Injector\CLINJECT.obj %OBJDIR%\Kernel\CLKERN.lib /out:%OUTDIR%\bin\CLINJECT.EXE
%LINKBIN% /SUBSYSTEM:WINDOWS %LINK_OPT% /DEBUG %OBJDIR%\Injector\CLINJECT.obj %OBJDIR%\Kernel\CLKERN.lib /out:%OUTDIR%\bin\CLINJECT.EXE
if errorlevel 1 goto errlink
goto TheEnd

:errlink
echo _
echo Error while linking!
goto TheEnd

:errasm
echo _
echo Error while assembling!
goto TheEnd

:TheEnd
endlocal
rem pause
