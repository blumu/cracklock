@echo off
for %%i in (*.rc) do rc %%i
cl /c *.c ..\common.cpp
link *.res *.obj ..\ShellExt\Release\CLSHEX.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib shell32.lib /nologo /subsystem:windows /machine:I386 /out:"../Release/Bin/MCL.exe"
