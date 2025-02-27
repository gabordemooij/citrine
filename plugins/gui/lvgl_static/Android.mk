
LVGL_PATH ?= ${shell pwd}/jni/lvgl
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lvgl
LOCAL_SRC_FILES := $(shell find $(LVGL_PATH)/src -type f -name '*.c')

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/src 

LOCAL_CFLAGS := -DLV_CONF_INCLUDE_SIMPLE -I$(LVGL_PATH)

LOCAL_SHARED_LIBRARIES := SDL2


include $(BUILD_STATIC_LIBRARY)
