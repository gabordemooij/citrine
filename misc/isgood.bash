#!/bin/bash
result=`./ctr 2>&1 $1`
dest="${1:0: -4}.exp"
touch $dest
echo "$result" > $dest
echo "Stored result of program in expectation file."
