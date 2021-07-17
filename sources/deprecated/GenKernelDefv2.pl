#!c:/bin/prog/perl/5.005/bin;

# G‚n‚ration automatique du fichier kernel.def
#
# auteur : William BLUM
# modif‚ le 13 septembre 2002
#

#/////////  Fonctions … intercepter /////////
	@FONCTIONS_CRAQUEES = (
		"GetTimeZoneInformation",
		"GetLocalTime",
		"GetProcAddress",
		"GetSystemTime",
		"GetSystemTimeAsFileTime",
		"LoadLibraryA",
		"LoadLibraryExA",
		"LoadLibraryExW",
		"LoadLibraryW",
		"FreeLibrary",
		"FreeLibraryAndExitThread",
		"GetModuleHandleA",
		"GetModuleHandleW",
		"GetModuleFileNameA",
		"GetModuleFileNameW");
		
		
	@FONCTIONS_ABSENTES_DE_KERNEL32_DLL = (
		"AddConsoleAliasA",
		"AddConsoleAliasW",
		"BaseAttachCompleteThunk",
		"CloseConsoleHandle",
		"CloseProfileUserMapping",
		"CmdBatNotification",
		"ConsoleMenuControl",
		"CreateProcessInternalA",
		"CreateProcessInternalW",
		"CreateVirtualBuffer",
		"DelayLoadFailureHook",
		"DosPathToSessionPathA",
		"DosPathToSessionPathW",
		"DuplicateConsoleHandle",
		"ExitVDM",
		"ExpungeConsoleCommandHistoryA",
		"ExpungeConsoleCommandHistoryW",
		"ExtendVirtualBuffer",
		"FreeVirtualBuffer",
		"GetConsoleAliasA",
		"GetConsoleAliasExesA",
		"GetConsoleAliasExesLengthA",
		"GetConsoleAliasExesLengthW",
		"GetConsoleAliasExesW",
		"GetConsoleAliasW",
		"GetConsoleAliasesA",
		"GetConsoleAliasesLengthA",
		"GetConsoleAliasesLengthW",
		"GetConsoleAliasesW",
		"GetConsoleCharType",
		"GetConsoleCommandHistoryA",
		"GetConsoleCommandHistoryLengthA",
		"GetConsoleCommandHistoryLengthW",
		"GetConsoleCommandHistoryW",
		"GetConsoleCursorMode",
		"GetConsoleFontInfo",
		"GetConsoleHardwareState",
		"GetConsoleInputExeNameA",
		"GetConsoleInputExeNameW",
		"GetConsoleInputWaitHandle",
		"GetConsoleKeyboardLayoutNameA",
		"GetConsoleKeyboardLayoutNameW",
		"GetConsoleNlsMode",
		"GetDefaultSortkeySize",
		"GetLinguistLangSize",
		"GetNextVDMCommand",
		"GetNlsSectionName",
		"GetNumberOfConsoleFonts",
		"GetVDMCurrentDirectories",
		"HeapCreateTagsW",
		"HeapExtend",
		"HeapQueryTagW",
		"HeapSummary",
		"HeapUsage",
		"InvalidateConsoleDIBits",
		"NlsConvertIntegerToString",
		"NlsGetCacheUpdateCount",
		"NlsResetProcessLocale",
		"OpenConsoleW",
		"OpenDataFile",
		"OpenProfileUserMapping",
		"PrivCopyFileExW",
		"PrivMoveFileIdentityW",
		"QueryWin31IniFilesMappedToRegistry",
		"ReadConsoleInputExA",
		"ReadConsoleInputExW",
		"RegisterConsoleIME",
		"RegisterConsoleOS2",
		"RegisterConsoleVDM",
		"RegisterWaitForInputIdle",
		"RegisterWowBaseHandlers",
		"RegisterWowExec",
		"SetCPGlobal",
		"SetConsoleCommandHistoryMode",
		"SetConsoleCursorMode",
		"SetConsoleDisplayMode",
		"SetConsoleFont",
		"SetConsoleHardwareState",
		"SetConsoleIcon",
		"SetConsoleInputExeNameA",
		"SetConsoleInputExeNameW",
		"SetConsoleKeyShortcuts",
		"SetConsoleLocalEUDC",
		"SetConsoleMaximumWindowSize",
		"SetConsoleMenuClose",
		"SetConsoleNlsMode",
		"SetConsoleNumberOfCommandsA",
		"SetConsoleNumberOfCommandsW",
		"SetConsoleOS2OemFormat",
		"SetConsolePalette",
		"SetLastConsoleEventActive",
		"SetTermsrvAppInstallMode",
		"SetVDMCurrentDirectories",
		"ShowConsoleCursor",
		"TermsrvAppInstallMode",
		"TrimVirtualBuffer",
		"UTRegister",
		"UTUnRegister",
		"UnregisterConsoleIME",
		"VDMConsoleOperation",
		"VDMOperationStarted",
		"ValidateLCType",
		"ValidateLocale",
		"VerifyConsoleIoHandle",
		"VirtualBufferExceptionHandler",
		"WriteConsoleInputVDMA",
		"WriteConsoleInputVDMW");

