#!/bin/bash
rm /tmp/a
rm /tmp/b
touch /tmp/a
touch /tmp/b
result1=`./bin/Linux/ctrus $1 1>/tmp/a 2>/tmp/b`
result=`cat /tmp/a /tmp/b`
dest="${1:0: -4}.exp"
touch $dest
echo "$result" > $dest
echo "Stored result of program in expectation file."
