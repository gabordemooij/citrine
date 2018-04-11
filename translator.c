#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citrine.h"

int debug = 0;
struct ctr_dict {
	char type;
	char* word;
	ctr_size wordLength;
	char* translation;
	ctr_size translationLength;
	struct ctr_dict* next;
};

typedef struct ctr_dict ctr_dict;

void ctr_translate_generate_dicts(char* hfile1, char* hfile2) {
	FILE* f1 = fopen(hfile1, "r");
	FILE* f2 = fopen(hfile2, "r");
	char* word = calloc(80, 1);
	char* translation = calloc(80, 1);
	char* key1  = calloc(80, 1);
	char* key2  = calloc(80, 1);
	int lineCounter = 0;
	while( 
		fscanf( f1, "#define %80s \"%80[^\"]\"\n", key1, word ) > 0 &&
		fscanf( f2, "#define %80s \"%80[^\"]\"\n", key2, translation) > 0
	) {
		if (strlen(key1)!=strlen(key2)) {
			printf("Error: key mismatch %s %s on line %d\n", key1, key2, lineCounter);
			exit(1);
		}
		if (strncmp(key1, key2, strlen(key1))!=0) {
			printf("Error: key mismatch %s %s on line %d\n", key1, key2, lineCounter);
			exit(1);
		}
		printf("t \"%s\" \"%s\"\n", word, translation);
		lineCounter++;
	}
	close(f1);
	close(f2);
}

ctr_dict* ctr_translate_load_dictionary() {
	FILE* file = fopen(ctr_mode_dict_file,"r");
	char  translationType;
	char* word = calloc(80, 1);
	char* translation = calloc(80, 1);
	int wordLength = 0;
	int translationLength = 0;
	ctr_dict* entry;
	ctr_dict* previousEntry = NULL;
	int i;
	while( fscanf( file, "%c \"%80[^\"]\" \"%80[^\"]\"\n", &translationType, word, translation) > 0 ) {
		entry = (ctr_dict*) calloc( sizeof(ctr_dict), 1 );
		entry->type = translationType;
		entry->wordLength = strlen(word);
		entry->translationLength = strlen(translation);
		entry->word = calloc( entry->wordLength, 1 );
		entry->translation = calloc( entry->translationLength, 1 );
		memcpy(entry->word, word, entry->wordLength);
		memcpy(entry->translation, translation, entry->translationLength);
		if (previousEntry) {
			entry->next = previousEntry;
		} else {
			entry->next = NULL;
		}
		previousEntry = entry;
	}
	close(file);
	return entry;
}

void ctr_translate_unload_dictionary(ctr_dict* dictionary) {
	ctr_dict* entry = dictionary;
	ctr_dict* previousEntry;
	while( entry ) {
		free(entry->word);
		free(entry->translation);
		previousEntry = entry;
		entry = entry->next;
		free(previousEntry);
	}
}

int ctr_translate_translate(char* v, ctr_size l, ctr_dict* dictionary, char context, char* remainder) {
	int found = 0;
	ctr_dict* entry;
	entry = dictionary;
	while( entry ) {
		ctr_size ml;
		ml = entry->wordLength;
		if ( l == entry->wordLength && context == entry->type && strncmp( entry->word, v, ml ) == 0 ) {
			if (context == 't') {
				int i = 0;
				for (i = 0; i<entry->translationLength; i++) {
					fwrite(entry->translation + i,1,1,stdout);
					if (*(entry->translation + i)==':') {
						if (debug) printf("[snip...]");
						memcpy(remainder,entry->translation+i+1,(entry->translationLength-i));
						break;
					}
				}
			} else {
				fwrite(entry->translation, entry->translationLength, 1,stdout);
			}
			found = 1;
			break;
		}
		entry = entry->next;
	}
	return found;
}

