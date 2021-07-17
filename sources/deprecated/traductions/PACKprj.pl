#!d:/bin/prog/perl/5.005/bin;
$param = pop(@ARGV);
    
if( $param eq "" )
{
  $param = "*.prj";
}
@matchedfiles = grep { -f "$_" } glob($param);

foreach $fnprj ( @matchedfiles )
{
  print "---- $fnprj\n";
  open(PRJ, $fnprj) or die "\n(ERREUR) Impossible d'ouvrir le fichier $fnprj\n";
  @contenu =  <PRJ>;
  close(PRJ);
    
  open(DST, ">$fnprj") or die "\n(ERREUR) Impossible de cr‚er le fichier $fnprj\n";
  foreach $ligne (@contenu)
  {               
    # supprime le chemin d'accŠs au fichier
    $ligne =~ s/.*\\([^\\]*)/$1/;
    #print "$ligne";
    print DST $ligne;
  }  
  close DST;

  $fnprj =~ s/(.*).prj/$1/;     # conserve la bas du nom du fichier 
  unlink "pack\\$fnprj.zip";
  system("pkzip pack\\$fnprj.zip $fnprj.prj $fnprj.tok clres.mpj clres.mtk >nul") == 0    
      or die "\n(ERREUR) La commande suivante a ‚chou‚e :\npkzip pack\\$base.zip $base.prj $base.tok clres.mpj clres.mtk >nul\n";
  
}

print "Package termin‚ !\n";
