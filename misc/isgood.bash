#!/bin/bash
rm /tmp/a
rm /tmp/b
touch /tmp/a
touch /tmp/b
result1=`./bin/Linux/ctrxx $1 1>/tmp/a 2>/tmp/b`
result=`cat /tmp/a /tmp/b`
dest="tests/exp/xx2/test${1:10:4}.exp"
touch $dest
echo "$result" > $dest
echo "Stored result of program in expectation file."
