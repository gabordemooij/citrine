
all:ctr

ctr:tools.o lexer.o parser.o walker.o citrine.o
	gcc tools.o lexer.o parser.o walker.o citrine.o -lm -o ctr -lbsd

tools.o:
	gcc -lbsd -c tools.c -Wall -o tools.o

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
	
