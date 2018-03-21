#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include "citrine.h"
#include "siphash.h"
#include <unistd.h>
#include <sys/file.h>

#ifdef __MINGW32__
#include <windows.h>
#include "ext/dirent/include/dirent.h"
#else
#include <dirent.h>
#endif

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
 */
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	if (path == NULL) return CtrStdNil;
	return path;
}

/**
 * [File] toString
 *
 * Returns a string representation of the file. If a path has been associated
 * with this file, this message will return the path of the file on the file system.
 * If no path has been associated with the file, the string [File (no path)] will
 * be returned.
 */
ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_file_path(myself,argumentList);
	if ( path == CtrStdNil) {
		return ctr_build_string_from_cstring("[File (no path)]");
	}
	return ctr_internal_cast2string(path);
}

/**
 * [File] read
 *
 * Reads contents of a file. Send this message to a file to read the entire contents in
 * one go. For big files you might want to prefer a streaming approach to avoid
 * memory exhaustion (see readBytes etc).
 *
 * Usage:
 *
 * data := File new: '/path/to/mydata.csv', read.
 *
 * In the example above we read the contents of the entire CSV file callled mydata.csv
 * in the variable called data.
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate( sizeof(char) * ( vlen + 1 ) );
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "rb");
	error_code = errno;
	ctr_heap_free( pathString );
	if (!f) {
		ctr_error( "Unable to open file: %s.", error_code );
		return CtrStdNil;
	}
	fseek(f, 0, SEEK_END);
	fileLen=ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer=(char *)ctr_heap_allocate(fileLen+1);
	if (!buffer){
		fprintf(stderr,"Out of memory\n");
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
 *
 * data := '<xml>hello</xml>'.
 * File new: 'myxml.xml', write: data.
 *
 * In the example above we write the XML snippet in variable data to a file
 * called myxml.xml in the current working directory.
 */
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
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
		CtrStdFlow = ctr_error( "Unable to open file: %s.", error_code );
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
 */
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "ab+");
	ctr_heap_free( pathString );
	if (!f) {
		CtrStdFlow = ctr_build_string_from_cstring("Unable to open file.\0");
		CtrStdFlow->info.sticky = 1;
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
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int exists;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
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
 * [File] include
 *
 * Includes the file as a piece of executable code.
 */
ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_INCLUDE );
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_tnode* parsedCode;
	ctr_size vlen;
	char* pathString;
	char* prg;
	uint64_t program_size = 0;
	if (path == NULL) return myself;
	vlen = path->value.svalue->vlen;
	pathString = ctr_heap_allocate_tracked(sizeof(char)*(vlen+1)); //needed until end, pathString appears in stracktrace
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	prg = ctr_internal_readf(pathString, &program_size);
	parsedCode = ctr_cparse_parse(prg, pathString);
	ctr_heap_free( prg );
	ctr_cwlk_subprogram++;
	ctr_cwlk_run(parsedCode);
	ctr_cwlk_subprogram--;
	return myself;
}

/**
 * [File] delete
 *
 * Deletes the file.
 */
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
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
		CtrStdFlow = ctr_build_string_from_cstring( "Unable to delete file." );
		CtrStdFlow->info.sticky = 1;
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
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_size vlen;
	char* pathString;
	FILE* f;
	int prev, sz;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
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
 *
 * Usage:
 *
 * f := File new: '/path/to/file'.
 * f open: 'r+'. #opens file for reading and writing
 *
 * The example above opens the file in f for reading and writing.
 */
