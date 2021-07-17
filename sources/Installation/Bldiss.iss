#include "ModifyPath.iss"

[Setup]
PrivilegesRequired=none
AppName=Cracklock
AppPublisher=William Blum
AppPublisherURL=http://william.famille-blum.org/
AppSupportURL=http://william.famille-blum.org/
AppUpdatesURL=http://william.famille-blum.org/
AllowNoIcons=0
;AlwaysRestart=1
CreateAppDir=1
DefaultDirName={pf}\Cracklock
DefaultGroupName=Cracklock
DisableStartupPrompt=1
DisableDirPage=0
DisableProgramGroupPage=1
DisableReadyPage=0
DisableReadyMemo=1
DisableFinishedPage=0
DiskClusterSize=512
InfoBeforeFile=Install-message.rtf
ReserveBytes=0
Uninstallable=1
DiskSpanning=0
WindowVisible=0
OutputDir=output
AppVersion=x.x.x
AppVerName=Cracklock x.x.x
AppCopyright=Copyright XXX
OutputBaseFilename=cklkxxx
VersionInfoVersion=x.x.x
VersionInfoTextVersion=x.x.x



; If you want all languages to be listed in the "Select Setup Language"
; dialog, even those that can't be displayed in the active code page,
; uncomment the following two lines.
[LangOptions]
LanguageCodePage=0

[Languages]
Name: ar; MessagesFile: "compiler:Languages\Arabic-4-5.1.11.isl"
Name: chs; MessagesFile: "compiler:Languages\ChineseSimp-11-5.1.0.isl"
Name: cs; MessagesFile: "compiler:Languages\Czech-5-5.1.11.isl"
Name: de; MessagesFile: "compiler:Languages\German.isl"
Name: es; MessagesFile: "compiler:Languages\Spanish.isl"
Name: fr; MessagesFile: "compiler:Languages\French.isl"
Name: hr; MessagesFile: "compiler:Languages\Croatian-5-5.1.11.isl"
Name: hu; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: ko; MessagesFile: "compiler:Languages\Korean-5-5.1.11.isl"
Name: pt; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: us; MessagesFile: "compiler:Default.isl"



[Code]
const
WM_SETTINGCHANGE = $1A;
SMTO_ABORTIFHUNG = $2;

function SendMessageTimeout(hwnd :LongInt; msg :LongInt; wParam :LongInt;
lParam :String; fuFlags :LongInt; uTimeout :LongInt; var lpdwResult :LongInt): LongInt;
external 'SendMessageTimeoutA@user32.dll stdcall';

function CreateSemaphore(lpSemaphoreAttributes :LongInt; lInitialCount :LongInt; lMaximumCount :LongInt;
lpName :String): LongInt;
external 'CreateSemaphoreA@kernel32.dll stdcall';


function ReleaseSemaphore(hSemaphore :LongInt; lReleaseCount :LongInt; lpPreviousCount :LongInt ): bool;
external 'ReleaseSemaphore@kernel32.dll stdcall';

function CloseHandle(hSemaphore :LongInt ): bool;
external 'CloseHandle@kernel32.dll stdcall';


procedure KillResidantInjector();
var hSemaphore : LongInt;
begin
  	hSemaphore := CreateSemaphore(0, 0, 1, 'CLINJECT.EXE');
    ReleaseSemaphore(hSemaphore, 1, 0);
    CloseHandle(hSemaphore);
end;


// Get the name of the language file corresponding to the language selected by the user at the initialization of the setup
function GetLanguageDll(dummy:string) : string;
begin
    if ExpandConstant('{language}') = 'us' then
      Result := 'CLRESUS.dll'
    else
      Result := ExpandConstant('CLRESUS_{language}.dll');
end;

