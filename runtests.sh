#!/bin/bash
#set -x
#set -v

CITRINE_MEMORY_LIMIT_MB=10
export CITRINE_MEMORY_LIMIT_MB

BUILD="${1:-build}"

if [[ $BUILD == "build" ]]; then

LDFLAGS='-shared'
ISO="en" 
export ISO

make clean ; make
make -f makefile.win64 clean ; make -f makefile.win64
EXTRACFLAGS="-D TEST"
PACKAGE="gui"
export EXTRACFLAGS
export PACKAGE

make plugin-clean
NAME="libctrgui.so" make plugin
make -f makefile.win64 plugin-clean
NAME="libctrgui.dll" make -f makefile.win64 plugin

fi


unittest() {
	i=$1
	mmode=$2
	os=$3
	CITRINE_MEMORY_MODE=$mmode
	export CITRINE_MEMORY_MODE

	if [[ $os = "lin" ]]; then
		echo "test"  | ./ctren ../../../tests/t-$i.ctr 1>/tmp/rs 2>/tmp/err
		cat /tmp/rs /tmp/err > /tmp/out
	fi

	if [[ $os = "win" ]]; then
		echo "test"  | wine ./ctren.exe ../../../tests/t-$i.ctr 1>/tmp/rs 2>/tmp/err
		cat /tmp/rs /tmp/err > /tmp/out
	fi

	if ! test -f ../../../tests/exp/en/test${i}en.exp; then
		if ! touch ../../../tests/exp/en/test${i}en.exp; then
			exit 1
		fi
	fi

	
	skipcode=$(head -n1 ../../../tests/t-$1.ctr)
	if [[ "$skipcode" == "#Linux" && "$os" != "lin" ]]; then
		echo "SKIP Linux-only test"
		return
	fi
	
	if [[ "$skipcode" == "#Windows" && "$os" != "win" ]]; then
		echo "SKIP Windows-only test"
		return
	fi

	code="$(< ../../../tests/t-$i.ctr)"
	observed="$(< /tmp/out)"
	expected="$(< ../../../tests/exp/en/test${i}en.exp)"
	diff="$(diff -bBZ /tmp/out ../../../tests/exp/en/test${i}en.exp)"
    if [[  $diff != "" ]]; then
		echo "𐄂 test $i"
		echo $skipcode
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
pushd build/Linux/bin
for i in $(seq -f "%04g" 1 597);
do
    unittest $i 1 lin
    unittest $i 4 lin
    unittest $i 0 lin
done
popd

# run tests for win
pushd build/Win64/bin
for i in $(seq -f "%04g" 1 597);
do
    unittest $i 1 win
    unittest $i 4 win
    unittest $i 0 win
done
popd

