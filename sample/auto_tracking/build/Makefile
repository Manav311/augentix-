SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)


# specify CFLAGS and LDLIBS
INC := \
	-isystem$(JSON_INC) \
	-I$(LIBAVFTR_INC) -I$(LIBEAIF_INC) \
	-I$(MPP_INC) -I$(FEATURE_AUDIO_INC_PATH) -I$(FEATURE_VIDEO_INC_PATH) \
	-I$(LIBSAMPLE_INC) -I$(LIBMOTOR_INC)

LDLIBS := -L$(FEATURE_VIDEO_LIB_PATH) -L$(LIBSAMPLE_LIB) -L$(LIBMOTOR_LIB) -l:libvftr.a -l:libsample.a -l:libmotor.a
LDLIBS += -L$(JSON_LIB) -ljson-c -lsample
LDLIBS += -L$(MPP_LIB) -lmpp -lm -lpthread -lrt

CFLAGS += $(INC)


# specify bin
BIN_NAME = auto_tracking

# bin paths
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/auto_tracking
SRC_CONFIG_PATH := $(root)/configs
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

# sepcify targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: clean $(BIN_OUTPUT)

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
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)
