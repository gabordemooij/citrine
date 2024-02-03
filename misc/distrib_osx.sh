

cd ..

rm -rf dist/osx
mkdir dist/osx
rm bin/Mac/*

declare -a langs=("nl" "en")
for lang in "${langs[@]}"
do

mkdir -p dist/osx/$lang/mods/media
cp tpl/$lang/* dist/osx/$lang/
rm mods/media/libctrmedia.dylib
ISO="$lang" make -f makefile.mac clean
ISO="$lang" OS="Mac" make -f makefile.mac
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dylib" make -f makefile.mac plugin
mv ./bin/Mac/ctr$lang dist/osx/$lang/ctr$lang
mv ./mods/media/libctrmedia.dylib dist/osx/$lang/mods/media/

done
