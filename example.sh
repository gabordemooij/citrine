ulimit -S -c unlimited

EXAMPLE="${1:-vault}"
OS="${2:-Linux}"

echo $EXAMPLE
echo $OS

# ~ oude plugin ~

#cd lvgl/gui/lv_port_linux
# ~ voor Linux ~
#make -f Makefile_so clean
#make -f Makefile_so
#cd ../../..
#cp lvgl/gui/lv_port_linux/build/bin/gui.so mods/gui/libctrgui.so
#./bin/Linux/ctrnl gui.ctr


ISO="en"
export ISO

if [[ $OS = "Linux" ]]; then
	
	make clean
	make
	PACKAGE="gui" NAME="libctrgui.so" make plugin
	
	cd examples/${EXAMPLE}
	rm mods
	ln -s ../../build/Linux/bin/mods mods
	../../build/Linux/bin/ctren ${EXAMPLE}.ctr 
	
fi

if [[ $OS = "Win64" ]]; then
	
	make -f makefile.win64 clean
	make -f makefile.win64
	PACKAGE="gui" NAME="libctrgui.dll" make -f makefile.win64 plugin
	
	cd examples/${EXAMPLE}
	rm mods
	ln -s ../../build/Win64/bin/mods mods
	wine ../../build/Win64/bin/ctren.exe ${EXAMPLE}.ctr 
	
fi

#Linux
#make clean
#make
#mv ctr ./bin/Linux/ctr$ISO


#Windows
## WIN64
#CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64 clean
#CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64


#cd plugins/gui

# ~ voor Windows ~ - disable thorvg?
#make -f makefile.win64 clean
#CC=x86_64-w64-mingw32-gcc-win32 DLLTOOL=x86_64-w64-mingw32-dlltool make -f makefile.win64
#cd ../..
#wine ./bin/Win64/ctr${ISO}.exe vault.ctr

# ~ nieuwe plugin ~
#make -f makefile clean
#make -f makefile
#cd ../..
#./bin/Linux/ctren vault.ctr