procedure CurStepChanged(CurStep: TSetupStep);
// var filecopied : Boolean;
var res :LOngInt;
begin
  if CurStep = ssPostInstall then
  begin
    // Try to copy CLKERN.DLL in windows\system32
    //filecopied := FileCopy('{app}\Bin\CLKERN.DLL', '{sys}\CLKERN.DLL', false);

    // Add Cracklock\Bin to the path
    //  if not filecopied then
    //ModifyPath('{app}\Bin', pmAddOnlyIfDirExists + pmAddToEnd,  psCurrentUser);
    //SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0,'Environment', SMTO_ABORTIFHUNG, 5000, res);
  end
  else if CurStep = ssInstall then
  begin
    KillResidantInjector();
  end;
end;


procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
  begin
    KillResidantInjector();
  end
  else if CurUninstallStep = usPostUninstall then
  begin
    // Remove Cracklock\Bin from the path
    //ModifyPath('{app}\Bin', pmRemove,  psCurrentUser);
  end;
end;

function BackslashToSlash(src: string) : string;
var i : integer;
begin
  for i := 1 to length(src) do
    begin
      if src[i] = '\' then
        src [i] := '/'
    end
  Result := src  ;
end;


// IssFindModule called on install
function IssFindModule(hWnd: Integer; Modulename: PChar; Language: PChar; Silent: Boolean; CanIgnore: Boolean ): Integer;
external 'IssFindModule@files:IssProc.dll stdcall setuponly';

// IssFindModule called on uninstall
function IssFindModuleU(hWnd: Integer; Modulename: PChar; Language: PChar; Silent: Boolean; CanIgnore: Boolean ): Integer;
external 'IssFindModule@{app}\IssProc.dll stdcall uninstallonly';

//********************************************************************************************************************************************
// IssFindModule function returns: 0 if no module found; 1 if cancel pressed; 2 if ignore pressed; -1 if an error occured
//
//  hWnd        = main wizard window handle.
//
//  Modulename  = module name(s) to check. You can use a full path to a DLL/EXE/OCX or wildcard file name/path. Separate multiple modules with semicolon.
//                 Example1 : Modulename='*mymodule.dll';     -  will search in any path for mymodule.dll
//                 Example2 : Modulename=ExpandConstant('{app}\mymodule.dll');     -  will search for mymodule.dll only in {app} folder (the application directory)
//                 Example3 : Modulename=ExpandConstant('{app}\mymodule.dll;*myApp.exe');   - just like Example2 + search for myApp.exe regardless of his path.
//
//  Language    = files in use language dialog. Set this value to empty '' and default english will be used
//                ( see and include IssProcLanguage.ini if you need custom text or other language)
//
//  Silent      = silent mode : set this var to true if you don't want to display the files in use dialog.
//                When Silent is true IssFindModule will return 1 if it founds the Modulename or 0 if nothing found
//
//  CanIgnore   = set this var to false to Disable the Ignore button forcing the user to close those applications before continuing
//                set this var to true to Enable the Ignore button allowing the user to continue without closing those applications
//******************************************************************************************************************************************


function NextButtonClick(CurPage: Integer): Boolean;
var
  hWnd: Integer;
  sModuleName: String;
  nCode: Integer;  {IssFindModule returns: 0 if no module found; 1 if cancel pressed; 2 if ignore pressed; -1 if an error occured }
