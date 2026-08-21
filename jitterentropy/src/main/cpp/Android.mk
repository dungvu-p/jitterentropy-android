LOCAL_PATH := $(call my-dir)
JENT_WRAPPER_PATH := $(LOCAL_PATH)

# Build the native library with its upstream Android NDK makefile so Android
# keeps the compiler flags and source selection maintained by upstream.
include $(LOCAL_PATH)/../../../../vendor/jitterentropy-library/arch/android/Android.mk

LOCAL_PATH := $(JENT_WRAPPER_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := jitterentropy_android
LOCAL_SRC_FILES := jitterentropy_jni.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../vendor/jitterentropy-library
LOCAL_SHARED_LIBRARIES := jitterentropy

include $(BUILD_SHARED_LIBRARY)
