SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# specify CFLAGS, LDFLAGS and LDLIBS
INC := -I$(FEATURE_VIDEO_INC_PATH) -I$(MPP_INC) -I$(LIBEAIF_INC)
INC += -I$(LIBINF_INC) -I$(LIBAVFTR_INC) -I$(FEATURE_AUDIO_INC_PATH)
CFLAGS += $(INC) -g
LDFLAGS += -Wl,-gc-sections
LDLIBS += -L$(FEATURE_VIDEO_LIB_PATH) -l:libvftr.a -L$(LIBINF_LIB) -l:libinf.a -lpthread -L$(LIBEAIF_LIB) -l:libeaif.a
LDLIBS += -lrt -L$(MPP_LIB) -l:libmpp.a

ifeq ($(CONFIG_APP_VFTR_DUMP_SUPPORT_SEI),y)
CFLAGS += -DCONFIG_APP_VFTR_DUMP_SUPPORT_SEI
endif

# specify bin
BIN_NAME := vftr_dump 

# paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

# Delete all files created by building the app.
.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(BIN_TARGET)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
