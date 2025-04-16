#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn/jsmn.h"
#include <gui.h>
#include "../../citrine.h"
#include <ffi.h>

ctr_object* CtrMediaFFIObjectBase;
ctr_object* CtrMediaDataBlob;

ctr_object* ctr_ffi_object_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	return instance;
}


void ctr_media_blob_destructor(ctr_resource* resource_value) {
	// the destructor cant remove the memory automatically
	// because upon returning from a c-function it is not known
	// whether the memory belongs to us or to the called function
	// you have to call free / freestruct.
}

ffi_type* ctr_internal_gui_ffi_map_type(char* description);
void* ctr_internal_gui_ffi_convert_value(ffi_type* type, ctr_object* obj);


/**
 * @def
 * [ Blob ] deref.
 * 
 * @example
 * blob deref.
 *
 * @result
 * @info-blob-deref
 */
ctr_object* ctr_blob_deref(ctr_object* myself, ctr_argument* argumentList) {
	myself->value.rvalue->ptr = (void*) *((void**)myself->value.rvalue->ptr);
	return myself;
}

/**
 * @def
 * [ Blob ] fill: [ Sequence ]
 * 
 * @example
 * blob fill: (Sequence new ; 1 ; 2 ; 3).
 *
 * @result
 * @info-blob-fill
 */
ctr_object* ctr_blob_fill(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arr = argumentList->object;
	for(int i = 0; i < arr->value.avalue->head; i++) {
		*((char*)myself->value.rvalue->ptr + i) = (char) ctr_tonum(*(arr->value.avalue->elements + i));
	}
	return myself;
}

/**
 * @def
 * [ Blob ] free
 * 
 * @example
 * blob free
 *
 * @result
 * @info-blob-free
 */
ctr_object* ctr_blob_free(ctr_object* myself, ctr_argument* argumentList) {
	ctr_heap_free(myself->value.rvalue->ptr);
	myself->value.rvalue->ptr = NULL;
	return myself;
}


ctr_object* ctr_blob_tostring(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring(myself->value.rvalue->ptr);
}


/**
 * @def
 * [ Blob ] new: [ Number ]
 * 
 * @example
 * >> x := Blob new: 100.
 * 
 * @result
 * @info-blob-new
 */
ctr_object* ctr_blob_new_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	size_t size = (size_t) ctr_tonum(argumentList->object);
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->ptr = ctr_heap_allocate(size);
	buffer->destructor = &ctr_media_blob_destructor;
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

/**
 * @def
 * [ Blob ] utf8: [ Text ]
 * 
 * @example
 * blob utf8: ['abc'].
 * 
 * @result
 * @info-blob-utf8
 */
ctr_object* ctr_blob_utf8_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->ptr = ctr_heap_allocate_cstring(argumentList->object);
	buffer->destructor = &ctr_media_blob_destructor;
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

ffi_type* ctr_internal_gui_ffi_map_type_obj(ctr_object* obj);
ctr_object* ctr_blob_new_set_type(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer;
	ffi_type* type = ctr_internal_gui_ffi_map_type_obj(argumentList->next->object);
	if (type) {
		buffer = ctr_heap_allocate(sizeof(ctr_resource));
		buffer->destructor = &ctr_media_blob_destructor;
		buffer->ptr = ctr_internal_gui_ffi_convert_value(type, argumentList->object);
	} else {
		buffer = NULL;
	}
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

/**
 * @def
 * [ Blob ] from: [ Number ] length: [ Number ]
 * 
 * @example
 * >> data := blob from: 0 length 10.
 * 
 * @result
 * @info-blob-read
 */
ctr_object* ctr_blob_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* pushArg;
	ctr_argument* elnumArg;
	ctr_object* elnum;
	ctr_object* startElement = ctr_internal_cast2number(argumentList->object);
	ctr_object* count = ctr_internal_cast2number(argumentList->next->object);
	int start = (int) startElement->value.nvalue;
	int len = (int) count->value.nvalue;
	int i = 0;
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	for(i = start; i < start + len; i++) {
		pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnum = ctr_build_number_from_float((ctr_number) i);
		elnumArg->object = elnum;
		pushArg->object = ctr_build_number_from_float( (ctr_number) *((char*) myself->value.rvalue->ptr + (i - 1)) );
		ctr_array_push(newArray, pushArg);
		ctr_heap_free( elnumArg );
		ctr_heap_free( pushArg );
	}
	return newArray;
}

