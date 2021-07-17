@echo off

REM Uncomment one of the following line depending on which version of the Microsoft toolchain you are using
::set VSSETENV="C:\Program Files\Microsoft SDKs\Windows\v6.0\Bin\SetEnv.cmd"
::set VSSETENV=vars-vctoolkit2003.bat

set VSSETENV="%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"


if "%1" == "" 		  goto x86
if /i %1 == x86       goto x86
if /i %1 == x64       goto x64
if /i %1 == amd64     goto x64
goto error

:x86
set Arch=x86
@call %VSSETENV% x86
goto next

:x64
set Arch=x64
@call %VSSETENV% x86_amd64
goto next

:next
rem set MASM32PATH=d:\masm32
rem set INCLUDE=%INCLUDE%;%MASM32PATH%\include
rem set PATH=%MASM32PATH%\bin;%PATH%

set PATH=c:\cygwin\bin;%PATH%
Title Cracklock Build Environment %Arch%
goto :eof

:error
goto :eof


