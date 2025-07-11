SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

INSTALL_PATH := $(libdir)

# specify lib name and its version
LIB_NAME := ampc_linux
LIB_MAJOR := 1
LIB_MINOR := 0
LIB_PATCH := 0

INC := $(LIBMETAL_INC)
INC += $(LIBOPENAMP_INC)
INC += $(BUILDROOT_HOST_INC)/sysfs/
INC += $(SDKSRC_DIR)/include/
LIB := -L$(LIBMETAL_LIB) -lmetal -L$(LIBOPENAMP_LIB) -lopen_amp

CFLAGS += $(addprefix -I,$(INC))
CPPFLAGS += -DMETAL_INTERNAL
LIB_AMP = -I$(LIBAMPC_INC) -L$(LIBAMPC_LIB) -lampc_linux
DEPFLAGS += -Wl,-unresolved-symbols=ignore-in-shared-libs

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: test
test:
	$(Q)$(CC) ../test/*.c $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(LIB) $(LIB_AMP) -o $(LIBAMPC_PATH)/test/amp_test


.PHONY: all
all: $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

# Delete all files created by building the library.
.PHONY: clean
clean: clean-a clean-so
	$(Q)$(RM) -r $(LIB_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

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
