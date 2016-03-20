LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#OPENCV_CAMERA_MODULES:=off
#OPENCV_INSTALL_MODULES:=off
#OPENCV_LIB_TYPE:=SHARED
include /Users/user/Downloads/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

CORE_SOURCES_PATH := ../../../ 

LOCAL_SRC_FILES  := TestAnalysis.cpp $(CORE_SOURCES_PATH)/analysis.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(CORE_SOURCES_PATH)
LOCAL_LDLIBS     += -llog -ldl

LOCAL_MODULE     := test_analysis 

include $(BUILD_SHARED_LIBRARY)
