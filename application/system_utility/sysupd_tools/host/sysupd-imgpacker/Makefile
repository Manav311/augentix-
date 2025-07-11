SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../..)
include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

DEST := $(BINPKG_DIR)/sysupd-imgpacker

SRCDIR = script

TRGTS  := $(wildcard $(SRCDIR)/*)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(notdir $(TRGTS)))
UPD_DESC := fwupd.desc

.PHONY: all
all:
ifeq ($(CONFIG_HC1702_1752_1772_1782), y)
	$(Q)$(LN) -sfr product/hc18xx.desc $(SRCDIR)/$(UPD_DESC)
else
	$(Q)$(LN) -sfr product/$(CONFIG_MACH).desc $(SRCDIR)/$(UPD_DESC)
endif

#===============================================================================
# SDKSRC rules
#===============================================================================

.PHONY: install
install: $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: $(SRCDIR)/%
	$(Q)install $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)
	$(Q)rm -rf $(DEST)

.PHONY: clean
clean:
	$(Q)rm -rf *.swu *.bin $(SW_DESC)
	$(Q)rm -f $(SRCDIR)/$(UPD_DESC) 

.PHONY: distclean
distclean: uninstall clean
