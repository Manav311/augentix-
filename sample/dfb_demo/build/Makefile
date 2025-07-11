SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# for self-made firectfb applications
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INC := -isystem$(ZLIB_INC) -isystem$(DIRECTFB_INC)
LDFLAGS := -ldirectfb -lfusion -ldirect -lz -lm -lrt 
LIB := -L$(DIRECTFB_LIB)-L$(DIRECTFB_LIB) -L$(FUSION_LIB)  $(LDFLAGS)
CFLAGS += -pthread $(INC)


DIRECTFBEXMAPLEFS = $(SYSTEM_BIN)/dfb_demo
DIRECTFBEXMAPLESPACEDREAMFS = $(DIRECTFBEXMAPLEFS)/spacedream

SPACEDREAM_ASSETS := star1.bmp star2.bmp star3.bmp star4.bmp
ASSETS :=  apple-red.bmp convergence.bmp cursor.bmp cursor_red.bmp desktop.bmp destination_mask.bmp \
			dfblogo.bmp gnome-applets.bmp gnome-calendar.bmp gnome-foot.bmp gnome-gimp.bmp gnome-gmush.bmp \
			gnome-gsame.bmp gnu-keys.bmp joystick.bmp shot.bmp tux.bmp tux_alpha.bmp desktop.png cursor.png cursor_red.png
			
ASSETS_TARGET_PATH := $(root)/assets
ASSETS_PATH := $(DIRECTFBEXMAPLEFS)
ASSETS_SRC := $(addprefix $(ASSETS_TARGET_PATH)/,$(ASSETS))
ASSETS_TARGET := $(addprefix $(ASSETS_PATH)/,$(ASSETS))

SPACEDREAM_ASSETS_TARGET_PATH := $(root)/assets/spacedream
SPACEDREAM_ASSETS_PATH := $(DIRECTFBEXMAPLESPACEDREAMFS)
SPACEDREAM_ASSETS_SRC := $(addprefix $(SPACEDREAM_ASSETS_TARGET_PATH)/,$(SPACEDREAM_ASSETS))
SPACEDREAM_ASSETS_TARGET := $(addprefix $(SPACEDREAM_ASSETS_PATH)/,$(SPACEDREAM_ASSETS))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all:

.PHONY: clean
clean:
#	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: clean


.PHONY: install
install: all $ install-assets
#	@printf "  %-8s$(bindir)/*\n" "INSTALL"

.PHONY: uninstall
uninstall: uninstall-assets 
#	$(Q)$(RM) -f $(BIN_TARGET)

.PHONY: install-assets
install-assets: 
	$(Q)$(MKDIR_P) $(ASSETS_PATH)
	$(Q)$(MKDIR_P) $(SPACEDREAM_ASSETS_PATH)
	@printf "  %-8s$(ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(ASSETS_SRC)), \
		cp -r $(f) $(ASSETS_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(SPACEDREAM_ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(SPACEDREAM_ASSETS_SRC)), \
		cp -r $(f) $(SPACEDREAM_ASSETS_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-assets
uninstall-assets:
	@printf "  %-8s$(ASSETS_TARGET)\n" "RM"
	$(Q)$(RM) -f $(ASSETS_TARGET)
	$(Q)$(RM) -f $(SPACEDREAM_ASSETS_TARGET)

-include $(DEPS)