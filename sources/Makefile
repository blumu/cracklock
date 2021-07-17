# This file must be processed with cygwin's GNU make.
#
# Copyright William Blum
# Version 2.2

# Dernières modifications:
#   9 march 2008 : use rsync instead of cp/winscp/scp
#	1 aout 2007 : changed from multilizer to lingobit lozalizer
#	5 novembre 2005 :

# REQUIREMENT:
#  	- Cygwin's version of the GNU  utilities for win32 http://www.cygwin.com/)  including: 
#		make, rsync, sh, sed, date, cp, mv, rm, bc
#	The cygwing\bin directory must be in the PATH environment variable.
#
# 	NOTES:
#		- the GnuWin32 distribution(http://gnuwin32.sourceforge.net/packages.html) 
#		does not work for this makefile because the make utility is based on the windows command shell instead of the bash command shell.
#		- Do not use the deprecated UnxUtil package (http://unxutils.sourceforge.net/): the version of sed is outdated and produces errors with this Makefile.
#		- GNU make for win32 also available from http://www.mingw.org/ (mingw32-make.exe) has not been tested.

#	- MASM32 (http://www.masm32.com/)
#	- Visual C++ 2005 Express Edition (http://msdn.microsoft.com/vstudio/express/visualc/default.aspx)
#	- MSBuild (included in the .Net framework)
#	- Microsoft Server 2003 Platform SDK (http://www.microsoft.com/downloads/details.aspx?FamilyId=A55B6B43-E24F-4EA3-A93E-40C0EC4F68E5&displaylang=en)
#	- Microsoft HTML Help Workshop (http://www.microsoft.com/downloads/details.aspx?FamilyID=00535334-c8a6-452f-9aa0-d597d16580cc&displaylang=en)
#	- Inno Setup (version >5.1.5) (http://www.jrsoftware.org/isinfo.php)
#	- 7Zip (http://www.7-zip.org/)
#
#  OPTIONAL:
#	Lingobit Localizer (to regenerate the translated resources)   (Multilizer is not used anymore)
#	pscp (from putty)
#	winscp
#	msxsl (Microsoft XSLT parser) for the generation of the FAQ html files
#	XSLTproc for generating the documentation and changelog (in the xmldoc dir)

# the config file contains the path to the different tools
include config



##########
# Variables

# Pattern utilises pour extraire les infos des fichiers version2.h et package_version.h2
MAJOR_PATTERN = s/\#define\s*MAJOR_VERSION\s*\(.*\)/\1/p
MINOR_PATTERN = s/\#define\s*MINOR_VERSION\s*\(.*\)/\1/p
BUILD_PATTERN = s/\#define\s*BUILD\s*\(.*\)/\1/p

# Obtient le numero de version de Cracklock a partir du fichier version.h
MAJOR_VERSION := $(shell sed -n '$(MAJOR_PATTERN)' version.h2 )
MINOR_VERSION := $(shell sed -n '$(MINOR_PATTERN)' version.h2 )
BUILD := $(shell sed -n '$(BUILD_PATTERN)' version.h2 )
APP_VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(BUILD)

# Nom du fichier d'installation
SETUPFILENAME = Cracklock.$(APP_VERSION)

# Numero de version sans les points (xyz au lieu de x.y.z)
APP_VERSION_FLAT = $(shell echo $(APP_VERSION)| sed 's/\.//g')

# Cracklock avec le numero de version
APP_VERNAME = Cracklock $(APP_VERSION)

# Chaine de copyright
PATTERN_CR = s/\#define[ \t]*COPYRIGHT[ \t]*\"\(.*\)\"/\1/p
APP_COPYRIGHT = $(shell sed -n '$(PATTERN_CR)' version.h )

# Nom du fichier zip pour l'archivage des sources de Cracklock
ARCHIVENAME = cracklock_$(APP_VERSION_FLAT)_sources_$(shell date.exe +%d%b%y).zip

