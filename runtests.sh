#!/bin/bash
#set -x
#set -v

OS=`uname -s`
LDFLAGS='-shared'
if [ "$OS" = "Darwin" ]; then
  LDFLAGS='-shared -undefined dynamic_lookup'
fi
#Remove .so
find . -name "*.so" -exec rm {} +

#For plugin test, compile Percolator plugin
rm plugins/mock/percolator/libctrpercolator.so
rm mods/percolator/libctrpercolator.so
rm plugins/mockpercolator/percolator.o
cd plugins/mock/percolator;
cc -I . -c percolator.c -Wall -Werror -fPIC -o percolator.o
cc ${LDFLAGS} -o libctrpercolator.so percolator.o
cd ..
cd ..
cd ..
cp plugins/mock/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so

#request test
cd plugins/request/ccgi-1.2;
cc -c ccgi.c -Wall	-Werror -fPIC -o ccgi.o
cc -c prefork.c -Wall -Werror -fPIC -o prefork.o
cd ..

cc -c request.c -Wall -Werror -I ../../i18nsel/xx -D langXX -fPIC -o request.o
cc ${LDFLAGS} -o libctrrequest.so request.o ccgi-1.2/ccgi.o ccgi-1.2/prefork.o

cc -c request.c -Wall -Werror -I ../../i18nsel/nl -D langNL -fPIC -o verzoek.o
cc ${LDFLAGS} -o libctrverzoek.so verzoek.o ccgi-1.2/ccgi.o ccgi-1.2/prefork.o

cd ..
cd ..

cp plugins/request/libctrrequest.so mods/request/libctrrequest.so
cp plugins/request/libctrverzoek.so mods/verzoek/libctrverzoek.so

#json test
cd plugins/jsmn/jsmn;
gcc -c jsmn.c -Wall	-Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o
cd ..
gcc -c jsmn.c -Wall	-I ../../i18n/en -I i18n/xx -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o ; gcc -shared -o libctrjsmn.so jsmn.o jsmn/jsmn.o
gcc -c jsmn.c -Wall	-I ../../i18n/nl -I i18n/nl -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmnnl.o ; gcc -shared -o libctrjsmnnl.so jsmnnl.o jsmn/jsmn.o
cd ..
cd ..
cp plugins/jsmn/libctrjsmn.so mods/json/libctrjson.so
cp plugins/jsmn/libctrjsmnnl.so mods/jsonnl/libctrjsonnl.so


./mk.sh
cp bin/${OS}/ctrxx bin/Generic/ctr

#Add plugin translations
./bin/${OS}/ctrxx -g plugins/request/i18n/en/dictionary.h plugins/request/i18n/nl/dictionary.h >> dict/xxnl.dict


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
		rm tests/tmp/runner7.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner1.ctr 1> tests/tmp/runner7.ctr 2> tests/tmp/terrors7.log
		rm tests/tmp/runner8.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner2.ctr 1> tests/tmp/runner8.ctr 2> tests/tmp/terrors8.log
		rm tests/tmp/runner9.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner3.ctr 1> tests/tmp/runner9.ctr 2> tests/tmp/terrors9.log
		rm tests/tmp/runner10.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner4.ctr 1> tests/tmp/runner10.ctr 2> tests/tmp/terrors10.log
		rm tests/tmp/runner11.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner5.ctr 1> tests/tmp/runner11.ctr 2> tests/tmp/terrors11.log
		rm tests/tmp/runner12.ctr ; ./bin/${OS}/ctrxx -t dict/xxnl.dict tests/tmp/runner6.ctr 1> tests/tmp/runner12.ctr 2> tests/tmp/terrors12.log
		rm tests/tmp/runner13.ctr ; ./bin/${OS}/ctrnl -t dict/nlxx.dict tests/tmp/runner12.ctr 1> tests/tmp/runner13.ctr 2> tests/tmp/terrors13.log
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
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner7.ctr 1>/tmp/a7 2>/tmp/b7
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner8.ctr 1>/tmp/a8 2>/tmp/b8
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner9.ctr 1>/tmp/a9 2>/tmp/b9
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner10.ctr 1>/tmp/a10 2>/tmp/b10
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner11.ctr 1>/tmp/a11 2>/tmp/b11
				echo "test" | ./bin/${OS}/ctrnl tests/tmp/runner12.ctr 1>/tmp/a12 2>/tmp/b12
				echo "test" | ./bin/${OS}/ctrxx tests/tmp/runner13.ctr 1>/tmp/a13 2>/tmp/b13
				echo "test" | ./bin/${OS}/ctrro tests/tmp/runner14.ctr 1>/tmp/a14 2>/tmp/b14
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
	result[8]=`cat /tmp/a8 /tmp/b8`
	result[9]=`cat /tmp/a9 /tmp/b9`
	result[10]=`cat /tmp/a10 /tmp/b10`
	result[11]=`cat /tmp/a11 /tmp/b11`
	result[12]=`cat /tmp/a12 /tmp/b12`
	result[13]=`cat /tmp/a13 /tmp/b13`
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
		for q in {1..6}
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
						exit 1
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
echo ""
echo "All tests passed."
exit 0
