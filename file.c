/**
 * FileNew
 *
 * Creates a new file object based on the specified path.
 */
ctr_object* ctr_file_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* s = ctr_object_make();
	ctr_object* pathObject;
	s->info.type = CTR_OBJECT_TYPE_OTMISC;
	s->link = myself;
	s->value.rvalue = malloc(sizeof(ctr_resource));
	s->value.rvalue->type = 1;
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
 * FilePath
 *
 * Returns the path of a file.
 */
ctr_object* ctr_file_path(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return CtrStdNil;
	return path;
}

/**
 * FileRead
 *
 * Reads contents of a file.
 */
ctr_object* ctr_file_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_object* str;
	size_t vlen, fileLen;
	char* pathString;
	char *buffer;
	FILE* f;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "rb");
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
 * FileWrite
 *
 * Writes content to a file.
 */
ctr_object* ctr_file_write(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	FILE* f;
	size_t vlen;
	char* pathString;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "wb+");
	if (!f) {
		CtrStdError = ctr_build_string_from_cstring("Unable to open file.\0");
		return CtrStdNil;
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

/**
 * FileAppend
 *
 * Appends content to a file.
 */
ctr_object* ctr_file_append(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* str = ctr_internal_cast2string(argumentList->object);
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	size_t vlen;
	char* pathString;
	FILE* f;
	if (path == NULL) return CtrStdNil;
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "ab+");
	if (!f) {
		CtrStdError = ctr_build_string_from_cstring("Unable to open file.\0");
		return CtrStdNil;
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

/**
 * FileExists
 *
 * Returns True if the file exists and False otherwise.
 */
ctr_object* ctr_file_exists(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	size_t vlen;
	char* pathString;
	FILE* f;
	int exists;
	if (path == NULL) return ctr_build_bool(0);
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	f = fopen(pathString, "r");
	exists = (f != NULL );
	if (f) {
		fclose(f);
	}
	return ctr_build_bool(exists);
}

/**
 * FileInclude
 *
 * Includes the file as a piece of executable code.
 */
ctr_object* ctr_file_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	ctr_tnode* parsedCode;
	size_t vlen;
	char* pathString;
	char* prg;
	if (path == NULL) return ctr_build_bool(0);
	vlen = path->value.svalue->vlen;
	pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	prg = ctr_internal_readf(pathString);
	parsedCode = ctr_dparse_parse(prg);
	ctr_cwlk_run(parsedCode);
	return myself;
}

/**
 * FileDelete
 *
 * Deletes the file.
 */
ctr_object* ctr_file_delete(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	size_t vlen;
	char* pathString;
	int r;
	if (path == NULL) return ctr_build_bool(0);
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
 * FileSize
 *
 * Returns the size of the file.
 */
ctr_object* ctr_file_size(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	size_t vlen;
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