#  Version du dernier package produit
PACKAGE_MAJOR_VERSION = $(shell sed -n '$(MAJOR_PATTERN)' package_version.h2 )
PACKAGE_MINOR_VERSION = $(shell sed -n '$(MINOR_PATTERN)' package_version.h2 )
PACKAGE_BUILD = $(shell sed -n '$(BUILD_PATTERN)' package_version.h2 )
PACKAGE_APP_VERSION = $(PACKAGE_MAJOR_VERSION).$(PACKAGE_MINOR_VERSION).$(PACKAGE_BUILD)
PACKAGE_SETUPFILENAME = Cracklock.$(PACKAGE_APP_VERSION)
PACKAGE_APP_VERNAME = Cracklock $(PACKAGE_APP_VERSION)

CYGPATH_LOCALWWW_CRACKLOCK = $(shell cygpath "$(LOCALWWW)$(CRACKLOCK_DIR)")

RSYNC_PARAM =--rsync-path=~/bin/rsync --chmod=ugo=r

# Langues des fichiers d'aide a compiler en .CHM
HELP_LANGUAGES =  french english spanish arabic

#### Chemin d'access aux repertoires
# stockage des archives
ARCHIVES_REP = archives
# fichiers d'aide a compiler
HELPDIR = olddoc
# output de innosetup
OUTPUT_SETUP = Installation/Output


.PHONY: all release shellext kernel manager injector loader resources testunicode lpublish publish faq

all: release
	
	
##########
# Release build and clean release build


# Build the release executables
#release: shellext kernel manager injector loader resources testunicode
release:
	$(MSBUILD) Cracklock.sln /t:Build /p:Configuration=Release
# increment the build number


#clean: clean_shellext clean_kernel clean_manager  clean_injector clean_loader clean_resources clean_testunicode
clean:
	$(MSBUILD) Cracklock.sln /t:Clean /p:Configuration=Release
	$(MSBUILD) Cracklock.sln /t:Clean /p:Configuration=Debug

manager:
	cd Manager && $(MAKE) Release

kernel:
	cd Kernel && $(MAKE) Release

injector:
	cd Injector && $(MAKE) Release

loader:
	cd loader && $(MAKE) Release

resources:
	cd Resources && $(MAKE) Release

shellext:
	cd shellext && $(MAKE) Release

testunicode:
	cd testunicode && $(MAKE) Release

clean_manager:
	cd Manager && $(MAKE) clean_Release

clean_kernel:
	cd Kernel && $(MAKE) clean_Release

clean_injector:
	cd Injector && $(MAKE) clean_Release

clean_loader:
	cd loader && $(MAKE) clean_Release

clean_resources:
	cd Resources && $(MAKE) clean_Release

clean_shellext:
	cd shellext && $(MAKE) clean_Release

clean_testunicode:
	cd testunicode && $(MAKE) clean_Release


.PHONY: archive translation help setup

##########
# Archivage des sources de Cracklock

archive:
	@echo Archivage des sources de Cracklock dans $(ARCHIVENAME)
	$(RM) $(ARCHIVES_REP)/$(ARCHIVENAME)
	$(SEVENZIP) -tzip a $(ARCHIVES_REP)/$(ARCHIVENAME) -x!Cracklockcache.dat -x!xmldoc/docbook-xsl-* -x!archives -x!deprecated -xr!Installation/output -xr!translation/output -xr!*.pch -xr!*.sbr -xr!*.obj -xr!*.ilk -xr!*.ncb -xr!*.pdb -xr!*.idb -xr!*.res -xr!*.chm -xr!Debug/ -xr!Release/
	$(PSCP) $(ARCHIVES_REP)/$(ARCHIVENAME) $(SERVER):$(BACKUP_DIR)

##########
# Génération des resources traduites

#translation:
#	$(MULTILIZER) build Translation/cracklock.mpr

