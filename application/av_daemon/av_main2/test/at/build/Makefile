SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../..)

include $(SDKSRC_DIR)/application/internal.mk

root := $(realpath $(CURDIR)/..)

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
AUDIO_PATH := $(HANLDERS_PATH)/audio
IVA_HANDLERS := $(HANLDERS_PATH)/iva
CORE_PATH := $(TEST_SRC_PATH)/core

INC := \
	-isystem$(JSON_INC) \
	-I$(APP_INC) \
	-I$(FEATURE_AUDIO_INC_PATH) \
	-I$(FEATURE_VIDEO_INC_PATH) \
	-I$(LIBAVFTR_INC) \
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
	-leaif \
	-linf \
	-lasound

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

TEST_SRCS := \
	$(wildcard $(AUDIO_PATH)/*.c) \
	$(wildcard $(CORE_PATH)/*.c) \
	$(wildcard $(HANLDERS_PATH)/*.c) \
	$(wildcard $(IMAGE_PREF_HANDLERS)/*.c) \
	$(wildcard $(IVA_HANDLERS)/*.c) \
	$(wildcard $(NODES_PATH)/*.c) \
	$(wildcard $(VIDEO_CONTROL_HANDLERS)/*.c)

TARGET := node_traversal_test
TRAVERSAL_TEST_C := ../src/node_traversal_test.c
APPLY_TEST := apply_test
APPLY_TEST_C := ../src/apply_test.c

IVA_TEST := iva_test
IVA_TEST_C := ../src/iva_test.c

SAVE_ALARM_BELL := save_alarm_bell
SAVE_ALARM_BELL_C := ../src/save_alarm_bell.c

.PHONY: default
default: all

.PHONY: all
all: $(TARGET) $(APPLY_TEST) $(IVA_TEST) $(SAVE_ALARM_BELL)

.PHONY: log
log:
	echo $(TEST_SRC_PATH)

$(TARGET):
	$(Q)$(CC) -o $(TARGET) $(TRAVERSAL_TEST_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

$(APPLY_TEST):
	$(Q)$(CC) -o $(APPLY_TEST) $(APPLY_TEST_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

$(IVA_TEST):
	$(Q)$(CC) -o $(IVA_TEST) $(IVA_TEST_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

$(SAVE_ALARM_BELL):
	$(Q)$(CC) -o $(SAVE_ALARM_BELL) $(SAVE_ALARM_BELL_C) $(TEST_SRCS) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(Q) $(RM) $(TARGET)* $(APPLY_TEST)* $(IVA_TEST)* $(SAVE_ALARM_BELL)*
