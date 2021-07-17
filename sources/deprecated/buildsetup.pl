#!/usr/bin/perl;

use Cwd;
require('common.pl');
require('gen_chm.pl');
require('gen_setup.pl');


############
# Programme principal
#
{
	# affiche les infos du programme
	print "Pr‚paration du package de Cracklock - Version $VERSION\n";
	print "(c) William Blum 1998-2005\n\n";

	# génère le fichier script d'installation (ISS)
	print "   * G‚n‚ration du script d'installation\n";
	gen_setup_script() or goto err;

	# génération du programme d'installation (SETUP.EXE)
	gen_setup_exe() or goto err;

	# compression des fichier finaux (SETUP.EXE+FILE_ID.DIZ -> APP.ZIP)
	compression_finale() or goto err;
	
	# message de fin
	msg_fin();
	
	goto fin;
	
err:
	msg_err();
	
fin:
}

###########
# Compression en un fichier zip
#       
sub compression_finale
{
	print "\n   * Compression du ficher cklkxx.exe\n";

	my $orig_dir = Cwd::abs_path;
	chdir($OUTPUT_SETUP);
		launch("$SEVENZIP -tzip a $SETUPFILENAME.zip *.*", true);
	chdir($orig_dir);
        
	launch("copy $OUTPUT_SETUP\\$SETUPFILENAME.zip $WWWSITE", true);

1;
}


############
# message de fin
#
sub msg_fin
{
	print "\nLe package est termin‚ !\n\n";
}


############
# message de fin avec erreur
#
sub msg_err
{
	die "\nLe package n'est pas termin‚ !\n\n";
}

