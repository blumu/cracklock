#!c:/bin/prog/perl/5.005/bin;

#/////////  Fonctions craqu�es /////////
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
q#Cr�ation du fichier de d�finiton de CLKERN.DLL
==============================================
  #;


# d�termine le chemin d'acc�s � kernel32.dll � partir des param�tres
$kernelDLL = pop(@ARGV);  
$kernelDLL = "%SystemRoot%\\system32\\kernel32.dll" if( $param eq "" );

# ex�cute dumpbin pour g�n�rer la liste des fonctions export�es par kernel32
system "dumpbin /exports $kernelDLL /OUT:exp.tmp >nul";

# lit le fichier g�n�r� par dumpbin
open(DUMPF, "exp.tmp");
@contenu = <DUMPF>;
close(DUMPF);
#unlink "C:\\DEV\\VisualC\\Cracklock\\Kernel\\exp.tmp";

# cr�e le fichier de d�finition
open(FICHIERDEF, ">kernel.def") or die "\n(ERREUR) Impossible de cr�er le fichier kernel.tef\n";

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

# affiche le r�sum� 

print
qq#Le fichier kernel.def a �t� cr�� !

$nbfunc fonctions seront export�es par CLKERN.DLL parmi lesquelles :
    $nbredirs seront redirig�es vers KERNEL32.DLL
    $nbintercepts seront impl�ment�es dans CLKERN.DLL
 #
