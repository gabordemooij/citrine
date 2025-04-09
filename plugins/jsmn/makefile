LDFLAGS= -shared

install-libctrjson.so: libctrjsmn.so
	install libctrjsmn.so ../../bin/Linux/mods/json/libctrjson.so

libctrjsmn.so: jsmn.o jsmn/jsmn.o
	cc ${LDFLAGS} -o libctrjsmn.so jsmn.o jsmn/jsmn.o

jsmn.o:
	cc -c jsmn.c -Wall -I ../../i18n/en -I i18n/xx -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o

jsmn/jsmn.o:
	cd jsmn ; cc -c jsmn.c -Wall -Werror -fpic -DJSMN_STRICT -DJSMN_PARENT_LINKS -o jsmn.o

clean:
	rm -rf *.so
	rm -rf *.o
	rm -rf jsmn/*.o
	rm -rf jsmn/*.so