/**
 * @def
 * FFI
 * 
 * @example
 * media link: (
 * 		Sequence new ; ['libc.so.6'] ;
 * 		['printf'] ;
 * 		( Sequence new ; ['pointer'] ; ['int'] ) ;
 * 		['void'] ;
 * 		['stdio'] ;
 * 		['printf:'] ;
 * 		1
 * ).
 * >> b := Blob utf8: ['I got %d appels'].
 * stdio printf: (Sequence new ; b ; 6).
 * 
 * @result
 * I got 6 apples
 */
struct CtrMediaFFI {
	void* handle;
	void* symbol;
	ffi_type* args[20];
	ffi_type* rtype;
	int nargs;
	ffi_cif* cif;
	ctr_object* owner;
};
typedef struct CtrMediaFFI CtrMediaFFI;

struct ctr_media_test_struct {
	int a;
	int b;
};
typedef struct ctr_media_test_struct ctr_media_test_struct;

int ctr_media_internal_structtest(ctr_media_test_struct sum) {
	// printf("sum of %d and %d is: %d \n", sum.a, sum.b, sum.a + sum.b); -- for debugging
	return sum.a + sum.b;
}

void ctr_media_ffi_destructor(ctr_resource* resource_value) {
	CtrMediaFFI* ff = (CtrMediaFFI*) resource_value->ptr;
	ctr_heap_free(ff->cif);
	ff->cif = NULL;
	ctr_heap_free(ff);	
}


ffi_type* ctr_internal_gui_ffi_map_type(char* description) {
	if (strcmp(description, "void")==0) {
		return &ffi_type_void;
	} else if (strcmp(description, "uint")==0) {
		return &ffi_type_uint;
	} else if (strcmp(description, "int")==0) {
		return &ffi_type_sint;
	} else if (strcmp(description,"uint8_t")==0) {
		return &ffi_type_uint8;
	} else if (strcmp(description,"int8_t")==0) {
		return &ffi_type_sint8;
	} else if (strcmp(description,"uint16_t")==0) {
		return &ffi_type_uint16;
	} else if (strcmp(description,"int16_t")==0) {
		return &ffi_type_sint16;
	} else if (strcmp(description,"uint32_t")==0) {
		return &ffi_type_uint32;
	} else if (strcmp(description,"int32_t")==0) {
		return &ffi_type_sint32;
	} else if (strcmp(description,"uint64_t")==0) {
		return &ffi_type_uint64;
	} else if (strcmp(description,"int64_t")==0) {
		return &ffi_type_sint64;
	} else if (strcmp(description,"float")==0) {
		return &ffi_type_float;
	} else if (strcmp(description,"double")==0) {
		return &ffi_type_double;
	} else if (strcmp(description,"ushort")==0) {
		return &ffi_type_ushort;
	} else if (strcmp(description,"short")==0) {
		return &ffi_type_sshort;
	} else if (strcmp(description,"uchar")==0) {
		return &ffi_type_uchar;
	} else if (strcmp(description,"char")==0) {
		return &ffi_type_schar;
	} else if (strcmp(description,"pointer")==0) {
		return &ffi_type_pointer;
	} else if (strcmp(description,"ulong")==0) {
		return &ffi_type_ulong;
	} else if (strcmp(description,"long")==0) {
		return &ffi_type_slong;
	}
	return NULL;
}

ffi_type* ctr_internal_gui_ffi_map_type_obj(ctr_object* obj) {
	ffi_type* result;
	if (obj->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		char* description = ctr_heap_allocate_cstring(obj);
		result = ctr_internal_gui_ffi_map_type(description);
		ctr_heap_free(description);
	} else {
		result = obj->value.rvalue->ptr;
	}
	return result;
}

/**
 * @def
 * [ Blob ] struct: [ Sequence ]
 * 
 * @example
 * >> ints ≔ Blob struct: (Sequence new ; ['int'] ; ['int']).
 *
 * @result
 * @info-blob-struct
 */
