# Create Citrine distributions for every language and every system
# Windows 64-bit version

# Go back to Citrine root dir
cd ..

# Create icon resource
#cd dist/Win64/assets
x86_64-w64-mingw32-windres plugins/media/citrine.rc -O coff -o /tmp/citrine.res
#cd ../../..

# Copy the additional Chinese installation translation [manual]
# [MANUAL] cp plugins/media/i18n/zh2/Chinese.isl [Inno Setup dir]/Languages/Chinese.isl

# Create Linux folders for Linux distribution
rm -rf dist/Linux
mkdir dist/Linux
mkdir dist/Linux/ISO
mkdir dist/Linux/OUT

VERSION="1_0_0beta7" #for files
VERSION_NAME="1.0b7" #for display

declare -a langs=("en" "nl" "fy" "de" "fr" "no" "ru" "cs" "it" "hi" "pt_br" "uz" "pl" "id" "zh2" "fa" "es")
for lang in "${langs[@]}"
do

# Compile for Windows
ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 clean
EXTRA="/tmp/citrine.res" LFLAGS="-mconsole -mwindows" ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64
# WIN64 plugin
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dll" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 plugin


# Create dictionary
./bin/Linux/ctren -g i18n/en/dictionary.h i18n/$lang/dictionary.h > /tmp/dict_general.dict
./bin/Linux/ctren -g plugins/media/i18n/en/media.h plugins/media/i18n/$lang/media.h > /tmp/dict_media.dict
cat /tmp/dict_general.dict /tmp/dict_media.dict plugins/media/i18n/$lang/extra.dict > /tmp/dict_all.dict

