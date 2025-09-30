 VERSION="1_0_3" #for files
VERSION_NAME="1.0.3" #for display
lang="nl"
cd ..
rm -rf /tmp/dist/osx
mkdir -p /tmp/dist/osx/mods/media
rm mods/media/libctrmedia.dylib
ISO="$lang" make -f makefile.mac clean
ISO="$lang" OS="Mac" make -f makefile.mac
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dylib" make -f makefile.mac plugin
cp ./bin/Mac/ctr${lang} /tmp/dist/osx/ctr${lang}
cp ./mods/media/libctrmedia.dylib /tmp/dist/osx/mods/media/
cp ./distrib/assets/nl/* /tmp/dist/osx/
cp -R ~/frameworks/* /tmp/dist/osx/ # put SDL2 frameworks for macos here to craft dist
hdiutil create -volname ctr${lang}${VERSION} -srcfolder /tmp/dist/osx -ov -format UDZO dist/osx/ctr${lang}${VERSION}.dmg