ctr_object* ctr_blob_new_struct(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->destructor = &ctr_media_blob_destructor;
	ffi_type* cstruct = ctr_heap_allocate(sizeof(ffi_type));
	cstruct->type = FFI_TYPE_STRUCT;
	cstruct->alignment = 0;
	cstruct->size = 0;
	if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		ctr_object* arr = argumentList->object;
		int nargs = arr->value.avalue->head;
		cstruct->elements = (ffi_type**) ctr_heap_allocate(sizeof(ffi_type*) * (nargs+1));
		int i;
		for (i = 0; i < nargs; i++) {
			cstruct->elements[i] = ctr_internal_gui_ffi_map_type_obj(*(arr->value.avalue->elements + i));
		}
		cstruct->elements[i] = NULL;
	}
	buffer->ptr = cstruct;
	instance->value.rvalue = buffer;
	return instance;
}

/**
 * @def
 * [ Blob ] freestruct
 * 
 * @example
 * blob freestruct
 *
 * @result
 * @info-blob-freestruct
 */
ctr_object* ctr_blob_free_struct(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->info.type == CTR_OBJECT_TYPE_OTEX) {
		ctr_resource* buffer = (ctr_resource*) myself->value.rvalue;
		ffi_type* cstruct = (ffi_type*) buffer->ptr;
		if (cstruct->elements) {
			ctr_heap_free(cstruct->elements);
		}
	}
	return CtrStdNil;
}

void* ctr_internal_gui_ffi_convert_value(ffi_type* type, ctr_object* obj) {
	//allocate for return type, must be at least ffi_arg
	ctr_size size = type->size;
	if (obj == NULL) {
		if (type->size < sizeof(ffi_arg)) {
			size = sizeof(ffi_arg); 
		}
	}
	void* ptr = ctr_heap_allocate(size);
	if (obj != NULL) {
		if (type == &ffi_type_uint8) {
			*((uint8_t*)ptr) = (uint8_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint8) {
			*((int8_t*)ptr) = (int8_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint16) {
			*((uint16_t*)ptr) = (uint16_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint16) {
			*((int16_t*)ptr) = (int16_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint32) {
			*((uint32_t*)ptr) = (uint32_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint32) {
			*((int32_t*)ptr) = (int32_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint64) {
			*((uint64_t*)ptr) = (uint64_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint64) {
			*((int64_t*)ptr) = (int64_t) ctr_tonum(obj);
		} else if (type == &ffi_type_float) {
			*((float*)ptr) = (float) ctr_tonum(obj);
		} else if (type == &ffi_type_double) {
			*((double*)ptr) = (double) ctr_tonum(obj);
		} else if (type == &ffi_type_uchar) {
			*((unsigned char*)ptr) = (unsigned char) ctr_tonum(obj);
		} else if (type == &ffi_type_schar) {
			*((char*)ptr) = (char) ctr_tonum(obj);
		} else if (type == &ffi_type_ushort) {
			*((unsigned short*)ptr) = (unsigned short) ctr_tonum(obj);
		} else if (type == &ffi_type_sshort) {
			*((short*)ptr) = (short) ctr_tonum(obj);
		} else if (type == &ffi_type_uint) {
			*((unsigned int*)ptr) = (unsigned int) ctr_tonum(obj);
		} else if (type == &ffi_type_sint) {
			*((int*)ptr) = (int) ctr_tonum(obj);
		} else if (type == &ffi_type_ulong) {
			*((unsigned long*)ptr) = (unsigned long) ctr_tonum(obj);
		} else if (type == &ffi_type_slong) {
			*((long*)ptr) = (long) ctr_tonum(obj);
		} else if (type == &ffi_type_pointer) {
			if (obj == CtrStdNil) {
				*((void**)ptr) = NULL;
			}
			else if (obj->link == CtrMediaDataBlob) {
				*((void**)ptr) = (void*) obj->value.rvalue->ptr;
			}
			else {
				ctr_error("FFI Pointer requires Blob.\n", 0);
			}
		} else {
			memcpy(ptr, obj->value.rvalue->ptr, type->size);
		}
	}
	return ptr;
}

ctr_object* ctr_internal_gui_ffi_convert_value_back(ffi_type* type, void* ptr) {
	ctr_object* result = CtrStdNil;
	if (type == &ffi_type_void) {
		result = CtrStdNil;
	} else if (type == &ffi_type_uint8) {
		result = ctr_build_number_from_float( (ctr_number) *((uint8_t*) ptr) );
	} else if (type == &ffi_type_sint8) {
		result = ctr_build_number_from_float( (ctr_number) *((int8_t*) ptr) );
	} else if (type == &ffi_type_uint16) {
		result = ctr_build_number_from_float( (ctr_number) *((uint16_t*) ptr) );
	} else if (type == &ffi_type_sint16) {
		result = ctr_build_number_from_float( (ctr_number) *((int16_t*) ptr) );
	} else if (type == &ffi_type_uint32) {
		result = ctr_build_number_from_float( (ctr_number) *((uint32_t*) ptr) );
	} else if (type == &ffi_type_sint32) {
		result = ctr_build_number_from_float( (ctr_number) *((int32_t*) ptr) );
	} else if (type == &ffi_type_uint64) {
		result = ctr_build_number_from_float( (ctr_number) *((uint64_t*) ptr) );
	} else if (type == &ffi_type_sint64) {
		result = ctr_build_number_from_float( (ctr_number) *((int64_t*) ptr) );
	} else if (type == &ffi_type_float) {
		result = ctr_build_number_from_float( (ctr_number) *((float*) ptr) );
	} else if (type == &ffi_type_double) {
		result = ctr_build_number_from_float( (ctr_number) *((double*) ptr) );
	} else if (type == &ffi_type_uchar) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned char*) ptr) );
	} else if (type == &ffi_type_schar) {
		result = ctr_build_number_from_float( (ctr_number) *((char*) ptr) );
	} else if (type == &ffi_type_ushort) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned short*) ptr) );
	} else if (type == &ffi_type_sshort) {
		result = ctr_build_number_from_float( (ctr_number) *((short*) ptr) );
	} else if (type == &ffi_type_uint) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned int*) ptr) );
	} else if (type == &ffi_type_sint) {
		result = ctr_build_number_from_float( (ctr_number) *((int*) ptr) );
	} else if (type == &ffi_type_ulong) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned long*) ptr) );
	} else if (type == &ffi_type_slong) {
		result = ctr_build_number_from_float( (ctr_number) *((long*) ptr) );
	} else if (type == &ffi_type_pointer) {
		ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
		instance->link = CtrMediaDataBlob;
		ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
		buffer->ptr = *((void**)ptr);
		buffer->destructor = &ctr_media_blob_destructor;
		instance->value.rvalue = buffer;
		instance->info.sticky = 1;
		result = instance;
	}
	return result;
}

