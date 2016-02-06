#!/bin/bash

set -x
set -v

#Remove .so
find . -name *.so | xargs rm

#For plugin test, compile Percolator plugin
cd plugins/percolator;
gcc -c percolator.c -Wall -Werror -fpic -o percolator.o ; gcc -shared -o libctrpercolator.so percolator.o
cd ..
cd ..
cp plugins/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so

#request test
cd plugins/request/ccgi-1.2;
gcc -c ccgi.c -Wall	-Werror -fpic -o ccgi.o
gcc -c prefork.c -Wall -Werror -fpic -o prefork.o
cd ..
gcc -c request.c -Wall -Werror -fpic -o request.o ; gcc -shared -o libctrrequest.so request.o ccgi-1.2/ccgi.o ccgi-1.2/prefork.o
cd ..
cd ..
cp plugins/request/libctrrequest.so mods/request/libctrrequest.so


make clean;
./mk.bash

./ctr -c /tmp/dynamic_module.ast tests/assets/asset_mod_for_ast.ctr

j=1
for i in $(find tests -name 'test*.ctr'); do
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i%%.ctr}.exp"
	result=`./ctr ${fitem}`
	expected=`cat $fexpect`
	if [ "$result" == "$expected" ]; then
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
	fitem=$i
	echo -n "$fitem compiled";
	fexpect="${i%%.ctr}.exp"
	result=`./ctr -c /tmp/dump.ast ${fitem} ; ./ctr -r /tmp/dump.ast`
	expected=`cat $fexpect`
	if [ "$result" == "$expected" ]; then
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
done
echo ""
echo "All tests passed."

rm /tmp/dynamic_module.ast

exit 0
