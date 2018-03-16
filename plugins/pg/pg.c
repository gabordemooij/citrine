#include <libpq-fe.h>
#include <stdio.h>
#include <string.h>

#include "../../citrine.h"


#define CTR_OBJECT_RESOURCE_PG 40

/**
 * [Pg] new: [String].
 *
 * Creates a new PostgreSQL connection using the specified connection details
 * in the string.
 */
ctr_object* ctr_pg_new(ctr_object* myself, ctr_argument* argumentList) {
	PGconn *psql;
	char* connectionString = ctr_heap_allocate_cstring(argumentList->object);
	psql = PQconnectdb(connectionString);
	ctr_heap_free(connectionString);
	if (!psql) {
		CtrStdFlow = ctr_error_text("PostgreSQL: Unable to establish a connection.");
		return CtrStdNil;
	}
	if (PQstatus(psql) != CONNECTION_OK) {
		CtrStdFlow = ctr_error_text("PostgreSQL: Connection not OK.");
		return CtrStdNil;
	}
	ctr_object* pgInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	pgInstance->link = myself;
	pgInstance->info.type = CTR_OBJECT_TYPE_OTEX;
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->type = CTR_OBJECT_RESOURCE_PG;
	rs->ptr = psql;
	pgInstance->value.rvalue = rs;
	return pgInstance;	
}

/**
 * [Pg] query: [String] parameters: [Array].
 *
 * Executes the query with specified parameters.
 */
ctr_object* ctr_pg_query(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.rvalue == NULL) {
		return myself;
	}
	PGconn* psql;
	char* command = ctr_heap_allocate_cstring( ctr_internal_cast2string( argumentList->object ) );
	int resultFormat = 0;
	int nParams = 0;
	int i,q,j;
	char** paramValues;
	char* paramValue;
	ctr_argument* a;
	ctr_argument* b;
	a = ctr_heap_allocate(sizeof(ctr_argument));
	b = ctr_heap_allocate(sizeof(ctr_argument));
	ctr_object* params = argumentList->next->object;
	ctr_object* param;
	int len = (int) ctr_internal_cast2number( ctr_array_count( params, NULL ) )->value.nvalue;
	psql = (PGconn*) myself->value.rvalue->ptr;
	paramValues = ctr_heap_allocate( len * sizeof(char*) );
	for(q=0; q<len; q++) {
		b->object = ctr_build_number_from_float((ctr_number)q);
		param = ctr_array_get(params, b);
		nParams++;
		paramValue = ctr_heap_allocate_cstring(ctr_internal_cast2string(param));
		*(paramValues + q) = paramValue;
	}
	PGresult* res = PQexecParams(psql, command, nParams, NULL, (const char* const*) paramValues, NULL, NULL, resultFormat);
	for(q=0; q<len; q++) {
		paramValue = *(paramValues + q);
		ctr_heap_free(paramValue);
	}
	int rows = PQntuples(res);
	int cols = PQnfields(res);
	ctr_object* cells;
	char* field;
	char* value;
	int colno;
	ctr_object* crows = ctr_array_new(CtrStdArray, NULL);
	for(i=0; i<rows; i++) {
		cells = ctr_map_new(CtrStdMap,NULL);
		for(j=0; j<cols; j++) {
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
	ctr_heap_free(a);
	ctr_heap_free(b);
	ctr_heap_free(command);
	ctr_heap_free( paramValues );
	return crows;
}

/**
 * [Pg] close.
 *
 * Closes connection.
 */
ctr_object* ctr_pg_close(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->value.rvalue == NULL) {
		return myself;
	}
	PQfinish( (PGconn*) myself->value.rvalue->ptr );
	ctr_heap_free(myself->value.rvalue);
	myself->value.rvalue = NULL;
	return myself;
}

void begin(){
	ctr_object*  pgObject = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	pgObject->value.rvalue = NULL;
	ctr_internal_create_func(pgObject, ctr_build_string_from_cstring( "new:" ), &ctr_pg_new );
	ctr_internal_create_func(pgObject, ctr_build_string_from_cstring( "query:parameters:" ), &ctr_pg_query );
	ctr_internal_create_func(pgObject, ctr_build_string_from_cstring( "close" ), &ctr_pg_close );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Pg" ), pgObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	pgObject->link = CtrStdObject;
	pgObject->info.sticky = 1;
}
