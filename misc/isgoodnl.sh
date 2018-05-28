#!/bin/bash
rm /tmp/a
rm /tmp/b
touch /tmp/a
touch /tmp/b
./ctr -t ennl.dict $1 > /tmp/r.ctr
result1=`./ctrnl /tmp/r.ctr 1>/tmp/a 2>/tmp/b`
result=`cat /tmp/a /tmp/b`
dest="${1:0: -4}nl.exp"
touch $dest
echo "$result" > $dest
echo "Stored result of program in expectation file."
