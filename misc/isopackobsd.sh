OS="OpenBSD"
V="0.9.3"
for ISO in $(ls ../i18n)
do
	#Remove previous working dir
	rm -rf /tmp/dist
	#Create folder structure
	mkdir /tmp/dist
	mkdir /tmp/dist/bin
	mkdir /tmp/dist/examples
	mkdir /tmp/dist/fonts
	mkdir /tmp/dist/mods
	mkdir /tmp/dist/mods/request
	mkdir /tmp/dist/mods/json
	#Copy license
	cp ../LICENSE /tmp/dist/
	#Copy binary
	cp ../bin/${OS}/ctr${ISO} /tmp/dist/bin/ctr${ISO}	
	#Copy examples
	cp ../examples/${ISO}/* /tmp/dist/examples/
	#Copy font
	cp ../fonts/Citrine.ttf /tmp/dist/fonts/
	#Copy mods
	cp ../mods/${OS}/request/libctrrequest.so /tmp/dist/mods/request/
	cp ../mods/${OS}/json/libctrjson.so       /tmp/dist/mods/json/
	#Archive
	tar cvzf citrine${V}-${OS}-${ISO}.tar.gz -C /tmp dist
	#Sign
	signify-openbsd -Sz -s keys/privatekey.sec -m citrine${V}-${OS}-${ISO}.tar.gz -x downloads/${OS}/citrine${V}-${OS}-${ISO}.tgz
done
