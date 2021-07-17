# dernière modification 5 novembre 2005
# par william Blun

$VERSION = 1.2;

### Obtient le numero de version de Cracklock a partir du fichier version.h
open(file, 'version.h');
foreach $buff (<file>) {
	if( $buff =~ /#define[ \t]*APP_VERSION[ \t]*"(.*)"/ ) {
		$APP_VERSION = $1;
		break;
	}
}
close(file);
###

# Numero de version sans les points (xyz au lieu de x.y.z)
$APP_VERSION_FLAT = $APP_VERSION;  $APP_VERSION_FLAT =~ s/\.//g;
$SETUPFILENAME = "cklk$APP_VERSION_FLAT";
$HELPDIR = "help";
$HHC = "C:\\Program Files\\HTML Help Workshop\\hhc.exe";

sub launch
{
	my $testerr = pop();
	my $cmd = pop();
	print $cmd."\n";
	$ret = system($cmd);
	if($testerr and ($ret != 0) ) {
		die qq#\n(ERREUR) La commande suivante a ‚chou‚e :\n#.$cmd."\n";
	}
	$ret
}


1;