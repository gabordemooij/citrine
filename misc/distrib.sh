# Go back to Citrine root dir
cd ..
VERSION="1_0_3" #for files
VERSION_NAME="1.0.3" #for display
VERSION_DEB="-1"     #for debian
lang="nl"
echo "VERSION = $VERSION | $lang"

rm -rf dist
mkdir -p dist/Win64
mkdir -p dist/Linux

# Windows 64-bit version
rm -rf /tmp/dist/win
mkdir -p /tmp/dist/win/mods/media
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/dist
x86_64-w64-mingw32-windres plugins/media/citrine.rc -O coff -o /tmp/citrine.res
ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 clean
EXTRA="/tmp/citrine.res" LFLAGS="-mconsole -mwindows" ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dll" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 plugin
cp bin/Win64/ctr$lang.exe /tmp/dist/win/
cp ../dlls/*.dll /tmp/dist/win/
cp plugins/media/libctrmedia.dll /tmp/dist/win/mods/media/
cp misc/distrib/assets/picto.ico /tmp/dist/win/pictogram.ico 
cp misc/distrib/assets/nl/* /tmp/dist/win/ # Dutch examples
cp ./misc/distrib/assets/citrine_install_banner.bmp ~/.wine/drive_c/InnoSetupSourceDir/
cp ./misc/distrib/assets/install_mini.bmp ~/.wine/drive_c/InnoSetupSourceDir/
cp ./misc/distrib/assets/setup.ico ~/.wine/drive_c/InnoSetupSourceDir/
cp ./misc/distrib/assets/picto.ico ~/.wine/drive_c/InnoSetupSourceDir/
cp ./misc/distrib/assets/setup.png ~/.wine/drive_c/InnoSetupSourceDir/
cp plugins/media/assets/* ~/.wine/drive_c/InnoSetupSourceDir/
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/license.txt
cp plugins/media/assets/license.txt ~/.wine/drive_c/InnoSetupSourceDir/license.txt
cp -R /tmp/dist/win ~/.wine/drive_c/InnoSetupSourceDir/dist
suffix="$(echo "$lang" | tr 'a-z' 'A-Z')"
sed -e "s/{VERSION}/$VERSION/g" \
	-e "s/{VERSION_NAME}/$VERSION_NAME/g" \
	-e "s/ctrnl/ctr$lang/g" \
	-e "s/CitrineNL/Citrine$suffix/" \
	plugins/media/citrine.iss > ~/.wine/drive_c/InnoSetupSourceDir/citrine.iss
wine "C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe" "C:\\InnoSetupSourceDir\\citrine.iss"
# Copy to output dir
cp ~/.wine/drive_c/InnoSetupSourceDir/Output/Citrine${VERSION}.exe dist/Win64/
# Zip it
cd dist/Win64
zip Citrine${VERSION}.zip Citrine${VERSION}.exe
rm Citrine${VERSION}.exe
cd ../../

# Linux version
rm -rf /tmp/dist/lin
mkdir -p /tmp/dist/lin
OS="Linux" ISO="$lang" make -f makefile clean
OS="Linux" ISO="$lang" make -f makefile
OS="Linux" ISO="$lang" PACKAGE="media" NAME="libctrmedia.so" make -f makefile clean
OS="Linux" ISO="$lang" PACKAGE="media" NAME="libctrmedia.so" make -f makefile plugin
DEBPACKAGE=/tmp/dist/lin/citrine_${VERSION}${VERSION_DEB}
mkdir $DEBPACKAGE
mkdir $DEBPACKAGE/DEBIAN
mkdir $DEBPACKAGE/usr
mkdir $DEBPACKAGE/usr/bin
cp misc/distrib/assets/citrine_init.sh $DEBPACKAGE/usr/bin/citrine_init.sh
mkdir -p $DEBPACKAGE/usr/share/citrine/mods/media/
cp bin/Linux/ctr${lang} $DEBPACKAGE/usr/share/citrine/
cp mods/media/libctrmedia.so $DEBPACKAGE/usr/share/citrine/mods/media/
cp misc/distrib/assets/nl/* $DEBPACKAGE/usr/share/citrine/ # Dutch examples
sed -e "s/{VERSION}/${VERSION_NAME}/g" misc/distrib/assets/control > $DEBPACKAGE/DEBIAN/control
cp misc/distrib/assets/postinst $DEBPACKAGE/DEBIAN/postinst
chmod uog+x $DEBPACKAGE/DEBIAN/postinst
chmod uog-w $DEBPACKAGE/DEBIAN/postinst
dpkg-deb --build $DEBPACKAGE
mv /tmp/dist/lin/citrine_${VERSION}${VERSION_DEB}.deb dist/Linux/
