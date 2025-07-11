SDK_PATH := ../../../..
APP_PATH := $(SDK_PATH)/application
LIBINF_PATH = $(APP_PATH)/library/libinf
VFTR_PATH := $(SDK_PATH)/feature/video
MPP_PATH := $(SDK_PATH)/mpp

root := ..
dir := .
CC=gcc
CXX=g++

V ?= 1
INAPP ?= 1
USE_MICROLITE ?= 0
USE_TFLITE ?= 1

ifeq ($(V),0)
	Q := @
else
	Q :=
endif

DEBUG ?= 0
OPTIM ?= -O2

DEF_FLAGS := -DEAIF_SUPPORT_JSON -DEAIF_SUPPORT_CURL

ifeq ($(DEBUG), 1)
OPTIM = 
DEF_FLAGS += -DEAIF_ALGO_DEB
endif

CFLAGS_COMMON = -g $(OPTIM) -Wall -fPIC -fPIE -D_GNU_SOURCE -MMD $(DEF_FLAGS)
CCFLAGS := $(CFLAGS_COMMON) -std=gnu99
CXXFLAGS := $(CFLAGS_COMMON) -std=c++11

LIB_ENV := $(HOME)/local/libs/linux

FFMPEG_PATH=-L$(LIB_ENV)/ffmpeg/lib  -Wl,-rpath=$(LIB_ENV)/ffmpeg/lib

OPENCV_PATH=$(LIB_ENV)/opencv3
OPENCV_INC=-I$(OPENCV_PATH)/include
OPENCV_LIB_PATH= -L$(OPENCV_PATH)/lib -L$(OPENCV_PATH)/share/OpenCV/3rdparty/lib \
-Wl,-rpath=$(OPENCV_PATH)/lib -Wl,-rpath=$(OPENCV_PATH)/share/OpenCV/3rdparty/lib \
$(FFMPEG_PATH)

CURL_PATH := $(LIB_ENV)/curl
CURL_INC := -I$(CURL_PATH)/include
CURL_LIB_PATH := -L$(CURL_PATH)/lib -Wl,-rpath=$(CURL_PATH)/lib

JSON_PATH := $(LIB_ENV)/json-c
JSON_INC := -I$(JSON_PATH)/include/json-c
JSON_LIB_PATH := -L$(JSON_PATH)/lib -Wl,-rpath=$(JSON_PATH)/lib

MICROLITE_PATH := $(LIB_ENV)/tflite_micro
MICROLITE_INC := -I$(MICROLITE_PATH)/include
MICROLITE_LIB_PATH := $(MICROLITE_PATH)/lib/libtensorflow-microlite.a

TFLITE_PATH := $(LIB_ENV)/tflite
TFLITE_INC := -I$(TFLITE_PATH)/inc
TFLITE_LIB_PATH := $(TFLITE_PATH)/lib/libtensorflow-lite.a

LIBINF_LIB := -L$(LIBINF_PATH)/lib -linf -Wl,-rpath=$(LIBINF_PATH)/lib

