#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <sys/file.h>
#include <dirent.h>
#include "citrine.h"
#include "siphash.h"

/**
 * File
 *
 * Represents a File object.
 * Creates a new file object based on the specified path.
 *
 * Usage:
 *
 * File new: '/example/path/to/file.txt'.
 *
 * In other languages:
 * Dutch: Bestand nieuw. Maakt een nieuw bestand.
 * Voorbeeld: ☞ bestand := Bestand nieuw: 'voorbeeld.txt'.
 */
ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* s = ctr_object_make(myself, argumentList);
	ctr_object* pathObject;
	s->info.type = CTR_OBJECT_TYPE_OTEX; /* indicates resource for GC */
	s->link = myself;
	s->value.rvalue = NULL;
	pathObject = ctr_internal_cast2string( argumentList->object );
	ctr_internal_object_add_property( s, ctr_build_string_from_cstring( "path" ), pathObject, 0 );
	return s;
}

/**
 * [File] path
 *
 * Returns the path of a file. The file object will respond to this
 * message by returning a string object describing the full path to the
 * recipient.
 *
 * In other languages:
 * Dutch: [bestand] pad.
 * Geeft de locatie van het bestand terug.
 */
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	if (path == NULL) return CtrStdNil;
	return path;
}

/**
 * [File] string
 *
 * Returns a string representation of the file. If a path has been associated
 * with this file, this message will return the path of the file on the file system.
 * If no path has been associated with the file, the string [File (no path)] will
 * be returned.
 *
 * In other languages:
 * Dutch: [bestand] tekst. Geeft tekstrepresentatie van bestand terug.
 * Meestal is dit het pad.
 */
ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_file_path(myself,argumentList);
	if ( path == CtrStdNil) {
		return ctr_build_string_from_cstring( CTR_SYM_FILE );
	}
	return ctr_internal_cast2string(path);
}

/**
 * [File] read
 *
 * Reads contents of a file. Send this message to a file to read the entire contents in
 * one go. For big files you might want to prefer a streaming approach to avoid
 * memory exhaustion.
 * In the example we read the contents of the entire CSV file callled mydata.csv
 * in the variable called data.
 *
 * Usage:
 *
 * ☞ data := File new: '/path/to/mydata.csv', read.
 *
 * In other languages:
 * Dutch: [bestand] lees.
 * Leest het bestand in en geeft inhoud terug als tekst.
 * Voorbeeld: gegevens := Bestand nieuw: 'test.txt', lees.
 */
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_object* str;
	ctr_size vlen, fileLen;
	char* pathString;
	char *buffer;
	FILE* f;
	int error_code;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate( sizeof(char) * ( vlen + 1 ) );
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "rb");
	error_code = errno;
	ctr_heap_free( pathString );
	if (!f) {
		ctr_error( CTR_ERR_OPEN, error_code );
		return CtrStdNil;
	}
	fseek(f, 0, SEEK_END);
	fileLen=ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer=(char *)ctr_heap_allocate(fileLen+1);
	if (!buffer){
		fprintf(stderr, CTR_ERR_OOM );
		fclose(f);exit(1);
	}
	fread(buffer, fileLen, 1, f);
	fclose(f);
	str = ctr_build_string(buffer, fileLen);
	ctr_heap_free( buffer );
	return str;
}

