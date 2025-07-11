SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk

# sensor config
export SENSOR:=$(SENSOR)
SNS := $(shell echo $(SENSOR) | tr '[A-Z]' '[a-z]')


# install
INSTALL := install -c
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644

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

# paths
root := $(realpath $(CURDIR)/..)
bindir := $(SYSTEM_BIN)
SRC_PATH := $(root)/src
OBJ_PATH := $(root)/obj
DEP_PATH := $(root)/.deps
BIN_OUT_PATH := $(root)/bin
SRC_INC_PATH := $(SRC_PATH)/inc

DEPFLAGS = -MT $@ -MMD -MP -MF "$(DEP_PATH)/$*.d"
SRCS := $(shell find $(SRC_PATH) -type f -name '*.c' -not -path '*/\.*')
OBJS := $(patsubst $(SRC_PATH)/%,$(OBJ_PATH)/%,$(SRCS:.c=.o))
DEPS := $(patsubst $(SRC_PATH)/%,$(DEP_PATH)/%,$(SRCS:.c=.d))

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
	-I$(SENSOR_PATH) \
	-I$(LIBSAMPLE_INC) \
	-iquote"$(SRC_INC_PATH)"

# Enable ODv5
ifeq ($(CONFIG_APP_AV_MAIN2_ENABLE_ML),y)
INC += -I$(FEATURE_ML_INC_PATH)
endif

# compiler/linker flags
CFLAGS += $(INC)
CPPFLAGS += -D_GNU_SOURCE

# Select sensor type
CPPFLAGS += -DSNS=$(SNS) -DSNS0_ID=0
LDLIBS += -lsensor_0
ifneq ($(SENSOR1),)
CPPFLAGS += -DSNS1=$(SENSOR1) -DSNS1_ID=1
LDLIBS += -lsensor_1
endif
ifneq ($(SENSOR2),)
CPPFLAGS += -DSNS2=$(SENSOR2) -DSNS2_ID=2
LDLIBS += -lsensor_2
endif
ifneq ($(SENSOR3),)
CPPFLAGS += -DSNS3=$(SENSOR2) -DSNS3_ID=3
LDLIBS += -lsensor_3
endif


# use ccserver or httpserver for AGTX communication
ifeq ($(CONFIG_APP_AV_MAIN2_USE_CCSERVER),y)
CPPFLAGS += -DUSE_CCSERVER
endif

LDFLAGS += \
	-L$(ALSA_LIB) \
	-L$(APP_LIB) \
	-L$(FEATURE_AUDIO_LIB_PATH) \
	-L$(FEATURE_VIDEO_LIB_PATH) \
	-L$(JSON_LIB) \
	-L$(LIBADO_LIB) \
	-L$(LIBAVFTR_LIB) \
	-L$(LIBCM_LIB) \
	-L$(LIBEAIF_LIB) \
	-L$(LIBINF_LIB) \
	-L$(LIBSENSOR_LIB) \
	-L$(LIBSQL_PATH) \
	-L$(MPP_LIB) \
	-L$(SQLITE3_LIB) \
	-L$(ZLIB_LIB) \
	-L$(LIBSAMPLE_LIB) \
	-Wl,-rpath=$(LIBBROLTI_LIB)

LDLIBS += \
	-lmpp \
	-pthread \
	-ljson-c \
	-lrt \
	-lm \
	-lsample \
	-lcm \
	-lsql \
	-lsqlite3 \
	-lz \
	-lavftr \
	-leaif \
	-lvftr \
	-laftr \
	-lado \
	-lasound \
	-lstdc++ \

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDLIBS += -L$(LIBINF_LIB) -l:libinf.a
else
LDLIBS += -L$(LIBINF_LIB) -linf
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
else ifeq ($(CONFIG_LIBINF_NCNN), y)
LDFLAGS += -L$(NCNN_LIB)
LDLIBS += -lncnn
else ifeq ($(CONFIG_LIBINF_NCNN_COMPACT), y)
LDFLAGS += -L$(NCNN_COMPACT_LIB)
LDLIBS += -lncnn-compact
else ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDFLAGS += -L$(NCNN_MINIMAL_LIB)
LDLIBS += -lncnn-minimal
endif

