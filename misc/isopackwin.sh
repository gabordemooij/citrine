OS="windows"
V="0.9.2"
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
	cp ./${OS}/ctr${ISO}.exe /tmp/dist/bin/ctr${ISO}.exe
	cp ./${OS}/*.dll /tmp/dist/bin/
	#Copy examples
	cp ../examples/${ISO}/* /tmp/dist/examples/
	#Copy font
	cp ../fonts/Citrine.ttf /tmp/dist/fonts/
	#Archive
	tar cvzf citrine${V}-${OS}-${ISO}.tar.gz -C /tmp dist
	#Sign
	signify-openbsd -Sz -s keys/privatekey.sec -m citrine${V}-${OS}-${ISO}.tar.gz -x downloads/${OS}/citrine${V}-${OS}-${ISO}.tgz
done