/**
 * [File] write: [String]
 *
 * Writes content to a file. Send this message to a file object to write the
 * entire contents of the specified string to the file in one go. The file object
 * responds to this message for convience reasons, however for big files it might
 * be a better idea to use the streaming API if possible (see readBytes etc.).
 * In the example we write the XML snippet in variable data to a file
 * called myxml.xml in the current working directory.
 *
 * Usage:
 *
 * ☞ data := '<xml>hello</xml>'.
 * File new: 'myxml.xml', write: data.
 *
 * In other languages:
 * Dutch: [bestand] schrijf: gegevens.
 * Schrijf inhoud variabele naar bestand.
 * Voorbeeld: Bestand nieuw: 'test.txt', schrijf: 'inhoud'.
 */
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0 );
	FILE* f;
	ctr_size vlen;
	char* pathString;
	int error_code;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "wb+");
	error_code = errno;
	ctr_heap_free( pathString );
	if (!f) {
		CtrStdFlow = ctr_error( CTR_ERR_OPEN, error_code );
		return CtrStdNil;
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

/**
 * [File] append: [String]
 *
 * Appends content to a file. The file object responds to this message like it
 * responds to the write-message, however in this case the contents of the string
 * will be appended to the existing content inside the file.
 *
 * In other languages:
 * Dutch: [bestand] toevoegen: gegevens.
 * Voegt inhoud toe aan bestand.
 */
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	int error_code;
	char* pathString;
	FILE* f;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "ab+");
	error_code = errno;
	ctr_heap_free( pathString );
	if (!f) {
		CtrStdFlow = ctr_error( CTR_ERR_OPEN, error_code );
		return CtrStdNil;
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

/**
 * [File] exists
 *
 * Returns True if the file exists and False otherwise.
 *
 * In other languages:
 * Dutch: [bestand] bestaat.
 * Antwoordt Waar als het bestand bestaat en Onwaar als het bestaat
 * niet bestaat.
 */
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int exists;
	if (path == NULL) return ctr_build_bool(0);
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "r");
	ctr_heap_free( pathString );
	exists = (f != NULL );
	if (f) {
		fclose(f);
	}
	return ctr_build_bool(exists);
}

/**
 * [File] delete
 *
 * Deletes the file.
 *
 * In other languages:
 * Dutch: [bestand] verwijderen.
 */
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	int r;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate( sizeof( char ) * ( vlen + 1 ) );
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	r = remove(pathString);
	ctr_heap_free( pathString );
	if (r!=0) {
		CtrStdFlow = ctr_error( CTR_ERR_DELETE, 0 );
		return CtrStdNil;
	}
	return myself;
}

/**
 * [File] size
 *
 * Returns the size of the file.
 *
 * In other languages:
 * Dutch: [bestand] grootte.
 * Geeft de bestandsgrootte terug van het bestand.
 */
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int prev, sz;
	if (path == NULL) return ctr_build_number_from_float(0);
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate( sizeof(char) * ( vlen + 1 ) );
	memcpy(pathString, path->value.svalue->value, ( sizeof( char ) * vlen  ) );
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "r");
	ctr_heap_free( pathString );
	if (f == NULL) return ctr_build_number_from_float(0);
	prev = ftell(f);
	fseek(f, 0L, SEEK_END);
	sz=ftell(f);
	fseek(f,prev,SEEK_SET);
	if (f) {
		fclose(f);
	}
	return ctr_build_number_from_float( (ctr_number) sz );
}

/**
 * [File] open: [string]
 *
 * Open a file with using the specified mode.
 * The example opens the file in f for reading and writing.
 *
 * Usage:
 *
 * ☞ f := File new: '/path/to/file'.
 * f open: 'r+'.
 *
 * In other languages:
 * Dutch: [bestand] openen: modus.
 * Opent het bestand in de gegeven modus (r+) voor
 * lezen en schrijven.
 * Voorbeeld:
 * ☞ f := Bestand nieuw: 'test.txt'.
 * f openen: 'r+'.
 * f sluiten.
 */
ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathObj = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	char* mode;
	char* path;
	FILE* handle;
	ctr_resource* rs = ctr_heap_allocate(sizeof(ctr_resource));
	ctr_object* modeStrObj = ctr_internal_cast2string( argumentList->object );
	if ( myself->value.rvalue != NULL ) {
		ctr_heap_free( rs );
		CtrStdFlow = ctr_error( CTR_ERR_FOPENED, 0 );
		return myself;
	}
	if ( pathObj == NULL ) {
		ctr_heap_free( rs );
		return myself;
	}
	path = ctr_heap_allocate_cstring( pathObj );
	mode = ctr_heap_allocate_cstring( modeStrObj );
	handle = fopen(path,mode);
	ctr_heap_free( path );
	ctr_heap_free( mode );
	rs->type = 1;
	rs->ptr = handle;
	myself->value.rvalue = rs;
	return myself;
}

