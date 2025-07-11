SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk
include $(SDKSRC_DIR)/application/.config


# specify CFLAGS and LDLIBS
DISPOSABLE_PARAMS_SRC = $(SRC_PATH)/disposable_params.c
PSK_MANAGER_SRC = $(SRC_PATH)/pskManager.c

LIB_EXTRACT_PATH := $(root)/src/Mp4Extract

TUTK_INC_PATH = $(TUTK_SDK_DIR)/Include
TUTK_LIB_PATH = $(TUTK_SDK_DIR)/Libs/arm-augentix-linux-uclibcgnueabihf

ifeq ($(CONFIG_UCLIBC),y)
TUTK_LIB_PATH = $(TUTK_SDK_DIR)/Libs/arm-augentix-linux-uclibcgnueabihf
else

ifeq ($(CONFIG_CHIP), HC1703)
TUTK_LIB_PATH = $(TUTK_SDK_DIR)/Libs/arm-augentix-linux-gnueabihf
else ifeq ($(CONFIG_CHIP), HC1725)
TUTK_LIB_PATH = $(TUTK_SDK_DIR)/Libs/arm-augentix-linux-gnueabihf
else
TUTK_LIB_PATH = $(TUTK_SDK_DIR)/Libs/arm-augentix-linux-gnueabi
endif

endif

TUTK_DEBUG_LIBS := $(wildcard $(TUTK_LIB_PATH)/*T.a)
TUTK_RELEASE_LIBS := $(patsubst %T.a,%.a,$(TUTK_DEBUG_LIBS))
3RDPARTY_LIBS = libcurl_tutk.a libjson-c_tutk.a libssl_tutk.a libcrypto_tutk.a
TUTK_3RDPARTY_LIBS := $(addprefix $(TUTK_LIB_PATH)/, $(3RDPARTY_LIBS))

LDFLAGS := -L$(MPP_LIB) -lmpp -lm \
-L$(LIBCURL_LIB) -L$(OPENSSL_ROOT) -L$(ZLIB_LIB) \
-L$(ALSA_LIB) -L$(FDKAAC_LIB) -L$(JSON_LIB) -L$(LIBMP4V2_LIB) -L$(LIBLPW_LIB) -L$(LIB_EXTRACT_PATH) \
-L$(TUTK_LIB_PATH) \
-l:libAVAPIs.a -l:libIOTCAPIs.a -l:libNebulaAPIs.a -l:libTUTKGlobalAPIs.a \
-ljson-c -lcurl -lssl -lcrypto -lz -lasound -lfdk-aac -lmp4v2 -lextract -llpw
LDFLAGS += -Wl,--start-group $(TUTK_3RDPARTY_LIBS) -Wl,--end-group
LDFLAGS += -lpthread


INC := -I$(MPP_INC) -I$(SRC_INC_PATH) -I$(AUDIO_INC) -I$(ALSA_INC) $(addprefix -isystem,$(FDKAAC_INCS)) \
 -isystem$(JSON_INC) -I$(LIBMP4V2_INC)/../ -I$(LIBLPW_INC) -I$(LIB_EXTRACT_PATH) -I$(TUTK_INC_PATH) 
CFLAGS += -pthread $(INC)


SRC_ASSETS_PATH  := $(root)/assets
TARGET_ASSETS_PATH := $(USRDATAFS)/active_setting/
SRC_CONFIGS_PATH := $(root)/configs
TARGET_CONFIGS_PATH := $(SYSTEMFS)/tutk_service_config/

# specify bin
BIN_NAME = tutk_service

# bin paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

# specify targets
.DEFAULT_GOAL := default


.PHONY: default
default: all

.PHONY: all extract-lib
all: check-env clean extract-lib $(BIN_OUTPUT)

extract-lib:
	make share -C $(LIB_EXTRACT_PATH)

.PHONY: clean distclean clean-extract-lib
clean: clean-extract-lib
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

clean-extract-lib:
	make clean -C $(LIB_EXTRACT_PATH)

distclean: uninstall clean

.PHONY: install install-configs install-assets install-tutk-libs install-extract-lib
install: all install-configs install-assets install-tutk-libs install-extract-lib
	cp -r $(BIN_OUTPUT) $(BIN_TARGET)

install-tutk-libs:
	cp  $(TUTK_LIB_PATH)/libjson-c.so* $(SYSTEM_LIB);

install-configs: | $(TARGET_CONFIGS_PATH)
	@printf "  %-8s$(TARGET_CONFIGS_PATH)/*\n" "INSTALL"
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIGS_PATH)/*), \
		cp -r $(f) $(TARGET_CONFIGS_PATH)/$(notdir $(f)); \
	)

install-assets: | $(TARGET_ASSETS_PATH)
	@printf "  %-8s$(TARGET_ASSETS_PATH)/*\n" "INSTALL"
	$(Q)$(foreach f, $(wildcard $(SRC_ASSETS_PATH)/*), \
		cp -r $(f) $(TARGET_ASSETS_PATH)/$(notdir $(f)); \
	)

install-extract-lib:
	make install -C $(LIB_EXTRACT_PATH)

$(TARGET_ASSETS_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)mkdir -p $@

$(TARGET_CONFIGS_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)mkdir -p $@


.PHONY: uninstall uninstall-assets uninstall-tutk-libs uninstall-extract-lib
uninstall: uninstall-assets uninstall-tutk-libs uninstall-extract-lib
	$(Q)$(RM) $(BIN_TARGET)

uninstall-assets:
	@printf "  %-8s$(TARGET_ASSETS_PATH)\n" "RM"
	$(Q)rm -rf $(TARGET_ASSETS_PATH)

uninstall-tutk-libs:
	$(Q)rm -rf $(SYSTEM_LIB)/libjson-c.so*

uninstall-extract-lib:
	make uninstall -C $(LIB_EXTRACT_PATH)

.PHONY: check-env
check-env:
ifndef TUTK_SDK_DIR
	$(error Environment variable TUTK_SDK_DIR is undefined)
else
	$(Q)echo $(TUTK_SDK_DIR)
	$(Q)echo $(TUTK_LIB_PATH)
endif

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -g -Wall  -o $@ $^ $(LDFLAGS)

-include $(DEPS)
