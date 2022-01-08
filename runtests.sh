#!/bin/bash
#set -x
#set -v

CITRINE_MEMORY_LIMIT_MB=8
CITRINE_MEMORY_MODE=1
export CITRINE_MEMORY_LIMIT_MB
export CITRINE_MEMORY_MODE

#Determine which makefile to use
OS=$(uname -s)
LDFLAGS='-shared'
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" ]; then
	MAKEFILE=makefile.bsd
elif [ "$OS" = "Darwin" ]; then
	MAKEFILE=makefile.bsd
	LDFLAGS='-shared -undefined dynamic_lookup'
elif [ "$OS" = "Haiku" ]; then
	MAKEFILE=makefile.haiku
else
	MAKEFILE=makefile
fi

#Remove .so
find . -name "*.so" -exec rm {} +

#Run makefiles of plugins
make plugin PACKAGE="request" NAME="libctrrequest.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="request" NAME="libctrverzoek.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="mock/percolator" NAME="libctrpercolator.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjson.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjsonnl.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjsonhy.so" LDFLAGS=${LDFLAGS}

#Remove old dicts
rm dict/*

#Run regular makefiles through build script
./mk.sh
cp bin/${OS}/ctrxx bin/Generic/ctr

#Add plugin translations
./bin/${OS}/ctrxx -g plugins/request/i18n/en/dictionary.h plugins/request/i18n/nl/dictionary.h >> dict/xxnl.dict

#Extra for arabic comma
cat i18n/ar/extra2.dict >> dict/arxx.dict

j=1
for i in $(find tests -name 'test*.ctr'); do
	touch /tmp/a
	touch /tmp/b
	touch /tmp/c
	touch /tmp/d
	touch /tmp/transl.ctr
	fitem=$i
	echo "[suite: $fitem]";
	base=`basename ${fitem}`;
	fexpect="tests/exp/xx2/${base%%.ctr}.exp"
	fexpectnl="tests/exp/nl/${base%%.ctr}nl.exp"
	#test for every GC mode (0/1/4/8/9/12) mode x every language (EN/NL)
	rm tests/tmp/runner1.ctr ; echo "Program tidiness: 0." > tests/tmp/runner1.ctr ; cat ${fitem} >> tests/tmp/runner1.ctr
	rm tests/tmp/runner2.ctr ; echo "Program tidiness: 1." > tests/tmp/runner2.ctr ; cat ${fitem} >> tests/tmp/runner2.ctr
	rm tests/tmp/runner3.ctr ; echo "Program tidiness: 4." > tests/tmp/runner3.ctr ; cat ${fitem} >> tests/tmp/runner3.ctr
	rm tests/tmp/runner4.ctr ; echo "Program tidiness: 8." > tests/tmp/runner4.ctr ; cat ${fitem} >> tests/tmp/runner4.ctr
	rm tests/tmp/runner5.ctr ; echo "Program tidiness: 9." > tests/tmp/runner5.ctr ; cat ${fitem} >> tests/tmp/runner5.ctr
	rm tests/tmp/runner6.ctr ; echo "Program tidiness: 12." > tests/tmp/runner6.ctr ; cat ${fitem} >> tests/tmp/runner6.ctr
	directive=`head -n1 $fitem`
	echo "[translating...]"
	if [ "$directive" != "‘SINGLE_LANGUAGE’." ]; then
		rm tests/tmp/runner7a.ctr ; ./bin/${OS}/ctrxx -t dict/xxar.dict tests/tmp/runner1.ctr 1> tests/tmp/runner7a.ctr 2> tests/tmp/terrors7a.log
		rm tests/tmp/runner7.ctr ; ./bin/${OS}/ctrar -t dict/arxx.dict tests/tmp/runner7a.ctr 1> tests/tmp/runner7.ctr 2> tests/tmp/terrors7.log
		for ISO in $(ls i18nsel)
		do
			rm tests/tmp/runner${ISO}.ctr ; ./bin/${OS}/ctrxx -t dict/xx${ISO}.dict tests/tmp/runner1.ctr 1>tests/tmp/runner${ISO}.ctr 2>tests/tmp/terrors${ISO}.log
		done
	fi
	echo "[running...]";
	if [ "$directive" == "“NL-ONLY”." ]; then
		echo "test" | ./bin/${OS}/ctrnl ${fitem} 1>/tmp/a0 2>/tmp/b0
	else
		echo "test" | ./bin/${OS}/ctrxx ${fitem} 1>/tmp/a0 2>/tmp/b0
		if [ "$directive" != "‘SINGLE_RUN’." ]; then
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner1.ctr 1>/tmp/a1 2>/tmp/b1
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner2.ctr 1>/tmp/a2 2>/tmp/b2
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner3.ctr 1>/tmp/a3 2>/tmp/b3
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner4.ctr 1>/tmp/a4 2>/tmp/b4
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner5.ctr 1>/tmp/a5 2>/tmp/b5
			echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner6.ctr 1>/tmp/a6 2>/tmp/b6
			if [ "$directive" != "‘SINGLE_LANGUAGE’." ]; then
				echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner7.ctr 1>/tmp/a7 2>/tmp/b7
				for ISO in $(ls i18nsel)
				do
					echo "test" | ./bin/${OS}/ctr${ISO} tests/tmp/runner${ISO}.ctr 1>/tmp/a${ISO} 2>/tmp/b${ISO}
				done
			fi
		fi
	fi
	result[0]=`cat /tmp/a0 /tmp/b0`
	result[1]=`cat /tmp/a1 /tmp/b1`
	result[2]=`cat /tmp/a2 /tmp/b2`
	result[3]=`cat /tmp/a3 /tmp/b3`
	result[4]=`cat /tmp/a4 /tmp/b4`
	result[5]=`cat /tmp/a5 /tmp/b5`
	result[6]=`cat /tmp/a6 /tmp/b6`
	result[7]=`cat /tmp/a7 /tmp/b7`
	expected=`cat $fexpect`
	expectednl=$expected
	if [ -f $fexpectnl ]; then
		expectednl=`cat $fexpectnl`
	fi
	if [ "$directive" = "“NL-ONLY”." ]; then
		directive="‘SINGLE_RUN’."
		expected="$expectednl"
	fi
	if [ "${result[0]}" = "$expected" ]; then
		echo -n "[✓$j*]"
		j=$((j+1))
	else
		echo "FAIL."
		echo "EXPECTED:"
		echo "$expected"
		echo ""
		echo "BUT GOT:"
		echo "${result[0]}"
		exit 1
	fi
	if [ "$directive" != "‘SINGLE_RUN’." ]; then
	if [ "$directive" != "‘SINGLE_LANGUAGE’." ]; then
		for q in {1..7}
		do
			if [ "${result[$q]}" = "$expected" ]; then
				echo -n "[✓$j]"
				j=$((j+1))
			else
				echo "FAIL."
				echo "EXPECTED:"
				echo "$expected"
				echo ""
				echo "BUT GOT:"
				echo "${result[$q]}"
				exit 1
			fi
		done
		for ISO in $(ls i18nsel)
		do
			if [ $ISO != 'xx' ]; then
				actual=`cat /tmp/a${ISO} /tmp/b${ISO}`
				langexp="tests/exp/${ISO}/${base%%.ctr}${ISO}.exp"
				if [ -f $langexp ]; then
					expecting=`cat $langexp`
					if [ "$actual" = "$expecting" ]; then
						echo -n "[✓$j|${ISO}]"
						j=$((j+1))
					else
						echo "FAIL for Language: ${ISO}"
						echo "EXPECTED:"
						echo "$expecting"
						echo ""
						echo "BUT GOT:"
						echo "$actual"
						if [ "$1" = "--correct" ]; then
							echo "Correct expectation file? y/n"
							read accept
							if [ "$accept" = "y" ]; then
								rm $langexp
								touch $langexp
								echo "$actual" > $langexp
								echo "OK, replaced expectation."
							fi
						else
							exit 1
						fi
					fi
				else
					echo "> Missing: ${ISO}!"
					if [ "$1" = "--record" ]; then
						echo "=========================== CHECK RESULTS:"
						echo "EXPECTED BASE:"
						cat /tmp/a0
						echo ""
						echo "ISO RESULT ${ISO} (${langexp}):"
						echo "$actual"
						echo "---------------------------"
						echo "is this correct, create expectation file? y/n"
						read accept
						if [ "$accept" = "y" ]; then
							touch $langexp
							echo "$actual" > $langexp
							echo "OK, file created."
						fi
					fi
					if [ "$1" = "--record2" ]; then
						expectingnew=`cat /tmp/a0`
						if [ "$actual" = "$expectingnew" ]; then
							echo "SAME RESULT OK"
							touch $langexp
							echo "$actual" > $langexp
							echo "OK, file created."
						else
							echo "=========================== CHECK DIFF RESULTS:"
							echo "EXPECTED BASE:"
							cat /tmp/a0
							echo ""
							echo "ISO RESULT ${ISO} (${langexp}):"
							echo "$actual"
							echo "---------------------------"
							echo "is this correct, create expectation file? y/n"
							read accept
							if [ "$accept" = "y" ]; then
								touch $langexp
								echo "$actual" > $langexp
								echo "OK, file created."
							fi
						fi
					fi
				fi
			fi
		done
		if [ "${result[13]}" = "$expected" ]; then
			echo -n "[✓$j!]"
			j=$((j+1))
		fi
	fi
	fi
	echo "[done]"
done
echo "All tests passed."
exit 0
