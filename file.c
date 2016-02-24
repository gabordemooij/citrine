#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

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
 */
ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* s = ctr_object_make(myself, argumentList);
	ctr_object* pathObject;
	s->info.type = CTR_OBJECT_TYPE_OTOBJECT;
	s->link = myself;
	s->value.rvalue = NULL;
	pathObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTSTRING);
	pathObject->info.type = CTR_OBJECT_TYPE_OTSTRING;
	pathObject->value.svalue = (ctr_string*) malloc(sizeof(ctr_string));
	pathObject->value.svalue->value = (char*) malloc(sizeof(char) * argumentList->object->value.svalue->vlen);
	memcpy(pathObject->value.svalue->value, argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
	pathObject->value.svalue->vlen = argumentList->object->value.svalue->vlen;
	ctr_internal_object_add_property(s, ctr_build_string("path",4), pathObject, 0);
	return s;
}

/**
 * [File] path
 *
 * Returns the path of a file.
 */
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return CtrStdNil;
	return path;
}

/**
 * [File] read
 *
 * Reads contents of a file.
 */
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_object* str;
	ctr_size vlen, fileLen;
	char* pathString;
	char *buffer;
	FILE* f;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "rb");
	free(pathString);
	if (!f) {
		CtrStdError = ctr_build_string_from_cstring("Unable to open file.\0");
		return CtrStdNil;
	}
	fseek(f, 0, SEEK_END);
	fileLen=ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer=(char *)malloc(fileLen+1);
	if (!buffer){
		printf("Out of memory\n");
		fclose(f);exit(1);	
	}
	fread(buffer, fileLen, 1, f);
	fclose(f);
	str = ctr_build_string(buffer, fileLen);
	free(buffer);
	return str;
}

/**
 * [File] write: [String]
 *
 * Writes content to a file.
 */
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	FILE* f;
	ctr_size vlen;
	char* pathString;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "wb+");
	free(pathString);
	if (!f) {
		CtrStdError = ctr_build_string_from_cstring("Unable to open file.\0");
		return CtrStdNil;
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

/**
 * [File] append: [String]
 *
 * Appends content to a file.
 */
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "ab+");
	free(pathString);
	if (!f) {
		CtrStdError = ctr_build_string_from_cstring("Unable to open file.\0");
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
 */
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int exists;
	if (path == NULL) return ctr_build_bool(0);
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "r");
	free(pathString);
	exists = (f != NULL );
	if (f) {
		fclose(f);
	}
	return ctr_build_bool(exists);
}

/**
 * [File] include
 *
 * Includes the file as a piece of executable code.
 */
ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_tnode* parsedCode;
	ctr_size vlen;
	char* pathString;
	char* prg;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	prg = ctr_internal_readf(pathString);
	free(pathString);
	parsedCode = ctr_dparse_parse(prg);
	ctr_cwlk_run(parsedCode);
	return myself;
}

/**
 * [File] run
 *
 * Includes the file as a piece of executable code.
 */
ctr_object* ctr_file_include_ast(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_tnode* parsedCode;
	ctr_size vlen;
	char* pathString;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	parsedCode = ctr_serializer_unserialize(pathString);
	ctr_cwlk_run(parsedCode);
	return myself;
}

/**
 * [File] delete
 *
 * Deletes the file.
 */
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_size vlen;
	char* pathString;
	int r;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	r = remove(pathString);
	if (r!=0) {
		CtrStdError = ctr_build_string_from_cstring("Unable to delete file.\0");
		return CtrStdNil;
	}
	return myself;
}

/**
 * [File] size
 *
 * Returns the size of the file.
 */
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int prev, sz;
	if (path == NULL) return ctr_build_number_from_float(0);
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "r");
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
 *
 * Usage:
 *
 * f := File new: '/path/to/file'.
 * f open: 'r+'. #opens file for reading and writing
 *
 * The example above opens the file in f for reading and writing.
 */
ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathObj = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	char* mode;
	FILE* handle;
	ctr_resource* rs = malloc(sizeof(ctr_resource));
	if (pathObj == NULL) return myself;
	char* path;
	CTR_2CSTR(path, pathObj);
	CTR_2CSTR(mode, ctr_internal_cast2string(argumentList->object));
	handle = fopen(path,mode);
	rs->type = 1;
	rs->ptr = handle;
	myself->value.rvalue = rs;
	return myself;
}

