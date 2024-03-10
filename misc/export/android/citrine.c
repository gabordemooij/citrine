#include <jni.h>
#include <citrine.h>
#include <SDL.h>
#include <stdio.h>


/* JNI callback template
extern JNIEXPORT jstring JNICALL Java_com_citrine_citrineandroid_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject this) {

    }
    return (*env)->NewStringUTF(env, "Callback");
}
*/


static char* embedded_program = "\
☞ media ≔ Media nieuw.\
\
☞ data ≔ Pakketje nieuw: “data”.\
media koppel: data.\
media use: “__1__”.\
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