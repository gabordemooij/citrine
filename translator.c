#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "citrine.h"

const int CTR_TRANSLATE_MAX_WORD_LEN = 180;

struct ctr_dict {
	char type;
	char* word;
	ctr_size wordLength;
	char* translation;
	ctr_size translationLength;
	struct ctr_dict* next;
};

typedef struct ctr_dict ctr_dict;
struct ctr_note {
	char* attachment;
	char* attachedTo;
	int mark;
	struct ctr_note* next;
};

typedef struct ctr_note ctr_note;

ctr_note* previousNote = NULL;
ctr_note* firstNote = NULL;

ctr_note* ctr_note_create( char* pointer ) {
	ctr_note* note = (ctr_note*) calloc(sizeof(ctr_note), 1);
	note->attachedTo = pointer;
	note->attachment = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	note->next = NULL;
	note->mark = -1;
	return note;
}

void ctr_notebook_add( ctr_note* note, int mark ) {
	if (previousNote == NULL) {
		firstNote = note;
	} else {
		previousNote->next = note;
	}
	note->mark = mark;
	previousNote = note;
}

void ctr_notebook_remove() {
	ctr_note* note = firstNote;
	while(note) {
		if (note->mark > -1) {
			 note->mark = -1;
			 note->attachedTo = NULL;
		 }
		note = note->next;
	}
}

void ctr_notebook_clear_marks() {
	ctr_note* note = firstNote;
	while(note) {
		note->mark = -1;
		note = note->next;
	}
}

void ctr_note_attach( ctr_note* note, char* buffer ) {
	memcpy(note->attachment,buffer,strlen(buffer));
}

ctr_note* ctr_notebook_search( char* codePoint ) {
	ctr_note* note = firstNote;
	while(note) {
		if ( note->attachedTo == codePoint ) {  break; }
		note = note->next;
	}
	return note;
}

ctr_note* ctr_note_grab( int mark ) {
	ctr_note* note = firstNote;
	while(note != NULL) {
		if (note->mark == mark) {
			note->mark = -1;
			break;
		}
		note = note->next;
	}
	return note;
}

void ctr_note_collect( char* remainder ) {
	int qq;
	int jj;
	int k;
	char* buff;
	if (strlen(remainder)>CTR_TRANSLATE_MAX_WORD_LEN) {
		printf("Translation error, message too long.\n");
		exit(1);
	}
	buff = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	qq = 0;
	jj = 0;
	for(k=0; k<strlen(remainder); k++) {
		*(buff+(jj++))=*(remainder+k);
		if (*(remainder + k) == ':') {
			ctr_note_attach( ctr_note_grab( qq++ ), buff );
			memset(buff, 0, CTR_TRANSLATE_MAX_WORD_LEN);
			jj=0;
		}
	}
	free(buff);
}

