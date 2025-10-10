#!/bin/sh

if [ -z "$OS" ]; then
OS=$(uname -s)
fi

if [ "$OS" = "Darwin" ]; then
OS="Mac"
fi

if [ -z "$MAKEFILE" ]; then
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" ]; then
	MAKEFILE=makefile.bsd
elif [ "$OS" = "Haiku" ]; then
	MAKEFILE=makefile.haiku
elif [ "$OS" = "Mac" ]; then
	MAKEFILE=makefile.mac
elif [ "$OS" = "Win32" ]; then
	MAKEFILE=makefile.win32
elif [ "$OS" = "Win64" ]; then
	MAKEFILE=makefile.win64
else
	MAKEFILE=makefile
fi
fi

echo "USING: ${MAKEFILE}"

#Get list of ISO codes to build.
ISOs=$(ls i18nsel)
if [ $# -gt 0 ]; then
	ISOs=$@
fi

#Run make for every ISO code.
for ISO in $ISOs
do
	echo ; echo $ISO
	export ISO
	export OS
	export CC
	export DLLTOOL
	make -f $MAKEFILE clean
	make -f $MAKEFILE all
	if [ -f "dict/en${ISO}.dict" ]; then
		rm   dict/en${ISO}.dict
	fi
	./ctr -g i18n/en/dictionary.h i18n/${ISO}/dictionary.h > dict/en${ISO}.dict
	./ctr -g i18n/${ISO}/dictionary.h i18n/en/dictionary.h > dict/${ISO}en.dict
	cat i18n/${ISO}/extra.dict >> dict/en${ISO}.dict
	cat i18n/${ISO}/extra_reverse.dict >> dict/${ISO}en.dict
done

#Clear object files and binary
rm *.o ctr
