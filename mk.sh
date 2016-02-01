#MK Script
#Written by Gabor de Mooij
#Decides which makefile to use
OS=$(uname -s)
if [[ $OS -eq "OpenBSD" ]];then
	print "using OpenBSD Makefile."
	make -f makefile.openbsd
else
	print "using Linux Makefile."
	make -f makefile
fi

#Clear object files
rm *.o
