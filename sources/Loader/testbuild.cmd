@echo off
setlocal
if "%Platform%" == "" set Platform=x86
goto go

:ms
if /I %Platform% == x86 (set OPT=/ms) else (set OPT= )
echo on
goasm %OPT% /%Platform% %1.asm
link /OUT:%1.exe /MAP /SUBSYSTEM:CONSOLE /MACHINE:%Platform% /ENTRY:START %1.obj kernel32.lib
echo off
goto :eof

:go
echo on
goasm /%Platform% %1.asm
golink /console %1.obj Kernel32.dll
echo off
goto :eof

endlocal