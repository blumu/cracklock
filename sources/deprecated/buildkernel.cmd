@echo off
perl GenKernelDefv21.pl %SystemRoot%\\system32\\kernel32.dll
for %%i in (*.rc) do rc %%i
cl /c *.cpp ..\common.cpp /wd4530
link *.res *.obj ..\ShellExt\Release\CLSHEX.lib KERNEL32.LIB user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib shell32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Release/Bin/CLKERN.dll" /DEF:kernel.def
pause