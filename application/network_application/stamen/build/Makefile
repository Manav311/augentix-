SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# append CFLAGS
INC := -I$(APP_INC) -isystem$(JSON_INC) -isystem$(OPENSSL_INC)
CFLAGS += $(INC)
LDFLAGS += -L$(APP_LIB) -L$(JSON_LIB) -L$(OPENSSL_LIB)
LDLIBS += -ljson-c -lssl -lcrypto

# specify bin
BIN_NAME := stamen

# paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

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
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