translation:
	@echo    * Compilation des traducations
	rm -rf translation/lingobit/Output/*
	pushd translation/lingobit &&  $(LINGOBITLOCALIZER) cracklock.loc -scan && $(LINGOBITLOCALIZER) cracklock.loc -create:all && popd
	@(for f in translation/lingobit/output/* ; \
	do \
		if [ $${f##*.} = "dll" ]; then \
		echo Keeping $$f; \
		else \
		echo Renaming $$f "->" $${f/CLRESUS./CLRES_}.dll; \
		mv $$f $${f/CLRESUS./CLRES_}.dll; \
		fi \
    done; )
	

##########
# Génération des fichiers d'aide (.chm)

## Comme HHC retourne toujours un code d'erreur, cela interrompt le processus
## de make. C'est pour eviter cela, que `|| date` a ete rajoute a la fin de la commande qui suit.
help:
	@echo "    * Compilation des fichier d'aide (.chm)"
	@(for f in $(foreach lang, $(HELP_LANGUAGES), $(wildcard $(HELPDIR)/$(lang)/*.hhp) ) ; \
	do \
        echo "      - $$f"; \
        $(HHC) $$f; \
    done; ) || date

##########
# Génération du setup avec Innosetup

setup:
	@echo "    * Génération du setup de Cracklock $(APP_VERSION)"
	@echo "      - génération du script ISS"
	@sed -e 's/AppVersion=.*/AppVersion=${APP_VERSION}/' \
		-e 's/VersionInfoVersion=.*/VersionInfoVersion=${APP_VERSION}/' \
		-e 's/VersionInfoTextVersion=.*/VersionInfoTextVersion=${APP_VERSION}/' \
		-e 's/AppVerName=.*/AppVerName=$(APP_VERNAME)/' \
		-e 's/AppCopyright=.*/AppCopyright=$(APP_COPYRIGHT)/' \
		-e 's/OutputBaseFilename=.*/OutputBaseFilename="$(SETUPFILENAME)"/' \
		Installation/bldiss.iss > Installation/~tempiss.iss
	@echo "      - génération du programme d'installation"
	@$(INNOCOMPILE) "Installation\\~tempiss.iss"
	@echo "      - détruit le script inno temporaire"
	@$(RM) Installation/~tempiss.iss
	echo "Le package de $(APP_VERNAME) est terminé !"


faq:
	cd faq && $(MSXSL) cracklockfaq.xml faq-html-en.xsl >cracklockfaq.html
	cd faq && $(MSXSL) cracklockfaq.xml faq-website-html-en.xsl >cracklock-webfaq.html