/**
 * [File] close.
 *
 * Closes the file represented by the recipient.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * f close.
 *
 * The example above opens and closes a file.
 */
ctr_object* ctr_file_close(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	fclose((FILE*)myself->value.rvalue->ptr);
	myself->value.rvalue = NULL;
	return myself;
}

/**
 * [File] readBytes: [Number].
 *
 * Reads a number of bytes from the file.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * x := f readBytes: 10.
 * f close.
 *
 * The example above reads 10 bytes from the file represented by f
 * and puts them in buffer x.
 */
ctr_object* ctr_file_read_bytes(ctr_object* myself, ctr_argument* argumentList) {
	int bytes;
	char* buffer;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	bytes = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	if (bytes < 0) return ctr_build_string_from_cstring("");
	buffer = (char*) malloc(bytes);
	if (buffer == NULL) {
		CtrStdError = ctr_build_string_from_cstring("Cannot allocate memory for file buffer.");
		return ctr_build_string_from_cstring("");
	}
	fread(buffer, sizeof(char), (int)bytes, (FILE*)myself->value.rvalue->ptr);
	return ctr_build_string_from_cstring(buffer);
}

/**
 * [File] writeBytes: [String].
 *
 * Takes a string and writes the bytes in the string to the file
 * object. Returns the number of bytes actually written.
 *
 * Usage:
 *
 * f := File new: '/path/to/file.txt'.
 * f open: 'r+'.
 * n := f writeBytes: 'Hello World'.
 * f close.
 *
 * The example above writes 'Hello World' to the specified file as bytes.
 * The number of bytes written is returned in variable n.
 */
ctr_object* ctr_file_write_bytes(ctr_object* myself, ctr_argument* argumentList) {
	int bytes, written;
	char* buffer;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	ctr_object* string2write = ctr_internal_cast2string(argumentList->object);
	CTR_2CSTR(buffer, string2write);
	bytes = string2write->value.svalue->vlen;
	written = fwrite(buffer, sizeof(char), (int)bytes, (FILE*)myself->value.rvalue->ptr);
	return ctr_build_number_from_float((double_t) written);
}

/**
 * [File] seek: [Number].
 *
 * Moves the file pointer to the specified position in the file
 * (relative to the current position).
 *
 * Usage:
 *
 * file open: 'r', seek: 10.
 *
 * The example above opens a file for reading and moves the
 * pointer to position 10 (meaning 10 bytes from the beginning of the file).
 * The seek value may be negative.
 */
ctr_object* ctr_file_seek(ctr_object* myself, ctr_argument* argumentList) {
	int offset;
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	offset = (long int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	error = fseek((FILE*)myself->value.rvalue->ptr, offset, SEEK_CUR);
	if (error) CtrStdError = ctr_build_string_from_cstring("Seek failed.");
	return myself;
}

/**
 * [File] rewind.
 *
 * Rewinds the file. Moves the file pointer to the beginning of the file.
 *
 * Usage:
 *
 * file open: 'r'.
 * x := file readBytes: 10. #read 10 bytes
 * file rewind.        #rewind, set pointer to begin again
 * y := file readBytes: 10. #re-read same 10 bytes
 *
 * The example above reads the same sequence of 10 bytes twice, resulting
 * in variable x and y being equal.
 */
ctr_object* ctr_file_seek_rewind(ctr_object* myself, ctr_argument* argumentList) {
	int offset;
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_SET);
	if (error) CtrStdError = ctr_build_string_from_cstring("Seek rewind failed.");
	return myself;
}

/**
 * [File] end.
 *
 * Moves the file pointer to the end of the file. Use this in combination with
 * negative seek operations.
 *
 * Usage:
 *
 * file open: 'r'.
 * file end.
 * x := file seek: -10, readBytes: 10.
 *
 * The example above will read the last 10 bytes of the file. This is
 * accomplished by first moving the file pointer to the end of the file,
 * then putting it back 10 bytes (negative number), and then reading 10
 * bytes.
 */
ctr_object* ctr_file_seek_end(ctr_object* myself, ctr_argument* argumentList) {
	int offset;
	int error;
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_END);
	if (error) CtrStdError = ctr_build_string_from_cstring("Seek end failed.");
	return myself;
}
