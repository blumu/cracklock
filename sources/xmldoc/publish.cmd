@call ..\config.cmd


cp -vpuf * %LOCALWWW%/software/cracklock/xmldoc/
winscp.com william@www.famille-blum.org /command "option exclude docbook-xsl-1.73.2" "synchronize remote . %WWWROOT%/software/cracklock/xmldoc" exit