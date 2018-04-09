#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include "citrine.h"

struct ctr_dict {
	char type;
	char* word;
	ctr_size wordLength;
	char* translation;
	ctr_size translationLength;
	struct ctr_dict* next;
};

typedef struct ctr_dict ctr_dict;

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
	while( fscanf( file, "%c %s %s\n", &translationType, word, translation) > 0 ) {
		entry = (ctr_dict*) calloc( sizeof(ctr_dict), 1 );
		entry->type = translationType;
		entry->wordLength = strlen(word);
		entry->translationLength = strlen(translation);
		entry->word = calloc( entry->wordLength, 1 );
		entry->translation = calloc( entry->translationLength, 1 );
		for(i=0; i<entry->wordLength; i++) {
			char c = word[i];
			if (c == '~') c = ' ';
			entry->word[i] = c;
		}
		for(i=0; i<entry->translationLength; i++) {
			char c = translation[i];
			if (c == '~') c = ' ';
			entry->translation[i] = c;
		}
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


int ctr_translate_translate(char* v, ctr_size l, ctr_dict* dictionary, char context) {
	int found = 0;
	ctr_dict* entry;
	entry = dictionary;
	while( entry ) {
		ctr_size ml;
		ml = entry->wordLength;
		if ( l == entry->wordLength && context == entry->type && strncmp( entry->word, v, ml ) == 0 ) {
			fwrite(entry->translation, entry->translationLength, 1,stdout);
			found = 1;
			break;
		}
		entry = entry->next;
	}

	return found;
}

void ctr_translate_program(char* prg, char* pathString) {
	ctr_dict* entry;
	ctr_dict* dictionary = ctr_translate_load_dictionary();
	ctr_clex_set_ignore_modes(1);
	ctr_clex_load(prg);
	int t;
	t = ctr_clex_tok();
	char* p;
	p = prg;
	char* e;
	ctr_size l;
	while ( 1 ) {
		if ( t == CTR_TOKEN_FIN ) {
			e = ctr_clex_code_pointer();
			fwrite(p, ((e - l ) - p),1, stdout);
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
			if (!ctr_translate_translate(s,l,dictionary,'s')) {
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
			e = ctr_clex_code_pointer();
			l =   ctr_clex_tok_value_length();
			char* v = ctr_clex_tok_value();
			int found = 0;		
			fwrite(p, ((e - l ) - p),1, stdout);
			if (!ctr_translate_translate( v, l, dictionary, 't' )) {
				fwrite(e-l, l, 1, stdout);
			}
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