# publish the files to the LOCAL copy of the website
lpublish: changelog wwwdoc 
	echo '<?php \$$lastest_version = "${PACKAGE_APP_VERSION}";' >$(LOCALWWW)$(CRACKLOCK_DIR)latest.php
	echo '\$$lastest_setupfile = "${PACKAGE_SETUPFILENAME}.exe";?>' >>$(LOCALWWW)$(CRACKLOCK_DIR)latest.php
	echo '"${PACKAGE_SETUPFILENAME}.exe",,"$(shell date +"%d %B %Y")","${PACKAGE_APP_VERNAME}"'> $(LOCALWWW)$(CRACKLOCK_DIR)filesdescr.tmp
	cat $(LOCALWWW)$(CRACKLOCK_DIR)filesdescr.csv >> $(LOCALWWW)$(CRACKLOCK_DIR)filesdescr.tmp
	mv $(LOCALWWW)$(CRACKLOCK_DIR)filesdescr.tmp $(LOCALWWW)$(CRACKLOCK_DIR)filesdescr.csv	
	rsync -v cracklock-pad.xml faq/faqfond.jpg faq/faq.css faq/cracklock-webfaq.html $(CYGPATH_LOCALWWW_CRACKLOCK)
	rsync -v "$(OUTPUT_SETUP)/$(PACKAGE_SETUPFILENAME).exe" $(CYGPATH_LOCALWWW_CRACKLOCK)/downloads
	rsync -v xmldoc/* $(CYGPATH_LOCALWWW_CRACKLOCK)xmldoc/

# publish the files to the website
publish: lpublish publish_rsync


# putty-based publish
publish_putty:
	pscp cracklock-pad.xml faq/faqfond.jpg faq/faq.css faq/cracklock-webfaq.html $(CYGPATH_LOCALWWW_CRACKLOCK)filesdescr.csv $(CYGPATH_LOCALWWW_CRACKLOCK)latest.php $(WWW)$(CRACKLOCK_DIR)
	pscp "$(OUTPUT_SETUP)/$(PACKAGE_SETUPFILENAME).exe" $(WWW)$(CRACKLOCK_DIR)downloads/
	winscp.com $(SERVER) /command "synchronize remote help $(WWWROOT)$(CRACKLOCK_DIR)help/" exit
	winscp.com $(SERVER) /command "option exclude docbook-xsl-1.73.2" "synchronize remote xmldoc $(WWWROOT)$(CRACKLOCK_DIR)xmldoc/" exit

# rsync-based publishing
publish_rsync:
	rsync -v $(RSYNC_PARAM) cracklock-pad.xml faq/faqfond.jpg faq/faq.css faq/cracklock-webfaq.html $(CYGPATH_LOCALWWW_CRACKLOCK)filesdescr.csv $(CYGPATH_LOCALWWW_CRACKLOCK)latest.php $(WWW)$(CRACKLOCK_DIR)
	rsync -v $(RSYNC_PARAM) "$(OUTPUT_SETUP)/$(PACKAGE_SETUPFILENAME).exe" $(WWW)$(CRACKLOCK_DIR)downloads/
	rsync -vr $(RSYNC_PARAM) olddoc/ $(WWW)$(CRACKLOCK_DIR)help/
	rsync -v $(RSYNC_PARAM) xmldoc/* $(WWW)$(CRACKLOCK_DIR)xmldoc/


# scp-based publishing
publish_scp: 
	scp cracklock-pad.xml faq/faqfond.jpg faq/faq.css faq/cracklock-webfaq.html $(CYGPATH_LOCALWWW_CRACKLOCK)filesdescr.csv $(CYGPATH_LOCALWWW_CRACKLOCK)latest.php $(WWW)$(CRACKLOCK_DIR)
	scp "$(OUTPUT_SETUP)/$(PACKAGE_SETUPFILENAME).exe" $(WWW)$(CRACKLOCK_DIR)downloads/
	scp -r olddoc/ $(WWW)$(CRACKLOCK_DIR)help/
	scp xmldoc/* $(WWW)$(CRACKLOCK_DIR)xmldoc/

	
xmldoc/changelog-web.html: xmldoc/website-template.xsl xmldoc/changelog.xml
	$(XSLTPROC) --xinclude -o xmldoc/changelog-web.html xmldoc/website-template.xsl xmldoc/changelog.xml

xmldoc/htmlhelp.hhp: xmldoc/cracklock-doc.xml
	$(XSLTPROC) --xinclude --stringparam html.stylesheet xmldoc/doc.css -o xmldoc/cracklock-doc-web.chm  "$(DOCBOOK-XSL)/htmlhelp/profile-htmlhelp.xsl" xmldoc/cracklock-doc.xml;

xmldoc/Cracklock-en.chm: xmldoc/htmlhelp.hhp
	$(HHC) xmldoc/htmlhelp.hhp || date
	mv xmldoc/htmlhelp.chm xmldoc/Cracklock-en.chm
	
xmldoc/cracklock-doc-web.html:xmldoc/website-template.xsl xmldoc/cracklock-doc.xml
	$(XSLTPROC) --xinclude -o xmldoc/cracklock-doc-web.html xmldoc/website-template.xsl xmldoc/cracklock-doc.xml;

	
changelog: xmldoc/changelog-web.html 
	
chmdoc: xmldoc/Cracklock-en.chm

wwwdoc: xmldoc/cracklock-doc-web.html
	
	
# increments the build number
incr_build_numer:
	cp version.h2 package_version.h2 
	echo '#define MAJOR_VERSION		$(MAJOR_VERSION)' >version.h2
	echo '#define MINOR_VERSION		$(MINOR_VERSION)' >>version.h2
	echo '#define BUILD				$(shell echo 1+$(BUILD)|bc)' >>version.h2
	echo ''>>version.h2

	
package: chmdoc release translation help setup incr_build_numer 
