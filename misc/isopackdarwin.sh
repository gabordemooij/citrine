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
	cp ../bin/darwin/ctr${ISO} /tmp/dist/bin/ctr${ISO}
	#Copy examples
	cp ../examples/${ISO}/* /tmp/dist/examples/
	#Copy font
	cp ../fonts/Citrine.ttf /tmp/dist/fonts/
	#Archive
	tar cvzf /tmp/citrine${V}-darwin-${ISO}.tar.gz -C /tmp dist
	#Sign
	signify-openbsd -Sz -s keys/privatekey.sec -m /tmp/citrine${V}-darwin-${ISO}.tar.gz -x downloads/Darwin/citrine${V}-darwin-${ISO}.tgz
done
