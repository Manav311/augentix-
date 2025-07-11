SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# specify CFLAGS and LDLIBS
INC := -I$(AUDIO_INC) -I$(ALSA_INC) -I$(FEATURE_AUDIO_INC_PATH) -I$(MPP_INC) -I$(JSON_INC)

LDLIBS := -L$(ALSA_LIB) -lasound -L$(FEATURE_AUDIO_LIB_PATH) -L$(MPP_LIB) -lmpp -L$(JSON_LIB) -ljson-c -lstdc++ -lpthread

CFLAGS += $(INC) -g
CXXFLAGS += $(INC) -g

ifeq ($(CONFIG_APP_SD_DEMO_REMOTE_CORE),y)
	LDLIBS += -l:libaftr_rtos.a
	LDLIBS += -L $(LIBAMPC_LIB)
	LDLIBS += -L $(LIBMETAL_LIB)
	LDLIBS += -L $(LIBOPENAMP_LIB)
	LDLIBS += -L $(BUILDROOT_OUTPUT_BUILD_PATH)/libsysfs-2.1.0/lib/.libs
	LDLIBS += -lampc_linux
	LDLIBS += -lmetal
	LDLIBS += -lopen_amp
	LDLIBS += -lsysfs

else
	LDLIBS += -l:libaftr.a
endif

SRCS = $(wildcard $(SRC_PATH)/*.cc)
OBJS = $(patsubst $(SRC_PATH)/%.cc,$(OBJ_PATH)/%.o,$(SRCS))
DEPS = $(patsubst $(SRC_PATH)/%.cc,$(DEP_PATH)/%.d,$(SRCS))

# specify bin
BIN_NAME = sd_demo

# bin paths
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/sd_config
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

# $(BIN_OUTPUT): $(OBJS)
# 	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
# 	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: check
check:
	@echo INC $(INC)
	@echo CFLAGS $(CFLAGS)
	@echo CXXFLAGS $(CXXFLAGS)
	@echo LDLIBS $(LDLIBS)

-include $(DEPS)

