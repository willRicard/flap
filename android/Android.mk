LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

FLAP_ROOT := $(LOCAL_PATH)/..

LOCAL_MODULE := flap

LOCAL_C_INCLUDES := $(FLAP_ROOT)

LOCAL_SRC_FILES := $(FLAP_ROOT)/src/main.c \
	$(FLAP_ROOT)/src/rect.c \
	$(FLAP_ROOT)/src/renderer_vk.c \
	$(FLAP_ROOT)/src/pipeline_vk.c \
	$(FLAP_ROOT)/src/rect_vk.c \
	$(FLAP_ROOT)/src/window_android_vk.c \
	$(FLAP_ROOT)/src/assets_android.c

LOCAL_LDLIBS := -llog -landroid -lvulkan

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