CtrMediaFFI* ctr_internal_gui_ffi_get(ctr_object* obj, ctr_object* property) {
	ctr_object* resource_holder = ctr_internal_object_find_property(
		obj,
		ctr_internal_cast2string( property ),
		0
	);
	if (!resource_holder) {
		return NULL;
	}
	ctr_resource* resource = resource_holder->value.rvalue;
	if (!resource) {
		return NULL;
	}
	CtrMediaFFI* ff = (CtrMediaFFI*) resource->ptr;
	return ff;
}

ctr_object* ctr_media_ffi_respond_to_and_and_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_gui_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[3];
	values[0] = ctr_internal_gui_ffi_convert_value(ff->args[0], argumentList->next->object);
	values[1] = ctr_internal_gui_ffi_convert_value(ff->args[1], argumentList->next->next->object);
	values[2] = ctr_internal_gui_ffi_convert_value(ff->args[2], argumentList->next->next->next->object);
	return_value = ctr_internal_gui_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_gui_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(values[0]);
	ctr_heap_free(values[1]);
	ctr_heap_free(values[2]);
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to_and_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_gui_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[20];
	values[0] = ctr_internal_gui_ffi_convert_value(ff->args[0], argumentList->next->object);
	values[1] = ctr_internal_gui_ffi_convert_value(ff->args[1], argumentList->next->next->object);
	return_value = ctr_internal_gui_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_gui_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(values[0]);
	ctr_heap_free(values[1]);
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	ctr_object* arr;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_gui_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[20];
	if (argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		arr = argumentList->next->object;
		if (arr->value.avalue->head>20) {
			return ctr_error("Too many parameters for FFI.", 0);
		}
		for(int i = 0; i<arr->value.avalue->head; i++) {
			if (i > ff->nargs) break;
			values[i] = ctr_internal_gui_ffi_convert_value(ff->args[i], *(arr->value.avalue->elements+i));
		}
	} else {
		values[0] = ctr_internal_gui_ffi_convert_value(ff->args[0], argumentList->next->object);
	}
	return_value = ctr_internal_gui_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_gui_ffi_convert_value_back(ff->rtype, return_value);
	if (argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		arr = argumentList->next->object;
		for(int i = 0; i<arr->value.avalue->head; i++) {
			if (i > ff->nargs) break;
			ctr_heap_free(values[i]);
		}
	} else {
		ctr_heap_free(values[0]);
	}
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_gui_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	return_value = ctr_internal_gui_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, NULL);
	result = ctr_internal_gui_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(return_value);
	return result;
}

