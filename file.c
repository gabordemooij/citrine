

ctr_object* ctr_file_new(ctr_object* myself, args* argumentList) {
	ctr_object* s = ctr_object_make();
	s->info.type = OTMISC;
	s->link = myself;
	s->info.flagb = 1;
	if (!argumentList->object) {
		printf("Missing argument\n");
		exit(1);
	}
	s->value.rvalue = malloc(sizeof(cres));
	s->value.rvalue->type = 1;
	ctr_object* pathObject = ctr_internal_create_object(OTSTRING);
	pathObject->info.type = OTSTRING;
	pathObject->value.svalue = (ctr_string*) malloc(sizeof(ctr_string));
	pathObject->value.svalue->value = (char*) malloc(sizeof(char) * argumentList->object->value.svalue->vlen);
	memcpy(pathObject->value.svalue->value, argumentList->object->value.svalue->value, argumentList->object->value.svalue->vlen);
	pathObject->value.svalue->vlen = argumentList->object->value.svalue->vlen;
	ctr_internal_object_add_property(s, ctr_build_string("path",4), pathObject, 0);
	return s;
}

ctr_object* ctr_file_path(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return Nil;
	return path;
}

ctr_object* ctr_file_read(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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
	ctr_object* str = ctr_build_string(buffer, fileLen);
	free(buffer);
	//free(f);
	return str;
}

ctr_object* ctr_file_write(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	ctr_object* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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

ctr_object* ctr_file_append(ctr_object* myself, args* argumentList) {
	if (!argumentList->object) {
		printf("Missing string argument to write to file.\n");
		exit(1);
	}
	ctr_object* str = argumentList->object;
	if (str->info.type != OTSTRING) {
		printf("First argument must be string\n");
		exit(1);
	}
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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

ctr_object* ctr_file_exists(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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

ctr_object* ctr_file_include(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
	if (path == NULL) return ctr_build_bool(0);
	long vlen = path->value.svalue->vlen;
	char* pathString = malloc(vlen + 1);
	memcpy(pathString, path->value.svalue->value, vlen);
	memcpy(pathString+vlen,"\0",1);
	char* prg = readf(pathString);
	tnode* parsedCode = dparse_parse(prg);
	cwlk_run(parsedCode);
}

ctr_object* ctr_file_delete(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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


ctr_object* ctr_file_size(ctr_object* myself) {
	ctr_object* path = ctr_internal_object_find_property(myself, ctr_build_string("path",4), 0);
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
