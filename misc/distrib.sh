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

declare -a langs=("nl" "en_ascii" "ru" "en" "hi" "fr" "no" "pt_br" "uz" "pl" "cs" "id" "zh2")
for lang in "${langs[@]}"
do

# Compile for Windows
ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 clean
EXTRA="/tmp/citrine.res" LFLAGS="-mwindows" ISO="$lang" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64
# WIN64 plugin
ISO="$lang" PACKAGE="media" NAME="libctrmedia.dll" CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 plugin


# Create dictionary
./bin/Linux/ctrnl -g i18n/nl/dictionary.h i18n/$lang/dictionary.h > /tmp/dict_general.dict
./bin/Linux/ctrnl -g plugins/media/i18n/nl/media.h plugins/media/i18n/$lang/media.h > /tmp/dict_media.dict
# Add dictionary for examples 1-6 and 7 (editor)
cat /tmp/dict_general.dict /tmp/dict_media.dict plugins/media/i18n/$lang/extra.dict > /tmp/dict_all.dict


# (Re-)Create folder for lang specific distribution
rm -rf dist/Win64/ISO/$lang
mkdir dist/Win64/ISO/$lang
mkdir dist/Win64/ISO/$lang/mods
mkdir dist/Win64/ISO/$lang/mods/media
# Add executable
cp bin/Win64/ctr$lang.exe dist/Win64/ISO/$lang/
# Add dynamic libraries
cp plugins/media/*.dll dist/Win64/ISO/$lang/
cp plugins/media/libctrmedia.dll dist/Win64/ISO/$lang/mods/media/


# Translate assets with script
source plugins/media/i18n/$lang/assets.sh
# Add assets
cp /tmp/translated_assets/* dist/Win64/ISO/$lang/
# Add shortcut icon
cp plugins/media/assets/picto.ico dist/Win64/ISO/$lang/pictogram.ico 

# Translate examples
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__1__ > dist/Win64/ISO/$lang/__1__ 2>/tmp/err1.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__2__ > dist/Win64/ISO/$lang/__2__ 2>/tmp/err2.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__3__ > dist/Win64/ISO/$lang/__3__ 2>/tmp/err3.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__4__ > dist/Win64/ISO/$lang/__4__ 2>/tmp/err4.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__5win__ > dist/Win64/ISO/$lang/__5__ 2>/tmp/err5win.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__6__ > dist/Win64/ISO/$lang/__6__ 2>/tmp/err6.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__7__ > dist/Win64/ISO/$lang/__7__ 2>/tmp/err7.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__7ai__ > dist/Win64/ISO/$lang/__7ai__ 2>/tmp/err7.log
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/client > dist/Win64/ISO/$lang/client 2>/tmp/errclient.log

# Copy assets to setup creator work dir
cp plugins/media/assets/* ~/.wine/drive_c/InnoSetupSourceDir/
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/license.txt
cp plugins/media/assets/license.txt ~/.wine/drive_c/InnoSetupSourceDir/license.txt
rm -rf ~/.wine/drive_c/InnoSetupSourceDir/dist
cp -R dist/Win64/ISO/$lang ~/.wine/drive_c/InnoSetupSourceDir/dist


# Copy setup-creator script to work dir
suffix="$(echo "$lang" | tr 'a-z' 'A-Z')"
sed -e "s/ctrnl/ctr$lang/g" -e "s/CitrineNL/Citrine$suffix/" plugins/media/citrine.iss > ~/.wine/drive_c/InnoSetupSourceDir/citrine.iss

# Start setup-creator
wine "C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe" "C:\\InnoSetupSourceDir\\citrine.iss"

# Copy to output dir
mkdir -p dist/Win64/OUT/$lang/
cp ~/.wine/drive_c/InnoSetupSourceDir/Output/Citrine096.exe dist/Win64/OUT/$lang/

# Zip it
cd dist/Win64/OUT/$lang/
zip Citrine096.zip Citrine096.exe
rm Citrine096.exe
cd ../../../../


# Use the Win64 output as template
cp -R dist/Win64/ISO/$lang dist/Linux/ISO/$lang
./bin/Linux/ctrnl -t /tmp/dict_all.dict plugins/media/examples/__5__ > dist/Linux/ISO/$lang/__5__ 2>/tmp/err5.log
mkdir -p dist/Linux/OUT/$lang

# Compile for Linux
OS="Linux" ISO="$lang" make -f makefile clean
OS="Linux" ISO="$lang" make -f makefile
OS="Linux" ISO="$lang" PACKAGE="media" NAME="libctrmedia.so" make -f makefile plugin


# Add executable
cp bin/Linux/ctr$lang dist/Linux/ISO/$lang/
# Add dynamic libraries
cp plugins/media/libctrmedia.so dist/Linux/ISO/$lang/mods/media/
rm dist/Linux/ISO/$lang/*.dll
rm dist/Linux/ISO/$lang/*.exe
sed -e "s/ctrnl/ctr$lang/g" plugins/media/assets/citrine.sh > dist/Linux/ISO/$lang/citrine.sh
chmod uog+x dist/Linux/ISO/$lang/citrine.sh
tar cvzf "dist/Linux/OUT/$lang/citrine${lang}096.tar.gz" -C dist/Linux/ISO/ ${lang}


# Create Linux AppImage distribution
rm -rf /tmp/${lang}/Citrine.AppDir
mkdir /tmp/${lang}
cp -r misc/Citrine.AppDir /tmp/${lang}/
cp  dist/Linux/ISO/${lang}/ctr${lang} /tmp/${lang}/Citrine.AppDir/usr/bin/
cp  -R dist/Linux/ISO/${lang}/mods /tmp/${lang}/Citrine.AppDir/

sed -e "s/ctrnl/ctr$lang/g" misc/Citrine.AppDir/AppRun > /tmp/${lang}/Citrine.AppDir/AppRun

./appimagetool-x86_64.AppImage /tmp/${lang}/Citrine.AppDir citrine_app ; cp citrine_app dist/Linux/ISO/${lang}/ctrapp_${lang}
chmod uog+x dist/Linux/ISO/${lang}/ctrapp_${lang}
sed -e "s/ctrnl/ctrapp_$lang/g" plugins/media/assets/citrine.sh > dist/Linux/ISO/$lang/citrine_app.sh
sed -e "s/ctrapp_nl/ctrapp_$lang/g" dist/Linux/ISO/$lang/__7ai__ > dist/Linux/ISO/$lang/__7__ 


chmod uog+x dist/Linux/ISO/${lang}/citrine_app.sh
tar cvzf "dist/Linux/OUT/$lang/citrine${lang}096ai.tar.gz" -C dist/Linux/ISO/ ${lang}

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
