#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <locale.h>
#include "citrine.h"

const int CTR_TRANSLATE_MAX_WORD_LEN = 180;

/**
 * Dictionary Structure
 * A dictionary is used to translate citrine code from
 * one human language to another. The structure consists of
 * a type (i.e. s for string and t for token), word, translation
 * and lengths. It's a linked list.
 */
struct ctr_dict {
	char type;
	char* word;
	ctr_size wordLength;
	char* translation;
	ctr_size translationLength;
	struct ctr_dict* next;
};

/**
 * A Note structure is used by the translation system to make notes.
 * Notes form a linked list known as the notebook and can have markers
 * (integers) and attachments (string buffers) and are attached to
 * certain positions in the Citrine code (code pointers).
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
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

ctr_dict* ctr_trans_d;
ctr_dict* ctr_trans_x;

/**
 * Creates a note and attached it to the specified pointer.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
ctr_note* ctr_note_create( char* pointer ) {
	ctr_note* note = (ctr_note*) calloc(sizeof(ctr_note), 1);
	note->attachedTo = pointer;
	note->attachment = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	note->next = NULL;
	note->mark = -1;
	return note;
}

/**
 * Adds the specified note to the notebook and
 * marks the note using the specified code.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
void ctr_notebook_add( ctr_note* note, int mark ) {
	if (previousNote == NULL) {
		firstNote = note;
	} else {
		previousNote->next = note;
	}
	note->mark = mark;
	previousNote = note;
}

/**
 * Removes the entire notebook.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
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

/**
 * Clear all marks in the notebook.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
void ctr_notebook_clear_marks() {
	ctr_note* note = firstNote;
	while(note) {
		note->mark = -1;
		note = note->next;
	}
}

/**
 * Attaches a buffer to the note.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
void ctr_note_attach( ctr_note* note, char* buffer ) {
	if (!note) {
		printf("Invalid");
		exit(1);
	}
	memcpy(note->attachment,buffer,strlen(buffer));
}

/**
 * Searches the notebook for a note associated with the
 * specified code pointer.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
ctr_note* ctr_notebook_search( char* codePoint ) {
	ctr_note* found = NULL;
	ctr_note* note = firstNote;
	while(note) {
		if ( note->attachedTo == codePoint ) { found = note;  break; }
		note = note->next;
	}
	return found;
}

/**
 * Returns the note object associated with the specified
 * marker.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
ctr_note* ctr_note_grab( int mark ) {
	ctr_note* found = NULL;
	ctr_note* note = firstNote;
	while(note != NULL) {
		if (note->mark == mark) {
			note->mark = -1;
			found = note;
			break;
		}
		note = note->next;
	}
	return found;
}

/**
 * Attaches string buffers of message parts to all
 * marked notes in the notebook. Given the remainder of a message
 * (i.e. everything after the first ':') this function will
 * find the marked notes (and their code pointers) and attach
 * the string buffers associated with the remaining message parts.
 * The notebook is used to associate parts of messages with the message
 * signature like 'and:' in 'respond:and:'. String buffers containing
 * message parts ('and:') are stored in the notebook and associated with
 * code pointers. This way messages can be translated 'as a whole'.
 */