/**
 * FFI creates objects in the world whose properties
 * contain function names and call definitions.
 * The generic methods are then used to map those properties
 * to messages.
 */
CtrMediaFFI* CtrMediaPreviousFFIEntry = NULL;
void ctr_internal_gui_ffi(ctr_object* ffispec) {
	char* library_path;
	char* symbol_name;
	char* ffi_property_name;
	char* rtype_desc;
	CtrMediaFFI* ff;
	ctr_object* arg1;
	ctr_object* arg2;
	ctr_object* arg3;
	ctr_object* arg4;
	ctr_object* arg5;
	ctr_object* arg6;
	ctr_object* arg7 = NULL;
	ctr_object* cif_resource_holder;
	ctr_resource* resource;
	// Check number of required arguments for FFI-binding
	if (ffispec->value.avalue->head < 6) {
		ctr_error("Too few arguments to create FFI binding",0);
		return;
	}
	// Load arguments	
	arg1 = *(ffispec->value.avalue->elements + 0);
	arg2 = *(ffispec->value.avalue->elements + 1);
	arg3 = *(ffispec->value.avalue->elements + 2);
	arg4 = *(ffispec->value.avalue->elements + 3);
	arg5 = *(ffispec->value.avalue->elements + 4);
	arg6 = *(ffispec->value.avalue->elements + 5);
	if (ffispec->value.avalue->head == 7) {
		arg7 = *(ffispec->value.avalue->elements + 6);	
	}
	// Create FFI entry
	ff = ctr_heap_allocate(sizeof(CtrMediaFFI));
	if (!ff) {
		ctr_error("Unable to allocate FFI handle.", 0);
		return;
	}
	ff->cif = ctr_heap_allocate(sizeof(ffi_cif));
	ff->handle = NULL;
	// Load dynamic library
	if (arg1 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->handle = CtrMediaPreviousFFIEntry->handle;
		} else {
			ctr_error("No FFI handle", 0);
			return;
		}
	} else {
		library_path = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg1));
		if (strcmp("@structtest", library_path)!=0) {
		#ifdef WIN
		ff->handle = LoadLibrary(library_path);
		#else
		ff->handle = dlopen(library_path, RTLD_NOW);
		#endif
		ctr_heap_free(library_path);
		if ( !ff->handle ) {
			#if defined WIN
			ctr_error("Unable to open library",0);
			#else
			ctr_error(dlerror(),0);
			#endif
			return;
		}
		} else {
			ctr_heap_free(library_path);
		}
	}
	// Obtain symbol reference
	if (arg2 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->symbol = CtrMediaPreviousFFIEntry->symbol;
		} else {
			ctr_error("No FFI symbol", 0);
			return;
		}
	} else {
		symbol_name = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg2));
		if (strcmp("@structtest", symbol_name)==0) {
			ff->symbol = &ctr_media_internal_structtest;
			ctr_heap_free(symbol_name);
		} else {
			#ifdef WIN
			ff->symbol = GetProcAddress( ff->handle, symbol_name ); 
			#else
			ff->symbol = dlsym( ff->handle, symbol_name );
			#endif
			ctr_heap_free(symbol_name);
			if (!ff->symbol) {
				#ifdef WIN
				ctr_error("No symbol",0);
				#else
				ctr_error(dlerror(),0);
				#endif
				return;
			}
		}
	}
	// Build the argument list
	if (arg3->link != CtrStdArray) {
		ctr_error("No FFI arguments", 0);
		return;
	}
	// Load the number of arguments
	ff->nargs = arg3->value.avalue->head;
	//No more than 20 arguments are supported for an FFI binding
	if (ff->nargs > 20) {
		ctr_error("FFI: up to 20 arguments supported per call.", 0);
		return;
	}
	for(int i = 0; i<ff->nargs; i++) {
		ff->args[i] = ctr_internal_gui_ffi_map_type_obj(*(arg3->value.avalue->elements + i));
		if (!ff->args[i]) {
			ctr_error("Unable to map argument type.", 0);
			return;
		}
		
	}
	// Build the return type
	rtype_desc = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg4));
	ff->rtype = NULL;
	ff->rtype = ctr_internal_gui_ffi_map_type(rtype_desc);
	ctr_heap_free(rtype_desc);
	
	if (!ff->rtype) {
		ctr_error("Invalid FFI return type.",0);
		return;
	}
	ffi_status ok;
	if (arg7) {
		ok = ffi_prep_cif_var(ff->cif, FFI_DEFAULT_ABI, (int) arg7->value.nvalue, ff->nargs, ff->rtype, ff->args);
	} else {
		ok = ffi_prep_cif(ff->cif, FFI_DEFAULT_ABI, ff->nargs, ff->rtype, ff->args);
	}
	if (ok != FFI_OK) {
		ctr_error("Invalid FFI function signature",0);
		return;
	}
	// Create the owner object in the world
	if (arg5 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->owner = CtrMediaPreviousFFIEntry->owner;
		} else {
			ctr_error("No FFI bridge object",0);
			return;
		}
	} else {
		ff->owner = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
		ff->owner->link = CtrMediaFFIObjectBase;
		ctr_internal_object_add_property(
			CtrStdWorld,
			ctr_internal_cast2string(arg5),
			ff->owner,
			CTR_CATEGORY_PUBLIC_PROPERTY
		);
	}
	// Create the message bridge
	if (arg6 == CtrStdNil) {
		ctr_error("FFI: no message mapping",0);
		return;
	}
	resource = ctr_heap_allocate(sizeof(ctr_resource));
	resource->ptr = ff;
	resource->type = 2;
	resource->destructor = &ctr_media_ffi_destructor;
	cif_resource_holder = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	cif_resource_holder->value.rvalue = resource;
	ffi_property_name = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(arg6)
	);
	ctr_internal_object_property(
		ff->owner,
		ffi_property_name,
		cif_resource_holder
	);
	ctr_heap_free(ffi_property_name);
	// Succesfully created FFI bridge, store this bridge in cache
	CtrMediaPreviousFFIEntry = ff;
}

