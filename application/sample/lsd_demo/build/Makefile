SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# specify CFLAGS and LDLIBS
INC := -I$(AUDIO_INC) -I$(ALSA_INC) -I$(FEATURE_AUDIO_INC_PATH) -I$(MPP_INC) -I$(JSON_INC)

LDLIBS := -L$(ALSA_LIB) -lasound -L$(FEATURE_AUDIO_LIB_PATH) -laftr -L$(MPP_LIB) -lmpp -L$(JSON_LIB) -ljson-c -lstdc++

CFLAGS += $(INC)

# specify bin
BIN_NAME = lsd_demo

# bin paths
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/lsd_config
SRC_CONFIG_PATH := $(root)/assets
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

# sepcify targets
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
	
.PHONY: install-config
install-config: 
	@printf "  %-8s$(TARGET_CONFIG_PATH)/*\n" "INSTALL"
	$(Q)$(MKDIR_P) $(TARGET_CONFIG_PATH)
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIG_PATH)/*), \
		cp -r $(f) $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-config
uninstall-config:
	@printf "  %-8s$(TARGET_CONFIG_PATH)\n" "RM"
	$(Q)rm -rf $(TARGET_CONFIG_PATH)

.PHONY: install
install: all $(BIN_TARGET) install-config

.PHONY: uninstall
uninstall: uninstall-config
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)

