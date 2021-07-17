#!/usr/bin/perl;


############
# Génération des fichiers .chm
#
sub gen_chm
{
	my $HHC = pop();
	my $HELPDIR = pop();


	opendir(DIR, $HELPDIR) || print "\nImpossible d'ouvrir le r,pertoire $HELPDIR: $!\n";
		@subreps = grep { -d "$HELPDIR\\$_" }  readdir(DIR) ;
	closedir(DIR);

	foreach $subrep ( @subreps )
	{
		opendir(DIR, "$HELPDIR\\$subrep") || print "\nImpossible d'ouvrir le r,pertoire $HELPDIR\\$subrep: $!\n";
			@fichiers = grep { -f "$HELPDIR\\$subrep\\$_" } readdir(DIR);
			@fichiers = grep( /.*\.hhp/, @fichiers);
			foreach $helpprj ( @fichiers )
			{
			  print "    - ficher $helpprj\n";
			  launch(qq#"$HHC" $HELPDIR\\$subrep\\$helpprj#, false);
			}
		closedir(DIR);
	}
1;
}

1;