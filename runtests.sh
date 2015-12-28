#!/bin/bash
./mk.sh
j=1
for i in $(find tests -name '*.ctr'); do
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
	result=`./ctr ${fitem} --roundtrip`
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
exit 0
