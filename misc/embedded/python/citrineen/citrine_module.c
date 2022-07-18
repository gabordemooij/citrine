#include "citrine.h"
#include <Python.h>


PyObject* callback;

ctr_object* ctr_python_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	PyObject *arglist;
	PyObject *result = NULL;
	ctr_object* answer = CtrStdNil;
	char* s;
	char* argument1 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	arglist = Py_BuildValue("(ssss)", argument1, NULL, NULL, NULL);
	result = PyObject_CallObject(callback, arglist);
	if (result != NULL) {
		if (PyArg_Parse(result,"s",&s)) {
			answer = ctr_build_string_from_cstring(s);
		}
	}
	ctr_heap_free(argument1);
	Py_DECREF(result);
	Py_DECREF(arglist);
	return answer;
}

ctr_object* ctr_python_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	PyObject *arglist;
	PyObject *result = NULL;
	ctr_object* answer = CtrStdNil;
	char* s;
	char* argument1 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char* argument2 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	arglist = Py_BuildValue("(ssss)", argument1, argument2, NULL, NULL);
	result = PyObject_CallObject(callback, arglist);
	if (result != NULL) {
		if (PyArg_Parse(result,"s",&s)) {
			answer = ctr_build_string_from_cstring(s);
		}
	}
	ctr_heap_free(argument1);
	ctr_heap_free(argument2);
	Py_DECREF(result);
	Py_DECREF(arglist);
	return answer;
}

ctr_object* ctr_python_respond_to_and_and(ctr_object* myself, ctr_argument* argumentList) {
	PyObject *arglist;
	PyObject *result = NULL;
	ctr_object* answer = CtrStdNil;
	char* s;
	char* argument1 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char* argument2 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	char* argument3 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->next->object));
	arglist = Py_BuildValue("(ssss)", argument1, argument2, argument3, NULL);
	result = PyObject_CallObject(callback, arglist);
	if (result != NULL) {
		if (PyArg_Parse(result,"s",&s)) {
			answer = ctr_build_string_from_cstring(s);
		}
	}
	ctr_heap_free(argument1);
	ctr_heap_free(argument2);
	ctr_heap_free(argument3);
	Py_DECREF(result);
	Py_DECREF(arglist);
	return answer;
}

ctr_object* ctr_python_respond_to_and_and_and(ctr_object* myself, ctr_argument* argumentList) {
	PyObject *arglist;
	PyObject *result = NULL;
	ctr_object* answer = CtrStdNil;
	char* s;
	char* argument1 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char* argument2 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	char* argument3 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->next->object));
	char* argument4 = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->next->next->object));
	arglist = Py_BuildValue("(ssss)", argument1, argument2, argument3, argument4);
	result = PyObject_CallObject(callback, arglist);
	if (result != NULL) {
		if (PyArg_Parse(result,"s",&s)) {
			answer = ctr_build_string_from_cstring(s);
		}
	}
	ctr_heap_free(argument1);
	ctr_heap_free(argument2);
	ctr_heap_free(argument3);
	ctr_heap_free(argument4);
	Py_DECREF(result);
	Py_DECREF(arglist);
	return answer;
}

void citrine_connector(char* str) {
	ctr_tnode* program;
	ctr_in_message = 0;
	ctr_callstack_index = 0;
	ctr_sandbox_steps = 0;
	ctr_source_map_head = NULL;
	ctr_source_mapping = 0;
	CtrStdFlow = NULL;
	ctr_source_mapping = 1;
	ctr_deserialize_mode = 0;
	ctr_clex_keyword_me_icon = CTR_DICT_ME_ICON;
	ctr_clex_keyword_my_icon = CTR_DICT_MY_ICON;
	ctr_clex_keyword_var_icon = CTR_DICT_VAR_ICON;
	ctr_clex_keyword_my_icon_len = strlen( ctr_clex_keyword_my_icon );
	ctr_clex_keyword_var_icon_len = strlen( ctr_clex_keyword_var_icon );
	ctr_clex_keyword_eol_len = strlen( CTR_DICT_END_OF_LINE );
	ctr_clex_keyword_chain_len = strlen( CTR_DICT_MESSAGE_CHAIN );
	ctr_clex_keyword_num_sep_dec_len = strlen( CTR_DICT_NUM_DEC_SEP );
	ctr_clex_keyword_num_sep_tho_len = strlen( CTR_DICT_NUM_THO_SEP );
	ctr_clex_keyword_qo_len = strlen( CTR_DICT_QUOT_OPEN );
	ctr_clex_keyword_qc_len = strlen( CTR_DICT_QUOT_CLOSE );
	ctr_clex_keyword_assignment_len = strlen( CTR_DICT_ASSIGN );
	ctr_clex_keyword_return_len = strlen( CTR_DICT_RETURN );
	ctr_clex_param_prefix_char = CTR_DICT_PARAMETER_PREFIX[0];
 	ctr_gc_memlimit = 10 * 1000000; /* Default memory limit: 10MB */
	ctr_gc_mode = 1;                /* Default GC mode: regular GC, no pool. */
	ctr_program_length = strlen(str);
	program = ctr_cparse_parse(str, "Embedded");
	if (program == NULL) {
		printf("Could not read program\n");
		return;
	}
	ctr_initialize_world();
	ctr_object* CtrStdPython = ctr_internal_create_object( CTR_OBJECT_TYPE_OTOBJECT );
	ctr_internal_create_func(CtrStdPython, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ), &ctr_python_respond_to );
	ctr_internal_create_func(CtrStdPython, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ), &ctr_python_respond_to_and );
	ctr_internal_create_func(CtrStdPython, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND ), &ctr_python_respond_to_and_and );
	ctr_internal_create_func(CtrStdPython, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND_AND ), &ctr_python_respond_to_and_and_and );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Python" ), CtrStdPython, 0 );
	CtrStdPython->link = CtrStdObject;
	CtrStdPython->info.sticky = 1;
	ctr_cwlk_run(program);
	ctr_gc_sweep(1);
	ctr_heap_free_rest();
	//For memory profiling
	if ( ctr_gc_alloc != 0 ) {
		fprintf( stderr, "[WARNING] Citrine has detected an internal memory leak of: %" PRIu64 " bytes.\n", ctr_gc_alloc );
		exit(1);
	}
}


static PyObject* citrine_eval_en(PyObject* self, PyObject* args)
{
    char* value;
    if (!PyArg_ParseTuple(args, "sO", &value, &callback))
        return NULL;
    Py_INCREF(callback);
	citrine_connector(value);
	Py_XDECREF(callback); 
	return Py_BuildValue("i", !(CtrStdFlow && CtrStdFlow != CtrStdExit));
}

static PyMethodDef CitrineMethods[] =
{
     {"citrine_eval_en", citrine_eval_en, METH_VARARGS, "evaluate Citrine expression"},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cModPyDem =
{
    PyModuleDef_HEAD_INIT,
    "citrine_module_en", "Embedded Citrine",
    -1,
    CitrineMethods
};
PyMODINIT_FUNC
PyInit_citrine_module_en(void)
{
    return PyModule_Create(&cModPyDem);
}
