LDFLAGS= -shared

install-libctrverzoek.so: libctrverzoek.so
	install libctrverzoek.so ../../mods/verzoek

install-libctrrequest.so: libctrrequest.so
	install libctrrequest.so ../../mods/request

libctrverzoek.so: verzoek.o ccgi-1.2/ccgi.o
	cc ${LDFLAGS} -o libctrverzoek.so verzoek.o ccgi-1.2/ccgi.o

libctrrequest.so: request.o ccgi-1.2/ccgi.o
	cc ${LDFLAGS} -o libctrrequest.so request.o ccgi-1.2/ccgi.o

verzoek.o:
	cc -c request.c -Wall -Werror -I ../../i18n/nl -D langNL -fPIC -o verzoek.o

request.o:
	cc -c request.c -Wall -Werror -I ../../i18n/xx -D langXX -fPIC -o request.o

ccgi-1.2/ccgi.o:
	cc -c ccgi-1.2/ccgi.c -Wall	-Werror -fPIC -o ccgi-1.2/ccgi.o
	
clean:
	rm -rf *.so
	rm -rf *.o
	rm -rf ccgi-1.2/*.o