begin
  Result := true;

 if CurPage = wpReady then
   begin
      Result := false;
      ExtractTemporaryFile('IssProcLanguage.ini');                          { extract extra language file - you don't need to add this line if you are using english only }
      hWnd := StrToInt(ExpandConstant('{wizardhwnd}'));                     { get main wizard handle }
      sModuleName :=ExpandConstant('*CLMNGR.exe;*CLKERN.dll;CLSHEX.dll;*MCL.exe;*CLINJECT.EXE;');                        { searched modules. Tip: separate multiple modules with semicolon Ex: '*mymodule.dll;*mymodule2.dll;*myapp.exe'}

     nCode:=IssFindModule(hWnd,sModuleName,'en',false,true);                { search for module and display files-in-use window if found  }
     //sModuleName:=IntToStr(nCode);
    // MsgBox ( sModuleName, mbConfirmation, MB_YESNO or MB_DEFBUTTON2);

     if nCode=1 then  begin                                                 { cancel pressed or files-in-use window closed }
          PostMessage (WizardForm.Handle, $0010, 0, 0);                     { quit setup, $0010=WM_CLOSE }
     end else if (nCode=0) or (nCode=2) then begin                          { no module found or ignored pressed}
          Result := true;                                                   { continue setup  }
     end;

  end;

end;


function InitializeUninstall(): Boolean;
var
  sModuleName: String;
  nCode: Integer;  {IssFindModule returns: 0 if no module found; 1 if cancel pressed; 2 if ignore pressed; -1 if an error occured }

begin
    Result := false;
     sModuleName := ExpandConstant('*CLMNGR.exe;*CLKERN.dll;CLSHEX.dll;*MCL.exe;*CLINJECT.EXE');          { searched module. Tip: separate multiple modules with semicolon Ex: '*mymodule.dll;*mymodule2.dll;*myapp.exe'}

     nCode:=IssFindModuleU(0,sModuleName,'enu',false,true); { search for module and display files-in-use window if found  }

     if (nCode=0) or (nCode=2) then begin                    { no module found or ignored pressed}
          Result := true;                                    { continue setup  }
     end;

    // Unload the InnoSetup extension, otherwise it will not be deleted by the uninstaller
    UnloadDLL(ExpandConstant('{app}\IssProc.dll'));

end;




[Icons]
Name: "{group}\Cracklock Manager"; Filename: "{app}\Bin\CLMNGR.EXE"; WorkingDir: "{app}"; Components: shortcuts
Name: "{group}\Documentation"; Filename: "{app}\Bin\CLMNGR.EXE"; Parameters: "-help"; WorkingDir: "{app}"; IconFilename: "{sys}\SHELL32.dll"; IconIndex: 23 ; Components: Help and shortcuts
Name: "{group}\Cracklock website"; Filename: "http://william.famille-blum.org/software/cracklock/index.html"; Components: shortcuts
Name: "{group}\Uninstall Cracklock"; Filename: "{uninstallexe}"; Components: shortcuts
;Name: "{group}\Control panel date/time"; Filename: "{app}\Bin\MCL.EXE"; Parameters: """{sys}\RUNDLL32.EXE"" {sys}\shell32.dll,Control_RunDLL ""{sys}\timedate.cpl"""; IconFilename: "{sys}\SHELL32.dll"; IconIndex: 276 ; Components: shortcuts



[INI]
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Settings"; Key:"DefaultInjectionMode"; String:"dword:1"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Dlls"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "Name"; String:"string:Date and Time Control Panel Applet"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "Parameters"; String:"string:{sys}\shell32.dll,Control_RunDLL ""{sys}\timedate.cpl"""
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "LoaderFile"; String:"string:"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "Flags"; String:"dword:113"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "Date"; String:"binary:001A1C70B3F6BC01"
Filename: "{app}\Cracklock.settings"; Section: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; Key: "Timezone"; String:"binary:C4FFFFFF57002E0020004500750072006F007000650020005300740061006E0064006100720064002000540069006D0065000000FEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFE00000A000000050003000000000000000000000057002E0020004500750072006F007000650020004400610079006C0069006700680074002000540069006D0065000000FEFEFEFEFEFEFEFEFEFEFEFEFEFEFEFE00000300000005000200000000000000C4FFFFFF"



; Set the language file for Cracklock according to the language selection made by the user during the setup
Filename: "{app}\Cracklock.settings"; Section: "Software\MicroBest\Cracklock\3.0\Settings"; Key: "Language"; String:"string:{code:GetLanguageDll}"


[Registry]
;Root: HKCU; Subkey: "SOFTWARE\MicroBest"; ValueType: string; Flags: uninsdeletevalue uninsdeletekeyifempty;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock"; ValueType: string; Flags: uninsdeletevalue uninsdeletekeyifempty;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0"; ValueType: string; Flags: uninsdeletevalue uninsdeletekeyifempty;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps"; ValueType: string; ValueName: ""; ValueData: ""; Flags: uninsdeletekeyifempty;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Dlls"; ValueType: string; ValueName: ""; ValueData: ""; Flags: uninsdeletekeyifempty;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Settings"; ValueType: string; ValueName: ""; ValueData: ""; Flags: uninsdeletekeyifempty ;

;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: string; ValueName: "Name"; ValueData: "Windows Control Panel Applet"; Flags: uninsdeletekeyifempty ;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: string; ValueName: "Parameters"; ValueData: "{sys}\shell32.dll,Control_RunDLL ""{sys}\timedate.cpl"""; Flags: uninsdeletekeyifempty ;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: string; ValueName: "LoaderFile"; ValueData: ""; Flags: uninsdeletekeyifempty ;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: dword;  ValueName: "Flags"; ValueData: $71; Flags: uninsdeletekeyifempty ;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: binary; ValueName: "Date"; ValueData: 00 1a 1c 70 b3 f6 bc 01; Flags: uninsdeletekeyifempty ;
;Root: HKCU; Subkey: "SOFTWARE\MicroBest\Cracklock\3.0\Apps\{code:BackslashToSlash|{sys}\RUNDLL32.EXE}"; ValueType: binary; ValueName: "Timezone"; ValueData: c4 ff ff ff 57 00 2e 00 20 00 45 00 75 00 72 00 6f 00 70 00 65 00 20 00 53 00 74 00 61 00 6e 00 64 00 61 00 72 00 64 00 20 00 54 00 69 00 6d 00 65 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0a 00 00 00 05 00 03 00 00 00 00 00 00 00 00 00 00 00 57 00 2e 00 20 00 45 00 75 00 72 00 6f 00 70 00 65 00 20 00 44 00 61 00 79 00 6c 00 69 00 67 00 68 00 74 00 20 00 54 00 69 00 6d 00 65 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00 00 00 05 00 02 00 00 00 00 00 00 00 c4 ff ff ff; Flags: uninsdeletekeyifempty ;


Root: HKCR; Subkey: "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}"; ValueType: string; ValueName: ""; ValueData: "Cracklock"; Flags: uninsdeletekey ; Components: "system\shellext"
Root: HKCR; Subkey: "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}\InProcServer32"; ValueType: string; ValueName: ""; ValueData: "{app}\Bin\CLSHEX.DLL"; Flags: uninsdeletekey ; Components: "system\shellext"
Root: HKCR; Subkey: "CLSID\{{6EF84290-174B-11d1-B524-0080C8141490}\InProcServer32"; ValueType: string; ValueName: "ThreadingModel"; ValueData: "Apartment"; Flags: uninsdeletekey ; Components: "system\shellext"
Root: HKCR; Subkey: "*\shellex\PropertySheetHandlers\Cracklock"; ValueType: string; ValueName: ""; ValueData: "{{6EF84290-174B-11d1-B524-0080C8141490}"; Flags: uninsdeletekey ; Components: "system\shellext"
Root: HKCR; Subkey: "exefile\shellex\ContextMenuHandlers\Cracklock"; ValueType: string; ValueName: ""; ValueData: "{{6EF84290-174B-11d1-B524-0080C8141490}"; Flags: uninsdeletekey; Components: "system\shellext"
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved"; ValueType: string; ValueName: "{{6EF84290-174B-11d1-B524-0080C8141490}"; ValueData: "Cracklock"; Flags: uninsdeletekey ; Components: "system\shellext"



[DeleteFiles]

[Run]
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -add-path"; Components:shared
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -remove-path"; Components: not shared
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -set-storage-appdata"; Components:storage\appdata
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -set-storage-bin"; Components:storage\bin
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -set-storage-winreg"; Components:storage\winreg
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -set-syswide"; Components:system\systemwideinjection
Filename: "{app}\Bin\CLMNGR.exe"; Description: "Launch the Cracklock Manager"; Flags:postinstall nowait;


[UninstallRun]
Filename: "{app}\Bin\CLMNGR.exe"; Parameters: " -uninstall";


[Dirs]
Name: "{app}\Bin"
Name: "{app}\Languages"
Name: "{app}\Examples"; Components: Examples
Name: "{app}\Help"; Components: Help

[Types]
Name: "full"; Description: "Full installation"
Name: "compact"; Description: "Compact installation"
Name: "flashdisk"; Description: "Flashdisk installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom


[Components]
Name: main; Description: "Main Files"; Types: full compact custom flashdisk; Flags: fixed
Name: storage; Description: "Storage location for Cracklock settings:"; Types: full compact custom; Flags: fixed
Name: storage\winreg; Description: "Windows registry"; Types: full compact custom ; Flags:exclusive
Name: storage\bin; Description: "INI file in Cracklock's directory"; Types: full compact custom flashdisk; Flags:exclusive
Name: storage\appdata; Description: "INI file in user Appdata directory"; Types: full compact custom; Flags:exclusive
Name: shortcuts; Description: "Create shortcuts"; Types: full compact custom;
Name: shared; Description: "Shared installation (add cracklock to the PATH environment variable)"; Types: full compact custom;

Name: "system"; Description: "Features requiring administrator rights"
;Name: "system\copydll"; Description: "Copy hook dll to Windows system32 folder"; Types: full
Name: "system\shellext"; Description: "Explorer shell extention"
Name: "system\systemwideinjection"; Description: "System-wide injection"
Name: "examples"; Description: "Examples"; Types: full flashdisk
Name: "help"; Description: "Help Files"; Types: full flashdisk

[Files]
;------ add IssProc (Files In Use Extension)
Source: IssProc.dll; DestDir: {tmp}; Flags: dontcopy
;------ add IssProc extra language file (you don't need to add this file if you are using english only)
Source: IssProcLanguage.ini; DestDir: {tmp}; Flags: dontcopy
;------ Copy IssProc in your app folder if you want to use it on unistall
Source: IssProc.dll; DestDir: {app}
Source: IssProcLanguage.ini; DestDir: {app}
;------

Source: "Cracklock.settings.default"; DestDir: "{app}"; DestName:"Cracklock.settings"; Permissions: authusers-modify; Components: main
Source: "..\Release\Bin\MCL.exe"; DestDir: "{app}\Bin"; Components: main
Source: "..\Release\Bin\CLINJECT.exe"; DestDir: "{app}\Bin"; Components: main
Source: "..\Release\Bin\CLSHEX.dll"; DestDir: "{app}\Bin"; Flags: restartreplace; Components: main
Source: "..\Release\Bin\CLMNGR.exe"; DestDir: "{app}\Bin"; Components: main
Source: "..\Release\Bin\CLKERN.DLL"; DestDir: "{app}\Bin"; Components: main
Source: "Install-message.rtf"; DestDir: "{app}";  Components: main
Source: "..\translation\lingobit\Output\*.*"; DestDir: "{app}\Languages"; Flags: restartreplace; Components: main
;Source: "..\Release\Languages\*.*"; DestDir: "{app}\Languages"; Components: main
;Source: "..\Release\Bin\CLKERN.DLL"; DestDir: "{sys}"; Components: "system\copydll"

Source: "..\VCClock\Release\VCClock.exe"; DestDir: "{app}\Examples\"; Components: Examples
Source: "..\CSharpClock\bin\Release\CSharpClock.exe"; DestDir: "{app}\Examples\"; Components: Examples
Source: "..\VBClock\VBClock.exe"; DestDir: "{app}\Examples\"; Components: Examples
Source: "..\xmldoc\Cracklock-en.chm"; DestDir: "{app}\Help"; Components: Help
Source: "..\olddoc\*.chm"; DestDir: "{app}\Help"; Components: Help