/**
 * [Blob] new
 *
 * Creates a new instance of the Blob object.
 * The Blob object allows you to manage custom memory blobs.
 */
ctr_object* ctr_blob_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* blobInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	blobInstance->link = myself;
	return blobInstance;
}

void begin_ffi() {
	
	CtrMediaDataBlob = ctr_blob_new(CtrStdObject, NULL);
	CtrMediaDataBlob->link = CtrStdObject;
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_blob_new_set);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_blob_tostring);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_BYTES_SET ), &ctr_blob_fill);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_UTF8_SET ), &ctr_blob_utf8_set);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_NEW_TYPE_SET ), &ctr_blob_new_set_type);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_DEREF ), &ctr_blob_deref);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FREE ), &ctr_blob_free);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_STRUCT_SET ), &ctr_blob_new_struct);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FREE_STRUCT ), &ctr_blob_free_struct);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FROM_LENGTH ), &ctr_blob_read);
	
	
	
	CtrMediaFFIObjectBase = ctr_ffi_object_new(CtrStdObject, NULL);
	CtrMediaFFIObjectBase->link = CtrStdObject;
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ), &ctr_media_ffi_respond_to );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ), &ctr_media_ffi_respond_to_and );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND ), &ctr_media_ffi_respond_to_and_and );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND_AND ), &ctr_media_ffi_respond_to_and_and_and );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_BLOB_OBJECT), CtrMediaDataBlob, CTR_CATEGORY_PUBLIC_PROPERTY);
}