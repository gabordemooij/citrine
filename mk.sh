#!/bin/sh
#MK Script
#Written by Gabor de Mooij
#Decides which makefile to use
OS=$(uname -s)
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" ]; then
	echo "using BSD Makefile."
	make -f makefile.bsd clean
	make -f makefile.bsd all
	./ctr -g dictionary.h i18n/nl/dictionarynl.h > ennl.dict #build Dutch dictionary
	./ctr -g plugins/request/i18n/en/dictionary.h plugins/request/i18n/nl/dictionary.h >> ennl.dict #request
	./ctr -g plugins/jsmn/i18n/en/dictionary.h plugins/jsmn/i18n/nl/dictionary.h >> ennl.dict #json
	cat i18n/nl/extra.dict >> ennl.dict #extra translations
	make -f makefile.bsd.nl clean
	make -f makefile.bsd.nl all
else
	echo "using Linux Makefile."
	make -f makefile clean
	make -f makefile all
	./ctr -g dictionary.h i18n/nl/dictionarynl.h > ennl.dict #build Dutch dictionary
	./ctr -g plugins/request/i18n/en/dictionary.h plugins/request/i18n/nl/dictionary.h >> ennl.dict #request
	./ctr -g plugins/jsmn/i18n/en/dictionary.h plugins/jsmn/i18n/nl/dictionary.h >> ennl.dict #json
	cat i18n/nl/extra.dict >> ennl.dict #extra translations
	make -f makefile.nl clean
	make -f makefile.nl all
fi

#Clear object files
rm *.o