void ctr_translate_generate_dicts(char* hfile1, char* hfile2) {
	FILE* f1 = fopen(hfile1, "r");
	FILE* f2 = fopen(hfile2, "r");
	if (f1 == NULL) {
		ctr_print_error("Error opening source dictionary.", 1);
	}
	if (f2 == NULL) {
		ctr_print_error("Error opening target dictionary.", 1);
	}
	char* word = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* translation = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* key1  = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* key2  = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	int lineCounter = 0;
	while( 
		fscanf( f1, "#define %180s \"%180[^\"]\"\n", key1, word ) > 0 &&
		fscanf( f2, "#define %180s \"%180[^\"]\"\n", key2, translation) > 0
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
	fclose(f1);
	fclose(f2);
}

ctr_dict* ctr_translate_load_dictionary() {
	FILE* file = fopen(ctr_mode_dict_file,"r");
	if (file == NULL) {
		printf("Unable to open dictionary.\n");
		exit(1);
	}
	char  translationType;
	char* word = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* translation = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	ctr_dict* entry;
	ctr_dict* previousEntry = NULL;
	ctr_dict* e;
	while( fscanf( file, "%c \"%80[^\"]\" \"%80[^\"]\"\n", &translationType, word, translation) > 0 ) {
		entry = (ctr_dict*) calloc( sizeof(ctr_dict), 1 );
		entry->type = translationType;
		entry->wordLength = strlen(word);
		entry->translationLength = strlen(translation);
		if (entry->wordLength > CTR_TRANSLATE_MAX_WORD_LEN || entry->translationLength > CTR_TRANSLATE_MAX_WORD_LEN) {
			printf("Dictionary entry too long.\n");
			exit(1);
		} 
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
		e = entry;
		while(e->next != NULL) {
			e = e->next;
			if (e->type == entry->type) {
				if ( e->wordLength == entry->wordLength ) {
					if ( strncmp( e->word, entry->word, entry->wordLength ) == 0 ) {
						printf( "Ambigious word in dictionary. %s \n", entry->word );
						exit(1);
					}
				}
				if ( e->translationLength == entry->translationLength ) {
					if ( strncmp( e->translation, entry->translation, entry->translationLength ) == 0 ) {
						printf( "Ambigious translation in dictionary %s.\n", entry->translation );
						exit(1);
					}
				}
			}
		}
	}
	fclose(file);
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

/**
 * Translates a word in the program using a dictionary and a context flag.
 * If the word is a keyword message and there is translation available, the remainder
 * gets filled for the next parts of the message to come.
 */
int ctr_translate_translate(char* v, ctr_size l, ctr_dict* dictionary, char context, char* remainder) {
	int found = 0;
	int i;
	ctr_dict* entry;
	entry = dictionary;
	while( entry ) {
		ctr_size ml;
		ml = entry->wordLength;
		if ( l == entry->wordLength && context == entry->type && strncmp( entry->word, v, ml ) == 0 ) {
			if (context == 't') {
				for (i = 0; i<entry->translationLength; i++) {
					fwrite(entry->translation + i,1,1,stdout);
					if (*(entry->translation + i)==':' && entry->translationLength > (i+1)) {
						if ((entry->translationLength-i)>CTR_TRANSLATE_MAX_WORD_LEN) {
							printf("Unable to copy translation to buffer.\n");
							exit(1);
						}
						memcpy(remainder,entry->translation+i+1,(entry->translationLength-i-1));
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
	if (context == 't' && !found && ctr_internal_memmem(v,l,":",1,0)>((char*)NULL)) {
		for (i = 0; i<l; i++) {
				fwrite(v+i,1,1,stdout);
				if (*(v + i)==':') {
					memcpy(remainder,v+i+1,(l-i));
					found = 1;
					break;
				}
			}
	}
	return found;
}

/**
 * Prints translatable strings in the program.
 */
char* ctr_translate_string(char* codePointer, ctr_dict* dictionary) {
	char* s;
	ctr_size l;
	char* e;
	char* p;
	p = codePointer;
	if (ctr_string_interpolation) {
		ctr_string_interpolation = 0;
	}
	s = ctr_clex_readstr();
	l =  ctr_clex_tok_value_length(s);
	e = ctr_clex_code_pointer();
	if (ctr_string_interpolation) {
		e -= 3;
	}
	fwrite(p, e-p-l, 1, stdout);
	if (!ctr_translate_translate(s,l,dictionary,'s',NULL)) {
		fwrite(s,l,1,stdout);
	}
	if (!ctr_string_interpolation) {
		ctr_clex_tok();
	}
	return e;
}

/**
 * Prints a translatable reference in the program.
 */
char* ctr_translate_ref(char* codePointer, ctr_dict* dictionary) {
	char* message;
	int noteCount;
	char skipColon;
	char* remainder;
	char* p = codePointer;
	char* e;
	ctr_size l;
	ctr_size ol;
	ctr_note* foundNote;
	skipColon = 0;
	e = ctr_clex_code_pointer();
	l = ctr_clex_tok_value_length();
	ol = l;
	message = ctr_clex_tok_value();
	fwrite(p, ((e - l ) - p),1, stdout);
	noteCount = 0;
	/* is this part of a keyword message (end with colon?) */
	if (*(e)==':') {
		ctr_notebook_clear_marks();
		skipColon = 1;
		ctr_size q;
		message = calloc(CTR_TRANSLATE_MAX_WORD_LEN,1);
		if (l+1 > CTR_TRANSLATE_MAX_WORD_LEN) {
			printf("Token length exceeds maximum buffer size.\n");
			exit(1);
		}
		memcpy(message, e-l,l+1);
		ctr_size i = 1;
		while(ctr_clex_forward_scan(e, ":.,)\"", &i)) {
			if (*(e+i)=='.' || *(e+i)==')' || *(e+i)==',') break;
			if (*(e+i)==':') {
				ctr_notebook_add( ctr_note_create(e+i), noteCount );
				noteCount++;
				q = 0;
				if (ctr_clex_backward_scan(e+i, "\n\t )}", &q, CTR_TRANSLATE_MAX_WORD_LEN)) {
					if ((l+1)+((e+i+1)-(e+i-q+1))>CTR_TRANSLATE_MAX_WORD_LEN) {
						printf("Part of keyword message token exceeds buffer limit.\n");
						exit(1);
					}
					memcpy(message+l+1,e+i-q+1, (e+i+1)-(e+i-q+1));
					l += ((e+i+1)-(e+i-q+1));
				} else {
					printf("error.");
					exit(1);
				}
			}
			i++;
		}
		l++;
	}
	foundNote = ctr_notebook_search( e );
	if (foundNote) {
		fwrite(foundNote->attachment, strlen(foundNote->attachment),1,stdout);
	} else {
		remainder = calloc(CTR_TRANSLATE_MAX_WORD_LEN,1);
		if (!ctr_translate_translate( message, l, dictionary, 't', remainder )) {
			skipColon = 0;
			fwrite(e-ol, ol, 1, stdout);
			ctr_notebook_remove();
		} else {
			if (noteCount>0) ctr_note_collect(remainder);
		}
		free(remainder);
	}
	return (e + skipColon);
}

/**
 * Prints a part of the program that does not contain
 * translatable items.
 */
char* ctr_translate_rest(char* codePointer) {
	char* p;
	char* e;
	p = codePointer;
	e = ctr_clex_code_pointer();
	fwrite(p, e-p,1,stdout);
	return e;
}

/**
 * Prints the remaining part of the program.
 */
void ctr_translate_fin(char* codePointer) {
	char* e;
	char* p;
	p = codePointer;
	ctr_size l;
	e = ctr_clex_code_pointer();
	l = ctr_clex_tok_value_length();
	fwrite(p, ((e - l) - p),1, stdout);
	fwrite(e-l, l, 1, stdout);
}

/**
 * Translates a program from one human language to another.
 */
void ctr_translate_program(char* prg, char* programPath) {
	ctr_dict* dictionary;
	int t;
	char* p;
	dictionary = ctr_translate_load_dictionary();
	ctr_clex_set_ignore_modes(1);
	ctr_clex_load(prg);
	t = ctr_clex_tok();
	p = prg;
	while ( 1 ) {
		if ( t == CTR_TOKEN_FIN ) {
			ctr_translate_fin(p);
			break;
		}
		else if ( t == CTR_TOKEN_QUOTE ) {
			p = ctr_translate_string(p, dictionary);
		} 
		else if ( t == CTR_TOKEN_REF || t == CTR_TOKEN_BOOLEANYES || t == CTR_TOKEN_BOOLEANNO || t == CTR_TOKEN_NIL ) {
			p = ctr_translate_ref(p,dictionary);
		}
		else {
			p = ctr_translate_rest(p);
		}
		t = ctr_clex_tok();
	}
	ctr_translate_unload_dictionary( dictionary );
}
