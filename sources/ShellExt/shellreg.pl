# Le script perl suivant ajoute l'enregistrement suivant dans la base de registre:
#
#
# REGEDIT4
# [HKEY_CLASSES_ROOT\CLSID\{6EF84290-174B-11d1-B524-0080C8141490}\InProcServer32]
# @="c:\\documents\\Cracklock\\Release\\Bin\\CLSHEX.DLL"
# "ThreadingModel"="Apartment"
#

  use Win32;
	use Win32::TieRegistry ( Delimiter=>"/", ArrayValues=>1 );
	$Registry->Delimiter("/");                  # Set delimiter to "/".
	
	if( scalar(@ARGV) > 0) {
		$fichier_dll = pop(@ARGV); } 
	else {
		$fichier_dll = "..\\Release\\Bin\\CLSHEX.DLL"; }
		
	$fichier_dll  = Win32::GetFullPathName($fichier_dll );
	
	print("Enregistrement du fichier \"$fichier_dll\" dans la base de registre\n");
	
  $shellext = $Registry->{ "HKEY_CLASSES_ROOT/CLSID/" };
  
  $shellext->{"{6EF84290-174B-11d1-B524-0080C8141490}/"} = {
    	"/" => "MicroBest Cracklock",
    	"InProcServer32/" => {
            "/" => $fichier_dll,
       			"/ThreadingModel" => "Apartment"
      }
    };

