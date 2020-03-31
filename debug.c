
void ctr_internal_print_object( ctr_object* object ) {

	char* types[9];
	
	types[0] = "nil";
	types[1] = "bool";
	types[2] = "number";
	types[3] = "string";
	types[4] = "block";
	types[5] = "object";
	types[6] = "native function";
	types[7] = "array";
	types[8] = "misc";
	types[9] = "external";

	char* value = calloc(40,1);
	
	
	
	if ( object->info.type == 3) {
		memcpy( value, object->value.svalue->value, object->value.svalue->vlen );
		printf("%p: %s %s \n", object, types[object->info.type], value);
	} else {
		char* type;
		type = types[object->info.type];
		if (object->link == CtrStdMap) {
			type = "object (map)";
		}
		printf("%p: %s \n", object, type);
	}
	free(value);
}
