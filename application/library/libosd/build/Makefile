SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INC := -isystem$(ZLIB_INC) -isystem$(FREETYPE_INC)
DEPS :=

ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
INC += -isystem$(SDL2_INC) -isystem$(SDL2TTF_INC) -isystem$(LIBPNG_INC)
DEFS += SDL_ENABLE
endif

CFLAGS += -ffunction-sections -fdata-sections $(INC)
CFLAGS += $(addprefix -D,$(DEFS))

INSTALL_PATH := $(libdir)

LIB_NAME = osd
LIB_MAJOR = 1
LIB_MINOR = 0
LIB_PATCH = 0


.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

.PHONY: clean 
clean: clean-a clean-so
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: uninstall clean
	$(Q)$(RM) -rf $(LIB_OUT_PATH)


.PHONY: install
install: all
	$(Q)$(CP) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_SONAME)

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
