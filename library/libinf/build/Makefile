################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)
include config.mk

MAKEFLAGS += -j

HOST?=SDK
USE_STB?=1

LIB_ENV=
CPU_FLAG=-mcpu=$(CONFIG_TARGET_CPU)

ifeq ($(HOST),linux)
	CROSS_COMPILE=
	CPU_FLAG=
endif

include $(SDKSRC_DIR)/build/build_flags.mk

################################################################################
# Build tools and commands
################################################################################

### First, specify the build tools.
AR := $(CROSS_COMPILE)ar
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
ARFLAGS = rcsD
MKDIR := mkdir
MKDIR_P := $(MKDIR) -p
CP := cp
MV := mv
RM := rm -f
INSTALL = install
LN := ln

### paths

root   = $(realpath ..)
dir    = $(root)/build
bindir = $(SYSTEM_BIN)
libdir = $(SYSTEM_LIB)
LIB_OUT_PATH = $(root)/lib
BUILD_PATH = $(root)/build
OBJ_PATH = $(root)/obj
SRC_PATH = $(root)/src
SRC_INC_PATH = $(root)/src/inc
INSTALL_PATH := $(libdir)

### Then, collect the knowledge about how to build all outputs.

INC_PATHS := \
$(root)/include \
$(root)/src \
$(root)/src/inc \
$(MPP_INC) \
$(FEATURE_VIDEO_INC_PATH) \
$(LIBEAIF_INC) \
$(LIBCPPREACT_INC) \
$(LIBTBB_INC)

ifeq ($(USE_STB),1)
CFLAGS_DEPS := -DUSE_STB
INC_PATHS += $(root)/3rd_party/stb
endif

ifeq ($(HOST), linux)
LIB_ENV=$(HOME)/local/libs/linux/tflite
CONFIG_INF_INCS=$(LIB_ENV)/inc
CONFIG_INF_LIB_PATH=$(LIB_ENV)/lib
endif
INC_PATHS+=$(CONFIG_INF_INCS)

INCS = $(addprefix -iquote, $(INC_PATHS)) -isystem$(JSON_INC) $(addprefix -isystem,$(CONFIG_INCS))

