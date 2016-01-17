
all:ctr

ctr: siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o world.o lexer.o parser.o walker.o citrine.o
	gcc siphash.o utf8.o memory.o util.o base.o collections.o file.o system.o world.o lexer.o parser.o walker.o citrine.o -lm -o ctr

siphash.o:
	gcc -c -mtune=native siphash.c -Wall -o siphash.o

utf8.o:
	gcc -c -pedantic-errors -mtune=native utf8.c -Wall -o utf8.o
	
memory.o:
	gcc -c -pedantic-errors -mtune=native memory.c -Wall -o memory.o
	
util.o:
	gcc -c -pedantic-errors -mtune=native util.c -Wall -o util.o

base.o:
	gcc -c -pedantic-errors -mtune=native base.c -Wall -o base.o

system.o:
	gcc -c -pedantic-errors -mtune=native system.c -Wall -o system.o

file.o:
	gcc -c -pedantic-errors -mtune=native file.c -Wall -o file.o

collections.o:
	gcc -c -pedantic-errors -mtune=native collections.c -Wall -o collections.o

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
	
