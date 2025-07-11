SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

# append CFLAGS
INC := -I$(MPP_INC)
LIB := -L$(MPP_LIB) -lmpp
CFLAGS += $(INC) $(LIB)

# specify lib name and its version
LIB_NAME := agtxapi
LIB_MAJOR := 1
LIB_MINOR := 0
LIB_PATCH := 0

INSTALL_PATH := $(SYSTEM_LIB)

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

.PHONY: clean
clean: clean-a clean-so
	$(Q)$(RM) -r $(LIB_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all
	$(Q)$(CP) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_LINKERNAME)*

$(LIB_STATIC_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(LIB_OUT_PATH)/$(LIB_LINKERNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_SONAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_REALNAME): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -shared -Wl,-soname,$(LIB_SONAME) $^ -o $@

-include $(DEPS)
