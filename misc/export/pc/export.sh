#/bin/bash

# usage (from rootdir & first compile regular!):
# Linux:
# MEDIALIB="libctrmedia.so" MAKEFILE="makefile" ISO="nl" bash misc/export/pc/export.sh
# Win64:
# MEDIALIB="libctrmedia.dll" MAKEFILE="makefile.win64" ISO="nl" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool bash misc/export/pc/export.sh
# don't forget to copy libs!

# create embed file
cat misc/opt/preface.h > misc/opt/embed.h
xxd -i -n CtrBlob data >> misc/opt/embed.h

# compile media plugin
PACKAGE="media" \
NAME=$MEDIALIB \
make -f $MAKEFILE clean

PACKAGE="media" NAME=$MEDIALIB \
make -f $MAKEFILE plugin

# compile Citrine with embed options
EXTRACFLAGS=" -D EMBED " \
make -f $MAKEFILE
