#!/bin/bash

#For plugin test, compile Percolator plugin
cd plugins/percolator;
gcc -c percolator.c -Wall -Werror -fpic -o percolator.o ; gcc -shared -o libctrpercolator.so percolator.o
cd ..
cd ..
cp plugins/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so

make clean;
./mk.bash

./ctr -c /tmp/dynamic_module.ast tests/assets/asset_mod_for_ast.ctr

j=1
for i in $(find tests -name 'test*.ctr'); do
	fitem=$i
	echo -n "$fitem interpret";
	fexpect="${i: 0:-4}.exp"
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
	fexpect="${i: 0:-4}.exp"
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
