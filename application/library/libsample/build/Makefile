SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(LIBSAMPLE_PATH)/build/libosd.mk
include $(LIBSAMPLE_PATH)/test/build/test.mk

include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# append CFLAGS

# specify product specific parameters
SNS := $(shell echo $(SENSOR) | tr '[A-Z]' '[a-z]')

DEPS := SNS=$(SNS) SNS0_ID=0
ifneq ($(SENSOR1),)
DEPS += SNS1=$(SENSOR1) SNS1_ID=1
endif
ifneq ($(SENSOR2),)
DEPS += SNS2=$(SENSOR2) SNS2_ID=2
endif
ifneq ($(SENSOR3),)
DEPS += SNS3=$(SENSOR3) SNS3_ID=3
endif

ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
DEPS += LIBOSD_ENABLE
endif

ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
ifeq ($(CONFIG_LIBSAMPLE_INSTALL_SIMPLIFIED_CHINESE), y)
CFLAGS += -DINSTALL_SIMPLIFIED_OTF
else
CFLAGS += -DINSTALL_TRADITIONAL_OTF
endif
endif

INC := -I$(MPP_INC) -I$(SENSOR_PATH) -isystem$(UDPS_INC) -I"$(INC_PATH)"
ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
INC += $(LIBOSD_DEPENDS_INCS)
endif

ifeq ($(CONFIG_LIBSAMPLE_ENABLE_MP4_RECORDING),y)
INC +=  -I$(LIBMP4V2_INC)/../ -I$(AUDIO_INC) -I$(ALSA_INC) $(addprefix -isystem,$(FDKAAC_INCS))
DEPS += MP4_ENABLE
endif

ifeq ($(CONFIG_APP_MPI_STREAM_ENABLE_ML),y)
INC += -I$(FEATURE_ML_INC_PATH)
DEPS += CB_BASED_OD
endif

ifeq ($(CONFIG_APP_AV_MAIN2_ENABLE_ML),y)
INC += -I$(FEATURE_ML_INC_PATH)
DEPS += CB_BASED_OD
endif

ifeq ($(CONFIG_APP_MPI_STREAM_USE_OD_PEOPLE_VEHICLE_PET),y)
DEPS += USE_PEOPLE_VEHICLE_PET_MODEL
endif
ifeq ($(CONFIG_APP_MPI_STREAM_USE_OD_LITE_PEOPLE_VEHICLE_PET),y)
DEPS += USE_PEOPLE_VEHICLE_PET_LITE_MODEL
endif
ifeq ($(CONFIG_APP_MPI_STREAM_USE_OD_LITE_PEOPLE),y)
DEPS += USE_PEOPLE_LITE_MODEL
endif

ifeq ($(CONFIG_APP_AV_MAIN2_USE_OD_PEOPLE_VEHICLE_PET),y)
DEPS += USE_PEOPLE_VEHICLE_PET_MODEL
endif
ifeq ($(CONFIG_APP_AV_MAIN2_USE_OD_LITE_PEOPLE_VEHICLE_PET),y)
DEPS += USE_PEOPLE_VEHICLE_PET_LITE_MODEL
endif
ifeq ($(CONFIG_APP_AV_MAIN2_USE_OD_LITE_PEOPLE),y)
DEPS += USE_PEOPLE_LITE_MODEL
endif


CFLAGS += -ffunction-sections -fdata-sections -D_FILE_OFFSET_BITS=64 $(INC)
CFLAGS += $(addprefix -D,$(DEPS))

# specify product specific parameters
SRC_ASSETS_PATH    := $(root)/assets
TARGET_FONT_PATH := $(SYSTEMFS)/mpp/font
TARGET_SCRIPT_PATH := $(SYSTEMFS)/mpp/script

# specify lib name and its version
LIB_NAME := sample
LIB_MAJOR := 1
LIB_MINOR := 0
LIB_PATCH := 0

INSTALL_PATH := $(SYSTEM_LIB)

# specify test target specific parameters
TEST_TARGET := libsample_test
INSTALL_TEST_PATH := $(SYSTEM_BIN)

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(LIB_STATIC_OUTPUT) test-clean test

.PHONY: clean 
clean: clean-a test-clean
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH) $(LIB_OUT_PATH) 

.PHONY: distclean
distclean: uninstall clean test-clean

.PHONY: install install-assets
install: all install-assets test-install
install-assets: | $(TARGET_FONT_PATH) $(TARGET_SCRIPT_PATH)
	$(Q)$(foreach f, $(wildcard $(SRC_ASSETS_PATH)/window.ini), \
		cp -r $(f) $(TARGET_SCRIPT_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(TARGET_SCRIPT_PATH)/window.ini\n" "INSTALL"
ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
ifeq ($(CONFIG_LIBSAMPLE_INSTALL_SIMPLIFIED_CHINESE), y)
	$(Q)$(foreach f, $(wildcard $(SRC_ASSETS_PATH)/SourceHanSansCN-Regular_1.otf), \
		cp -r $(f) $(TARGET_FONT_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(TARGET_FONT_PATH)/SourceHanSansCN-Regular_1.otf\n" "INSTALL"
else
	$(Q)$(foreach f, $(wildcard $(SRC_ASSETS_PATH)/SourceHanSansTC-Normal.otf), \
		cp -r $(f) $(TARGET_FONT_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(TARGET_FONT_PATH)/SourceHanSansTC-Normal.otf\n" "INSTALL"
endif
endif

$(TARGET_FONT_PATH) $(TARGET_SCRIPT_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)mkdir -p $@

.PHONY: uninstall uninstall-assets
uninstall:uninstall-assets test-uninstall
uninstall-assets:
	@printf "  %-8s$(TARGET_FONT_PATH)/*.otf\n" "RM"
	$(Q)rm -f $(TARGET_FONT_PATH)/*.otf
	$(Q)rm -f $(TARGET_SCRIPT_PATH)/window.ini

$(LIB_STATIC_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(LIB_OUT_PATH)/$(LIB_LINKERNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_SONAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_REALNAME): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $^ -o $@

-include $(DEPS)
