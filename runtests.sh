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
rm plugins/percolator/libctrpercolator.so
rm mods/percolator/libctrpercolator.so
cd plugins/percolator;
cc -c percolator.c -Wall -Werror -fPIC -o percolator.o
cc ${LDFLAGS} -o libctrpercolator.so percolator.o
cd ..
cd ..
cp plugins/percolator/libctrpercolator.so mods/percolator/libctrpercolator.so


rm plugins/percolator/libctrpercolator.so
cd plugins/percolator;
cc -DlangNL -c percolator.c -Wall -Werror -fPIC -o koffiezetter.o
cc ${LDFLAGS} -o libctrkoffiezetter.so koffiezetter.o
cd ..
cd ..
cp plugins/percolator/libctrkoffiezetter.so mods/koffiezetter/libctrkoffiezetter.so


#request test
cd plugins/request/ccgi-1.2;
cc -c ccgi.c -Wall	-Werror -fPIC -o ccgi.o
cc -c prefork.c -Wall -Werror -fPIC -o prefork.o
cd ..
cc -c request.c -Wall -Werror -fPIC -o request.o
cc ${LDFLAGS} -o libctrrequest.so request.o ccgi-1.2/ccgi.o ccgi-1.2/prefork.o
cd ..
cd ..
cp plugins/request/libctrrequest.so mods/request/libctrrequest.so

#json test
cd plugins/jsmn/jsmn;
gcc -c jsmn.c -Wall	-Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o
cd ..
gcc -c jsmn.c -Wall	-Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o ; gcc -shared -o libctrjsmn.so jsmn.o jsmn/jsmn.o
cd ..
cd ..
cp plugins/jsmn/libctrjsmn.so mods/json/libctrjson.so

#curl test
rm plugins/curl/src/libctrcurl.so
rm plugins/curl/src/curl.o
rm mods/curl/libctrcurl.so
cd plugins/curl/src;
gcc -DCTRPLUGIN_CURL_CURLOPT_URL -c curl.c -Wall	-Werror -fpic -o curl.o
gcc -shared -o libctrcurl.so curl.o -lcurl
cd ..
cd ..
cd ..
cp plugins/curl/src/libctrcurl.so mods/curl/libctrcurl.so

#pg test
rm plugins/pg/libctrpg.so
rm plugins/pg/pg.o
rm mods/pg/libctrpg.so
cd plugins/pg;
gcc -c pg.c -Wall -I/usr/include/postgresql/ -lpq -Werror -fpic -o pg.o
gcc -shared -o libctrpg.so pg.o -I/usr/include/postgresql/ -lpq
cd ..
cd ..
cp plugins/pg/libctrpg.so mods/pg/libctrpg.so

