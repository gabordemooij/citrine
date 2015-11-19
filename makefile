
all:ctr

ctr:world.o lexer.o parser.o walker.o citrine.o
	gcc world.o lexer.o parser.o walker.o citrine.o -lm -o ctr

world.o:
	gcc -c -mtune=native world.c -Wall -o world.o

lexer.o:
	gcc -c -mtune=native lexer.c -Wall -o lexer.o

parser.o:
	gcc -c -mtune=native parser.c -Wall -o parser.o

walker.o:
	gcc -c -mtune=native walker.c -Wall -o walker.o

citrine.o:
	gcc -c -mtune=native citrine.c -Wall -o citrine.o

clean:
	rm -rf *.o ctr
	
