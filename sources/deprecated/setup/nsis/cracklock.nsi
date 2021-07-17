!ifdef NO_COMPRESSION
SetCompress off
SetDatablockOptimize off
!endif

!ifdef NO_CRC
CRCCheck off
!endif

Name "Cracklock"
Caption "Cracklock Setup"
OutFile cklk384.exe

#BGGradient 000000 800000 FFFFFF
#InstallColors FF8080 000000

LicenseText "You must read the following license before installing:"
;*LicenseData license.txt
ComponentText "This will install Cracklock 3.8.4 on your computer:"
InstType Normal
AutoCloseWindow false
ShowInstDetails show
DirText "Please select a location to install Cracklock (or use the default):"
SetOverwrite on
SetDateSave on
!ifdef HAVE_UPX
  !packhdr tmp.dat "c:\outils\upx\upx --best --compress-icons=1 tmp.dat"
!endif

InstallDir $PROGRAMFILES\Cracklock
;InstallDirRegKey HKLM SOFTWARE\Cracklock ""

Section "Cracklock system (required)"
  SectionIn 1 2
  SetOutPath $INSTDIR
  File "..\Documentation\Cracklock.rtf"

  SetOutPath $INSTDIR\Bin
  File "..\Release\Bin\MCL.exe"
  File "..\Release\Bin\CLINJECT.exe"
  File "..\Release\Bin\CLSHEX.dll"
  File "..\Release\Bin\CLMNGR.exe"

  SetOutPath $INSTDIR\Languages
  File "..\Release\Languages\*.DLL"

  SetOutPath $SYSDIR
  File "..\Release\Bin\CLKERN.DLL"

  ExecWait "$INSTDIR\Bin\CLMNGR.exe -setup"

  SetOutPath $SMPROGRAMS\Cracklock
  CreateShortCut "$SMPROGRAMS\Cracklock\Cracklock Manager.lnk" \
                 "$INSTDIR\Bin\CLMNGR.EXE"
  CreateShortCut "$SMPROGRAMS\Cracklock\Cracklock.net web site.lnk" \
                 "http://www.cracklock.net/"

  CreateShortCut "$SMPROGRAMS\Cracklock\Uninstall Cracklock.lnk" \
                 "$INSTDIR\uninst-cklk.exe"
SectionEnd


Section "Cracklock Examples"
  SectionIn 1 2
  SetOutPath $INSTDIR\Examples
  File "..\Examples\VCDATE.EXE"
  File  "..\Examples\VBDate.exe"
SectionEnd

Section "Cracklock Help"
  SectionIn 1 2
  SetOutPath $INSTDIR
  File /r "C:\DEV\VisualC\Cracklock\Help"

  CreateShortCut "$SMPROGRAMS\Cracklock\Documentation.lnk" \
                 "$INSTDIR\Bin\CLMNGR.EXE" -help $SYSDIR\SHELL32.dll 23
SectionEnd

Section "Cracklock Shell Extensions"
  SectionIn 1 2

  WriteRegStr HKLM "SOFTWARE\MicroBest\Cracklock\3.0\Apps" "" "Apps"
  ; Flags: uninsdeletekeyifempty;
  WriteRegStr HKLM "SOFTWARE\MicroBest\Cracklock\3.0\Settings" "" ""
  WriteRegStr HKLM "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}" "" "MicroBest Cracklock"
  WriteRegStr HKLM "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}\InProcServer32" "" "{app}\Bin\CLSHEX.DLL"
  WriteRegStr HKLM "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}\InProcServer32" "ThreadingModel" "Apartment"
  WriteRegStr HKLM "*\shellex\PropertySheetHandlers\Cracklock" "" "{{6EF84290-174B-11d1-B524-0080C8141490}"
  WriteRegStr HKLM "exefile\shellex\ContextMenuHandlers\Cracklock" "" "{{6EF84290-174B-11d1-B524-0080C8141490}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved" "" "MicroBest Cracklock"
  WriteRegStr HKLM "SOFTWARE\MicroBest\Cracklock\3.0" "" ""
  ; Flags: uninsdeletevalue uninsdeletekeyifempty;
  WriteRegStr HKLM "SOFTWARE\MicroBest\Cracklock" "" ""
  ; Flags: uninsdeletevalue uninsdeletekeyifempty;
  WriteRegStr HKLM "SOFTWARE\MicroBest" Flags: uninsdeletevalue uninsdeletekeyifempty;



  ; back up old value of .nsi
  ReadRegStr $1 HKCR ".nsi" ""
  StrCmp $1 "" Label1
    StrCmp $1 "NSISFile" Label1
    WriteRegStr HKCR ".nsi" "backup_val" $1


Label1:

  WriteRegStr HKLM ".nsi" "" "NSISFile"
  WriteRegStr HKLM "NSISFile" "" "NSI Script File"
  WriteRegStr HKLM "NSISFile\shell" "" "open"
  WriteRegStr HKLM "NSISFile\DefaultIcon" "" $INSTDIR\makensis.exe,0
  WriteRegStr HKLM "NSISFile\shell\open\command" "" 'notepad.exe "%1"'
  WriteRegStr HKLM "NSISFile\shell\compile" "" "Compile NSI"
  WriteRegStr HKLM "NSISFile\shell\compile\command" "" '"$INSTDIR\makensis.exe" /CD /PAUSE "%1"'
SectionEnd



!ifndef NO_SOURCE
;SectionDivider

