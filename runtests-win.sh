#!/bin/sh

set -x
set -v

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
	fi
	rm /tmp/a
	rm /tmp/b
	headline=$(head -n 1 $fitem)
done
echo ""
echo "All tests passed."
exit 0
