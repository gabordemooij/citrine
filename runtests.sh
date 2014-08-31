#!/bin/bash
j=1
for i in $(find tests -name '*.ctr'); do
	fitem=$i
	fexpect="${i: 0:-4}.exp"
	result=`./ctr ${fitem}`
	expected=`cat $fexpect`
	echo -n $fitem;
	if [ "$result" == "$expected" ]; then
		echo "[$j]"
		j=$((j+1))
	else
		echo "FAIL."
		exit 1
	fi
done
echo ""
echo "All tests passed."
exit 0
