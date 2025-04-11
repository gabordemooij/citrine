#include "citrine.h"

char* np;
int errstack;

/**
 * Determines the size of the specified file.
 */
int fsize(char* filename) {
  int size;
  FILE* fh;
  fh = fopen(filename, "rb");
  if(fh != NULL){
    if( fseek(fh, 0, SEEK_END) ){
      fclose(fh);
      return -1;
    }
    size = ftell(fh);
    fclose(fh);
    return size;
  }
  return -1;
}

/**
 * @internal
 * Internal plugin loader.
 * Tries to locate a plugin by its name.
 *
 * In Citrine a plugin is loaded automatically as soon as a symbol
 * has been enountered that cannot be found in global scope.
 *
 * Before raising an error Citrine will attempt to load a plugin
 * to meet the dependency.
 *
 * To install a plugin, copy the plugin folder to the mods folder
 * in the working directory of your script.
 *
 * So, for instance, to install the 'Coffee Percolator Plugin',
 * we copy the libctrpercolator.so in folder mods/percolator/,
 * the resulting path is: mods/percolator/libctrpercolator.so.
 *
 * General path format:
 *
 * mods/X/libctrX.so
 *
 * where X is the name of the object the plugin offers in lowercase.
 *
 * On loading, the plugin will get a chance to add its objects to the world
 * through a constructor function.
 */
#ifdef REPLACE_PLUGIN_SYSTEM
#else
typedef void* (*plugin_init_func)();
void* ctr_internal_plugin_find(ctr_object* key) {
	ctr_object* modNameObject = ctr_internal_cast2string(key);
	void* handle;
	char  pathNameMod[1024];
	char* modName;
	char* modNameLow;
	plugin_init_func init_plugin;
	char* realPathModName = NULL;
	modName = ctr_heap_allocate_cstring( modNameObject );
	modNameLow = modName;
	for ( ; *modNameLow; ++modNameLow) *modNameLow = tolower(*modNameLow);
	snprintf(pathNameMod, 1024, "mods/%s/libctr%s.so", modName, modName);
	ctr_heap_free( modName );
	realPathModName = realpath(pathNameMod, NULL);
	if (access(realPathModName, F_OK) == -1) return NULL;
	handle =  dlopen(realPathModName, RTLD_NOW);
	free(realPathModName);
	if ( !handle ) {
		printf("%s\n",CTR_ERR_FOPEN);
		printf("%s\n",dlerror());
		exit(1);
	}
	/* the begin() function will add the missing object to the world */
	*(void**)(&init_plugin) = dlsym( handle, "begin" );
	if ( !init_plugin ) {
		printf("%s\n",CTR_ERR_FOPEN);
		printf("%s\n",dlerror());
		exit(1);
	}
	(void) init_plugin();
	return handle;
}
#endif

/**
 * @internal
 *
 * Causes the program flow to switch to error mode.
 * This function takes a pointer to a message string and
 * an integer error code (errno). It will create a string object
 * containing the specified message and the integrated error message
 * derived from the specified error code. This error string will
 * be assigned to the CtrStdFlow variable which will cause
 * program execution to go into error mode, i.e. all remaining
 * instructions will be ignored until either the error is dealt with
 * by a catch clause or the program ends in which case an error will
 * produced to stderr.
 *
 * Note on snprintf: (excess arguments are ignored)
 * Online C Draft Standard (n1256), section 7.19.6.1, paragraph 2:
 * The fprintf function writes output to the stream pointed to by stream,
 * under control of the string pointed to by format that specifies how
 * subsequent arguments are converted for output. If there are insufficient arguments
 * for the format, the behavior is undefined. If the format is exhausted while
 * arguments remain, the excess arguments are evaluated (as always)
 * but are otherwise ignored. The fprintf function returns when
 * the end of the format string is encountered.
 * Behavior for all the other printf() functions is the same
 * wrt excess arguments except for vprintf() (obviously).
 */
#ifndef REPLACE_ERROR_SYSTEM
ctr_object* ctr_error( char* message, int error_code ) {
	char* errstr;
	errstr = ctr_heap_allocate( sizeof(char) * 500 );
	snprintf( errstr, 500, message, strerror( error_code ) );
	CtrStdFlow = ctr_build_string_from_cstring( errstr );
	ctr_heap_free( errstr );
	CtrStdFlow->info.sticky = 1;
	errstack = 0;
	return CtrStdFlow;
}
#endif


/**
 * @internal
 *
 * Prints a message to the error stream.
 */
void ctr_print_error( char* error, int code ) {
	fwrite( error, sizeof(char), strlen(error), stderr );
	fwrite( "\n", sizeof(char), 1, stderr );
	if ( code > -1 ) {
		exit(code);
	}
}
