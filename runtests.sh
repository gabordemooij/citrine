#!/bin/bash
#set -x
#set -v

CITRINE_MEMORY_LIMIT_MB=10
export CITRINE_MEMORY_LIMIT_MB

LDFLAGS='-shared'
ISO="en" 
export ISO

make clean ; make
EXTRACFLAGS="-D TEST" ISO="en" PACKAGE="gui" NAME="libctrgui.so" make plugin
 
cd build/Linux/bin

unittest() {
	i=$1
	mmode=$2
	CITRINE_MEMORY_MODE=$mmode
	export CITRINE_MEMORY_MODE
	echo "test"  | ./ctren ../../../tests/t-$i.ctr 1>/tmp/out 2>/tmp/err
	if ! test -f ../../../tests/exp/en/test${i}en.exp; then
		if ! touch ../../../tests/exp/en/test${i}en.exp; then
			exit 1
		fi
	fi
	code="$(< ../../../tests/t-$i.ctr)"
	observed="$(< /tmp/out)$(< /tmp/err)"
	expected="$(< ../../../tests/exp/en/test${i}en.exp)"
	diff="$(diff -bBZ /tmp/out ../../../tests/exp/en/test${i}en.exp)"
    if [[  $diff != "" ]]; then
		echo "𐄂 test $i"
		echo "expected:"
		echo "|$expected|"
		echo "observed:"
		echo "|$observed|"
		echo "diff:"
		diff -bBZ ../../../tests/exp/en/test${i}en.exp /tmp/out
		echo "code:"
		echo $code
		read -p "save new test result? (y/n)" answer
		if [[ $answer == "y" ]]; then
			cat /tmp/out > ../../../tests/exp/en/test${i}en.exp
			echo "recorded."
		else
			exit 1
		fi
	else
		echo "✓ test $i | $mmode"
	fi

}

# run tests for linux
for i in $(seq -f "%04g" 1 374);
do
    unittest $i 1
    unittest $i 4
    unittest $i 0
done