#sodium
rm plugins/crypt/libctrpassword.so
rm plugins/crypt/crypt.o
rm mods/password/libctrpassword.so
cd plugins/crypt;
gcc -c crypt.c -Wall -I/usr/local/include/  -Werror -fpic -o crypt.o
gcc -shared -o libctrpassword.so crypt.o -L/usr/local/lib/ -lsodium
cd ..
cd ..
cp plugins/crypt/libctrpassword.so mods/password/libctrpassword.so
./mk.sh
j=1
for i in $(find tests -name 'test*.ctr'); do
	touch /tmp/a
	touch /tmp/b
	touch /tmp/c
	touch /tmp/d
	touch /tmp/transl.ctr
	fitem=$i
	echo "[suite: $fitem]";
	fexpect="${i%%.ctr}.exp"
	#test for every GC mode (0/1/4/8/9/12) mode x every language (EN/NL)
	rm tests/runner1.ctr ; echo "Broom tidiness: 0." > tests/runner1.ctr ; cat ${fitem} >> tests/runner1.ctr
	rm tests/runner2.ctr ; echo "Broom tidiness: 1." > tests/runner2.ctr ; cat ${fitem} >> tests/runner2.ctr
	rm tests/runner3.ctr ; echo "Broom tidiness: 4." > tests/runner3.ctr ; cat ${fitem} >> tests/runner3.ctr
	rm tests/runner4.ctr ; echo "Broom tidiness: 8." > tests/runner4.ctr ; cat ${fitem} >> tests/runner4.ctr
	rm tests/runner5.ctr ; echo "Broom tidiness: 9." > tests/runner5.ctr ; cat ${fitem} >> tests/runner5.ctr
	rm tests/runner6.ctr ; echo "Broom tidiness: 12." > tests/runner6.ctr ; cat ${fitem} >> tests/runner6.ctr
	echo "[translating...]"
	rm tests/runner7.ctr ; ./ctr -t ennl.dict tests/runner1.ctr 1> tests/runner7.ctr 2> tests/terrors7.log
	rm tests/runner8.ctr ; ./ctr -t ennl.dict tests/runner2.ctr 1> tests/runner8.ctr 2> tests/terrors8.log
	rm tests/runner9.ctr ; ./ctr -t ennl.dict tests/runner3.ctr 1> tests/runner9.ctr 2> tests/terrors9.log
	rm tests/runner10.ctr ; ./ctr -t ennl.dict tests/runner4.ctr 1> tests/runner10.ctr 2> tests/terrors10.log
	rm tests/runner11.ctr ; ./ctr -t ennl.dict tests/runner5.ctr 1> tests/runner11.ctr 2> tests/terrors11.log
	rm tests/runner12.ctr ; ./ctr -t ennl.dict tests/runner6.ctr 1> tests/runner12.ctr 2> tests/terrors12.log
	echo "[running...]";
	echo "test" | ./ctr ${fitem} 1>/tmp/a0 2>/tmp/b0
	echo "test" | ./ctr tests/runner1.ctr 1>/tmp/a1 2>/tmp/b1
	echo "test" | ./ctr tests/runner2.ctr 1>/tmp/a2 2>/tmp/b2
	echo "test" | ./ctr tests/runner3.ctr 1>/tmp/a3 2>/tmp/b3
	echo "test" | ./ctr tests/runner4.ctr 1>/tmp/a4 2>/tmp/b4
	echo "test" | ./ctr tests/runner5.ctr 1>/tmp/a5 2>/tmp/b5
	echo "test" | ./ctr tests/runner6.ctr 1>/tmp/a6 2>/tmp/b6
	echo "test" | ./ctrnl tests/runner7.ctr 1>/tmp/a7 2>/tmp/b7
	echo "test" | ./ctrnl tests/runner8.ctr 1>/tmp/a8 2>/tmp/b8
	echo "test" | ./ctrnl tests/runner9.ctr 1>/tmp/a9 2>/tmp/b9
	echo "test" | ./ctrnl tests/runner10.ctr 1>/tmp/a10 2>/tmp/b10
	echo "test" | ./ctrnl tests/runner11.ctr 1>/tmp/a11 2>/tmp/b11
	echo "test" | ./ctrnl tests/runner12.ctr 1>/tmp/a12 2>/tmp/b12
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
	expected=`cat $fexpect`
	if [ "${result[0]}" = "$expected" ]; then
		echo -n "[✓$j*]"
		j=$((j+1))
	else
		echo "FAIL."
		echo "EXPECTED:"
		echo $expected
		echo ""
		echo "BUT GOT:"
		echo $result[0]
		exit 1
	fi
	directive=`head -n1 $fitem`
    if [ "$directive" != "'SINGLE_LANGUAGE'." ]; then
		for q in {1..12}
		do
			if [ "${result[$q]}" = "$expected" ]; then
				echo -n "[✓$j]"
				j=$((j+1))
			else
				echo "FAIL."
				echo "EXPECTED:"
				echo $expected
				echo ""
				echo "BUT GOT:"
				echo "${result[$q]}"
				exit 1
			fi
		done
	fi
	echo "[done]"
done
echo ""
echo "All tests passed."
exit 0