/**
 * [File] close.
 *
 * Closes the file represented by the recipient.
 * The example above opens and closes a file.
 *
 * Usage:
 *
 * ☞ f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * f close.
 *
 * In other languages:
 * Dutch: [bestand] sluiten.
 * Sluit het geopende bestand.
 * Voorbeeld:
 * ☞ f := Bestand nieuw: 'test.txt'.
 * f openen: 'r+'.
 * f sluiten.
 */
ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	if (myself->value.rvalue->ptr) {
		fclose((FILE*)myself->value.rvalue->ptr);
	}
	ctr_heap_free( myself->value.rvalue );
	myself->value.rvalue = NULL;
	return myself;
}

/**
 * [File] read bytes: [Number].
 *
 * Reads a number of bytes from the file.
 * The example reads 10 bytes from the file represented by f
 * and puts them in buffer x.
 *
 * Usage:
 *
 * ☞ f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * ☞ x := f read bytes: 10.
 * f close.
 *
 * In other languages:
 * Dutch: [bestand] lees bytes: [getal].
 * Leest aantal bytes uit bestand en geeft dit terug.
 * In het onderstaande voorbeeld lezen we de eerste 10
 * bytes uit het bestand test.txt.
 * Voorbeeld:
 * ☞ f := Bestand nieuw: 'test.txt'.
 * f openen: 'r+'.
 * ☞ x := f lees bytes: 10.
 * f sluiten.
 */
ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList) {
	int bytes;
	char* buffer;
	ctr_object* result;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	bytes = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	if (bytes < 0) return ctr_build_string_from_cstring("");
	buffer = (char*) ctr_heap_allocate(bytes);
	if (buffer == NULL) {
		CtrStdFlow = ctr_error( CTR_ERR_OOM, 0 );
		return ctr_build_string_from_cstring("");
	}
	fread(buffer, sizeof(char), (int)bytes, (FILE*)myself->value.rvalue->ptr);
	result = ctr_build_string(buffer, bytes);
	ctr_heap_free( buffer );
	return result;
}

/**
 * [File] write bytes: [String].
 *
 * Takes a string and writes the bytes in the string to the file
 * object. Returns the number of bytes actually written.
 * The example above writes 'Hello World' to the specified file as bytes.
 * The number of bytes written is returned in variable n.
 *
 * Usage:
 *
 * ☞ f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * ☞ n := f write bytes: 'Hello World'.
 * f close.
 *
 * In other languages:
 * Dutch: [bestand] schrijf bytes: [tekst].
 * Schrijft bytes naar het bestand. In het onderstaande voorbeeld
 * schrijven we de tekst 'hallo' naar het bestand test.txt.
 * Voorbeeld:
 * ☞ f := Bestand nieuw: 'test.txt'.
 * f openen: 'r+'.
 * ☞ x := f schrijf bytes: 'hallo'.
 * f sluiten.
 */
ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList) {
	int bytes, written;
	ctr_object* string2write;
	char* buffer;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	string2write = ctr_internal_cast2string(argumentList->object);
	buffer = ctr_heap_allocate_cstring( string2write );
	bytes = string2write->value.svalue->vlen;
	written = fwrite(buffer, sizeof(char), (int)bytes, (FILE*)myself->value.rvalue->ptr);
	ctr_heap_free( buffer );
	return ctr_build_number_from_float((double_t) written);
}

