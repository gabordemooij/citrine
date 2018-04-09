CTRLANG = langUS

CFLAGS = -mtune=native -Wall -D forLinux -D$(CTRLANG)
OBJS = siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o \
       world.o lexer.o parser.o walker.o translator.o citrine.o

.SUFFIXES:	.o .c

all:ctr

install: ctr
	cp ./ctr /usr/bin/ctr

ctr:	$(OBJS)
	$(CC) $(OBJS) -rdynamic -lm -ldl -lbsd -o ctr

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf ${OBJS} ctr
	
