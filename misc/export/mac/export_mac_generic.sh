#/bin/bash

# usage (from rootdir & first compile regular!):
# OS="Mac" MAKEFILE="makefile.mac" ISO="nl" bash misc/export/pc/export.sh
# OS="Mac" MAKEFILE="makefile.mac" ISO="nl" ./misc/export/mac/export.sh

MAKEFILE="makefile.mac"
# create embed file
cat misc/opt/preface_mac.h > misc/opt/embed.h
xxd -i -n CtrBlob data >> misc/opt/embed.h

# compile Citrine with embed options
#ln -s ../../media.o plugins/media/media.o
#ln -s ../../jsmn.o plugins/media/jsmn.o
make -f plugins/media/makefile.mac clean

make -f $MAKEFILE clean
EXTRACFLAGS=" -D SDL -D LIBCURL -D FFI -D EMBED -D DESKTOP_FULLSCREEN -I plugins/media/i18n/${ISO} -I plugins/media " \
EMBED=" plugins/media/jsmn.o plugins/media/media.o " \
LDFLAGS=" -F/PATH/TO/Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf -framework SDL2_mixer -rpath @executable_path -lcurl -lffi " \
make -f $MAKEFILE

rm -rf ./bin/Mac/Citrine${ISO}.app
cp -R ./misc/export/assets/Citrine.app ./bin/Mac/Citrine${ISO}.app
cp ./bin/Mac/ctr${ISO} ./bin/Mac/Citrine${ISO}.app/Contents/MacOS/
sed -i '' -e s/{CTRVERSION}/ctr${ISO}/g ./bin/Mac/Citrine${ISO}.app/Contents/Info.plist 
