SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# specify CFLAGS and LDLIBS
CFLAGS += \
	-isystem$(LIBLPW_INC) -isystem$(JSON_INC)
LDFLAGS := -L$(LIBLPW_LIB) -L$(JSON_LIB) -lpthread
LDLIBS := -llpw -ljson-c

# specify bin
BIN_NAME := lpwio

# paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

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

.PHONY: install
install: all $(BIN_TARGET)
#install: all 
#	$(Q)$(INSTALL) -d $(BIN_OUTPUT)
#	$(Q)$(INSTALL) -m 777 $(BIN_TARGET) $(BIN_OUTPUT)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
