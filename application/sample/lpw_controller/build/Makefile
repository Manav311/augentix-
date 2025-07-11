SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INSTALL := install -c

ifeq ($(CONFIG_LPW_HI3861L),y)
TARGET_MODULE := ../src/hi3861l
CFLAGS+=-DLPW_HI3861L
else
ifeq ($(CONFIG_LPW_AIW4211L),y)
TARGET_MODULE := ../src/aiw4211l
CFLAGS+=-DLPW_AIW4211L
else
$(error No LPW module selected)
endif
endif

DSTDIR = $(SYSROOT)/usr/sbin
TARGET_ELF := ../bin/lpw_controller

DST_TARGET_ELF := $(subst ../bin,$(DSTDIR),$(TARGET_ELF))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all clean
all clean: 
ifeq ($(CONFIG_LPW_HI3861L),y)
	@echo "CONFIG_LPW_HI3861L="$(CONFIG_LPW_HI3861L)
endif
ifeq ($(CONFIG_LPW_AIW4211L),y)
	@echo "CONFIG_LPW_AIW4211L="$(CONFIG_LPW_AIW4211L)
endif
	$(Q)$(MAKE) -C $(TARGET_MODULE) $@

.PHONY: install
install: all
	$(Q)$(INSTALL) -d $(DSTDIR)
	$(Q)$(INSTALL) -m 755 $(TARGET_ELF) $(DSTDIR)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(DST_TARGET_ELF)


.PHONY: distclean
distclean: uninstall clean
