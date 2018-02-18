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
