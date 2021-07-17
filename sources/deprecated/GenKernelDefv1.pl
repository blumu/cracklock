#!c:/bin/prog/perl/5.005/bin;

#/////////  Fonctions craquÚes /////////
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

#//////////////////////////////////////	

print
q#Création du fichier de définiton de CLKERN.DLL
==============================================
  #;


# détermine le chemin d'accès à kernel32.dll à partir des paramètres
$kernelDLL = pop(@ARGV);  
$kernelDLL = "%SystemRoot%\\system32\\kernel32.dll" if( $param eq "" );

# exÚcute dumpbin pour gÚnÚrer la liste des fonctions exportÚes par kernel32
system "dumpbin /exports $kernelDLL /OUT:exp.tmp >nul";

# lit le fichier généré par dumpbin
open(DUMPF, "exp.tmp");
@contenu = <DUMPF>;
close(DUMPF);
#unlink "C:\\DEV\\VisualC\\Cracklock\\Kernel\\exp.tmp";

# crée le fichier de définition
open(FICHIERDEF, ">kernel.def") or die "\n(ERREUR) Impossible de créer le fichier kernel.tef\n";

print FICHIERDEF
qq#LIBRARY\t\t\tCLKERN
;DESCRIPTION\t\t'Cracklock Kernel'
EXPORTS
\tCracklockInit
\tCracklockDeinit
	
#;

$nbredirs = 0;
$nbintercepts = 0;
$import = -2;
foreach $ln (@contenu)
{
  ($ordinal, $hint, $rva, $name) = ( $ln =~ / *([^ ]*) *([^ ]*) *([^ ]*) *([^\n ]*)/ ) ;
  $name = $rva  if( $name =~ /\(forwarded/ );
  
  if( $ln =~ /([^ ]*) number of functions/ )
  {
    $nbfunc = $1;
  }  
  elsif( ($import == -2 and $ordinal eq "ordinal" and  $hint eq "hint" and  $rva eq "RVA" and  $name eq "name" )
    or ($import == -1 ) )
  {
    $import++;    
  }
  elsif ($import >= 0 && $import < $nbfunc  )
  {
    $import++;
    if( grep(/^$name$/, @FONCTIONS_CRAQUEES) > 0 )
    {
    	$nbintercepts++;
    	print FICHIERDEF "\t$name = ".$name."CRK\n";
    }
    else
    {
    	$nbredirs++;
    	print FICHIERDEF "\t$name = KERNEL32.$name\n";
    }
  }
}
close FICHIERDEF;

# affiche le résumé 

print
qq#Le fichier kernel.def a été créé !

$nbfunc fonctions seront exportées par CLKERN.DLL parmi lesquelles :
    $nbredirs seront redirigées vers KERNEL32.DLL
    $nbintercepts seront implémentées dans CLKERN.DLL
 #
