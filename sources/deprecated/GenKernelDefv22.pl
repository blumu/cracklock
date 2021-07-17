#!/bin/perl

# Génération automatique du fichier kernel.def
#
# auteur : William BLUM
# modifé le 5 novembre 2005
#

#/////////  Fonctions à intercepter /////////
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
q#Création du fichier de définiton de CLKERN.DLL
==============================================
  #;


# récupère le chemin d'accès à kernel32.dll (ou à kernel32.lib) fourni en paramètre
	$kernelDLL = pop(@ARGV);
	$kernelDLL = $ENV{'SYSTEMROOT'}.q#\\system32\\kernel32.dll# if( $kernelDLL eq "" );

# exécute PEDUMP pour générer la liste des fonctions exportées par kernel32
    print "Lecture du fichier a hooker (dll ou lib): $kernelDLL\n";
	chdir "../tools";
	system qq§"pedump.exe" /EXP "$kernelDLL" >"../kernel/exp.tmp"§; # or die "Impossible d'exécuter pedump.exe";
	chdir  "../kernel";

# lit le fichier généré par dumpbin
	open(DUMPF, "exp.tmp");
	@contenu = <DUMPF>;
	close(DUMPF);
	#unlink "exp.tmp";

# crée le fichier de définition
    print "  Creation du fichier de definitions kernel.def\n";
	open(FICHIERDEF, ">kernel.def") or die "\n(ERREUR) Impossible de créer le fichier kernel.tef\n";

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

			# suppression du caractère fin de ligne
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
# affiche le résumé de l'opération

print
qq#
Generation terminee, $symboles symboles trouves dont:
  $exclus symboles exclus
  $nbredirs redirigées par CLKERN.DLL vers KERNEL32.DLL
  $nbintercepts implémentées dans CLKERN.DLL
 #