ifeq (x$(CONFIG_LIBEAIF_REMOTE_DISABLE),x)
LDFLAGS += -L$(LIBCURL_LIB) -L$(OPENSSL_ROOT)
LDLIBS += -lcurl -lssl -lcrypto
endif

# Enable ODv5
DEFS :=

ifeq ($(CONFIG_APP_AV_MAIN2_ENABLE_ML),y)
DEFS += CB_BASED_OD
LDFLAGS += -L$(LIBSAMPLE_LIB) \
		   -Wl,--gc-sections
LDLIBS += -lsample
endif

ML_LOCAL_COMMON_LDFLAGS := -L$(FEATURE_ML_LIB_PATH) \
						   -L$(NCNN_COMPACT_LIB)

ML_LOCAL_COMMON_LDLIBS := -lncnn-compact \
						  -lstdc++

ML_REMOTE_COMMON_LDFLAGS := -L$(FEATURE_ML_LIB_PATH)\
							-L$(NCNN_COMPACT_LIB) \
							-L$(LIBAMPC_LIB) \
							-L$(LIBMETAL_LIB) \
							-L$(LIBOPENAMP_LIB) \
							-L$(BUILDROOT_TARGET_LIB)

ML_REMOTE_COMMON_LDLIBS := -lampc_linux \
						   -lmetal \
						   -lopen_amp \
						   -lsysfs \
						   -lstdc++

# specify the corresponding libml_XXX.so to install (skip suffix after .so), no need for static link
TARGET_ML_DLIBS :=

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_PEOPLE_VEHICLE_PET_LOCAL),y)
LDFLAGS += $(ML_LOCAL_COMMON_LDFLAGS)
LDLIBS += -lml_peoplevehiclepet \
          $(ML_LOCAL_COMMON_LDLIBS)
DEFS += USE_PEOPLE_VEHICLE_PET_MODEL \
        CB_BASED_OD_LOCAL
TARGET_ML_DLIBS += libml_peoplevehiclepet.so
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_LITE_PEOPLE_VEHICLE_PET_LOCAL),y)
LDFLAGS += $(ML_LOCAL_COMMON_LDFLAGS)
LDLIBS += -lml_peoplevehiclepet_lite \
          $(ML_LOCAL_COMMON_LDLIBS)
DEFS += USE_PEOPLE_VEHICLE_PET_LITE_MODEL \
        CB_BASED_OD_LOCAL
TARGET_ML_DLIBS += libml_peoplevehiclepet_lite.so
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_LITE_PEOPLE_LOCAL),y)
LDFLAGS += $(ML_LOCAL_COMMON_LDFLAGS)
LDLIBS += -lml_people_lite \
          $(ML_LOCAL_COMMON_LDLIBS)
DEFS += USE_PEOPLE_LITE_MODEL \
        CB_BASED_OD_LOCAL
TARGET_ML_DLIBS += libml_people_lite.so
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_PEOPLE_VEHICLE_PET_REMOTE),y)
LDFLAGS += $(ML_REMOTE_COMMON_LDFLAGS)
LDLIBS += -lml_peoplevehiclepet_rtos \
		  $(ML_REMOTE_COMMON_LDLIBS)
DEFS += USE_PEOPLE_VEHICLE_PET_MODEL \
        CB_BASED_OD_REMOTE
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_LITE_PEOPLE_VEHICLE_PET_REMOTE),y)
LDFLAGS += $(ML_REMOTE_COMMON_LDFLAGS)
LDLIBS += -lml_peoplevehiclepet_lite_rtos \
		  $(ML_REMOTE_COMMON_LDLIBS)
DEFS += USE_PEOPLE_VEHICLE_PET_LITE_MODEL \
        CB_BASED_OD_REMOTE
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ML_OD_LITE_PEOPLE_REMOTE),y)
LDFLAGS += $(ML_REMOTE_COMMON_LDFLAGS)
LDLIBS += -lml_people_lite_rtos \
		  $(ML_REMOTE_COMMON_LDLIBS)
DEFS += USE_PEOPLE_LITE_MODEL \
        CB_BASED_OD_REMOTE
endif

# specify bin
BIN_NAME := av_main2

