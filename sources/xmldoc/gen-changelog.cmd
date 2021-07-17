@call ..\config.cmd

:changelog-website-html
::d:\tools\saxon\transform -xi:off -o:changelog-web.html changelog.xml website-template.xsl
%XSLTPROC% --xinclude -o changelog-web.html website-template.xsl changelog.xml
::%MSXSL% changelog.xml website-template.xsl -o changelog-web.html 
@goto exit


:exit 