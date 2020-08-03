#!/bin/sh
OS=$(uname -s)
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" -o "$OS" = "Darwin" ]; then
	MAKEFILE=makefile.bsd
else
	MAKEFILE=makefile
fi
echo "USING: ${MAKEFILE}"
for ISO in $(ls i18nsel)
do
	echo $ISO
	export ISO
	export OS
	make -f $MAKEFILE clean
	make -f $MAKEFILE all
	rm dict/xx${ISO}.dict
	bin/${OS}/ctr -g i18n/xx/dictionary.h i18n/${ISO}/dictionary.h > dict/xx${ISO}.dict
	bin/${OS}/ctr -g i18n/${ISO}/dictionary.h i18n/xx/dictionary.h > dict/${ISO}xx.dict
	cat i18n/${ISO}/extra.dict >> dict/xx${ISO}.dict
done

#Clear object files
rm *.o
