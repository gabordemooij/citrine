for ISO in $(ls ../i18n)
do
	mkdir ${ISO}
	rm ${ISO}/example1
	rm ${ISO}/example2
	rm ${ISO}/example3
	../bin/Linux/ctrus -t ../dict/en${ISO}.dict xx/example1.ctr > ${ISO}/example1.ctr 2>/dev/null
	../bin/Linux/ctrus -t ../dict/en${ISO}.dict xx/example2.ctr > ${ISO}/example2.ctr 2>/dev/null
	../bin/Linux/ctrus -t ../dict/en${ISO}.dict xx/example3.ctr > ${ISO}/example3.ctr 2>/dev/null
done