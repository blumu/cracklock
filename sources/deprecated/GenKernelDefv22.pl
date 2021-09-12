#!/bin/perl

# G�n�ration automatique du fichier kernel.def
#
# auteur : William BLUM
# modif� le 5 novembre 2005
#

#/////////  Fonctions � intercepter /////////
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

    @SYMBOLES_A_EXCLURE = (
        "AddVectoredContinueHandler",
        "DecodePointer",
        "DecodeSystemPointer",
        "EncodePointer",
        "EncodeSystemPointer",
        "EnumSystemFirmwareTables",
        "GetSystemFileCacheSize",
        "GetSystemFirmwareTable",
        "NeedCurrentDirectoryForExePathA",
        "NeedCurrentDirectoryForExePathW",
        "RemoveVectoredContinueHandler",
        "SetSystemFileCacheSize",
        "SetThreadStackGuarantee",
        "Wow64DisableWow64FsRedirection",
        "Wow64EnableWow64FsRedirection",
        "Wow64RevertWow64FsRedirection",
);

#//////////////////////////////////////


print
q#Cr�ation du fichier de d�finiton de CLKERN.DLL
==============================================
  #;


# r�cup�re le chemin d'acc�s � kernel32.dll (ou � kernel32.lib) fourni en param�tre
	$kernelDLL = pop(@ARGV);
	$kernelDLL = $ENV{'SYSTEMROOT'}.q#\\system32\\kernel32.dll# if( $kernelDLL eq "" );

# ex�cute PEDUMP pour g�n�rer la liste des fonctions export�es par kernel32
    print "Lecture du fichier a hooker (dll ou lib): $kernelDLL\n";
	chdir "../tools";
	system qq�"pedump.exe" /EXP "$kernelDLL" >"../kernel/exp.tmp"�; # or die "Impossible d'ex�cuter pedump.exe";
	chdir  "../kernel";

# lit le fichier g�n�r� par dumpbin
	open(DUMPF, "exp.tmp");
	@contenu = <DUMPF>;
	close(DUMPF);
	#unlink "exp.tmp";

# cr�e le fichier de d�finition
    print "  Creation du fichier de definitions kernel.def\n";
	open(FICHIERDEF, ">kernel.def") or die "\n(ERREUR) Impossible de cr�er le fichier kernel.tef\n";

	print FICHIERDEF
qq#LIBRARY\t\t\tCLKERN
;DESCRIPTION\t\t'Cracklock Kernel'
EXPORTS
\tCracklockInit
\tCracklockDeinit

#;

	$symboles = 0;
	$nbredirs = 0;
	$nbintercepts = 0;
	$premier = 0;
	$exclus = 0;
	foreach $name (@contenu) {
		if( $premier eq 1 ) {
			if( $name =~ /@/ ) {
				$name =~ s/_([^\r\@]*)\@\d*\r/$1/;
			}
			else {
				$name =~ s/([^\r]*)\r/$1/;
			}

			# suppression du caract�re fin de ligne
			$name =~ s/(\w*)\n/$1/;

            $symboles++;

			if( grep(/^$name$/, @SYMBOLES_A_EXCLURE) <= 0 ) {
				if( grep(/^$name$/, @FONCTIONS_CRAQUEES) > 0 ) {
					$nbintercepts++;
					print FICHIERDEF "\t$name = ".$name."CRK\n";
				}
				else {
					$nbredirs++;
					print FICHIERDEF "\t$name = KERNEL32.$name\n";
				}
			}
			else
            {
                $exclus++;
			}
		}
		$premier = 1;
	}
	close FICHIERDEF;


#############
# affiche le r�sum� de l'op�ration

print
qq#
Generation terminee, $symboles symboles trouves dont:
  $exclus symboles exclus
  $nbredirs redirig�es par CLKERN.DLL vers KERNEL32.DLL
  $nbintercepts impl�ment�es dans CLKERN.DLL
 #
