LDFLAGS= -shared

install-libctrpercolator.so: libctrpercolator.so
	install libctrpercolator.so ../../../bin/Linux/mods/percolator/

libctrpercolator.so: percolator.o
	cc ${LDFLAGS} -o libctrpercolator.so percolator.o

percolator.o:
	cc -I . -c percolator.c -Wall -Werror -fPIC -o percolator.o

clean:
	rm -rf *.so
	rm -rf *.o
