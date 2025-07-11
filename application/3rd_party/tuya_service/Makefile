###########################
# General SDK Environment #
###########################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/.config

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar

#########
# Paths #
#########

SRC_DIR = src
INC_DIR = inc

TUYA_INC_DIR = $(TUYA_IPC_SDK_DIR)/include
TUYA_LIB_DIR = $(TUYA_IPC_SDK_DIR)/libs

WIFI_TOOLS_INC = $(BUILDROOT_OUTPUT_BUILD_PATH)/wireless_tools-30.pre9

#################
# Compile Flags #
#################

CFLAGS += -Wl,-rpath=$(LIBBROLTI_LIB) -D_GNU_SOURCE -MMD -MP
CFLAGS += -isystem$(WIFI_TOOLS_INC) -I$(SDK_INC) -I$(GEN_INC)

###########################
# Source and Output Files #
###########################

BIN := tuya_service
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))
DEPS := $(patsubst %.o, %.d, $(OBJS))

DEST := $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(BIN))

###############################
# Include Paths and Libraries #
###############################
INC += -I$(LIBAVFTR_INC) -I$(FEATURE_AUDIO_INC_PATH) -I$(LIBEAIF_INC) -I$(FEATURE_VIDEO_INC_PATH) -L$(LIBAVFTR_LIB) -L$(LIBEAIF_LIB) -L$(LIBINF_LIB) -L$(FEATURE_AUDIO_LIB_PATH) -L$(FEATURE_VIDEO_LIB_PATH) -L$(LIBEAIF_LIB)
LIB += -lavftr -lvftr -laftr

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LIB += -l:libeaif.a -l:libinf.a
else
LIB += -leaif -linf
endif

ifeq ($(CONFIG_LIBINF_TFLITE),y)
INC += -isystem$(TFLITE_INC)
LIB += -L$(TFLITE_LIB) -ltensorflow-lite -lstdc++
else ifeq ($(CONFIG_LIBINF_TFLITE_OPTIM), y)
INC += -isystem$(TFLITE_OPTIM_INC)
LIB += -L$(TFLITE_OPTIM_LIB) -ltensorflow-lite-optim -lstdc++
else ifeq ($(CONFIG_LIBINF_MICROLITE), y)
INC += -isystem$(MICROLITE_INC)
LIB += -L$(MICROLITE_LIB) -ltensorflow-microlite -lstdc++
ifeq ($(CONFIG_LIBINF_REMOTE_CORE), y)
LIB += -L$(LIBAMPC_LIB) -L$(LIBMETAL_LIB) -L$(LIBOPENAMP_LIB) -L$(BUILDROOT_TARGET_LIB) -lampc_linux -lmetal -lopen_amp -lsysfs
endif
else ifeq ($(CONFIG_LIBINF_NCNN), y)
INC += -isystem$(NCNN_INC)
LIB += -L$(NCNN_LIB) -lncnn -lstdc++
else ifeq ($(CONFIG_LIBINF_NCNN_COMPACT), y)
INC += -isystem$(NCNN_COMPACT_INC)
LIB += -L$(NCNN_COMPACT_LIB) -lncnn-compact -lstdc++
else ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
INC += -isystem$(NCNN_MINIMAL_INC)
LIB += -L$(NCNN_MINIMAL_LIB) -lncnn-minimal -lstdc++
endif

INC += -I$(MPP_INC) -L$(MPP_LIB) -I$(DEBUG_INC) -L$(DEBUG_PATH) -I$(LIBAGTX_INC)
LIB += -lmpp

INC += -isystem$(ALSA_INC) -I$(AUDIO_INC) -L$(ALSA_LIB)
LIB += -lasound

INC += -I$(TUYA_INC_DIR) -L$(TUYA_LIB_DIR)
LIB += -ltuya_ipc -lmbedtls

INC += -I$(LIBCM_INC) -I$(APP_INC) -L$(LIBCM_PATH)
LIB += -lcm

INC += -isystem$(JSON_INC) -L$(JSON_LIB)
LIB += -ljson-c

INC += -L$(ZLIB_LIB) -L$(LIBCURL_LIB) -L$(OPENSSL_LIB)
LIB += -lz -lcurl -lssl -lcrypto

INC += -isystem$(LIBJPEG_INC) -L$(LIBJPEG_LIB)
LIB += -ljpeg

INC += -isystem$(ZBAR_INC) -L$(ZBAR_LIB) $(addprefix -L,$(V4L2_LIB))
LIB += -lzbar -lv4l2 -lv4lconvert

INC += -I$(LIBLEDEVT_INC) -L$(LIBLEDEVT_LIB)
LIB += -lledevt

INC += -I$(INC_DIR)
LIB += -pthread -lm -lrt

INC += $(addprefix -isystem,$(FDKAAC_INCS)) -L$(FDKAAC_LIB)
LIB += -lfdk-aac

INC += -I$(LIBADO_INC) -L$(LIBADO_LIB)
LIB += -lado

#################
# Build Targets #
#################

.DEFAULT_GOAL := default

default: all

all: check-env $(BIN) tuya_watchd tuya_script

$(BIN): $(OBJS)
	@printf "  %-8s$@\n" "AR"
	$(Q)$(CC) $^ $(LIB) $(CFLAGS) -o $@ $(INC)

%.o: %.c
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(INC)

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS) tuya_watchd-install tuya_script-install
	@mkdir -p $(SYSTEMFS)
	$(Q)cp -f  ./siren/siren.ul $(SYSTEMFS)/factory_default
	$(Q)install -m 644 $(APP_PATH)/3rd_party/tuya_service/augentix_ap.conf $(USRDATAFS)/augentix_ap.conf
	$(Q)install -m 644 $(APP_PATH)/3rd_party/tuya_service/dhcpd.conf $(SYSROOT)/etc/dhcp/dhcpd.conf

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: doc
doc:
	$(Q)make -C ./doc doc

.PHONY: doc-clean
doc-clean:
	$(Q)make -C ./doc clean

.PHONY: clean
clean: doc-clean tuya_watchd-clean tuya_script-clean
	@printf "  %-8s$(BIN)\n" "CLEAN"
	@rm -f $(BIN)
	@printf "  %-8s$(SRC_DIR)/*.[do]\n" "CLEAN"
	@rm -f $(SRC_DIR)/*.[do]

.PHONY: distclean
distclean: uninstall clean

.PHONY: tuya_watchd
tuya_watchd:
	 $(Q)$(MAKE) -C ./tuya_watchd all

.PHONY: tuya_watchd-install
tuya_watchd-install:
	 $(Q)$(MAKE) -C ./tuya_watchd install

.PHONY: tuya_watchd-clean
tuya_watchd-clean:
	$(Q)$(MAKE) -C ./tuya_watchd clean

.PHONY: tuya_script
tuya_script:
	 $(Q)$(MAKE) -C ./tuya_script all

.PHONY: tuya_script-install
tuya_script-install:
	 $(Q)$(MAKE) -C ./tuya_script install

.PHONY: tuya_script-clean
tuya_script-clean:
	$(Q)$(MAKE) -C ./tuya_script clean

####################
# Autodependencies #
####################

-include $(DEPS)

########################
# Environment Checking #
########################

.PHONY: check-env
check-env:
ifndef TUYA_IPC_SDK_DIR
	$(error Environment variable TUYA_IPC_SDK_DIR is undefined)
endif
