#!/bin/sh

tests=$(find t -name '*.ctr')
for test in $tests; do
	echo :=======Test: $test=======:
	echo -n :=============
	for i in `seq 1 ${#test}`; do
		echo -n =
	done
	echo =======:
	ctr $test
	echo -n :=============
	for i in `seq 1 ${#test}`; do
		echo -n =
	done
	echo =======:
	echo
done