#//////////////////////////////////////	


print
q#Cr‚ation du fichier de d‚finiton de CLKERN.DLL
==============================================
  #;


# r‚cupŠre le chemin d'accŠs … kernel32.dll (ou … kernel32.lib) fourni en paramŠtre
	$kernelDLL = pop(@ARGV);  
	$kernelDLL = "%SystemRoot%\\system32\\kernel32.dll" if( $kernelDLL eq "" );

# ex‚cute PEDUMP pour g‚n‚rer la liste des fonctions export‚es par kernel32
	chdir "../tools";
	system qqõ"pedump.exe" /EXP '$kernelDLL' >"../kernel/exp.tmp"õ; # or die "Impossible d'ex‚cuter pedump.exe";
	chdir  "../kernel";
	
# lit le fichier g‚n‚r‚ par dumpbin
	open(DUMPF, "exp.tmp");
	@contenu = <DUMPF>;
	close(DUMPF);
	#unlink "exp.tmp";

# cr‚e le fichier de d‚finition
	open(FICHIERDEF, ">kernel.def") or die "\n(ERREUR) Impossible de cr‚er le fichier kernel.tef\n";

	print FICHIERDEF
qq#LIBRARY\t\t\tCLKERN
;DESCRIPTION\t\t'Cracklock Kernel'
EXPORTS
\tCracklockInit
\tCracklockDeinit
	
#;

	$import = 0;
	$nbredirs = 0;
	$nbintercepts = 0;
	$premier = 0;
	foreach $name (@contenu) {
		if( $premier eq 1 ) {
			if( $name =~ /@/ ) {
				$name =~ s/_([^\n@]*)@\d*\n/$1/;
			}
			else {
				$name =~ s/([^\r]*)\r/$1/;
			}
			
			# suppression du caractŠre fin de ligne
			$name =~ s/(\w*)\n/$1/;

			if( grep(/^$name$/, @FONCTIONS_ABSENTES_DE_KERNEL32_DLL) <= 0 ) {
				$import++;
				if( grep(/^$name$/, @FONCTIONS_CRAQUEES) > 0 ) {
					$nbintercepts++;
					print FICHIERDEF "\t$name = ".$name."CRK\n";
				}
				else {
					$nbredirs++;
					print FICHIERDEF "\t$name = KERNEL32.$name\n";
				}
			}
		}
		$premier = 1;
	}
	close FICHIERDEF;


#############
# affiche le r‚sum‚ de l'op‚ration

print
qq#Le fichier kernel.def a ‚t‚ cr‚‚ !

$import fonctions seront export‚es par CLKERN.DLL parmi lesquelles :
    $nbredirs seront redirig‚es vers KERNEL32.DLL
    $nbintercepts seront impl‚ment‚es dans CLKERN.DLL
 #
