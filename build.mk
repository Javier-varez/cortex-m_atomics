LOCAL_DIR := $(call current-dir)

include $(CLEAR_VARS)
LOCAL_NAME := cortex_m_atomics
LOCAL_CFLAGS := \
    -mcpu=cortex-m0 \
    -mfloat-abi=soft \
    -mthumb \
    -I$(LOCAL_DIR)/inc \
    -DARMV6_ARCH \
    -Os \
    -g3 \
    -Wall \
    -Werror \
    -Wextra \
    -Wpedantic \
    -ffunction-sections \
    -fdata-sections
LOCAL_CXXFLAGS := \
    $(LOCAL_CFLAGS) \
    -std=gnu++17 \
    -fno-exceptions \
    -fno-rtti
LOCAL_SRC := \
    $(LOCAL_DIR)/src/atomic.cpp
LOCAL_ARM_ARCHITECTURE := v6-m
LOCAL_ARM_FPU := nofp
LOCAL_COMPILER := arm_clang
LOCAL_ARFLAGS := -rcs
include $(BUILD_STATIC_LIB)

