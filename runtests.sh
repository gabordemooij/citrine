#!/bin/bash
#set -x
#set -v

CITRINE_MEMORY_LIMIT_MB=10
export CITRINE_MEMORY_LIMIT_MB




BUILD="${1:-build}"
CI="${2:-full}"

	
LDFLAGS='-shared'
ISO="en" 
EXTRACFLAGS="-D TEST"
PACKAGE="media"


export ISO
export EXTRACFLAGS
export PACKAGE

buildlin(){
if [[ $BUILD == "build" ]]; then
make clean
make
NAME="libctrmedia.so" make testplugin
cp ctr bin/Linux/ctren
fi
}

buildwin(){
if [[ $BUILD == "build" ]]; then
CC=x86_64-w64-mingw32-gcc-win32
DLLTOOL=x86_64-w64-mingw32-dlltool 
export CC
export DLLTOOL
make -f makefile.win64 clean
make -f makefile.win64
NAME="libctrmedia.dll" make -f makefile.win64 testplugin
cp ctr bin/Win64/ctren
fi
}

unittest() {
	i=$1
	mmode=$2
	os=$3
	CITRINE_MEMORY_MODE=$mmode
	export CITRINE_MEMORY_MODE

	if [[ $os = "lin" ]]; then
		echo "test"  | ./bin/Linux/ctren tests/en/t-$i.ctr 1>/tmp/rs 2>/tmp/err
		cat /tmp/rs /tmp/err > /tmp/out
	fi

	if [[ $os = "win" ]]; then
		echo "test"  | wine ./bin/Win64/ctren.exe tests/en/t-$i.ctr 1>/tmp/rs 2>/tmp/err
		cat /tmp/rs /tmp/err > /tmp/out
	fi

	if ! test -f tests/en/exp/test${i}en.exp; then
    		if ! touch tests/en/exp/test${i}en.exp; then
			exit 1
		fi
	fi

	
	skipcode=$(head -n1 tests/en/t-$1.ctr)
	if [[ "$skipcode" == "#Linux" && "$os" != "lin" ]]; then
		echo "SKIP Linux-only test"
		return
	fi
	
	if [[ "$skipcode" == "#Windows" && "$os" != "win" ]]; then
		echo "SKIP Windows-only test"
		return
	fi
	
	if [[ "$skipcode" == "#Full" && "$CI" != "full" ]]; then
		echo "SKIP CI-full test"
		return
	fi

	code="$(< tests/en/t-$i.ctr)"
	observed="$(< /tmp/out)"
	expected="$(< tests/en/exp/test${i}en.exp)"
	diff="$(diff -bBZ /tmp/out tests/en/exp/test${i}en.exp)"
    if [[  $diff != "" ]]; then
		echo "𐄂 test $i"
		echo $skipcode
		echo "expected:"
		echo "|$expected|"
		echo "observed:"
		echo "|$observed|"
		echo "diff:"
		diff -bBZ tests/en/exp/test${i}en.exp /tmp/out
		echo "code:"
		echo $code
		read -p "save new test result? (y/n)" answer
		if [[ $answer == "y" ]]; then
			cat /tmp/out > tests/en/exp/test${i}en.exp
			echo "recorded."
		else
			exit 1
		fi
	else
		echo "✓ test $i | $mmode"
	fi

}

# select range
FROM=1
TIL=624

# run tests for linux
buildlin
for i in $(seq -f "%04g" $FROM $TIL);
do
    unittest $i 1 lin
    unittest $i 4 lin
    unittest $i 0 lin
done

if [[ $CI == "full" ]]; then
# run tests for win
buildwin
for i in $(seq -f "%04g" $FROM $TIL);
do
    unittest $i 1 win
    unittest $i 4 win
    unittest $i 0 win
done
fi
