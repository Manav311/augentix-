SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# specify CFLAGS, LDFLAGS and LDLIBS
INC := -I$(LIBFOO_INC)
CFLAGS += $(INC)
LDFLAGS += -L$(LIBFOO_LIB)
LDLIBS += -lfoo

# specify bin
BIN_NAME := hello

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
