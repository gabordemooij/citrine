#!/bin/sh

set -x
set -v

OS=`uname -s`
LDFLAGS='-shared'
if [ "$OS" = "Darwin" ]; then
  LDFLAGS='-shared -undefined dynamic_lookup'
fi
#Remove .so
find . -name "*.so" -exec rm {} +

#For plugin test, compile Percolator plugin
cd plugins/percolator;
cc -c percolator.c -Wall -Werror -fPIC -o percolator.o
cc ${LDFLAGS} -o libctrpercolator.so percolator.o
cd ..
cd ..
cp plugins/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so

#request test
cd plugins/request/ccgi-1.2;
cc -c ccgi.c -Wall	-Werror -fPIC -o ccgi.o
cc -c prefork.c -Wall -Werror -fPIC -o prefork.o
cd ..
cc -c request.c -Wall -Werror -fPIC -o request.o
cc ${LDFLAGS} -o libctrrequest.so request.o ccgi-1.2/ccgi.o ccgi-1.2/prefork.o
cd ..
cd ..
cp plugins/request/libctrrequest.so mods/request/libctrrequest.so

#json test
cd plugins/jsmn/jsmn;
gcc -c jsmn.c -Wall	-Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o
cd ..
gcc -c jsmn.c -Wall	-Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o ; gcc -shared -o libctrjsmn.so jsmn.o jsmn/jsmn.o
cd ..
cd ..
cp plugins/jsmn/libctrjsmn.so mods/json/libctrjson.so

#curl test
rm plugins/curl/src/libctrcurl.so
rm plugins/curl/src/curl.o
rm mods/curl/libctrcurl.so
cd plugins/curl/src;
gcc -c curl.c -Wall	-Werror -fpic -o curl.o
gcc -shared -o libctrcurl.so curl.o -lcurl
cd ..
cd ..
cd ..
cp plugins/curl/src/libctrcurl.so mods/curl/libctrcurl.so

#pg test
rm plugins/pg/libctrpg.so
rm plugins/pg/pg.o
rm mods/pg/libctrpg.so
cd plugins/pg;
gcc -c pg.c -Wall -I/usr/include/postgresql/ -lpq -Werror -fpic -o pg.o
gcc -shared -o libctrpg.so pg.o -I/usr/include/postgresql/ -lpq
cd ..
cd ..
cp plugins/pg/libctrpg.so mods/pg/libctrpg.so



make clean;
./mk.sh

j=1
for i in $(find tests -name 'test*.ctr'); do
	touch /tmp/a
	touch /tmp/b
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i%%.ctr}.exp"
	result1=`echo "test" | ./ctr ${fitem} 1>/tmp/a 2>/tmp/b`
	result=`cat /tmp/a /tmp/b`
	expected=`cat $fexpect`
	if [ "$result" = "$expected" ]; then
		echo "[$j]"
		j=$((j+1))
	else
		echo "FAIL."
		echo "EXPECTED:"
		echo $expected
		echo ""
		echo "BUT GOT:"
		echo $result
		exit 1
	fi
	rm /tmp/a
	rm /tmp/b
	headline=$(head -n 1 $fitem)
done
echo ""
echo "All tests passed."
exit 0
