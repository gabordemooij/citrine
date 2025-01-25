 
VERSION="1_0_0" #for files
VERSION_NAME="1.0.0" #for display

cd ..

rm -rf dist/osx
mkdir dist/osx
rm bin/Mac/*


declare -a langs=("en" "nl")
for lang in "${langs[@]}"
do

rm mods/media/libctrmedia.dylib
ISO="$lang" make -f makefile.mac clean
ISO="$lang" OS="Mac" make -f makefile.mac
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dylib" make -f makefile.mac plugin

# Create dictionary
./bin/Mac/ctren -g i18n/en/dictionary.h i18n/$lang/dictionary.h > /tmp/dict_general.dict
./bin/Mac/ctren -g plugins/media/i18n/en/media.h plugins/media/i18n/$lang/media.h > /tmp/dict_media.dict
cat /tmp/dict_general.dict /tmp/dict_media.dict plugins/media/i18n/$lang/extra.dict > /tmp/dict_all.dict

# (Re-)Create folder for lang specific distribution

mkdir -p dist/osx/ISO/$lang
mkdir -p dist/osx/OUT/$lang

mkdir dist/osx/ISO/$lang/mods
mkdir dist/osx/ISO/$lang/mods/media

# Add executable
cp ./bin/Mac/ctr$lang dist/osx/ISO/$lang/ctr$lang
# Add dynamic libraries
cp ./mods/media/libctrmedia.dylib dist/osx/ISO/$lang/mods/media/

# Add assets
cp demodata dist/osx/ISO/$lang/

# Translate examples
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo1.ctr > dist/osx/ISO/$lang/demo1.ctr 2>/tmp/err1.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo2.ctr > dist/osx/ISO/$lang/demo2.ctr 2>/tmp/err2.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo3.ctr > dist/osx/ISO/$lang/demo3.ctr 2>/tmp/err3.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo4.ctr > dist/osx/ISO/$lang/demo4.ctr 2>/tmp/err4.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo5.ctr > dist/osx/ISO/$lang/demo5.ctr 2>/tmp/err5.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo6.ctr > dist/osx/ISO/$lang/demo6.ctr 2>/tmp/err6.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo7.ctr > dist/osx/ISO/$lang/demo7.ctr 2>/tmp/err7.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo8.ctr > dist/osx/ISO/$lang/demo8.ctr 2>/tmp/err8.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo9.ctr > dist/osx/ISO/$lang/demo9.ctr 2>/tmp/err9.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo10.ctr > dist/osx/ISO/$lang/demo10.ctr 2>/tmp/err10.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo11.ctr > dist/osx/ISO/$lang/demo11.ctr 2>/tmp/err11.log
./bin/Mac/ctren -t /tmp/dict_all.dict misc/distrib/assets/pak-o-mat.ctr > dist/osx/ISO/$lang/pak-o-mat.ctr 2>/tmp/pak-o-mat.log

rm -rf /tmp/dist$lang
cp -R dist/osx/ISO/$lang /tmp/dist$lang
cp -R ~/frameworks/* /tmp/dist$lang # put SDL2 frameworks for macos here to craft dist
#cp -R ~/libs/* /tmp/dist$lang # other libs

hdiutil create -volname ctr${lang}${VERSION} -srcfolder /tmp/dist$lang -ov -format UDZO dist/osx/OUT/$lang/ctr${lang}${VERSION}.dmg

done
