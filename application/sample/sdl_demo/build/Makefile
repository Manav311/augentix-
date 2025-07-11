SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# specify CFLAGS and LDLIBS
CFLAGS += \
	-isystem$(DIRECTFB_INC) \
	-isystem$(DIRECT_INC) \
	-isystem$(ZLIB_INC) \
	-isystem$(SDL2_INC)
CPPFLAGS += -D_REENTRANT
LDFLAGS := -L$(DIRECTFB_LIB) -L$(DIRECT_LIB) -L$(ZLIB_LIB) -L$(FUSION_LIB) -L$(SDL2_LIB)
LDLIBS := -ldirectfb -ldirect -lfusion -lz -lSDL2 -pthread

# specify bin
BIN_NAME := sdl_demo

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
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