ctr_object* ctr_file_open(ctr_object* myself, ctr_argument* argumentList) {
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	ctr_object* pathObj = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	char* mode;
	char* path;
	FILE* handle;
	ctr_resource* rs = ctr_heap_allocate(sizeof(ctr_resource));
	ctr_object* modeStrObj = ctr_internal_cast2string( argumentList->object );
	if ( myself->value.rvalue != NULL ) {
		ctr_heap_free( rs );
		CtrStdFlow = ctr_build_string_from_cstring( "File has already been opened." );
		CtrStdFlow->info.sticky = 1;
		return myself;
	}
	if ( pathObj == NULL ) return myself;
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
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
	ctr_object* result;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	bytes = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	if (bytes < 0) return ctr_build_string_from_cstring("");
	buffer = (char*) ctr_heap_allocate(bytes);
	if (buffer == NULL) {
		CtrStdFlow = ctr_build_string_from_cstring("Cannot allocate memory for file buffer.");
		CtrStdFlow->info.sticky = 1;
		return ctr_build_string_from_cstring("");
	}
	fread(buffer, sizeof(char), (int)bytes, (FILE*)myself->value.rvalue->ptr);
	result = ctr_build_string(buffer, bytes);
	ctr_heap_free( buffer );
	return result;
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	offset = (long int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	error = fseek((FILE*)myself->value.rvalue->ptr, offset, SEEK_CUR);
	if (error) {
		CtrStdFlow = ctr_build_string_from_cstring("Seek failed.");
		CtrStdFlow->info.sticky = 1;
	}
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
	int error;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_SET);
	if (error) {
		CtrStdFlow = ctr_build_string_from_cstring("Seek rewind failed.");
		CtrStdFlow->info.sticky = 1;
	}
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
	int error;
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	if (myself->value.rvalue == NULL) return myself;
	if (myself->value.rvalue->type != 1) return myself;
	error = fseek((FILE*)myself->value.rvalue->ptr, 0, SEEK_END);
	if (error) {
		CtrStdFlow = ctr_build_string_from_cstring("Seek end failed.");
		CtrStdFlow->info.sticky = 1;
	}
	return myself;
}

ctr_object* ctr_file_temp_directory(ctr_object* myself, ctr_argument* argumentList) {
#ifdef __MINGW32__
	char tempDirectory[_MAX_PATH + 1];
	int pathLength = GetTempPath(_MAX_PATH + 1, tempDirectory);
	if (pathLength == 0) {
		CtrStdFlow = ctr_build_string_from_cstring("Unable to get temp directory.");
		CtrStdFlow->info.sticky = 1;
	}
	tempDirectory[pathLength - 1] = '\0';	// Remove trailing backslash.
#else
	char* tempDirectory = getenv("TMPDIR");
	if (tempDirectory == NULL) {
		tempDirectory = "/tmp";
	}
#endif

	return ctr_build_string_from_cstring(tempDirectory);	
}


#ifndef __MINGW32__
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_WRITE );
	pathObj = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	path = ctr_heap_allocate_cstring( pathObj );
	fdObjKey = ctr_build_string_from_cstring("fileDescriptor");
	fdObj = ctr_internal_object_find_property(
		myself,
		fdObjKey,
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	if (fdObj == NULL) {
		fd = open( path, O_CREAT );
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
 */
ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_file_lock_generic( myself, argumentList, LOCK_EX | LOCK_NB );
}

#else

ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(0);
}

ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_bool(1);
}

#endif

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
 * files := File list: '/tmp/testje'.
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
	ctr_check_permission( CTR_SECPRO_NO_FILE_READ );
	path = ctr_internal_cast2string( argumentList->object );
	fileList = ctr_array_new(CtrStdArray, NULL);
	pathValue = ctr_heap_allocate_cstring( path );
	d = opendir( pathValue );
	if (d == 0) {
		CtrStdFlow = ctr_error_text("Failed to open folder.");
		ctr_heap_free(pathValue);
		return CtrStdNil;
	}
	putArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	addArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	putArgumentList->next = ctr_heap_allocate( sizeof( ctr_argument ) );
	while((entry = readdir(d))) {
		fileListItem = ctr_map_new(CtrStdMap, NULL);
		putArgumentList->next->object = ctr_build_string_from_cstring( "file" );
		putArgumentList->object = ctr_build_string_from_cstring(entry->d_name);
		ctr_map_put(fileListItem, putArgumentList);
		putArgumentList->next->object = ctr_build_string_from_cstring( "type" );
		switch(entry->d_type) {
			case DT_REG:
				putArgumentList->object = ctr_build_string_from_cstring("file");
				break;
			case DT_DIR:
				putArgumentList->object = ctr_build_string_from_cstring("folder");
				break;
#ifndef __MINGW32__
// On windows, link and socket file types cannot be distinguished from block devices 
			case DT_LNK:
				putArgumentList->object = ctr_build_string_from_cstring("symbolic link");
				break;
			case DT_SOCK:
				putArgumentList->object = ctr_build_string_from_cstring("socket");
				break;
#endif
			case DT_CHR:
				putArgumentList->object = ctr_build_string_from_cstring("character device");
				break;
			case DT_BLK:
				putArgumentList->object = ctr_build_string_from_cstring("block device");
				break;
			case DT_FIFO:
				putArgumentList->object = ctr_build_string_from_cstring("named pipe");
				break;
			default:
				putArgumentList->object = ctr_build_string_from_cstring("other");
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
