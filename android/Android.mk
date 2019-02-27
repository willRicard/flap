LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

FLAP_ROOT := $(LOCAL_PATH)/..

LOCAL_MODULE := flap

LOCAL_C_INCLUDES := $(FLAP_ROOT)

LOCAL_SRC_FILES := $(FLAP_ROOT)/src/main.c \
    $(FLAP_ROOT)/src/game.c \
	$(FLAP_ROOT)/src/rect.c \
	$(FLAP_ROOT)/src/error.c \
	$(FLAP_ROOT)/src/instance.c \
	$(FLAP_ROOT)/src/device.c \
	$(FLAP_ROOT)/src/swapchain.c \
	$(FLAP_ROOT)/src/shader.c \
	$(FLAP_ROOT)/src/pipeline.c \
    $(FLAP_ROOT)/src/buffer.c \
	$(FLAP_ROOT)/src/assets_android.c \
	$(FLAP_ROOT)/src/window_android.c

LOCAL_LDLIBS := -llog -landroid -lvulkan

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
