OS="windows"
V="0.9.5"
for ISO in $(ls ../i18n)
do
	#Remove previous working dir
	rm -rf /tmp/dist
	#Create folder structure
	mkdir /tmp/dist
	mkdir /tmp/dist/bin
	mkdir /tmp/dist/examples
	mkdir /tmp/dist/fonts
	#Copy license
	cp ../LICENSE /tmp/dist/
	#Copy binary
	cp ../bin/Win64/ctr${ISO}.exe /tmp/dist/bin/ctr${ISO}.exe
	#Copy examples
	cp ../examples/${ISO}/* /tmp/dist/examples/
	#Copy font
	cp ../fonts/Citrine.ttf /tmp/dist/fonts/
	#Archive
	cd /tmp
	rm citrine${V}-${OS}-${ISO}.zip
	zip -r citrine${V}-${OS}-${ISO}.zip dist/*
	cd -
	cp /tmp/citrine${V}-${OS}-${ISO}.zip downloads/Windows64/citrine${V}-${OS}-${ISO}.zip
done
