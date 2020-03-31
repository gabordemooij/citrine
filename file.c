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
 * @def
 * File
 *
 * @example
 * ☞ f := File new: '/tmp/test.txt'.
 * f write: 'test'.
 * f close.
 * ☞ q := File new: '/tmp/test.txt'.
 * ✎ write: q read, stop.
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
 * @def
 * [ File ] path
 *
 * @example
 * ☞ f := File new: '/tmp/test.txt'.
 * ✎ write: f path, stop.
 */
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	if (path == NULL) return CtrStdNil;
	return path;
}

/**
 * @def
 * [ File ] string
 * 
 * @example
 * ☞ x := File new.
 * ✎ write: x, stop.
 * ☞ y := File new: '/tmp/a.txt'.
 * ✎ write: y, stop.
 */
ctr_object* ctr_file_to_string(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_file_path(myself,argumentList);
	if ( path == CtrStdNil) {
		return ctr_build_string_from_cstring( CTR_SYM_FILE );
	}
	return ctr_internal_cast2string(path);
}

/**
 * @def
 * [ File ] read
 *
 * @example
 * ☞ f := File new: '/tmp/test.txt'.
 * f write: 'test'.
 * f close.
 * ☞ q := File new: '/tmp/test.txt'.
 * ✎ write: q read, stop.
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
 * @def
 * [ File ] write: [ String ]
 *
 * @example
 * ☞ f := File new: '/tmp/test.txt'.
 * f write: 'test'.
 * f close.
 * ☞ q := File new: '/tmp/test.txt'.
 * ✎ write: q read, stop.
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
 * @def
 * [ File ] append: [ String ]
 *
 * @example
 * ☞ x := File new: '/tmp/a.txt'.
 * x write: '123'.
 * ✎ write: x read, stop.
 * x append: '345'.
 * ✎ write: x read, stop.
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
 * @def
 * [ File ] exists
 *
 * @example
 * ☞ f := '/tmp/a.txt'.
 * ☞ x := File new: f.
 * Program shell: 'rm ' + f.
 * ✎ write: f exists, stop.
 * Program shell: 'touch ' + f.
 * ✎ write: f exists, stop.
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
 * @def
 * [ File ] delete
 * 
 * @example
 * ☞ x := File new: '/tmp/a.txt'.
 * x write: 'abc'.
 * ✎ write: x exists, stop.
 * x delete.
 * ✎ write: x exists, stop.
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
 * @def
 * [ File ] size
 * 
 * @example
 * ☞ x := File new: '/tmp/a.txt'.
 * x write: 'abc'.
 * ✎ write: x size, stop.
 * x append: 'def'.
 * ✎ write: x size, stop.
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
 * @def
 * [ File ] open: [ String ].
 *
 * @example
 * ☞ f := File new: '/tmp/a.txt'.
 * f write: 'abcdefgh'.
 * f open: 'r'.
 * ☞ x := f end, seek: -5, read bytes: 3.
 * ☞ y := f rewind, seek: 5, read bytes: 3.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
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
 * @def
 * [ File ] close
 * 
 * @example
 * ☞ f := '/tmp/a.txt'.
 * f open: 'r+'
 * ☞ n := f write bytes: '123'.
 * ✎ write: n, stop.
 * f close.
 * ☞ q := '/tmp/a.txt'.
 * q open: 'r'.
 * ☞ x := q read bytes: 2.
 * ✎ write: x, stop.
 * q close.
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
 * @def
 * [ File ] read bytes: [ Number ]
 * 
 * @example
 * ☞ f := '/tmp/a.txt'.
 * f open: 'r+'
 * ☞ n := f write bytes: '123'.
 * ✎ write: n, stop.
 * f close.
 * ☞ q := '/tmp/a.txt'.
 * q open: 'r'.
 * ☞ x := q read bytes: 2.
 * ✎ write: x, stop.
 * q close.
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
 * @def
 * [ File ] write bytes: [ String ]
 * 
 * @example
 * ☞ f := '/tmp/a.txt'.
 * f open: 'r+'
 * ☞ n := f write bytes: '123'.
 * ✎ write: n, stop.
 * f close.
 * ☞ q := '/tmp/a.txt'.
 * q open: 'r'.
 * ☞ x := q read bytes: 2.
 * ✎ write: x, stop.
 * q close.
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
 * @def
 * [ File ] seek
 *
 * @example
 * ☞ f := File new: '/tmp/a.txt'.
 * f write: 'abcdefgh'.
 * f open: 'r'.
 * ☞ x := f end, seek: -5, read bytes: 3.
 * ☞ y := f rewind, seek: 5, read bytes: 3.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
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
 * @def
 * [ File ] rewind
 *
 * @example
 * ☞ f := File new: '/tmp/a.txt'.
 * f write: 'abcdefgh'.
 * f open: 'r'.
 * ☞ x := f end, seek: -5, read bytes: 3.
 * ☞ y := f rewind, seek: 5, read bytes: 3.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
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
 * @def
 * [ File ] end.
 *
 * @example
 * ☞ f := File new: '/tmp/a.txt'.
 * f write: 'abcdefgh'.
 * f open: 'r'.
 * ☞ x := f end, seek: -5, read bytes: 3.
 * ☞ y := f rewind, seek: 5, read bytes: 3.
 * ✎ write: x, stop.
 * ✎ write: y, stop.
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
 * @def
 * [ File ] unlock
 * 
 * @example
 * ☞ f := File new: 'x.txt'.
 * ☞ l := f unlock.
 * ✎ write: l, stop.
 */
ctr_object* ctr_file_unlock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_file_lock_generic( myself, argumentList, LOCK_UN | LOCK_NB );
}

/**
 * @def
 * [ File ] lock
 * 
 * @example
 * ☞ f := File new: 'x.txt'.
 * ☞ l := f lock.
 * ✎ write: l, stop.
 */
ctr_object* ctr_file_lock(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_file_lock_generic( myself, argumentList, LOCK_EX | LOCK_NB );
}

/**
 * @def
 * [ File ] list: [ String ].
 * 
 * @example
 * Program shell: 'mkdir /tmp/files'.
 * Program shell: 'touch /tmp/files/a.txt'.
 * Program shell: 'touch /tmp/files/b.txt'.
 * ☞ x := File list: '/tmp/files'.
 * ✎ write: x, stop.
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
