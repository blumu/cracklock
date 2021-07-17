@echo off
call vsvars32.bat

echo Compilation des sources de cracklock
echo =============================

echo.
echo   * Cracklock Resources
echo.
cd resources
call build.cmd

echo   * ShellExtension
echo.
cd ..\shellext
call build.cmd

echo.
echo   * Cracklock Injector
echo.
cd ..\injector
call bldrelease.bat

echo   * Cracklock Loader
echo.
cd ..\loader
call build.cmd

echo   * Cracklock Manager
echo.
cd ..\manager
call build.cmd

echo   * Cracklock Kernel
echo.
cd ..\kernel
call build.cmd

:package

cd Installation
perl buildpackage.pl
cd ..


pause
