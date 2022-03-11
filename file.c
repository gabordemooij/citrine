#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>
#include "citrine.h"

/**
 * Path Separator.
 */
const char* PATH_SEP =
#if defined _WIN32 || defined __CYGWIN__
	"\\";
#else
	"/";
#endif

/**
 * @def
 * File
 *
 * @example
 * ☞ f ≔ File new: (Path /tmp: ‘test.txt’).
 * f write: ‘test’.
 * ☞ q ≔ File new: (Path /tmp: ‘test.txt’).
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
 * ☞ f ≔ File new: (Path tmp: ‘test.txt’).
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
 * ☞ x ≔ File new.
 * ✎ write: x, stop.
 * ☞ y ≔ File new: (Path /tmp: ‘a.txt’).
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
 * ☞ f ≔ File new: (Path /tmp: ‘test.txt’).
 * f write: ‘test’.
 * ☞ q ≔ File new: (Path /tmp: ‘test.txt’).
 * ✎ write: q read, stop.
 */
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	ctr_object* str;
	ctr_size fileLen;
	char* pathString;
	char *buffer;
	size_t bytesRead;
	FILE* f;
	int error_code;
	if (path == NULL) return CtrStdNil;
	pathString = ctr_heap_allocate_cstring( path );
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
	bytesRead = fread(buffer, 1, fileLen, f);
	fclose(f);
	if (bytesRead != fileLen) {
		ctr_error( CTR_ERR_OPEN, error_code );
		ctr_heap_free( buffer );
		return CtrStdNil;
	}
	str = ctr_build_string(buffer, fileLen);
	ctr_heap_free( buffer );
	return str;
}

/**
 * @def
 * [ File ] write: [ String ]
 *
 * @example
 * ☞ f ≔ File new: (Path /tmp: ‘test.txt’).
 * f write: ‘test’.
 * ☞ q ≔ File new: (Path /tmp: ‘test.txt’).
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
 * ☞ x ≔ File new: (Path /tmp: ‘a.txt’).
 * x write: ‘123’.
 * ✎ write: x read, stop.
 * x append: ‘345’.
 * ✎ write: x read, stop.
 */
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	int error_code;
	char* pathString;
	FILE* f;
	if (path == NULL) return myself;
	pathString = ctr_heap_allocate_cstring( path );
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
 * ☞ x ≔ File new: (Path /tmp unknown).
 * ✎ write: x exists, stop.
 */
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	char* pathString;
	FILE* f;
	int exists;
	if (path == NULL) return CtrStdBoolFalse;
	pathString = ctr_heap_allocate_cstring( path );
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
 * ☞ x ≔ File new: (Path /tmp: ‘a.txt’).
 * x write: ‘abc’.
 * ✎ write: x exists, stop.
 * x delete.
 * ✎ write: x exists, stop.
 */
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	char* pathString;
	int r;
	if (path == NULL) return myself;
	pathString = ctr_heap_allocate_cstring( path );
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
 * ☞ x ≔ File new: (Path /tmp: ‘a.txt’).
 * x write: ‘abc’.
 * ✎ write: x size, stop.
 * x append: ‘def’.
 * ✎ write: x size, stop.
 */
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string_from_cstring( "path" ), 0);
	char* pathString;
	FILE* f;
	int prev, sz;
	if (path == NULL) return ctr_build_number_from_float(0);
	pathString = ctr_heap_allocate_cstring( path );
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
 * [ File ] list: [ String ].
 * 
 * @example
 * ☞ x ≔ File list: Path /usr games.
 * ✎ write: x, stop.
 */
ctr_object* ctr_file_list(ctr_object* myself, ctr_argument* argumentList) {
	/**
	 * Returns a list with strings, not file objects, because
	 * there can be all sorts of objects in a folder and only
	 * files are represented by objects. If we would have returned
	 * objects, we would need objects for File, Folder, Link,
	 * Device etc. (depending on OS). Since Citrine only offers
	 * a limited IO-interface (because of minimalism and it might just
	 * as well be embedded into FAAS/SAAS, not needing any of this)
	 * this would quickly go beyond the scope of the project.
	 */
	DIR* d;
	struct dirent* entry;
	char* pathValue;
	char  pathBuf[PATH_MAX + 1];
	char  fullPath[PATH_MAX + 1];
	ctr_object* fileList;
	ctr_object* fileListItem;
	ctr_object* path;
	ctr_argument* putArgumentList;
	ctr_argument* addArgumentList;
	path = ctr_internal_cast2string( argumentList->object );
	fileList = ctr_array_new(CtrStdArray, NULL);
	pathValue = ctr_heap_allocate_cstring( path );
	struct stat st;
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
		if ((strlen(fullPath) + strlen(PATH_SEP) + strlen(entry->d_name)) > PATH_MAX) {
			continue;
		}
		strcpy( fullPath, pathValue );
		strcat( fullPath, PATH_SEP );
		strcat( fullPath, entry->d_name);
		realpath( fullPath, pathBuf );
		/* lstat is slow, but we have no choice, there is no other way to keep this portable */
		lstat(pathBuf, &st);
		if (S_ISREG(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_FILE );
		else if (S_ISDIR(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_FLDR );
		else if (S_ISLNK(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_SLNK );
		else if (S_ISCHR(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_CDEV );
		else if (S_ISBLK(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_BDEV );
		else if (S_ISSOCK(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_SOCK );
		else if (S_ISFIFO(st.st_mode))
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_NPIP );
		else
			putArgumentList->object = ctr_build_string_from_cstring( CTR_MSG_DSC_OTHR );
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