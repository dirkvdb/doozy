LOCAL_PATH              := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE            := doozy-prebuilt
LOCAL_SRC_FILES         := ../jniLibs/$(TARGET_ARCH_ABI)/libdoozylib.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := doozydroid
LOCAL_SRC_FILES         := doozy.cpp
LOCAL_SHARED_LIBRARIES  := doozy-prebuilt
LOCAL_LDLIBS            := -llog
LOCAL_CFLAGS            := -std=c++1y -frtti -fexceptions
include $(BUILD_SHARED_LIBRARY)