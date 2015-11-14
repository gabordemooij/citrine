
all:ctr

ctr:world.o lexer.o parser.o walker.o citrine.o
	gcc lib/utf8proc-1.3.1/utf8proc.o world.o lexer.o parser.o walker.o citrine.o -lm -o ctr

world.o:
	gcc -c world.c -Wall -o world.o

lexer.o:
	gcc -c lexer.c -Wall -o lexer.o

parser.o:
	gcc -c parser.c -Wall -o parser.o

walker.o:
	gcc -c walker.c -Wall -o walker.o

citrine.o:
	gcc -c citrine.c -Wall -o citrine.o

clean:
	rm -rf *.o ctr
	
