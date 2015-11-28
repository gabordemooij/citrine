
all:ctr

ctr: siphash.o world.o lexer.o parser.o walker.o citrine.o
	gcc siphash.o world.o lexer.o parser.o walker.o citrine.o -lm -o ctr

siphash.o:
	gcc -c -mtune=native siphash.c -Wall -o siphash.o

world.o:
	gcc -c -pedantic-errors -mtune=native world.c -Wall -o world.o

lexer.o:
	gcc -c -pedantic-errors -mtune=native lexer.c -Wall -o lexer.o

parser.o:
	gcc -c -pedantic-errors -mtune=native parser.c -Wall -o parser.o

walker.o:
	gcc -c -pedantic-errors -mtune=native walker.c -Wall -o walker.o

citrine.o:
	gcc -c -pedantic-errors -mtune=native citrine.c -Wall -o citrine.o

clean:
	rm -rf *.o ctr
	