;Section "NSIS Source Code"
  ;SectionIn 2
  ;SetOutPath $INSTDIR\Source
  ;File Source\*.cpp
  ;File Source\*.h
  ;File Source\script1.rc
  ;File Source\makenssi.dsp
  ;File Source\makenssi.dsw
  ;File Source\icon.ico
  ;SetOutPath $INSTDIR\Source\zlib
  ;File Source\zlib\*.*
  ;SetOutPath $INSTDIR\Source\exehead
  ;File Source\exehead\*.c
  ;File Source\exehead\*.h
  ;File Source\exehead\resource.rc
  ;File Source\exehead\exehead.dsp
  ;File Source\exehead\exehead.dsw
  ;File Source\exehead\nsis.ico
  ;File Source\exehead\uninst.ico
  ;File Source\exehead\bitmap1.bmp
  ;;File Source\exehead\bitmap2.bmp
  ;File Source\exehead\bin2h.exe
  ;SetOutPath $INSTDIR\Source\Splash
  ;File Source\Splash\splash.c
  ;File Source\Splash\splash.dsp
  ;File Source\Splash\splash.dsw
  ;SetOutPath $INSTDIR\Source\zip2exe
  ;File Source\zip2exe\*.cpp
  ;File Source\zip2exe\*.ico
  ;File Source\zip2exe\*.h
  ;File Source\zip2exe\*.rc
  ;File Source\zip2exe\*.dsw
  ;File Source\zip2exe\*.dsp
  ;SetOutPath $INSTDIR\Source\zip2exe\zlib
  ;File Source\zip2exe\zlib\*.*
  ;IfFileExists $SMPROGRAMS\NSIS 0 NoSourceShortCuts
    ;CreateShortCut "$SMPROGRAMS\NSIS\MakeNSIS project workspace.lnk" \
     ;              "$INSTDIR\source\makenssi.dsw"
    ;CreateShortCut "$SMPROGRAMS\NSIS\ZIP2EXE project workspace.lnk" \
    ;;               "$INSTDIR\source\zip2exe\zip2exe.dsw"
    ;CreateShortCut "$SMPROGRAMS\NSIS\Splash project workspace.lnk" \
                   ;"$INSTDIR\source\splash\splash.dsw"
  ;NoSourceShortCuts:
;SectionEnd
;
!endif

Section -post
  SetOutPath $INSTDIR

  ; since the installer is now created last (in 1.2+), this makes sure 
  ; that any old installer that is readonly is overwritten.
  Delete $INSTDIR\uninst-nsis.exe 

  WriteRegStr HKLM SOFTWARE\NSIS "" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cracklock" \
                   "DisplayName" "NSIS Development Kit (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cracklock" \
                   "UninstallString" '"$INSTDIR\uninst-nsis.exe"'
  ExecShell open '$INSTDIR'
  Sleep 500
  BringToFront
SectionEnd

Function .onInstSuccess
  MessageBox MB_YESNO|MB_ICONQUESTION \
             "Setup has completed. View readme file now?" \
             IDNO NoReadme
    ExecShell open '$INSTDIR\makensis.htm'
  NoReadme:
FunctionEnd

!ifndef NO_UNINST
UninstallText "This will uninstall Cracklock from your system:"
UninstallExeName uninst-cklk.exe

Section Uninstall
  ExecWait "$INSTDIR\Bin\CLMNGR.exe -uninstall"

  ReadRegStr $1 HKCR ".nsi" ""
  StrCmp $1 "NSISFile" 0 NoOwn ; only do this if we own it
    ReadRegStr $1 HKCR ".nsi" "backup_val"
    StrCmp $1 "" 0 RestoreBackup ; if backup == "" then delete the whole key
      DeleteRegKey HKCR ".nsi"
    Goto NoOwn
    RestoreBackup:
      WriteRegStr HKCR ".nsi" "" $1
      DeleteRegValue HKCR ".nsi" "backup_val"
  NoOwn:

  DeleteRegKey HKCR "NSISFile"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cracklock384"
  DeleteRegKey HKLM SOFTWARE\Cracklock
  Delete $SMPROGRAMS\NSIS\*.lnk
  RMDir $SMPROGRAMS\Cracklock
  Delete $DESKTOP\MakeNSIS.lnk
  Delete $INSTDIR\makensis.exe
  Delete $INSTDIR\zip2exe.exe
  Delete $INSTDIR\splash.txt
  Delete $INSTDIR\splash.exe
  Delete $INSTDIR\makensis.htm
  Delete $INSTDIR\makensis.rtf
  Delete $INSTDIR\uninst-nsis.exe
  Delete $INSTDIR\nsisconf.nsi
  Delete $INSTDIR\makensis.nsi
  Delete $INSTDIR\example1.nsi
  Delete $INSTDIR\example2.nsi
  Delete $INSTDIR\waplugin.nsi
  Delete $INSTDIR\viewhtml.nsi
  Delete $INSTDIR\bigtest.nsi
  Delete $INSTDIR\uglytest.nsi
  Delete $INSTDIR\spin.nsi
  Delete $INSTDIR\wafull.nsi
  Delete $INSTDIR\main.ico
  Delete $INSTDIR\makensis-license.txt
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\uninst.ico
  Delete $INSTDIR\bitmap1.bmp
  Delete $INSTDIR\bitmap2.bmp
  RMDir /r $INSTDIR\Source
  RMDir $INSTDIR

  ; if $INSTDIR was removed, skip these next ones
  IfFileExists $INSTDIR 0 Removed 
    MessageBox MB_YESNO|MB_ICONQUESTION \
      "Remove all files in your Cracklock directory? (If you have anything\
 you created that you want to keep, click No)" IDNO Removed
    Delete $INSTDIR\*.* ; this would be skipped if the user hits no
    RMDir /r $INSTDIR
    IfFileExists $INSTDIR 0 Removed 
      MessageBox MB_OK|MB_ICONEXCLAMATION \
                 "Note: $INSTDIR could not be removed."
  Removed:
SectionEnd

!endif
