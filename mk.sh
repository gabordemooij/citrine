#MK Script
#Written by Gabor de Mooij
#Decides which makefile to use
OS=$(uname -s)
if [[ $OS = "OpenBSD" ]];then
	echo "using OpenBSD Makefile."
	make -f makefile.openbsd
elif [[ $OS = "Darwin" ]];then
        echo "using Mac OS X Makefile."
        make -f makefile.macosx
else
	echo "using Linux Makefile."
	make -f makefile
fi

#Clear object files
rm *.o