SRCS = \
$(wildcard $(root)/src/*.c) \
$(wildcard $(root)/src/*.cc)

OBJS := \
$(patsubst $(SRC_PATH)/%.o, $(OBJ_PATH)/%.o, \
$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SRCS))))

DEPS := \
$(patsubst $(SRC_PATH)/%.d, $(OBJ_PATH)/%.d, \
$(patsubst %.c,%.d,$(patsubst %.cc,%.d,$(SRCS))))

### Then, define global compilation and linking flags.

CFLAGS_COMMON := $(CPU_FLAG) -MMD -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DBSD=1
CFLAGS_COMMON += $(CONFIG_INF_CFLAGS) $(CFLAGS_DEPS) -Wno-missing-field-initializers -Wno-unused-parameter
CFLAGS_COMMON += -ffunction-sections -fdata-sections

ifeq ($(HOST), linux)
CFLAGS_COMMON += -D_HOST_LINUX
else
CFLAGS_COMMON += -L$(FEATURE_VIDEO_LIB_PATH) -lvftr
endif

ifeq ($(CONFIG_APP_EAI_DEMO),y)
CFLAGS_COMMON += -DLIBEAI
endif

ifeq ($(DEBUG),0)
CFLAGS_COMMON += -ftree-vectorize
endif

CFLAGS    += $(INCS) $(CFLAGS_COMMON)
CXXFLAGS  += $(INCS) $(CFLAGS_COMMON)

LDLIBS := \
-L$(MPP_LIB) -lmpp \
-L$(JSON_LIB) -ljson-c

ifeq ($(CONFIG_APP_EAI_DEMO),y)
LDLIBS += -L$(LIBTBB_LIB) -ltbb \
          -L$(LIBCPPREACT_LIB) -lCppReact \
          -lm
endif

################################################################################
# Build rules
################################################################################

.PHONY: all

LIB_NAME = inf
LIB_MAJOR = 1
LIB_MINOR = 0
LIB_PATCH = 0

LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)
LIB_ARNAME = lib$(LIB_NAME).a
LIB_REALNAME = lib$(LIB_NAME).so.$(LIB_VERSION)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_MAJOR)
LIB_LINKERNAME = lib$(LIB_NAME).so

LIB_STATIC = $(LIB_ARNAME)
LIB_STATIC_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_STATIC))

LIB_SHARED = $(LIB_LINKERNAME) $(LIB_SONAME) $(LIB_REALNAME)
LIB_SHARED_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_SHARED))
LIB_SHARED_TARGET = $(addprefix $(libdir)/,$(LIB_SHARED))

### Finally, define essential build targets.

HUMAN_DETECT_MODELS_TF := $(EAIF_SERV_PATH)/models/classifiers
HUMAN_DETECT_MODELS_NCNN := $(EAIF_SERV_PATH)/models/ncnn/classifiers
FACE_MODELS := $(EAIF_SERV_PATH)/models/facereco

TARGET_MODELS :=

ifeq ($(CONFIG_LIBINF_NCNN),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_NCNN)
endif
ifeq ($(CONFIG_LIBINF_NCNN_COMPACT),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_NCNN)
endif
ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_NCNN)
endif
ifeq ($(CONFIG_LIBINF_TFLITE),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_TF)
endif
ifeq ($(CONFIG_LIBINF_TFLITE_OPTIM),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_TF)
endif
ifeq ($(CONFIG_LIBINF_MICROLITE),y)
TARGET_MODELS+=$(HUMAN_DETECT_MODELS_TF)
endif

ifneq ($(CONFIG_LIBINF_INSTALL_HUMAN_DETECT_MODELS),y)
TARGET_MODELS :=
endif

ifeq ($(CONFIG_LIBINF_INSTALL_FACE_MODELS),y)
TARGET_MODELS+=$(FACE_MODELS)
endif

### Finally, define essential build targets.
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(Q)$(MKDIR_P) $(dir $@)
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cc
	$(Q)$(MKDIR_P) $(dir $@)
	@printf "  %-8s$@\n" "CX"
	$(Q)$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all
all:
    # List targets sequentially to build them one by one.
ifneq ($(CONFIG_INF_MSG),"")
    # Echo warning message if .config is not generated, then
    # default build config is applied to libinf.
    # See build/config.mk for details.
	@echo $(CONFIG_INF_MSG)
endif
ifeq ($(CONFIG_APP_EAI_DEMO),y)
	$(Q)$(MAKE) -C $(LIBTBB_PATH) extra_inc=big_iron.inc compiler=gcc arch=arm tbb
	$(Q)mkdir -p $(LIBCPPREACT_PATH)/build && cd $(LIBCPPREACT_PATH)/build && cmake .. -DCMAKE_TOOLCHAIN_FILE=../hc21xx_linux.cmake -Dbuild_examples=OFF
	$(Q)$(MAKE) -C $(LIBCPPREACT_PATH)/build all
endif
	$(Q)$(MAKE) -C . $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)
ifeq ($(CONFIG_LIBINF_TFLITE_TOOL),y)
	$(Q)$(MAKE) -C . tool
endif

.PHONY: test
test: $(TARGET_TESTS)

.PHONY: clean
clean: uninstall
ifeq ($(CONFIG_APP_EAI_DEMO),y)
	$(Q)$(MAKE) -C $(LIBTBB_PATH) clean arch=arm
	$(Q)$(RM) -r $(LIBCPPREACT_PATH)/build
endif
	@printf "  %-8s$(root)\n" "CLEAN"
	$(Q)$(RM) -r $(OBJ_PATH) $(LIB_OUT_PATH)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install-model
install-model: $(TARGET_MODELS) | $(SYSTEMFS)/eaif/models
	$(Q)$(foreach f, $(TARGET_MODELS), \
		printf "  %-8s$(f)\n" "INSTALL"; \
		$(CP) -r $(f) $(SYSTEMFS)/eaif/models; \
	)

.PHONY: install
ifeq ($(CONFIG_LIBINF_TFLITE_TOOL), y)
install: all install-tool install-model
else
install: all install-model
endif
	$(Q)$(CP) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)

.PHONY: uninstall-model
uninstall-model:
	$(Q)printf "  %-8s$(SYSTEMFS)/eaif\n" "RM"
	$(Q)$(RM) -r $(SYSTEMFS)/eaif

.PHONY: uninstall
uninstall: uninstall-model uninstall-tool
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_SONAME)
	$(Q)$(RM) $(LIB_STATIC_OUTPUT)

$(LIB_STATIC_OUTPUT): $(OBJS) | $(LIB_OUT_PATH)
	@printf "  %-8s$@\n" "AR"
	$(Q)$(AR) $(ARFLAGS) $@ $^ $(JSON_LIB)/libjson-c.a

$(LIB_SHARED_OUTPUT): $(OBJS) | $(LIB_OUT_PATH)
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) -o $@ $^ $(DEPFLAGS) $(CFLAGS) -L$(CONFIG_INF_LIB_PATH) $(CONFIG_INF_LIB) -shared -Wl,-soname,$(LIB_SONAME) -Wl,--start-group $(LDLIBS) -Wl,--end-group

$(LIB_OUT_PATH) $(SYSTEMFS)/eaif/models:
	$(Q)$(MKDIR_P) $@

################################################################################
# General rules
################################################################################

subdir := $(root)/test
include $(subdir)/Rules.mk

subdir := $(root)/src/tools/lite
include $(subdir)/Rules.mk

-include $(DEPS)

################################################################################
# Prevent make from removing any build targets, including intermediate ones
################################################################################


