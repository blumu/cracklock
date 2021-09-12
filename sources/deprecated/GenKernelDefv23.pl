#!/bin/perl
# G�n�ration automatique des fichiers clkern.def et kernel32.lib a partir de kernel32.dll
#
# auteur : William BLUM
# modif� le 5 aout 2007
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

	@SYMBOLES_A_EXCLURE = ();

#//////////////////////////////////////


print
q#Cr�ation des fichiers de d�finiton pour le hookage de DLL
=========================================================
#;

# base du nom de la dll interceptrice
	$hookDLL = "CLKERN";

# chemin d'acc�s (fourni en param�tre) d'un fichier LIB dont la table d'export AVEC DECORATION (Function@XX) contient 
# la liste des functions � intercepter. Valeur par d�faut: %MSSdk%\lib\kernel32.lib.
	$hookedLibPath = pop(@ARGV);
	$hookedLibPath = $ENV{'MSSDK'}.q#\\lib\\KERNEL32.lib# if( $hookedLibPath eq "" );

# chemin d'acc�s (fourni en param�tre) d'un fichier DLL dont la table d'export SANS DECORATION contient 
# potentiellement d'autres fonctions  exportes qui ne sont pas presentes dans la table d'export de la LIB correspondante a la DLL
# (aussi appellees PRIVATE EXPORTS FUNCTION)
# Valeur par d�faut: c:\windows\system32\kernel32.dll.
	$hookedDllPath = pop(@ARGV);
	$hookedDllPath = $ENV{'SYSTEMROOT'}.q#\\system32\\KERNEL32.dll# if( $hookedDllPath eq "" );

# extrait la base du nom de la dll "hook�e"
	if ($hookedDllPath =~ m/^.+[\\|\/]([^\.]+?)\..*$/ ) { $hookedBasename = $1; }
	else { die "cannot extract filename form filepath '$hookedDllPath'!\n"; }

# ex�cute PEDUMP pour g�n�rer la liste des fonctions export�es par la DLL � hooker
    print "  - Extrait la table d'export du fichier LIB associe a la DLL a hooker ($hookedLibPath)\n";
	system qq�"../tools/pedump.exe" /EXP "$hookedLibPath" >"exp.tmp"�; # or die "Impossible d'ex�cuter pedump.exe";
# lit le fichier g�n�r� par dumpbin
	open(DUMPF, "exp.tmp");
	@explib = <DUMPF>;
	close(DUMPF);
	unlink "exp.tmp";

# ex�cute PEDUMP pour g�n�rer la liste des fonctions export�es par la DLL � hooker
    print "  - Extrait la table d'export de la DLL a hooker ($hookedDllPath)\n";
	system qq�"../tools/pedump.exe" /EXP "$hookedDllPath" >"exp.tmp"�; # or die "Impossible d'ex�cuter pedump.exe";
# lit le fichier g�n�r� par dumpbin
	open(DUMPF, "exp.tmp");
	@expdll = <DUMPF>;
	close(DUMPF);
	unlink "exp.tmp";


	
# cr�e le fichier de d�finition
    print "  - Creation des fichiers de definitions ($hookedBasename.def et $hookDLL.def)\n";
	open(HOOKDEF, ">".$hookDLL.".def") or die "\n(ERREUR) Impossible de cr�er le fichier $hookDLL.def\n";
	print HOOKDEF
qq#LIBRARY\t\t\t$hookDLL
EXPORTS
\tCracklockInit
\tCracklockDeinit

#;

open(HOOKEDDEF, ">".$hookedBasename.".def") or die "\n(ERREUR) Impossible de cr�er le fichier $hookedBasename.def\n";
	print HOOKEDDEF
qq#LIBRARY\t\t\t$hookedBasename
EXPORTS

#;

$symboles = 0;
$nbredirs = 0;
$nbintercepts = 0;
$exclus = 0;

sub write_export_table
{
	local (*explist, *explist2) = @_;
	$premier = 0;
	foreach $name (@explist) {
		if( $premier eq 1 ) 
		{
			# enleve les retours chariots
			$name =~ s/([^\r]*)\r\n/$1/;

			# enleve la decoration
			$undecoratedname = $name;
			if( $name =~ /@/ ) {
				$name =~ s/_(.*)/$1/; # enleve le underscore
				$undecoratedname =~ s/_([^\@]*)\@\d*/$1/;
			}
			
				
			# enleve l'entree correspondante de la liste des functions exportees par le fichier DLL
			@explist2 = grep(!/$undecoratedname\r/, @explist2); 
			
			$symboles++;

			print HOOKEDDEF "\t$name\n";
			if( grep(/^$undecoratedname$/, @SYMBOLES_A_EXCLURE) <= 0 ) {
				if( grep(/^$undecoratedname$/, @FONCTIONS_CRAQUEES) > 0 ) {
					$nbintercepts++;
					print HOOKDEF "\t".$undecoratedname." = ".$undecoratedname."CRK\n";
				}
				else {
					$nbredirs++;
					print HOOKDEF "\t".$undecoratedname." = ".$hookedBasename.".".$undecoratedname."\n";
				}
			}
			else {				
				$exclus++;
			}
		}
		$premier = 1;
	}
}

	write_export_table(\@explib,\@expdll);
	
	# parcours la liste des fonctions presentes dans la table d'export de la DLL et qui ne sont pas presentes dans la table d'export de la LIB
	print HOOKEDDEF "\n\n\n ; PRIVATE EXPORTS: fonctions exportees par la DLL mais pas par la LIB originale\n";
	print HOOKDEF "\n\n\n ; PRIVATE EXPORTS: fonctions exportees par la DLL mais pas par la LIB originale\n";
	write_export_table(\@expdll,\@null);

	
	close HOOKDEF;
	close HOOKEDDEF;

########### Les commandes suivantes visent a reconstruire une bibliotheque d'import pour la DLL hookee contenant les
########### les functions qui etaients originellement declarees comme privees par la DLL. 
########### Cette methode est temporairement abandonnee car elle ne fonctionne pas correctement lorsque la DLL interceptrice
########### utilisent des fonctions de la DLL d'origine (probleme de decoration des noms de fonctions)
# g�n�ration de la biblioth�que d'import de la dll hook�e
	# LIB /machine:x86 "/def:hookedDLL.def" "/out:Release\hookedDLL.lib"
	#print "  - Creation d'une nouvelle biblioth�que d'import ($hookedBasename.lib) pour la dll a hooker\n";
	#system qq#LIB /machine:x86 "/def:$hookedBasename.def" "/out:$hookedBasename.lib"#;

#############
# affiche le r�sum� de l'op�ration

print
qq#
Generation terminee, $symboles symboles trouves dont:
  $nbredirs seront redirig�es de $hookDLL.DLL vers $hookedDllPath
  $nbintercepts seront intercept�es par $hookDLL.DLL
  $exclus symboles exclus
 #