# bin paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))
ASSETS_PATH := $(root)/assets
TARGET_ASSETS_PATH := $(SYSTEMFS)/mpp/script
TARGET_ASSETS_FONT_PATH := $(SYSTEMFS)/mpp/font

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(BIN_TARGET) install-libml install-assets

.PHONY: uninstall
uninstall: uninstall-libml uninstall-assets
	$(Q)$(RM) $(BIN_TARGET)

.PHONY: install-assets
install-assets: $(wildcard $(ASSETS_PATH)/*.ini) $(wildcard $(ASSETS_PATH)/*.ayuv)
	@printf "  %-8s$(TARGET_ASSETS_PATH)/*.ini\n" "INSTALL"
	$(Q)$(MKDIR_P) $(TARGET_ASSETS_PATH)
	$(Q)$(MKDIR_P) $(TARGET_ASSETS_FONT_PATH)
	$(Q)$(INSTALL_PROGRAM) $< $(patsubst $(ASSETS_PATH)/%, $(TARGET_ASSETS_PATH)/%, $<)
	$(Q)$(INSTALL_PROGRAM) $(wildcard $(ASSETS_PATH)/*.ayuv) $(patsubst $(ASSETS_PATH)/%, $(TARGET_ASSETS_FONT_PATH)/%, $(wildcard $(ASSETS_PATH)/*.ayuv))

.PHONY: install-libml
install-libml:
	$(Q)mkdir -p $(CUSTOM_LIB)
	$(Q)$(foreach f, $(TARGET_ML_DLIBS), \
		$(eval ACTUAL_LIB := $(shell find $(FEATURE_ML_LIB_PATH) -type f -name "$(basename $(f)).*" | grep "\.so\.")) \
		$(if $(ACTUAL_LIB), \
			$(eval MAJOR_VER := $(shell echo $(ACTUAL_LIB) | sed -E 's/.*\.so\.([0-9]+).*/\1/')) \
			$(if $(MAJOR_VER), \
				find $(CUSTOM_LIB) -type f -name "$(basename $(f)).so.*" -exec rm -rf {} \;; \
				install -m 644 -t $(CUSTOM_LIB) $(ACTUAL_LIB); \
				$(eval SHORT_NAME := $(f).$(MAJOR_VER)) \
				ln -sfT $(notdir $(ACTUAL_LIB)) $(CUSTOM_LIB)/$(SHORT_NAME); \
				printf "  %-8s$(CUSTOM_LIB)/$(notdir $(ACTUAL_LIB))\n" "INSTALL", \
				printf "ERROR: Cannot extract version number from $(ACTUAL_LIB)\n" \
			), \
			printf "WARNING: Library $(f).* not found in $(FEATURE_ML_LIB_PATH)\n" \
		) \
	)

.PHONY: uninstall-assets
uninstall-assets:
	$(Q)$(RM) $(patsubst $(ASSETS_PATH)/%, $(TARGET_ASSETS_PATH)/%, $(wildcard $(ASSETS_PATH)/*.ini))
	$(Q)$(RM) $(patsubst $(ASSETS_PATH)/%, $(TARGET_ASSETS_FONT_PATH)/%, $(wildcard $(ASSETS_PATH)/*.ayuv))

.PHONY: uninstall-libml
uninstall-libml:
	$(Q)if [ -d "$(CUSTOM_LIB)" ]; then \
		$(foreach f, $(TARGET_ML_DLIBS), \
			$(eval BASE_NAME := $(basename $(f))) \
			find $(CUSTOM_LIB) -name "$(BASE_NAME).so.*" -exec rm -vf {} \;; \
		) \
		if [ -z "$$(ls -A $(CUSTOM_LIB))" ]; then \
			printf "  RM\t$(CUSTOM_LIB)\n"; \
			rmdir $(CUSTOM_LIB); \
		fi \
	fi

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# objects
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(dir $@) $(dir $(patsubst $(OBJ_PATH)/%.o,$(DEP_PATH)/%.d,$@))
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(addprefix -D,$(DEFS)) -c -o $@ $<

# install targets
$(bindir)/%: $(BIN_OUT_PATH)/%
	$(Q)$(INSTALL_PROGRAM) $< $@

# dependencies
$(DEP_PATH)/%.d: ;
.PRECIOUS: $(DEP_PATH)/%.d

-include $(DEPS)
