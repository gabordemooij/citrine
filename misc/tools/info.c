#include <stdio.h>
#include <stdlib.h>
/**
 * Example program that displays the contents of an AST-export.
 * Visualizes the structure of a Citrine Program using the
 * AST-export as input.
 *
 * Usage:
 *
 * ctrXX -x my_program.ctr | info
 */
void read_program(int indent);

void skip_chars(int n) {
	for(int i = 0; i < n; i ++) if (getchar() == EOF) { printf("Error: Unexpected end of file."); exit(1); }
}

void get_next_cell(char* buffer) {                       /* Read the next cell in the stream */
	int i = 0; char x;                                   /* Cells are separated by ; */
	while((x = getchar())!=EOF && x != ';') {
		if (x == EOF) { printf("Error: Unexpected end of file."); exit(1); }
		buffer[i++] = x;
	}
	buffer[i] = 0;
}

void print_buffer_cell(int length) {                      /* Print the contents of Buffer/Value Cell */
	int i = 0; char x;                                    /* Contains the value associated with a node */
	while(i++ < length){                                  /* Print every character as-is (Unicode, 0-byte) */
		x = getchar();
		if (x == EOF) { printf("Error: Unexpected end of file."); exit(1); }
		putchar(x);
	}
	skip_chars(1);                                         /* skip the remaining ; */
}

int read_node(int indent) {                               /* Node format: <TOKEN>;<MODIFIER>;<VALUE LENGTH>;<VALUE>;<SUB NODES..> */
	char token[2]; char value_length[10]; int length = 0; int code = 0;
	get_next_cell(token);                                  /* Read the next Cell */
	if (token[0]==']') return 0;                          /* ] marks the end of a list of sub nodes, return 0 to indicate end */
	code = atoi(token);                                    /* Convert Token Code to integer */
	skip_chars(2);                                         /* Skip modifier (0 = existing var, 1 = declaration, 2 = property of object) and boundary ; */
	for(int i = 0; i < indent; i++) printf(" ");          /* Print indent spaces */
	switch(code) {                                        /* Print description of token */
		case 51: printf("Assignment");         break;     /* 51 = Assignment expression */
		case 52: printf("Message");            break;     /* 52 = Message expression */
		case 53: printf("Unary Message");      break;     /* 53 = Unary message to object (i.e. without any arguments) */
		case 54: printf("Binary Message");     break;     /* 54 = Binary message to object (i.e. with 1 argument) */
		case 55: printf("Keyword Message");    break;     /* 55 = Keyword message to object (i.e. with 1 or more arguments ) */
		case 56: printf("String Literal");     break;     /* 56 = String literal */
		case 57: printf("Object Reference");   break;     /* 57 = Object reference */
		case 58: printf("Numeric Literal");    break;     /* 58 = Number literal */
		case 59: printf("Code Block Literal"); break;     /* 59 = Code Block { .. } */
		case 60: printf("Return Statement");   break;     /* 60 = Code Block Return Symbol */
		case 76: printf("Parameters");         break;     /* 76 = List of Parameters at the start of a Code Block */
		case 77: printf("Instructions");       break;     /* 77 = List of statements inside a Code Block */
		case 78: printf("Parameter");          break;     /* 78 = Parameter in a list of Parameters in a Code Block */
		case 79: printf("End\n"); return 0;   break;     /* 79 = End of Program */
		case 80: printf("Nested Code");        break;     /* 80 = Nested nodes */
		default: printf("???"); exit(1);       break;     /* Unknown node - Error */
	 }
	 get_next_cell(value_length);                          /* Obtain the length of the Buffer Cell */
	 length = atoi(value_length);                          /* Use the length to print the Buffer (fixed number of bytes) */
	 if (length) {                                         /* Only print the buffer if the length > 0 */
		putchar('('); print_buffer_cell(length); putchar(')');
	} else {
		skip_chars(1);                                     /* If no buffer, skip the remaining boundary symbol ; */
	}
	putchar('\n');                                         /* End of line, for readability */
	char nesting = getchar();                              /* [ signals the presence of nested nodes */
	if (nesting=='[') read_program(++indent);              /* Recursively call read_program() to read the nested nodes */
	return 1;                                              /* Return 1 means continue to next node */
}

void read_program(int indent) {                            /* Reads an entire AST-export program */
	int q = 1;                                              /* Read the AST-export */
	while(q == 1) q = read_node(indent);                   /* Reads one node from the AST-export stream */
}

int main(int argc, char* argv[]) {	read_program(0); return 0; }