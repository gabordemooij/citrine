#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>

#include "../../citrine.h"

ctr_object* ctr_pg_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pgInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	pgInstance->link = myself;
	ctr_internal_object_set_property(
		pgInstance, 
		ctr_build_string_from_cstring( "connection" ),
		argumentList->object,
		CTR_CATEGORY_PRIVATE_PROPERTY
	);
	return pgInstance;	
}

ctr_object* ctr_pg_query(ctr_object* myself, ctr_argument* argumentList) {
	char* connection = ctr_heap_allocate_cstring(
		ctr_internal_object_find_property(
			myself,
			ctr_build_string_from_cstring("connection"),
			CTR_CATEGORY_PRIVATE_PROPERTY
		)
	);
	PGconn *psql;
	psql = PQconnectdb(connection);
	if (!psql) {
		fprintf(stderr, "libpq error: PQconnectdb returned NULL.\n\n");
	}
	if (PQstatus(psql) != CONNECTION_OK) {
		fprintf(stderr, "libpq error: PQstatus(psql) != CONNECTION_OK\n\n");
	}
	char* command = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	int resultFormat = 0;
	int nParams = 0;
	char** paramValues;
	char* paramValue;
	ctr_argument* a;
	ctr_argument* b;
	a = ctr_heap_allocate(sizeof(ctr_argument));
	b = ctr_heap_allocate(sizeof(ctr_argument));
	ctr_object* params = argumentList->next->object;
	ctr_object* param;
	int len = (int) ctr_internal_cast2number( ctr_array_count( params, NULL ) )->value.nvalue;
	paramValues = ctr_heap_allocate( len );
	for(int q=0; q<len; q++) {
		b->object = ctr_build_number_from_float((ctr_number)q);
		param = ctr_array_get(params, b);
		nParams++;
		paramValue = ctr_heap_allocate_cstring(param);
		*(paramValues + q) = paramValue;
	}
	PGresult* res = PQexecParams(psql, command, nParams, NULL, (const char* const*) paramValues, NULL, NULL, resultFormat);
	int rows = PQntuples(res);
	int cols = PQnfields(res);
	ctr_object* cells;
	char* field;
	char* value;
	int colno;
	ctr_object* crows = ctr_array_new(CtrStdArray, NULL);
	for(int i=0; i<rows; i++) {
		cells = ctr_map_new(CtrStdMap,NULL);
		for(int j=0; j<cols; j++) {
			field = PQfname(res, j);
			colno = PQfnumber(res, field);
			value = PQgetvalue(res, i, colno);
			a->object = ctr_build_string_from_cstring(field);
			b->object = ctr_build_string_from_cstring(value);
			b->next = a;
			ctr_map_put(cells, b);
		}
		a->object = cells;
		ctr_array_push(crows, a);
	}
	PQfinish( psql );
	return crows;
}

void begin(){
	ctr_object*  pgObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(pgObject, ctr_build_string_from_cstring( "new:" ), &ctr_pg_new );
	ctr_internal_create_func(pgObject, ctr_build_string_from_cstring( "query:parameters:" ), &ctr_pg_query );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Pg" ), pgObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	pgObject->link = CtrStdObject;
	pgObject->info.sticky = 1;
}
