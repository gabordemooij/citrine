

obj* ctr_file_new(obj* myself, args* argumentList) {
	obj* s = ctr_object_make();
	s->info.type = OTMISC;
	s->link = myself;
	s->info.flagb = 1;
	if (!argumentList->object) {
		printf("Missing argument\n");
		exit(1);
	}
	s->value.rvalue = malloc(sizeof(cres));
	s->value.rvalue->type = 1;
	obj* pathObject = ctr_internal_create_object(OTSTRING);
	pathObject->info.type = OTSTRING;
	pathObject->value.svalue = (ctr_string*) malloc(sizeof(ctr_string));
	pathObject->value.svalue->value = (char*) malloc(sizeof(char) * argumentList->object->value.svalue->vlen);
	memcpy(pathObject->value.svalue->value, argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
	pathObject->value.svalue->vlen = argumentList->object->value.svalue->vlen;
	ctr_internal_object_add_property(s, ctr_build_string("path",4), pathObject, 0);
	return s;
}

obj* ctr_file_path(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	return path;
}

obj* ctr_file_read(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "rb");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	char *buffer;
	unsigned long fileLen;
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
	obj* str = ctr_build_string(buffer, fileLen);
	free(buffer);
	//free(f);
	return str;
}

obj* ctr_file_write(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	obj* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "wb+");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	//free(f);
	return myself;
}

obj* ctr_file_append(obj* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	obj* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "ab+");
	if (!f) {
		printf("Unable to open file!\n");
		exit(1);
	}
	fwrite(str->value.svalue->value, sizeof(char), str->value.svalue->vlen, f);
	fclose(f);
	return myself;
}

obj* ctr_file_exists(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "r");
	int exists = (f != NULL );
	if (f) {
		fclose(f);
		//free(f);
	}
	return ctr_build_bool(exists);
}

obj* ctr_file_include(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	char* prg = readf(pathString);
	tnode* parsedCode = dparse_parse(prg);
	cwlk_run(parsedCode);
}

obj* ctr_file_delete(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	int r = remove(pathString);
	if (r!=0) {
		printf("Cant delete file.");
		exit(1);
	}
	return myself;
}


obj* ctr_file_size(obj* myself) {
	obj* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_number_from_float(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	FILE* f = fopen(pathString, "r");
	if (f == NULL) return ctr_build_number_from_float(0);
	int prev = ftell(f);
    fseek(f, 0L, SEEK_END);
    int sz=ftell(f);
    fseek(f,prev,SEEK_SET); //go back to where we were
    if (f) {
		fclose(f);
		//free(f);
	}
    return ctr_build_number_from_float( (ctr_number) sz );
}
