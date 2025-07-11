SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# append CFLAGS
INC := -I$(LIBSAMPLE_INC)
LIB := -L$(LIBSAMPLE_LIB) -l:libsample.a -lsample
CFLAGS += $(INC) $(LIB)

# specify lib name and its version
LIB_NAME := motor
LIB_MAJOR := 1
LIB_MINOR := 0
LIB_PATCH := 0

INSTALL_PATH := $(SYSTEM_LIB)

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(LIB_STATIC_OUTPUT)

.PHONY: clean
clean: clean-a clean-so
	$(Q)$(RM) -r $(LIB_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: uninstall clean test-clean

.PHONY: install
install: all
	
.PHONY: uninstall
uninstall:

.PHONY: test test-clean
test:
	make clean -C $(LIBMOTOR_PATH)/test/build && make -C $(LIBMOTOR_PATH)/test/build;
test-clean:
	make clean -C $(LIBMOTOR_PATH)/test/build


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