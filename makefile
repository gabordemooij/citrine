CFLAGS = -g -mtune=native -Wall -D CTRLANG=${ISO}
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o translator.o citrine.o

.SUFFIXES:	.o .c

all:ctr

install: ctr
	cp ./ctr /usr/bin/ctr

ctr:	$(OBJS)
	$(CC) $(OBJS) -g -rdynamic -lm -ldl -o ctr
	cp ctr bin/${OS}/ctr
	cp ctr bin/${OS}/ctr${ISO}

.c.o:
	$(CC) $(CFLAGS) -I i18n/${ISO} -c $<

clean:
	rm -rf ${OBJS} ctr
	
