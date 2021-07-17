#!/usr/bin/perl;

use Cwd;
require('common.pl');

my ($sec,$min,$hour,$mday,$mon,$year, $wday,$yday,$isdst) = localtime time;
@months = ("jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec");
$curdate = $mday.$months[$mon].($year+1900);

$archivefile="$ARCHIVES_REP\\cracklock_".$APP_VERSION_FLAT."_sources_$curdate.zip";


############
# Programme principal
#
{
	# affiche les infos du programme
	print "Archivage des sources de Cracklock - Version $VERSION\n";
	print "(c) William Blum 1998-2005\n\n";

	# compression des sources
	compression() or goto err;

	launch("pscp.exe $archivefile william\@www.famille-blum.org:", TRUE);

	# message de fin
	msg_fin();
	
	goto fin;
	
err:
	msg_err();
	
fin:
}



###########
# Compression des sources en un fichier zip
#       
sub compression
{
	print "\n   Compression des sources:\n";

	unlink($archivefile);
	launch("$SEVENZIP -tzip a $archivefile -x!archives -x!deprecated -x!Installation\output -xr!*.pch -xr!*.sbr -xr!*.obj -xr!*.ilk -xr!*.ncb -xr!*.pdb -xr!*.idb -xr!*.res -xr!*.chm -xr!Debug\\ -xr!Release\\ -xr!Installation\\Output", true);
        
1;
}


############
# message de fin
#
sub msg_fin
{
	print "\nL'archivage des sources est termin‚ !\n\n";
}


############
# message de fin avec erreur
#
sub msg_err
{
	die "\nL'archivage des sources n'est pas termin‚ !\n\n";
}


