#!/bin/sh
#MK Script
#Written by Gabor de Mooij
#Decides which makefile to use
OS=$(uname -s)
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" ]; then
	echo "using BSD Makefile."
	make -f makefile.bsd
else
	echo "using Linux Makefile."
	make -f makefile
fi

#Clear object files
rm *.o
