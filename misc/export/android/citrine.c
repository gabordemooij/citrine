#include <jni.h>
#include <citrine.h>
#include <SDL.h>
#include <stdio.h>

extern SDL_Window* CtrMediaWindow;
extern void ctr_internal_media_sync(void);

ctr_object* ctr_network_basic_text_send(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* rs;
	char* data = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char* url = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	JNIEnv* env = (JNIEnv*) SDL_AndroidGetJNIEnv();
	jclass mha = (*env)->FindClass(env, "org/libsdl/app/MediaHelperAndroid");
	jmethodID mid = (*env)->GetStaticMethodID(env, mha, "httpRequest", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	jstring jurl=(*env)->NewStringUTF(env, url);
	jstring jdata=(*env)->NewStringUTF(env, data);
	jstring result = (jstring) (*env)->CallStaticObjectMethod(env,mha, mid, jurl, jdata);
	const char* nativeString = (const char*) (*env)->GetStringUTFChars(env, result, 0);
	rs = ctr_build_string_from_cstring( (char*) nativeString);
    (*env)->ReleaseStringUTFChars(env, result, nativeString);
    (*env)->DeleteLocalRef(env, jurl);
    (*env)->DeleteLocalRef(env, jdata);
    ctr_heap_free(url);
    ctr_heap_free(data);
    ctr_internal_media_sync();
    return rs;
}

static char* embedded_program = "\
Media _datastart.\
";

int main(int argc, char *argv[]) {
	uint64_t program_text_size = 0;
	ctr_tnode* program;
	ctr_init();
	program_text_size = strlen(embedded_program);
	ctr_program_length = program_text_size;
	program = ctr_cparse_parse(embedded_program, "/embedded.ctr");
	ctr_initialize_world();
	init_embedded_media_plugin();
	ctr_cwlk_run(program);
	ctr_gc_sweep(1);
	ctr_heap_free_rest();
	return 0;
}