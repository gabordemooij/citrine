# Usage example:
# ISO="nl" PACKAGE="media" NAME="libctrmedia.so" make plugin

LDFLAGS= -shared

install-libctrmedia.so: libctrmedia.so
	install libctrmedia.so ../../mods/media/

libctrmedia.so: media.o
	ld ${LDFLAGS} -lcurl -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -o libctrmedia.so media.o

media.o:
	cc -D SDL -D LIBCURL -D FFI $(EXTRACFLAGS) -I . -c media.c -I /usr/include/SDL -I ../../i18n/${ISO} -I i18n/${ISO} -Wall -Werror -fPIC -o media.o

clean:
	rm -rf libctrmedia.so
	rm -rf *.o