/**
 * [File] seek: [Number].
 *
 * Moves the file pointer to the specified position in the file
 * (relative to the current position).
 * The example opens a file for reading and moves the
 * pointer to position 10 (meaning 10 bytes from the beginning of the file).
 * The seek value may be negative.
 *
 * Usage:
 *
 * file open: 'r', seek: 10.
 *
 * In other languages:
 * Dutch: [bestand] zoeken: [getal].
 * Verplaatst de cursor naar de gespecificeerde positie in het bestand.
 * In onderstaand voorbeeld openen we een bestand voor lezen en verplaatsen
 * we de cursor naar positie 10 (10 bytes ten opzichte van het
 * begin van het bestand). Een zoekwaarde kan negatief zijn.
 * Voorbeeld:
 *
 * f open: 'r', zoeken: 10.
 */
ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList) {
	int offset;
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	offset = (long int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	error = fseek((FILE*)myself->value.rvalue->ptr, offset, SEEK_CUR);
	if (error) {
		CtrStdFlow = ctr_error( CTR_ERR_SEEK, 0 );
	}
	return myself;
}

/**
 * [File] rewind.
 *
 * Rewinds the file. Moves the file pointer to the beginning of the file.
 * The example reads the same sequence of 10 bytes twice, resulting
 * in variable x and y being equal.
 *
 * Usage:
 *
 * file open: 'r'.
 * ☞ x := file read bytes: 10.
 * file rewind.
 * ☞ y := file read bytes: 10.
 *
 * In other languages:
 * Dutch: [bestand] terugspoelen.
 * Plaatst de cursor weer aan het beginpunt van het bestand.
 */
ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList) {
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_SET);
	if (error) {
		CtrStdFlow = ctr_error( CTR_ERR_SEEK, 0 );
	}
	return myself;
}

/**
 * [File] end.
 *
 * Moves the file pointer to the end of the file. Use this in combination with
 * negative seek operations.
 * The example will read the last 10 bytes of the file. This is
 * accomplished by first moving the file pointer to the end of the file,
 * then putting it back 10 bytes (negative number), and then reading 10
 * bytes.
 *
 * Usage:
 *
 * file open: 'r'.
 * file end.
 * ☞ x := file seek: -10, read bytes: 10.
 *
 * In other languages:
 * Dutch: [bestand] terugspoelen.
 * Plaatst de cursor aan het einde van het bestand.
 */
ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList) {
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_END);
	if (error) {
		CtrStdFlow = ctr_error( CTR_ERR_SEEK, 0 );
	}
	return myself;
}

/**
 * @internal
 *
 * Locks a file or unlocks a file.
 * All locking functions use this function under the hood.
 */
ctr_object* ctr_file_lock_generic(ctr_object* myself, ctr_argument* argumentList, int lock) {
	int b;
	int fd;
	char* path;
	ctr_object* pathObj;
	ctr_object* answer;
	ctr_object* fdObj;
	ctr_object* fdObjKey;
	pathObj = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	if (pathObj == NULL) {
		CtrStdFlow = ctr_error( CTR_ERR_LOCK, 0 );
		return CtrStdNil;
	}
	path = ctr_heap_allocate_cstring( pathObj );
	fdObjKey = ctr_build_string_from_cstring("fileDescriptor");
	fdObj = ctr_internal_object_find_property(
		myself,
		fdObjKey,
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	if (fdObj == NULL) {
		fd = open( path, O_CREAT );
		if (fd < 0) {
			CtrStdFlow = ctr_error( CTR_ERR_LOCK, 0 );
			ctr_heap_free( path );
			return CtrStdNil;
		}
		fdObj = ctr_build_number_from_float( (ctr_size) fd );
		ctr_internal_object_set_property(
			myself, fdObjKey, fdObj, CTR_CATEGORY_PRIVATE_PROPERTY
		);
	} else {
		fd = (int) fdObj->value.nvalue;
	}
	b = flock( fd, lock );
	if (b != 0) {
		close(fd);
		answer = ctr_build_bool(0);
		ctr_internal_object_delete_property( myself, fdObjKey, CTR_CATEGORY_PRIVATE_PROPERTY );
	} else {
		answer = ctr_build_bool(1);
	}
	ctr_heap_free( path );
	return answer;
}

/**
 * [File] unlock.
 *
 * Attempts to unlock a file. This message is non-blocking, on failure
 * it will immediately return. Answers True if the file has been
 * unlocked succesfully. Otherwise, the answer is False.
 *
 * In other languages:
 * Dutch: [bestand] ontgrendel.
 * Poogt het bestand te ontgrendelen. Geeft Waar terug als poging slaagt.
 */
ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_file_lock_generic( myself, argumentList, LOCK_UN | LOCK_NB );
}