void ctr_note_collect( char* remainder ) {
	int qq;
	int jj;
	int k;
	char* buff;
	if (strlen(remainder)>CTR_TRANSLATE_MAX_WORD_LEN) {
		ctr_print_error(CTR_TERR_LONG,1);
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

/**
 * Given a pair of Citrine dictionary source headers this functions
 * generates a token translation dictionary.
 */
void ctr_translate_generate_dicts(char* hfile1, char* hfile2) {
	FILE* f1 = fopen(hfile1, "r");
	FILE* f2 = fopen(hfile2, "r");
	if (f1 == NULL || f2 == NULL) {
		ctr_print_error(CTR_TERR_DICT, 1);
	}
	char* word = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* translation = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* key1  = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* key2  = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* format;
	char* buffer;
	int lineCounter = 0;
	while( 
		fscanf( f1, "#define %180s \"%180[^\"]\"\n", key1, word ) > 0 &&
		fscanf( f2, "#define %180s \"%180[^\"]\"\n", key2, translation) > 0
	) {
		if (strlen(key1)!=strlen(key2) || strncmp(key1, key2, strlen(key1))!=0) {
			format = CTR_TERR_KMISMAT;
			buffer = ctr_heap_allocate(600);
			snprintf(buffer, 600, format, key1, key2, lineCounter);
			ctr_print_error(buffer, 1);
		}
		if (strncmp(key1, "CTR_DICT_NUM_DEC_SEP", strlen("CTR_DICT_NUM_DEC_SEP"))==0) {
			printf("d \"%s\" \"%s\"\n", word, translation);
		}
		else if (strncmp(key1, "CTR_DICT_NUM_THO_SEP", strlen("CTR_DICT_NUM_THO_SEP"))==0) {
			printf("x \"%s\" \"%s\"\n", word, translation);
		} else {
			printf("t \"%s\" \"%s\"\n", word, translation);
		}
		lineCounter++;
	}
	fclose(f1);
	fclose(f2);
}

/**
 * Loads a dictionary into memory.
 * A dictionary file has the following format:
 *
 * <t> "<word>" "<translation>"
 *
 * Where <t> is 't' for token (i.e. a programming language word)
 * or 's' for string (a literal series of bytes declared in the Citrine program).
 * This allows the dictionary to specify both translations for
 * objects, variables and messages as well as for user interface strings
 * and text snippets. There is no need to translate comments because
 * Citrine does not support them. This is always the reason why.
 */
ctr_dict* ctr_translate_load_dictionary() {
	FILE* file = fopen(ctr_mode_dict_file,"r");
	if (file == NULL) {
		ctr_print_error(CTR_TERR_DICT, 1);
	}
	char  translationType;
	char* word = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* translation = calloc(CTR_TRANSLATE_MAX_WORD_LEN, 1);
	char* buffer;
	char* format;
	ctr_dict* entry;
	ctr_dict* previousEntry = NULL;
	ctr_dict* e;
	int qq = 0;
	while( fscanf( file, "%c \"%200[^\"]\" \"%200[^\"]\"\n", &translationType, word, translation) > 0 ) {
		if (translationType != 't' && translationType != 's' && translationType != 'd' && translationType != 'x') {
			printf("Invalid translation line: %d \n",qq);
			exit(1);
		}
		entry = (ctr_dict*) calloc( sizeof(ctr_dict), 1 );
		entry->type = translationType;
		qq++;
		
		entry->wordLength = strlen(word);
		entry->translationLength = strlen(translation);
		if (entry->wordLength > CTR_TRANSLATE_MAX_WORD_LEN || entry->translationLength > CTR_TRANSLATE_MAX_WORD_LEN) {
			ctr_print_error(CTR_TERR_ELONG, 1);
		} 
		entry->word = calloc( entry->wordLength, 1 );
		entry->translation = calloc( entry->translationLength, 1 );
		memcpy(entry->word, word, entry->wordLength);
		memcpy(entry->translation, translation, entry->translationLength);
		
		if (translationType == 'd') {
			ctr_trans_d = entry;
			continue;
		}
		if (translationType == 'x') {
			ctr_trans_x = entry;
			continue;
		}
		
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
						format = CTR_TERR_AMWORD;
						buffer = ctr_heap_allocate( 600 * sizeof(char) );
						snprintf( buffer, 600 * sizeof(char), format, word );
						ctr_print_error( buffer, 1 );
					}
				}
				if ( e->translationLength == entry->translationLength ) {
					if ( strncmp( e->translation, entry->translation, entry->translationLength ) == 0 ) {
						format = CTR_TERR_AMTRANS;
						buffer = ctr_heap_allocate(600 * sizeof(char));
						snprintf( buffer, 600 * sizeof(char), format, translation);
						ctr_print_error( buffer, 1 );
					}
				}
			}
		}
	}
	fclose(file);
	return entry;
}

