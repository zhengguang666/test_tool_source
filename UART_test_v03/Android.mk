LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=UART.c main.c

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
  
LOCAL_MODULE:= UART_test
include $(BUILD_EXECUTABLE)

