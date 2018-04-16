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
rm plugins/percolator/libctrpercolator.so
rm mods/percolator/libctrpercolator.so
cd plugins/percolator;
cc -c percolator.c -Wall -Werror -fPIC -o percolator.o
cc ${LDFLAGS} -o libctrpercolator.so percolator.o
cd ..
cd ..
cp plugins/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so


rm plugins/percolator/libctrpercolator.so
cd plugins/percolator;
cc -DlangNL -c percolator.c -Wall -Werror -fPIC -o koffiezetter.o
cc ${LDFLAGS} -o libctrkoffiezetter.so koffiezetter.o
cd ..
cd ..
cp plugins/percolator/libctrkoffiezetter.so mods/koffiezetter/libctrkoffiezetter.so


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
gcc -DCTRPLUGIN_CURL_CURLOPT_URL -c curl.c -Wall	-Werror -fpic -o curl.o
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

#sodium
rm plugins/crypt/libctrpassword.so
rm plugins/crypt/crypt.o
rm mods/password/libctrpassword.so
cd plugins/crypt;
gcc -c crypt.c -Wall -I/usr/local/include/  -Werror -fpic -o crypt.o
gcc -shared -o libctrpassword.so crypt.o -L/usr/local/lib/ -lsodium
cd ..
cd ..
cp plugins/crypt/libctrpassword.so mods/password/libctrpassword.so


./mk.sh

j=1
for i in $(find tests -name 'test*.ctr'); do
	touch /tmp/a
	touch /tmp/b
	touch /tmp/c
	touch /tmp/d
	touch /tmp/transl.ctr
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i%%.ctr}.exp"
	result1=`echo "test" | ./ctr ${fitem} 1>/tmp/a 2>/tmp/b`
	trans=`./ctr -t ennl.dict ${fitem} > /tmp/transl.ctr`
	result2=`echo "test" | ./ctrnl /tmp/transl.ctr 1>/tmp/c 2>/tmp/d`
	result=`cat /tmp/a /tmp/b`
	resultB=`cat /tmp/c /tmp/d`
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

	directive=`head -n1 ${fitem}`
	if [ "$directive" != "#SINGLE_LANGUAGE" ]; then
		if [ "$resultB" = "$expected" ]; then
			echo "[$j]"
			j=$((j+1))
		else
			echo "FAIL."
			echo "EXPECTED:"
			echo $expected
			echo ""
			echo "BUT GOT:"
			echo $resultB
			exit 1
		fi
	fi
	rm /tmp/a
	rm /tmp/b
	rm /tmp/c
	rm /tmp/d
	rm /tmp/transl.ctr
	headline=$(head -n 1 $fitem)
done
echo ""
echo "All tests passed."
exit 0
