#!/bin/bash
./mk.sh
j=1
for i in $(find tests -name '*.ctr'); do
	fitem=$i
	echo -n $fitem;
	fexpect="${i: 0:-4}.exp"
	result=`./ctr ${fitem}`
	expected=`cat $fexpect`
	if [ "$result" == "$expected" ]; then
		echo "[$j]"
		j=$((j+1))
	else
		echo "FAIL.\n"
		echo "EXPECTED:\n"
		echo $expected
		echo "\n\n\n"
		echo "BUT GOT:\n"
		echo $result
		exit 1
	fi
done
echo ""
echo "All tests passed."
exit 0
