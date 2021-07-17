$APP_VERSION = 'x.y.z';
$SETUPFILENAME = "cklk$APP_VERSION";  
$SETUPFILENAME =~ s/\.//g;
print qq#"$SETUPFILENAME"#;

open(file, 'version.h');
foreach $buff (<file>)
{
	if( $buff =~ /#define[ \t]*APP_VERSION[ \t]*"(.*)"/ )
	{
		$version = $1;
		break;
	}
}
close(file);
print "-$version-";

