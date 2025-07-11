SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# specify CFLAGS and LDLIBS
INC := -I$(FEATURE_SECURE_ELEMENT_INC_PATH) -isystem$(OPENSSL_INC) -isystem$(JSON_INC)

LDLIBS := -L$(FEATURE_SECURE_ELEMENT_LIB_PATH) -lse -L$(OPENSSL_LIB) -lssl -lcrypto -L$(JSON_LIB) -ljson-c

CFLAGS += $(INC)

# specify bin
BIN_NAME = se_auth

# bin paths
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


.PHONY: install
install: all $(BIN_TARGET) 

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)
