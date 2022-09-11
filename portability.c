#include "citrine.h"

#ifdef WINDOWS_PROGRAM_PASSWORD
ctr_object* ctr_program_waitforpassword(ctr_object* myself, ctr_argument* argumentList) {
    ctr_object* answer;
    int i = 0;
    char ch = NULL;
    char* buff;
    ctr_size page = 10;
    buff = ctr_heap_allocate(page * sizeof(char));
    while ((ch = getch()) != '\r') {
		buff[i++] = ch;
		if (i >= page) {
			page *= 2;
			buff = (char*) ctr_heap_reallocate(buff, page * sizeof(char));
			if (buff == NULL) {
				CtrStdFlow = ctr_error( CTR_ERR_OOM, 0 );
				return CtrStdNil;
			}
		}
    }
	answer = ctr_build_string(buff, i);
    ctr_heap_free(buff);
	return answer;
}
#endif


#ifdef WINDOWS_PLUGIN_SYSTEM
typedef int (__cdecl *MYPROC)(); 
void* ctr_internal_plugin_find(ctr_object* key) {
	ctr_object* modNameObject = ctr_internal_cast2string(key);
	void* handle;
	char  pathNameMod[1024];
	char* modName;
	char* modNameLow;
	MYPROC init_plugin; 
	char* realPathModName = NULL;
	modName = ctr_heap_allocate_cstring( modNameObject );
	modNameLow = modName;
	for ( ; *modNameLow; ++modNameLow) *modNameLow = tolower(*modNameLow);
	snprintf(pathNameMod, 1024,"mods\\%s\\libctr%s.dll", modName, modName);
	ctr_heap_free( modName );
	realPathModName = realpath(pathNameMod, NULL);
	handle = LoadLibrary(realPathModName); 
	free(realPathModName);
	if ( !handle ) {
		printf("%s\n",CTR_ERR_FOPEN);
		exit(1);
	}
	/* the begin() function will add the missing object to the world */
	init_plugin = (MYPROC) GetProcAddress(handle, "begin"); 
	if ( !init_plugin ) {
		printf("%s\n",CTR_ERR_FOPEN);
		FreeLibrary(handle);
		exit(1);
	}
	(void) init_plugin();
	return handle;
}
#endif

#ifdef MACOS_PLUGIN_SYSTEM
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
	snprintf(pathNameMod, 1024, "mods/%s/libctr%s.dylib", modName, modName);
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

#ifdef WINDOWS_CLOCK_WAIT
ctr_object* ctr_clock_wait(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arg = ctr_internal_cast2number(argumentList->object);
	int n = (int) arg->value.nvalue;
	Sleep(n * 1000);
	return myself;
}
#endif

#ifdef WIN32
int putenv_old(const char* name, const char* value) {
	char* buffer = malloc( strlen(name) + strlen(value) + 1 + 1 );
	strcpy(buffer, name);
	strcat(buffer + strlen(name), "=");
	strcat(buffer + strlen(name) + 1, value);
	strcat(buffer + strlen(name) + 1 + strlen(value), "\0");
	putenv(buffer);
	free(buffer);
	return 0;
}
#endif
