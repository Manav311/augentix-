# realpat is the function for file.
# $(CURDIR)/../../../.. means jpl-r3.13 folder
SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
root = $(realpath $(CURDIR)/..)
MKDIR := mkdir -p

# include extension makefile.
include $(SDKSRC_DIR)/application/internal.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# Tange cloud SDK revision
TANGE_SDK_REV = 207

# paths
BIN_OUT_PATH = $(root)/bin
INC_PATH = $(root)/include
LIB_PATH = $(root)/lib/${CROSS_COMPILE}rev$(TANGE_SDK_REV)
OBJ_PATH = $(root)/obj
SRC_PATH = $(root)/src
DEP_PATH = $(root)/.deps

# # Tange_Cloud definition
PLATFORM?=__LINUX__
__OS__?=linux
ctp?=n
rec?=n
p2p?=webrtc
ARCH ?= arm
RELEASE ?= n

# specify CFLAGS and LDLIBS
CFLAGS := -D$(PLATFORM) -D__PLATFORM_$(ARCH)__ -DARCH=$(ARCH) -ffunction-sections
CFLAGS += -I$(INC_PATH) -I$(SRC_PATH)/inc -I$(LIBAGTX_INC) -I$(LIBCM_INC) -isystem$(JSON_INC) -I$(MPP_INC) -I$(AUDIO_INC) -I$(ALSA_INC)
CFLAGS += -I$(LIBSAMPLE_INC) -I$(UDPS_INC) -I$(LIBLPW_INC)

LDFLAGS += -Wl,-rpath=$(IOTC_LIB_PATH) -Wl,--disable-new-dtags \
-L$(LIB_PATH) -L$(MPP_LIB) -L$(LIBCURL_LIB) -L$(ALSA_LIB) -L$(LIBAGTX_LIB) \
-L$(LIBCM_LIB) -L$(JSON_LIB) -ljson-c -lasound -lcm -lmpp \
-L$(LIBSAMPLE_LIB) -lsample -L$(UDPS_LIB) -ludps 

DEPLIBS += -pthread -lrt -lzbar
OBJS:= $(OBJ_PATH)/tange_demo.o $(OBJ_PATH)/inifile.o $(OBJ_PATH)/tange_cloud_config.o $(OBJ_PATH)/tange_cloud_video.o 
OBJS+= $(OBJ_PATH)/tange_cloud_audio.o $(OBJ_PATH)/tange_cloud_cmd_utils.o $(OBJ_PATH)/tange_cloud_proc.o $(OBJ_PATH)/tange_cloud_comm.o

BIN_NAME := tange_cloud

# keep this for tange
ifeq ($(RELEASE),y)
	CFLAGS+=-O2 -DNDEBUG
else
	CFLAGS+=-g -D_DEBUG
	ifneq ($(pc),y)
		CFLAGS += -O2
	endif
endif

ifeq ($(CONFIG_TANGE_USE_AGT_MPI_STREAM),y)
CFLAGS+=-DTANGE_USE_AGT_MPI_STREAM
LDFLAGS += -L$(LIBLPW_LIB) -llpw
endif

ifeq ($(CONFIG_TANGE_USE_AV_MAIN2),y)
CFLAGS+=-DTANGE_USE_AV_MAIN2
endif

STARTGROUP=-Wl,--start-group
ENDGROUP=-Wl,--end-group
LDFLAGS += -Wl,--gc-sections

ifeq (webrtc,$(p2p))
	DEPLIBS += -ltgCloud-tgwebrtc -lm
	BIN_NAME?=webrtc_demo
else
	DEPLIBS += -ltgCloud
	BIN_NAME?=tg_demo
endif

# dependency flags
DEPFLAGS = -MT $@ -MMD -MP -MF "$(DEP_PATH)/$*.d"

# <src>/application/sample/tange_cloud/bin/tange_cloud
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
# <src>/fs/rootfs/output/system/bin/tange_cloud
BIN_TARGET := $(addprefix $(SYSTEM_BIN)/,$(BIN_NAME))

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
STRIP ?= $(CROSS_COMPILE)strip
LD := $(CROSS_COMPILE)gcc

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)
ifeq ($(CONFIG_TANGE_USE_AGT_MPI_STREAM),y)
	@echo "!!! CONFIG_TANGE_USE_AGT_MPI_STREAM="$(CONFIG_TANGE_USE_AGT_MPI_STREAM)
endif
ifeq ($(CONFIG_TANGE_USE_AV_MAIN2),y)
	@echo "!!! CONFIG_TANGE_USE_AV_MAIN2="$(CONFIG_TANGE_USE_AV_MAIN2)
endif

# objects
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(Q)$(MKDIR) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(Q)$(MKDIR) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CXX) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR) $(BIN_OUT_PATH)
	$(LD) $(WRAPS) -o $@ $(STARTGROUP) $^ $(DEPLIBS) $(LDFLAGS) $(TGEXTRALIB) $(ENDGROUP)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(OBJ_PATH) $(DEP_PATH) $(BIN_OUT_PATH) 

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all install-bin install-data

.PHONY: install-bin
install-bin: $(BIN_TARGET)

.PHONY: install-data
install-data:
	$(Q)mkdir -p $(USRDATAFS)/root
	$(Q)cp -f $(SRC_PATH)/testcase.ini $(USRDATAFS)/root/testcase.ini

$(BIN_TARGET): $(BIN_OUTPUT)
	$(Q)install -c $< $@

.PHONY: uninstall
uninstall: uninstall-bin

.PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(BIN_TARGET)