/**
 * Unloads a dictionary.
 * A dictionary file has the following format:
 *
 * <t> "<word>" "<translation>"
 *
 * Where <t> is 't' for token (i.e. a programming language word)
 * or 's' for string (a literal series of bytes declared in the Citrine program).
 * This allows the dictionary to specify both translations for
 * objects, variables and messages as well as for user interface strings
 * and text snippets. There is no need to translate comments because
 * Citrine does not support them. This is always the reason why.
 */
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
	int i, p, q;
	ctr_dict* entry;
	char* buffer;
	char* warning;
	entry = dictionary;
	ctr_size utf8TransLength;
	ctr_size utf8WordLength = ctr_getutf8len(v,l);
	while( entry ) {
		ctr_size ml;
		ml = entry->wordLength;
		if ( l == entry->wordLength && context == entry->type && strncmp( entry->word, v, ml ) == 0 ) {
			if (context == 't') {
				utf8TransLength = ctr_getutf8len(entry->translation,entry->translationLength);
				if ((utf8TransLength == 1 && utf8WordLength > 1) || (utf8WordLength == 1 && utf8TransLength > 1)) {
					buffer = ctr_heap_allocate( 600 );
					warning = CTR_TERR_TMISMAT;
					memcpy(buffer, warning, strlen(warning));
					memcpy(buffer + (strlen(warning)), v, l);
					ctr_print_error( buffer, 1 );
				}
				p = 0; q = 0;
				for (i = 0; i<entry->wordLength; i++) {
					if (*(entry->word + i)==':') p++;
				}
				for (i = 0; i<entry->translationLength; i++) {
					if (*(entry->translation + i)==':') q++;
				}
				if ( p != q ) {
					ctr_print_error(CTR_TERR_COLONS, 1);
				}
				for (i = 0; i<entry->translationLength; i++) {
					fwrite(entry->translation + i,1,1,stdout);
					if (*(entry->translation + i)==':' && entry->translationLength > (i+1)) {
						if ((entry->translationLength-i)>CTR_TRANSLATE_MAX_WORD_LEN) {
							ctr_print_error(CTR_TERR_BUFF, 1);
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
	if (!found) {
		buffer = ctr_heap_allocate( 600 );
		warning = CTR_TERR_WARN;
		memcpy(buffer, warning, strlen(warning));
		memcpy(buffer + (strlen(warning)), v, l);
		ctr_print_error( buffer, -1 );
		ctr_heap_free(buffer);
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
	e = ctr_clex_code_pointer();
	fwrite(p, ((e - ctr_clex_keyword_qo_len) - p),1, stdout);
	s = ctr_clex_readstr();
	l = ctr_clex_tok_value_length(s);
	e = ctr_clex_code_pointer();
	ctr_translate_translate(CTR_DICT_QUOT_OPEN,ctr_clex_keyword_qo_len,dictionary,'t',NULL);
	if (!ctr_translate_translate(s,l,dictionary,'s',NULL)) {
		fwrite(s,l,1,stdout);
	}
	//ctr_clex_tok();
	ctr_translate_translate(CTR_DICT_QUOT_CLOSE,ctr_clex_keyword_qc_len,dictionary,'t',NULL);
	e = ctr_clex_code_pointer();
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
			ctr_print_error(CTR_TERR_TOK, 1);
		}
		memcpy(message, e-l,l+1);
		ctr_size i = 1;
		while(ctr_clex_forward_scan(e, &i)) {
			if (strncmp(e+i,CTR_DICT_END_OF_LINE,ctr_clex_keyword_eol_len)==0 || *(e+i)==')' || *(e+i)==',') break;
			if (*(e+i)==':') {
				ctr_notebook_add( ctr_note_create(e+i), noteCount );
				noteCount++;
				q = 0;
				if (ctr_clex_backward_scan(e+i, "\n\t )}", &q, CTR_TRANSLATE_MAX_WORD_LEN)) {
					if ((l+1)+((e+i+1)-(e+i-q+1))>CTR_TRANSLATE_MAX_WORD_LEN) {
						ctr_print_error(CTR_TERR_PART, 1);
					}
					memcpy(message+l+1,e+i-q+1, (e+i+1)-(e+i-q+1));
					l += ((e+i+1)-(e+i-q+1));
				} else {
					ctr_print_error(CTR_MSG_ERROR,1);
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
 * Prints a language specific line terminator.
 * For instance, Indian languages end a line with danda instead of
 * a dot.
 */
char* ctr_translate_dot(char* codePointer, ctr_dict* dictionary) {
	ctr_translate_translate(CTR_DICT_END_OF_LINE,ctr_clex_keyword_eol_len,dictionary,'t',(char*)NULL);
	return ctr_clex_code_pointer();
}

/**
 * Translates a number from one language into another taking into
 * account numeric writing systems like decimal separators and thousand
 * separators.
 */
char* ctr_translate_number(char* codePointer) {
	char* p;
	char* e;
	p = codePointer;
	e = ctr_clex_code_pointer();
	while( p < e ) {
		if ( ctr_trans_d->wordLength <= ( e - p ) ) {
			if ( strncmp( ctr_trans_d->word, p, ctr_trans_d->wordLength ) == 0 ) {
				fwrite(ctr_trans_d->translation, ctr_trans_d->translationLength,1,stdout);
				p += ctr_trans_d->wordLength;
				continue;
			}
		}
		if ( ctr_trans_x->wordLength <= ( e - p ) ) {
			if ( strncmp( ctr_trans_x->word, p, ctr_trans_x->wordLength ) == 0 ) {
				fwrite(ctr_trans_x->translation, ctr_trans_x->translationLength,1,stdout);
				p += ctr_trans_x->wordLength;
				continue;
			}
		}
		fwrite(p, 1,1,stdout);
		p += 1;
	}
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
		else if ( t == CTR_TOKEN_DOT ) {
			p = ctr_translate_dot(p,dictionary);
		}
		else if ( t == CTR_TOKEN_NUMBER ) {
			p = ctr_translate_number(p);
		}	
		else {
			p = ctr_translate_rest(p);
		}
		t = ctr_clex_tok();
	}
	ctr_translate_unload_dictionary( dictionary );
}