void ctr_translate_program(char* prg, char* programPath) {
	ctr_dict* entry;
	ctr_dict* dictionary = ctr_translate_load_dictionary();
	ctr_clex_set_ignore_modes(1);
	ctr_clex_load(prg);
	int t;
	t = ctr_clex_tok();
	char* p;
	int partCount = 0;
	p = prg;
	char* e;
	ctr_size l;
	int n = 1000;
	int j = 0;
	char* notes[1000];
	char* parts[1000];
	int noteCount = 0;
	int springOverDeKomma = 0;
	while ( 1 ) {
		springOverDeKomma = 0;
		if ( t == CTR_TOKEN_FIN ) {
			e = ctr_clex_code_pointer();
			l =   ctr_clex_tok_value_length();
			fwrite(p, ((e - l) - p),1, stdout);
			fwrite(e-l, l, 1, stdout);
			break;
		}
		else if ( t == CTR_TOKEN_QUOTE ) {
			if (ctr_string_interpolation) {
				ctr_string_interpolation = 0;
			}
			char* s = ctr_clex_readstr();
			l =  ctr_clex_tok_value_length(s);
			e = ctr_clex_code_pointer();
			if (ctr_string_interpolation) {
				e -= 3;
			}
			char* v = ctr_clex_tok_value();
			fwrite(p, e-p-l, 1, stdout);
			if (!ctr_translate_translate(s,l,dictionary,'s',NULL)) {
				fwrite(s,l,1,stdout);
			}
			if (ctr_string_interpolation) {
				l += 3;
			} else {
				ctr_clex_tok();
			}
			p = e;
		} 
		else if ( t == CTR_TOKEN_REF) {
			if (debug) printf("{");
			e = ctr_clex_code_pointer();
			l =   ctr_clex_tok_value_length();
			char* v = ctr_clex_tok_value();
			int found = 0;		
			fwrite(p, ((e - l ) - p),1, stdout);
			if (debug) printf("|");
			noteCount = 0;
			/* is this part of a keyword message (end with colon?) */
			if (*(e)==':') {
				if (debug) printf("*");
				springOverDeKomma = 1;
				int nesting = 0;
				int blocks = 0;
				int quote = 0;
				int comment = 0;
				int i = 1;
				int q = 0;
				char* message = calloc(80,1);
				memcpy(message, e-l,l+1);
				v = message;
				if (debug) printf("[msg=%s]",message);
				while(e+i < ctr_eofcode) {
					if (*(e+i) == '(') nesting++;
					else if (*(e+i) == ')') nesting--;
					else if (*(e+i) == '{') blocks++;
					else if (*(e+i) == '}') blocks--;
					else if (!quote && *(e+i) == '\'') quote = 1;
					else if (quote && *(e+i) == '\'') quote = 0;
					else if (!comment && *(e+i) == '#') comment = 1;
					else if (comment && *(e+i) == '\n') comment = 0;
					else if (!nesting && !quote && !comment && !blocks) {
						if (*(e+i)=='.' || *(e+i)==')') {
							if (debug) printf("[ends> %d ]", i);
							break;
						}
						if (*(e+i)==':') {
							if (debug) printf("[next> %p ]", e+i);
							notes[j++] = e+i;
							noteCount++;
							/* back-scan */
							for(q=0; q<80; q++) {
								char backScanChar = *(e+i-q);
								if (
									backScanChar == '\n'||
									backScanChar == '\t'||
									backScanChar == ' ' ||
									backScanChar == ')' ||
									backScanChar == '}'
								) {
									memcpy(message+l+1,e+i-q+1, (e+i+1)-(e+i-q+1));
									if (debug) printf("[msgp=%s]",message);
									l += ((e+i+1)-(e+i-q+1));
									/* now we found the message */
									v = message;
									break;
								}
							}
						}
					}
					i++;
				}
				l++;
			}
			if (debug) printf("[p=%p]",e);
			int k = 0;
			int usedPart = 0;
			for (k=0; k<j; k++) {
				if (e == notes[k] && parts[k]) {
					if (debug) printf("[*note=%s]",parts[k]);
					fwrite(parts[k], strlen(parts[k]),1,stdout);
					usedPart = 1;
				}
			}
			if (debug) printf("[to transl=%s/%d]", v,l);
			char* remainder = calloc(80,1);
			if (!usedPart) {
				if (!ctr_translate_translate( v, l, dictionary, 't', remainder )) {
					fwrite(e-l, l, 1, stdout);
				} else {
					if (debug) printf("[r=%s]", remainder);
					/* we have notes, so disect the remainder */
					if (noteCount>0) {
						if (debug) printf("**");
						int s;
						for(partCount=j-noteCount; partCount<j; partCount++) {
							parts[partCount] = calloc(80,1);
						}
						partCount = j-noteCount;
						if (debug) printf("[nc=%d %d]",noteCount, partCount);
						int t;
						t = 0;
						for(s=0; s<strlen(remainder); s++) {
							*(parts[partCount] + (t++)) = *(remainder+s); 
							if (*(remainder + s) == ':') {
								t = 0;
								partCount++;
							}
						}
						for(partCount=j-noteCount; partCount<noteCount; partCount++) {
							if (debug) printf("[part %d %s]",partCount, parts[partCount]);
						}
					}
				}
			}
			if (debug) printf("}");
			if (springOverDeKomma) e++;
			p=e;
		}
		else {
			e = ctr_clex_code_pointer();
			fwrite(p, e-p,1,stdout);
			p=e;	
		}
		t = ctr_clex_tok();	
	}
	ctr_translate_unload_dictionary( dictionary );
}
