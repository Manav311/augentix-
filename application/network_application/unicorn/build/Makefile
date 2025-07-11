SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# append CFLAGS
INC := -I../include -I$(MPP_INC) -I$(LIBCM_INC) -I$(APP_INC) -isystem$(JSON_INC) -I$(CENTCTRL_INC) -I$(SDK_INC)
CFLAGS += $(INC)

LDFLAGS += -L$(MPP_LIB) -L$(JSON_LIB) -L$(APP_LIB) -L$(LIBCM_LIB) -L$(LIBAGTX_LIB)
LDLIBS += -pthread -lm -lrt -lmpp -ljson-c -l:libcm.a -l:libagtxapi.a

ifeq ($(CONFIG_CCSERVER_SUPPORT), y)
CPPFLAGS += -DCONFIG_CCSERVER_SUPPORT
endif

# specify bin
BIN_NAME := unicorn

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

$(BIN_OUTPUT): $(OBJS) $(LIBCM_LIB)/libcm.a $(LIBAGTX_LIB)/libagtxapi.a
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
