CFLAGS = -g -mtune=native -Wall -D CTRLANG=${ISO}
OBJS = test.o siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o translator.o citrine.o
prefix ?= /usr

.SUFFIXES:	.o .c

all:ctr

install:
	install -d $(DESTDIR)$(prefix)/bin $(DESTDIR)$(prefix)/share/fonts/citrine
	install ./bin/$(shell uname -s)/ctr* $(DESTDIR)$(prefix)/bin
	install ./fonts/Citrine.ttf          $(DESTDIR)$(prefix)/share/fonts/citrine

ctr:	$(OBJS)
	$(CC) $(OBJS) -g -rdynamic -lm -ldl -o ctr
	cp ctr bin/${OS}/ctr${ISO}

.c.o:
	$(CC) $(CFLAGS) -I i18n/${ISO} -c $<

clean:
	rm -rf ${OBJS} ctr
	
