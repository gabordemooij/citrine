LDFLAGS= -shared

install-libctrjson.so: libctrjsmn.so
	install libctrjsmn.so ../../mods/json/libctrjson.so

install-libctrjsonnl.so: libctrjsmnnl.so
	install libctrjsmnnl.so ../../mods/jsonnl/libctrjsonnl.so

install-libctrjsonhy.so: libctrjsmnhy.so
	install libctrjsmnhy.so ../../mods/jsonhy/libctrjsonhy.so

libctrjsmn.so: jsmn.o jsmn/jsmn.o
	cc ${LDFLAGS} -o libctrjsmn.so jsmn.o jsmn/jsmn.o

libctrjsmnnl.so: jsmnnl.o jsmn/jsmn.o
	cc ${LDFLAGS} -o libctrjsmnnl.so jsmnnl.o jsmn/jsmn.o
	
libctrjsmnhy.so: jsmnhy.o jsmn/jsmn.o
	 cc ${LDFLAGS} -o libctrjsmnhy.so jsmnhy.o jsmn/jsmn.o

jsmn.o:
	cc -c jsmn.c -Wall -I ../../i18n/en -I i18n/xx -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o

jsmnnl.o:
	cc -c jsmn.c -Wall -I ../../i18n/nl -I i18n/nl -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmnnl.o
	
jsmnhy.o:
	cc -D langHY -c jsmn.c -Wall -I ../../i18n/hy -I i18n/hy -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmnhy.o

jsmn/jsmn.o:
	cd jsmn ; cc -c jsmn.c -Wall -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o

clean:
	rm -rf *.so
	rm -rf *.o
	rm -rf jsmn/*.o
	rm -rf jsmn/*.so
