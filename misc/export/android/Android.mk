LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL
ISO := <Your Language Code>

# Create a symlink called orig that points to the Citrine source directory
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../Citrine/orig $(LOCAL_PATH)/../Citrine/orig/plugins/media \
$(LOCAL_PATH)/../Citrine/orig/i18n/$(ISO) \
$(LOCAL_PATH)/../Citrine/orig/plugins/media/i18n/$(ISO)

# Add your application source files here...
LOCAL_SRC_FILES :=  citrine.c  orig/base.c orig/file.c \
orig/memory.c orig/siphash.c orig/util.c \
orig/citrine.c orig/lexer.c orig/parser.c \
orig/system.c orig/translator.c orig/walker.c \
orig/collections.c  orig/portability.c orig/test.c \
orig/utf8.c orig/world.c orig/plugins/media/media.c

LOCAL_CFLAGS += -D SDL -D EXPORT_ANDROID -D FULLSCREEN

$(orig/*.c)

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_ttf

LOCAL_LDLIBS := -lOpenSLES -llog -landroid 

include $(BUILD_SHARED_LIBRARY)