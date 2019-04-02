#!/bin/sh
OS=$(uname -s)
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" -o "$OS" = "Darwin" ]; then
	MAKEFILE=makefile.bsd
else
	MAKEFILE=makefile
fi
echo "USING: ${MAKEFILE}"
for ISO in $(ls i18n)
do
	echo $ISO
	export ISO
	export OS
	make -f $MAKEFILE clean
	make -f $MAKEFILE all
	rm dict/en${ISO}.dict
	bin/${OS}/ctr -g i18n/us/dictionary.h i18n/${ISO}/dictionary.h > dict/en${ISO}.dict
	cat i18n/${ISO}/extra.dict >> dict/en${ISO}.dict
done

#Clear object files
rm *.o
