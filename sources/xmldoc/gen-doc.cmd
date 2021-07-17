@call ..\config.cmd


@goto htmlhelp

:htmlhelp
:: "D:\setupless_prog\docbook-xsl-1.73.2\htmlhelp\htmlhelp.xsl"
::%SAXON% -o:cracklock-doc.chm cracklock-doc.xml "%DOCBOOK-XSL%\htmlhelp\htmlhelp.xsl"
%XSLTPROC% --xinclude --stringparam html.stylesheet doc.css -o cracklock-doc.chm  "%DOCBOOK-XSL%\htmlhelp\profile-htmlhelp.xsl" cracklock-doc.xml
::%MSXSL% cracklock-doc.xml "%DOCBOOK-XSL%\htmlhelp\profile-htmlhelp.xsl" html.stylesheet=doc.css -o cracklock-doc-web.chm


%HHC% htmlhelp.hhp
mv htmlhelp.chm Cracklock-en.chm
::@goto exit

:website-xhtml
::%SAXON% -xi:off -o:cracklock-doc-web.html cracklock-doc.xml website-template.xsl
%XSLTPROC% --xinclude -o cracklock-doc-web.html  website-template.xsl cracklock-doc.xml
::%MSXSL% cracklock-doc.xml website-template.xsl -o cracklock-doc-web.html
@goto exit



:simple-html
::%SAXON% -xi:off -o:cracklock-doc.html cracklock-doc.xml "%DOCBOOK-XSL%\html\docbook.xsl"
%XSLTPROC% --xinclude -o cracklock-doc.html  "%DOCBOOK-XSL%\html\docbook.xsl" cracklock-doc.xml 
@goto exit

:exit