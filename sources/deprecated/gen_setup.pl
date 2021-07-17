#!/usr/bin/perl;

require('common.pl');

$APP_VERNAME = 'Cracklock '.$APP_VERSION;
$APP_COPYRIGHT = 'Copyright © 2005 William Blum';

############
# Génération du script (.ISS)
#
sub gen_setup_script
{
	$inc = 0;
	$racine = $HELPDIR;

	open(SRC, '<Installation\\bldiss.iss');
	open(DST, '>Installation\\~tempiss.iss');

		$copie_cette_section = 0;
		@buff = <SRC>;
		foreach $buff (@buff)
		{
			if( $buff =~ /\[(.*)\]/ )
			{
				$section = $1;

				if( $section =~ /^Dirs$/ )
				{
					$copie_cette_section = 1;
				}
				elsif ($section =~ /^Files$/)
				{
					$copie_cette_section = 2;
				}
				else
				{
					print "    - Copie de la section [$1]\n";
					$copie_cette_section = 0;
				}
			}

			if( $copie_cette_section eq 0 )
			{
				$buff =~s/AppVersion=.*/AppVersion=$APP_VERSION/;
				$buff =~s/AppVerName=.*/AppVerName=$APP_VERNAME/;
				$buff =~s/AppCopyright=.*/AppCopyright=$APP_COPYRIGHT/;
				$buff =~s/OutputBaseFilename=.*/OutputBaseFilename=$SETUPFILENAME/;

				print DST $buff;
			}
			elsif( $copie_cette_section eq 1 )
				{ $DIRSBuff = $DIRSBuff.$buff; }
			elsif( $copie_cette_section eq 2 )
				{ $FILESBuff = $FILESBuff.$buff; }
		}

		print "    - Cr‚ation des sections [Dirs] et [Files] : ";
		foreach $rep (@lst_rep)
		{
			$DIRSBuff = $DIRSBuff.qq#Name: "{app}\\Help\\$rep"\n#;

			if( grep { -f "$_" } glob ("$racine$rep\\*.*") )
			{
				$FILESBuff = $FILESBuff.qq#Source: "$racine$rep\\*.*"; DestDir: "{app}\\Help\\$rep"\n#;
			}
		}

		print DST $DIRSBuff;
		print DST $FILESBuff;
		print "\n";

	close(DST);
	close(SRC);
}