/**
 * [File] lock.
 *
 * Attempts to acquire an exclusive lock on file.
 * This message is non-blocking, on failure
 * it will immediately return. Answers True if the lock has been
 * acquired and False otherwise.
 *
 * In other languages:
 * Dutch: [bestand] vergrendel.
 * Poogt het bestand te vergrendelen. Geeft Waar terug als poging slaagt.
 */
ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_file_lock_generic( myself, argumentList, LOCK_EX | LOCK_NB );
}

/**
 * [File] list: [String].
 *
 * Returns the contents of the specified folder as a an array.
 * Each entry of the array contains a map with the keys 'file'
 * and 'type'. The 'file' entry contains a string with the name
 * of the file while the 'type' entry contains a string describing
 * the type of the file.
 *
 * Usage:
 *
 * ☞ files := File list: '/tmp/testje'.
 *
 *
 * In other languages:
 * Dutch: Bestand lijst: [tekst]
 * Geeft de inhoud van de opgegeven map terug.
 *
 * Voorbeeld:
 *
 * bestanden := Bestand lijst: '/mijnmap'.
 */
ctr_object* ctr_file_list(ctr_object* myself, ctr_argument* argumentList) {
	DIR* d;
	struct dirent* entry;
	char* pathValue;
	ctr_object* fileList;
	ctr_object* fileListItem;
	ctr_object* path;
	ctr_argument* putArgumentList;
	ctr_argument* addArgumentList;
	path = ctr_internal_cast2string( argumentList->object );
	fileList = ctr_array_new(CtrStdArray, NULL);
	pathValue = ctr_heap_allocate_cstring( path );
	d = opendir( pathValue );
	if (d == 0) {
		int error_code = errno;
		CtrStdFlow = ctr_error( CTR_ERR_OPEN, error_code );
		ctr_heap_free(pathValue);
		return CtrStdNil;
	}
	putArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	addArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	putArgumentList->next = ctr_heap_allocate( sizeof( ctr_argument ) );
	while((entry = readdir(d))) {
		fileListItem = ctr_map_new(CtrStdMap, NULL);
		putArgumentList->next->object = ctr_build_string_from_cstring( CTR_MSG_DSC_FILE );
		putArgumentList->object = ctr_build_string_from_cstring(entry->d_name);
		ctr_map_put(fileListItem, putArgumentList);
		putArgumentList->next->object = ctr_build_string_from_cstring( CTR_MSG_DSC_TYPE );
		switch(entry->d_type) {
			case DT_REG:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_FILE );
				break;
			case DT_DIR:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_FLDR );
				break;
			case DT_LNK:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_SLNK );
				break;
			case DT_CHR:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_CDEV );
				break;
			case DT_BLK:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_BDEV );
				break;
			case DT_SOCK:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_SOCK );
				break;
			case DT_FIFO:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_NPIP );
				break;
			default:
				putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_OTHR );
				break;
		}
		ctr_map_put(fileListItem, putArgumentList);
		addArgumentList->object = fileListItem;
		ctr_array_push(fileList, addArgumentList);
	}
	closedir(d);
	ctr_heap_free(putArgumentList->next);
	ctr_heap_free(putArgumentList);
	ctr_heap_free(addArgumentList);
	ctr_heap_free(pathValue);
	return fileList;
}