# (Re-)Create folder for lang specific distribution
rm -rf dist/Win64/ISO/$lang
mkdir dist/Win64/ISO/$lang
mkdir dist/Win64/ISO/$lang/mods
mkdir dist/Win64/ISO/$lang/mods/media
# Add executable
cp bin/Win64/ctr$lang.exe dist/Win64/ISO/$lang/
# Add dynamic libraries
cp mods/media/dll64/*.dll dist/Win64/ISO/$lang/
cp plugins/media/libctrmedia.dll dist/Win64/ISO/$lang/mods/media/

# Add assets
cp demodata dist/Win64/ISO/$lang/

# Add shortcut icon
cp plugins/media/assets/picto.ico dist/Win64/ISO/$lang/pictogram.ico 

# Translate examples

./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo1.ctr > dist/Win64/ISO/$lang/demo1.ctr 2>/tmp/err1.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo2.ctr > dist/Win64/ISO/$lang/demo2.ctr 2>/tmp/err2.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo3.ctr > dist/Win64/ISO/$lang/demo3.ctr 2>/tmp/err3.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo4.ctr > dist/Win64/ISO/$lang/demo4.ctr 2>/tmp/err4.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo5.ctr > dist/Win64/ISO/$lang/demo5.ctr 2>/tmp/err5.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo6.ctr > dist/Win64/ISO/$lang/demo6.ctr 2>/tmp/err6.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo7.ctr > dist/Win64/ISO/$lang/demo7.ctr 2>/tmp/err7.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo8.ctr > dist/Win64/ISO/$lang/demo8.ctr 2>/tmp/err8.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo9.ctr > dist/Win64/ISO/$lang/demo9.ctr 2>/tmp/err9.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo10.ctr > dist/Win64/ISO/$lang/demo10.ctr 2>/tmp/err10.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/demo11.ctr > dist/Win64/ISO/$lang/demo11.ctr 2>/tmp/err11.log
./bin/Linux/ctren -t /tmp/dict_all.dict misc/distrib/assets/pak-o-mat.ctr > dist/Win64/ISO/$lang/pak-o-mat.ctr 2>/tmp/pak-o-mat.log
sed -e "s/ctrnl/ctr$lang/g" misc/distrib/assets/export.bat > dist/Win64/ISO/$lang/export.bat



# Copy assets to setup creator work dir
cp plugins/media/assets/* ~/.wine/drive_c/InnoSetupSourceDir/
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/license.txt
cp plugins/media/assets/license.txt ~/.wine/drive_c/InnoSetupSourceDir/license.txt
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/dist
cp -R dist/Win64/ISO/$lang ~/.wine/drive_c/InnoSetupSourceDir/dist


# Copy setup-creator script to work dir
suffix="$(echo "$lang" | tr 'a-z' 'A-Z')"
sed -e "s/{VERSION}/$VERSION/g" \
	-e "s/{VERSION_NAME}/$VERSION_NAME/g" \
	-e "s/ctrnl/ctr$lang/g" \
	-e "s/CitrineNL/Citrine$suffix/" \
	plugins/media/citrine.iss > ~/.wine/drive_c/InnoSetupSourceDir/citrine.iss

# Start setup-creator
wine "C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe" "C:\\InnoSetupSourceDir\\citrine.iss"

# Copy to output dir
mkdir -p dist/Win64/OUT/$lang/
rm dist/Win64/OUT/$lang/* # clean up
cp ~/.wine/drive_c/InnoSetupSourceDir/Output/Citrine${VERSION}.exe dist/Win64/OUT/$lang/

# Zip it
cd dist/Win64/OUT/$lang/
zip Citrine${VERSION}.zip Citrine${VERSION}.exe
rm Citrine${VERSION}.exe
cd ../../../../


# Use the Win64 output as template
cp -R dist/Win64/ISO/$lang dist/Linux/ISO/$lang
mkdir -p dist/Linux/OUT/$lang

# Compile for Linux
OS="Linux" ISO="$lang" make -f makefile clean
OS="Linux" ISO="$lang" make -f makefile
OS="Linux" ISO="$lang" PACKAGE="media" NAME="libctrmedia.so" make -f makefile clean
OS="Linux" ISO="$lang" PACKAGE="media" NAME="libctrmedia.so" make -f makefile plugin


# Add executable
cp bin/Linux/ctr$lang dist/Linux/ISO/$lang/
# Add dynamic libraries
cp plugins/media/libctrmedia.so dist/Linux/ISO/$lang/mods/media/
rm dist/Linux/ISO/$lang/*.dll
rm dist/Linux/ISO/$lang/*.exe
sed -e "s/ctrapp_nl/ctrapp_$lang/g" misc/distrib/assets/export.sh > dist/Linux/ISO/$lang/export.sh
cp misc/distrib/assets/export.desktop dist/Linux/ISO/$lang/export.desktop


tar cvzf "dist/Linux/OUT/$lang/citrine${lang}${VERSION}.tar.gz" -C dist/Linux/ISO/ ${lang}


# Create Linux AppImage distribution
rm -rf /tmp/${lang}/Citrine.AppDir
mkdir /tmp/${lang}
cp -r misc/Citrine.AppDir /tmp/${lang}/
cp  dist/Linux/ISO/${lang}/ctr${lang} /tmp/${lang}/Citrine.AppDir/usr/bin/
cp  -R dist/Linux/ISO/${lang}/mods /tmp/${lang}/Citrine.AppDir/

sed -e "s/ctrnl/ctr$lang/g" misc/Citrine.AppDir/AppRun > /tmp/${lang}/Citrine.AppDir/AppRun


./appimagetool-x86_64.AppImage /tmp/${lang}/Citrine.AppDir citrine_app ; cp citrine_app dist/Linux/ISO/${lang}/ctrapp_${lang}
chmod uog+x dist/Linux/ISO/${lang}/ctrapp_${lang}
#sed -e "s/ctrnl/ctrapp_$lang/g" plugins/media/assets/citrine.sh > dist/Linux/ISO/$lang/citrine_app.sh


#chmod uog+x dist/Linux/ISO/${lang}/citrine_app.sh
tar cvzf "dist/Linux/OUT/$lang/citrine${lang}${VERSION}ai.tar.gz" -C dist/Linux/ISO/ ${lang}

# Compile for Windows 32bit
ISO="$lang" CC=i686-w64-mingw32-gcc-win32 DLLTOOL=i686-w64-mingw32-dlltool make -f makefile.win32 clean
EXTRA="/tmp/citrine.res" LFLAGS="-mwindows" ISO="$lang" CC=i686-w64-mingw32-gcc-win32 DLLTOOL=i686-w64-mingw32-dlltool make -f makefile.win32
# win32 plugin
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dll" CC=i686-w64-mingw32-gcc-win32 DLLTOOL=i686-w64-mingw32-dlltool make -f makefile.win32 plugin

# (Re-)Create folder for lang specific distribution
rm -rf dist/Win32/ISO/$lang
mkdir -p dist/Win32/ISO/$lang
mkdir -p dist/Win32/ISO/$lang/mods
mkdir -p dist/Win32/ISO/$lang/mods/media

# Re-use Win64 as template
cp -R dist/Win64/ISO/$lang/* dist/Win32/ISO/$lang/
# Add executable
cp bin/Win32/ctr$lang.exe dist/Win32/ISO/$lang/
# Add dynamic libraries
cp mods/media/dll32/*.dll dist/Win32/ISO/$lang/
cp plugins/media/libctrmedia.dll dist/Win32/ISO/$lang/mods/media/


done
