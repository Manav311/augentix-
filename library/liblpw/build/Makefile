SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

INSTALL_PATH := $(libdir)

# specify lib name and its version
LIB_NAME := lpw
LIB_MAJOR := 1
LIB_MINOR := 1
LIB_PATCH := 0

INC :=
LIB := -lpthread

CFLAGS += $(addprefix -I,$(INC))
LIB_LPW := -I$(LIBLPW_INC) -L$(LIBLPW_LIB) -llpw

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: test
test:
	$(Q)$(CC) ../test/*.c $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(LIB) $(LIB_LPW) -o $(LIBLPW_PATH)/test/lpw_test


.PHONY: all
all: $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

# Delete all files created by building the library.
.PHONY: clean
clean: clean-a clean-so
	$(Q)$(RM) -r $(LIB_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(RM) -r $(LIBLPW_PATH)/test/lpw_test

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all
	$(Q)$(CP) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)

.PHONY: uninstall
uninstall: uninstall-so

$(LIB_STATIC_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(LIB_OUT_PATH)/$(LIB_LINKERNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_SONAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_REALNAME): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(LIB) -shared -Wl,-soname,$(LIB_SONAME) $^ -o $@

-include $(DEPS)
