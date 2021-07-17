@echo off
for %%i in (*.rc) do rc %%i
cl *.cpp ..\common.cpp ..\listview.cpp /wd4530 /nologo /MT /W3 /GX /O2 /D "WIN32" /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_WINDOWS" /Fr /FD /c
link *.res *.obj ..\ShellExt\Release\CLSHEX.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib shell32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../Release/Bin/CLSHEX.dll" /DEF:shellext.def