SRCS_CC := \
$(dir)/src/app/test_face_reco.c \
$(dir)/src/app/test_inapp.c \
$(dir)/src/app/test_remote.c \
$(wildcard $(root)/src/*.c) \
$(wildcard $(dir)/src/*.c) \
$(wildcard $(dir)/src/fake/*.c)

INCS_CC := \
-I$(root)/src \
-I$(root)/include \
-I$(dir)/src \
-I$(MPP_PATH)/include \
-I$(VFTR_PATH)/include \
-I$(LIBINF_PATH)/include

INCS_CC += $(JSON_INC) $(CURL_INC)

SRCS_CXX := $(dir)/src/utils.cc
INCS_CXX := -I$(dir)/src $(OPENCV_INC)
OBJS_CXX := $(SRCS_CXX:.cc=.o)

TARGET_DEMO := $(dir)/eaif_demo.elf
TARGET_SINGLE := $(dir)/eaif_single.elf

export PKG_CONFIG_PATH := ${PKG_CONFIG_PATH}:${LIB_ENV}/pkg-config

LFLAGS_ALL := $(OPENCV_LIB_PATH) $(CURL_LIB_PATH) $(JSON_LIB_PATH)
LFLAGS_ALL += $(LIBINF_LIB) -pthread -lm `pkg-config --static --libs --cflags opencv` \
-lavutil -lavformat -lavdevice -lswscale -lavutil -lswresample \
-lcurl -ljson-c -lstdc++

ifeq ($(INAPP), 1)
CCFLAGS += -DEAIF_INFERENCE_INAPP
CXXFLAGS += -DEAIF_INFERENCE_INAPP
ifeq ($(USE_TFLITE), 1)
INCS_CC += $(TFLITE_INC)
LFLAGS_ALL += $(TFLITE_LIB_PATH)
endif
ifeq ($(USE_MICROLITE), 1)
INCS_CC += $(MICROLITE_INC)
LFLAGS_ALL += $(MICROLITE_LIB_PATH)
endif
endif


################################################################################
# Corresponding object files and auto-dependencies
################################################################################
OBJS := $(SRCS_CC:.c=.o) $(SRCS_CXX:.cc=.o) $(INF_OBJS)
DEPS := $(SRCS_CC:.c=.d) $(SRCS_CXX:.cc=.d) $(INF_DEPS)

$(OBJS): CFLAGS_LOCAL := $(addprefix -I,$(INCS_$(dir)))
-include	$(DEPS_$(dir))

%.o : %.cc
	@printf "  CXX\t$@\n"
	$(Q)$(CXX) -c $< -o $@  $(CXXFLAGS) $(INCS_CXX)

%.o : %.c
	@printf "  CC\t$@\n"
	$(Q)$(CC) -c $< -o $@  $(CCFLAGS) $(INCS_CC)

CLEAN_FILES += $(OBJS)

################################################################################
# App Demo
################################################################################

DEMO_SRCS := \
$(dir)/src/app/demo.c

DEMO_OBJS := $(DEMO_SRCS:.c=.o)

CLEAN_FILES += $(DEMO_OBJS) $(OBJS_CXX)

################################################################################
# App Single
################################################################################

SINGLE_SRCS := $(dir)/src/app/single.c
SINGLE_INCS := \
-I$(dir)/src \
-I$(MPP_PATH)/include \
-I$(LIBINF_PATH)/include \
$(INCS_CXX)
CLEAN_FILES += $(SINGLE_OBJS)

################################################################################
# Targets (Must be placed after "Subdirectories" section)
################################################################################

.PHONY: default
default : all

.PHONY: demo
demo: libinf $(TARGET_DEMO)

.PHONY: single
single: $(TARGET_SINGLE)

$(TARGET_DEMO): $(SRCS_CC) $(DEMO_OBJS) $(OBJS_CXX)
	@printf "  LD\t$@\n"
	$(Q)$(CC) -o $@  $^ $(CCFLAGS) $(INCS_CC) -Wl,--start-group $(LFLAGS_ALL) -Wl,--end-group

$(TARGET_SINGLE): $(SINGLE_SRCS) $(OBJS_CXX)
	@printf "  LD\t$@\n"
	$(Q)$(CC) -o $@ $^ $(CCFLAGS) $(SINGLE_INCS) -Wl,--start-group $(LFLAGS_ALL) -Wl,--end-group

.PHONY: all
all: demo single

.PHONY: call
call:
	echo $(notdir $(OBJS))
	echo $(MICROLITE_INC)
	echo $(LIBINF_PATH)

.PHONY: clean
clean:
	$(Q)rm -rf $(CLEAN_FILES)

.PHONY: distclean
distclean: clean

.PHONY: libinf
libinf:
	make -C $(LIBINF_PATH)/build HOST=linux all

.PHONY: libinf-clean
libinf-clean:
	make -C $(LIBINF_PATH)/build clean

-include $(DEPS)