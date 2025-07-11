SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INC := -I$(ZLIB_INC) -isystem$(OPENSSL_INC) -I$(FEATURE_SECURE_ELEMENT_INC_PATH)
DEPS :=

LDFLAGS := -L$(FEATURE_SECURE_ELEMENT_LIB_PATH) -lse

CFLAGS += -ffunction-sections -fdata-sections $(INC) $(LDFLAGS)
CFLAGS += $(addprefix -D,$(DEFS))

INSTALL_PATH := $(libdir)
INSTALL_ASSETS_PATH := $(SYSROOT)/etc/ssl/

ASSETS_PATH := $(root)/assets
ASSETS := $(wildcard $(ASSETS_PATH)/*)
ASSETS_FILES := $(notdir $(wildcard $(ASSETS_PATH)/*))
INSTALLED_ASSETS_FILES := $(patsubst %, $(INSTALL_ASSETS_PATH)%, $(ASSETS_FILES))

LIB_NAME = se_trng
LIB_MAJOR = 1
LIB_MINOR = 0
LIB_PATCH = 0


.DEFAULT_GOAL := default

.PHONY: default
default: all


.PHONY: all
all: $(LIB_SHARED_OUTPUT)

.PHONY: clean 
clean: clean-a clean-so
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH) $(LIB_OUT_PATH)

.PHONY: distclean
distclean: uninstall clean
	$(Q)$(RM) -rf $(LIB_OUT_PATH)


.PHONY: install
install: all
	$(Q)$(INSTALL) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)
	$(CP) -r $(ASSETS) $(INSTALLED_ASSETS_FILES)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(RM) $(INSTALL_PATH)/$(LIB_SONAME)
	$(Q)$(RM_RF) $(INSTALLED_ASSETS_FILES)


$(LIB_OUT_PATH)/$(LIB_LINKERNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_SONAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -nsf $< $@

$(LIB_OUT_PATH)/$(LIB_REALNAME): $(OBJS)
	$(Q)$(MKDIR_P) $(LIB_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -shared -Wl,-soname,$(LIB_SONAME) $^ -o $@

-include $(DEPS)

