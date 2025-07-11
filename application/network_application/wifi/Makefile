SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

INSTALL := install -c

SCRIPT_DSTDIR = $(SYSTEMFS)/script
CONF_DSTDIR = $(SYSROOT)/etc
TARGET_SCIPRTS := $(wildcard scripts/*.sh)
TARGET_CONF := $(wildcard scripts/*.conf)

DST_TARGET_SCRIPTS := $(subst scripts,$(SCRIPT_DSTDIR),$(TARGET_SCIPRTS))
DST_TARGET_CONF := $(subst scripts,$(CONF_DSTDIR),$(TARGET_CONF))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: ;

.PHONY: install
install:
	$(Q)$(INSTALL) -d $(CONF_DSTDIR)
	$(Q)$(INSTALL) -d $(SCRIPT_DSTDIR)
	$(Q)$(INSTALL) -m 755 $(TARGET_SCIPRTS) $(SCRIPT_DSTDIR)
	$(Q)$(INSTALL) -m 755 $(TARGET_CONF) $(CONF_DSTDIR)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(DST_TARGET_SCRIPTS)
	$(Q)$(RM) $(DST_TARGET_CONF)

.PHONY: clean
clean: ;

.PHONY: distclean
distclean: uninstall clean
