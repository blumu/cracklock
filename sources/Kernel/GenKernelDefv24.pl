#!/bin/perl
# Génération automatique des fichiers clkern.def et kernel32.lib a partir de kernel32.dll
#
# auteur : William BLUM
# modifé le 5 aout 2007
# modifé le 8 mars 2008
#

## Les outils suivant doivent etre installes:
# 	DUMPBIN
## Les variables suivantes peuvent etre configurees avant de lancer ce script:
# 	WindowsSdkDir : chemin vers Microsoft Platform SDK
#   		si cette variable n'existe pas, alors il faut preciser le chemin d'access a KERNEL32.lib a la ligne de commande
## Ligne de commande:
# perl GenKernel.pl  chemin_vers_kernel32.lib chemin_vers_kernel32.dll
# 	Si [chemin_vers_kernel32.dll] n'est pas precise alors le script prend par defaut la dll du repertoire windows\system
# 	Si [chemin_vers_kernel32.lib]  et chemin_vers_kernel32.dll]  ne sont pas precises alors le script prend par defaut la dll du repertoire windows\system
# 	et la lib KERNEL32.lib se trouvant dans le repertoire indique par la variable MSSDK.



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

	@SYMBOLES_A_EXCLURE = ();

#//////////////////////////////////////


# Extrait la table d'export d'une DLL ou d'une LIB.
# (necessite DUMPBIN.EXE fourni dans le MS Platform SDK)
sub read_export_table
{
	local ($filename) = @_;
	
	system (qq#DUMPBIN /EXPORTS "$filename" >"exp.tmp"#); #or die "Impossible d'exécuter dumpbin.exe";

	# lit le fichier généré par dumpbin
	open(DUMBINOUTPUT, "exp.tmp");
	
	my %exporttable = ();
	my $table = 0; # table d'export pas encore rencontree
	my $au_moins_une_entree_lue = 0; # en train de lire la table d'export (au moins une entree lue)
	foreach $line (<DUMBINOUTPUT>) {
		# saute les lignes vides
		if( $line =~ /^\r/ ) {
			# Si on rencontre une ligne vide pendant la lecture de la table d'export alors on a atteint la fin de la table!
			if( $au_moins_une_entree_lue > 0 ) {
				$table = 0;
			}
		}
		# table pas encore commencee?
		elsif( $table == 0 ) {
			if( $line =~ /^\s*ordinal\s*hint\s*RVA\s*name/ ) {
				$table = 1; # 1 pour DLL export table
			}
			elsif( $line =~ /^\s*ordinal\s*name/ ) {
				$table = 2; # 1 pour LIB export table
			}
		}
		# lecture d'une entree de la table d'export
		else
		{
			$au_moins_une_entree_lue = 1;
			
			# enleve les retours chariots
			$line =~ s/([^\r]*)\r\n/$1/;

			# DLL export table?
			if($table == 1 ) {
				if($line =~ /\s*(\d+)\s*\w+ ........ ([^\s]+)/ ) {
					$ordinal = "\@$1";
					$name = $2;
				}
			}
			elsif($table == 2) {
				if($line =~ /\s*([^\s]*)/ ) {
					$ordinal = "";
					$name = $1;
				}
			}
			
			# enleve la decoration
			$undecoratedname = $name;
			if( $name =~ /@/ ) {
				$name =~ s/_(\w+)/$1/; # enleve le underscore
				$undecoratedname =~ s/_([^\@]*)\@\d*/$1/;
			}
			# ajoute l'entree lue
			$exporttable{$undecoratedname} = [$ordinal, $name];	
		}
	}

	close(DUMBINOUTPUT);
	unlink "exp.tmp";
	
	return %exporttable;
}

print
q#Création des fichiers de définiton pour le hookage de DLL
=========================================================
#;

# base du nom de la dll interceptrice
	$hookDLL = "CLKERN";
	
# chemin d'accès (fourni en paramètre) d'un fichier LIB dont la table d'export AVEC DECORATION (Function@XX) contient 
# la liste des functions à intercepter. Valeur par défaut: %MSSdk%\lib\kernel32.lib.
	$hookedLibPath = pop(@ARGV);
	$hookedLibPath = $ENV{'WINDOWSSDKDIR'}.q#\\lib\\KERNEL32.lib# if( $hookedLibPath eq "" );
	#if( not -r $hookedLibPath ) { die "Cannot read the file $hookedLibPath"; }

# chemin d'accès (fourni en paramètre) d'un fichier DLL dont la table d'export SANS DECORATION contient 
# potentiellement d'autres fonctions  exportes qui ne sont pas presentes dans la table d'export de la LIB correspondante a la DLL
# (aussi appellees PRIVATE EXPORTS FUNCTION)
# Valeur par défaut: c:\windows\system32\kernel32.dll.
	$hookedDllPath = pop(@ARGV);
	$hookedDllPath = $ENV{'SYSTEMROOT'}.q#\\system32\\KERNEL32.dll# if( $hookedDllPath eq "" );
	#if( not -r $$hookedDllPath ) { die "Cannot read the file $hookedDllPath"; }

# extrait la base du nom de la dll "hookée"
	if ($hookedDllPath =~ m/^.+[\\|\/]([^\.]+?)\..*$/ ) { $hookedBasename = $1; }
	else { die "cannot extract filename form filepath '$hookedDllPath'!\n"; }

    print "  - Extrait la table d'export du fichier LIB associe a la DLL a hooker ($hookedLibPath)\n";
	%lib_exporttable = read_export_table($hookedLibPath);

    print "  - Extrait la table d'export de la DLL a hooker ($hookedDllPath)\n";
	%dll_exporttable = read_export_table($hookedDllPath);

	
# crée le fichier de définition
    print "  - Creation des fichiers de definitions ($hookedBasename.def et $hookDLL.def)\n";
	open(HOOKDEF, ">".$hookDLL.".def") or die "\n(ERREUR) Impossible de créer le fichier $hookDLL.def\n";
	print HOOKDEF
qq#LIBRARY\t\t\t$hookDLL
EXPORTS
\tCracklockInit
\tCracklockDeinit

#;

open(HOOKEDDEF, ">".$hookedBasename.".def") or die "\n(ERREUR) Impossible de créer le fichier $hookedBasename.def\n";
	print HOOKEDDEF
qq#LIBRARY\t\t\t$hookedBasename
EXPORTS

#;


	$symboles = 0;
	$nbredirs = 0;
	$nbintercepts = 0;
	$exclus = 0;
	$private = 0;
	for $name (keys(%dll_exporttable)) {
		$undecoratedname = $dll_exporttable{$name}[1];
		$ordinal = $dll_exporttable{$name}[0];
		$symboles++;
		
		# est-ce que l'entree est privee? (presente dans la table d'export de la DLL mais pas dans la table d'export de la LIB)
		if( grep { "$_" eq "$name" } keys(%lib_exporttable) ) {
 			$comment = "";
			$prefix = "";
		}		
		else {
			$prefix = ";";
			$comment = '; PRIVATE EXPORTS (fonctions exportees par la DLL mais pas par la LIB originale)';
			$private++;
		}
		
		print HOOKEDDEF "\t$name $ordinal$comment\n";
		if( grep(/^$undecoratedname$/, @SYMBOLES_A_EXCLURE) <= 0 ) {
			if( grep(/^$undecoratedname$/, @FONCTIONS_CRAQUEES) > 0 ) {
				$nbintercepts++;
				print HOOKDEF "\t".$prefix.$undecoratedname." = ".$undecoratedname."CRK ".$ordinal.$comment."\n";
			}
			else {
				$nbredirs++;
				print HOOKDEF "\t".$prefix.$undecoratedname." = ".$hookedBasename.".".$undecoratedname." ".$ordinal.$comment."\n";
			}
		}
		else {
			$exclus++;
		}
	}
	
	close HOOKDEF;
	close HOOKEDDEF;

########### Les commandes suivantes visent a reconstruire une bibliotheque d'import pour la DLL hookee contenant les
########### les functions qui etaients originellement declarees comme privees par la DLL. 
########### Cette methode est temporairement abandonnee car elle ne fonctionne pas correctement lorsque la DLL interceptrice
########### utilisent des fonctions de la DLL d'origine (probleme de decoration des noms de fonctions)
# génération de la bibliothèque d'import de la dll hookée
	# LIB /machine:x86 "/def:hookedDLL.def" "/out:Release\hookedDLL.lib"
	#print "  - Creation d'une nouvelle bibliothèque d'import ($hookedBasename.lib) pour la dll a hooker\n";
	#system qq#LIB /machine:x86 "/def:$hookedBasename.def" "/out:$hookedBasename.lib"#;

#############
# affiche le résumé de l'opération

print
qq#
Generation terminee, $symboles symboles trouves dont:
  $nbredirs seront redirigées de $hookDLL.DLL vers $hookedDllPath
  $nbintercepts seront interceptées par $hookDLL.DLL
  $private private exports seront non redirigées (present dans $hookedDllPath mais pas dans $hookedLibPath )
  $exclus symboles exclus
 #
