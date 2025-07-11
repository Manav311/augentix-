SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/build/sdksrc.mk
include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# specify CFLAGS and LDLIBS
INC := \
	-isystem$(OPENSSL_INC) \
	-I$(FEATURE_SECURE_ELEMENT_INC_PATH) \
	-I$(NRS_DRV_INC)

ifeq ($(CONFIG_SE_SQ7131), y)
CFLAGS += -DSE_SQ7131
endif

ifeq ($(CONFIG_SE_SQ7131S), y)
CFLAGS += -DSE_SQ7131S
endif

LDFLAGS += 

# Utilizing the Secure Element shared library: libse.so
LDLIBS := -L$(FEATURE_SECURE_ELEMENT_LIB_PATH) -lse -L$(OPENSSL_LIB) -lssl -lcrypto

# Utilizing the Secure Element static library: libse.a
#LDLIBS := $(FEATURE_SECURE_ELEMENT_LIB_PATH)/libse.a -L$(OPENSSL_LIB) -lssl -lcrypto

CFLAGS += $(INC)

# specify bin
BIN_NAME = secure_element_demo

# bin paths
TARGET_CONFIG_PATH := $(SYSROOT)/etc
SRC_CONFIG_PATH := $(root)/assets
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

# specify source
SRCS := main.c se_utils.c
SRCS += $(if $(CONFIG_SE_SQ7131), se_sq7131.c)
SRCS += $(if $(CONFIG_SE_SQ7131S), se_sq7131s.c)

# specify object files
OBJS := $(patsubst %.c,$(OBJ_PATH)/%.o,$(SRCS))

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
distclean: clean

.PHONY: install-config
install-config: 
	@printf "  %-8s$(TARGET_CONFIG_PATH)/*\n" "INSTALL"
	$(Q)$(MKDIR_P) $(TARGET_CONFIG_PATH)
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIG_PATH)/*), \
		cp -r $(f) $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-config
uninstall-config:
	@printf "  %-8s$(TARGET_CONFIG_PATH)/*\n" "RM"
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIG_PATH)/*), \
		$(Q)rm -rf $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)

.PHONY: install 
install: all $(BIN_TARGET) install-config

.PHONY: uninstall
uninstall: uninstall-config
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
