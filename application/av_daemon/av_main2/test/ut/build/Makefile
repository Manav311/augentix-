SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../..)

include $(SDKSRC_DIR)/application/internal.mk

# sensor config
export SENSOR:=$(SENSOR)
SENS:=$(MPP)/custom/sensor
export SENS:=$(SENS)

# utils
CP := cp
LN := ln
MKDIR := mkdir
MKDIR_P := mkdir -p
MV := mv
RM := rm -f

# some useful pre-defined variables
blank :=
space := $(blank) $(blank)
comma := $(blank),$(blank)

# toolchain
AR := $(CROSS_COMPILE)ar
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

TEST_SRC_PATH := $(realpath $(CURDIR)/../../../src)
TEST_INC_PATH := $(TEST_SRC_PATH)/inc
NODES_PATH := $(TEST_SRC_PATH)/nodes
HANLDERS_PATH := $(TEST_SRC_PATH)/handlers
VIDEO_CONTROL_HANDLERS := $(HANLDERS_PATH)/video_control
IMAGE_PREF_HANDLERS := $(HANLDERS_PATH)/image_preference
WIN_IMAGE_PREF_HANDLERS := $(HANLDERS_PATH)/win_image_preference
AUDIO_PATH := $(HANLDERS_PATH)/audio
IVA_HANDLERS := $(HANLDERS_PATH)/iva
CORE_PATH := $(TEST_SRC_PATH)/core

INC := \
    -isystem$(JSON_INC) \
	-I$(CMOCKA_INC) \
	-I$(FEATURE_AUDIO_INC_PATH) \
	-I$(FEATURE_VIDEO_INC_PATH) \
	-I$(LIBAVFTR_INC) \
    -I$(APP_INC) \
    -I$(LIBCM_INC) \
    -I$(LIBEAIF_INC) \
    -I$(LIBSQL_INC) \
    -I$(MPP_INC) \
    -I$(SENS) \
    -I$(SENSOR_PATH) \
	-iquote"$(TEST_INC_PATH)"

CFLAGS += -MMD -MP $(INC)
CPPFLAGS += -D_GNU_SOURCE

LDFLAGS += \
	-L$(ALSA_LIB) \
	-L$(APP_LIB) \
	-L$(CMOCKA_LIB) \
	-L$(FEATURE_AUDIO_LIB_PATH) \
	-L$(FEATURE_VIDEO_LIB_PATH) \
	-L$(JSON_LIB) \
	-L$(LIBADO_LIB) \
	-L$(LIBAVFTR_LIB) \
	-L$(LIBCM_LIB) \
	-L$(LIBCURL_LIB) \
	-L$(LIBEAIF_LIB) \
	-L$(LIBINF_LIB) \
	-L$(LIBSENSOR_LIB) \
	-L$(LIBSQL_PATH) \
	-L$(MPP_LIB) \
	-L$(OPENSSL_ROOT) \
	-L$(SQLITE3_LIB) \
	-L$(ZLIB_LIB)

LDLIBS += \
	-lmpp \
	-pthread \
	-ljson-c \
	-lrt \
	-lm \
	-lcm \
	-lsql \
	-lsqlite3 \
	-lz \
	-lavftr \
	-lvftr \
	-laftr \
	-lcurl \
	-lssl \
	-lcrypto \
	-lado \
	-lasound \
	-leaif \
	-linf \
	-lcmocka

# Select sensor type
CPPFLAGS += -DSNS0_ID=0
LDLIBS += -lsensor_0
ifneq ($(SENSOR1),)
CPPFLAGS += -DSNS1=$(SENSOR1) -DSNS1_ID=1
ifneq ($(SENSOR1),$(SENSOR))
LDLIBS += -lsensor_1
endif
endif

# use ccserver or httpserver for AGTX communication
ifeq ($(CONFIG_APP_AV_MAIN2_USE_CCSERVER),y)
CPPFLAGS += -DUSE_CCSERVER
endif

ifeq ($(CONFIG_LIBINF_TFLITE), y)
LDFLAGS += -L$(TFLITE_LIB) 
LDLIBS += -ltensorflow-lite
else ifeq ($(CONFIG_LIBINF_TFLITE_OPTIM), y)
LDFLAGS += -L$(TFLITE_OPTIM_LIB) 
LDLIBS += -ltensorflow-lite-optim
else ifeq ($(CONFIG_LIBINF_MICROLITE), y)
LDFLAGS += -L$(MICROLITE_LIB) 
LDLIBS += -ltensorflow-microlite
ifeq ($(CONFIG_LIBINF_REMOTE_CORE), y)
LDFLAGS += -L$(LIBAMPC_LIB) -L$(LIBMETAL_LIB) -L$(LIBOPENAMP_LIB) -L$(BUILDROOT_TARGET_LIB)
LDLIBS += -lampc_linux -lmetal -lopen_amp -lsysfs
endif
endif

ifeq (x$(CONFIG_LIBEAIF_REMOTE_DISABLE),x)
LDFLAGS += -L$(LIBCURL_LIB) -L$(OPENSSL_ROOT)
LDLIBS += -lcurl -lssl -lcrypto
endif

# please select your toolchain here
#ln -sf /mnt/nfs/ethnfs/libcmocka.so.0.7.0 /system/lib/libcmocka.so.0; sync

TEST_SRCS := \
	$(wildcard $(AUDIO_PATH)/*.c) \
	$(wildcard $(CORE_PATH)/*.c) \
	$(wildcard $(HANLDERS_PATH)/*.c) \
	$(wildcard $(IMAGE_PREF_HANDLERS)/*.c) \
	$(wildcard $(IVA_HANDLERS)/*.c) \
	$(wildcard $(NODES_PATH)/*.c) \
	$(wildcard $(VIDEO_CONTROL_HANDLERS)/*.c) \
	$(wildcard $(WIN_IMAGE_PREF_HANDLERS)/*.c)

TARGET := node_traversal
TRAVERSAL_TEST_C := ../src/node_traversal.c

APPLY_HANDLER := apply_handler
APPLY_HANDLER_C := ../src/apply_handler.c

.PHONY: default
default: all

.PHONY: all
all: $(TARGET) $(APPLY_HANDLER)

$(TARGET):
	$(Q)$(CC) -o $(TARGET) $(TRAVERSAL_TEST_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

$(APPLY_HANDLER):
	$(Q)$(CC) -o $(APPLY_HANDLER) $(APPLY_HANDLER_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(Q) $(RM) $(TARGET)* $(APPLY_HANDLER)*

.PHONY: log
log:
	echo $(TARGET)
