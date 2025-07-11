SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

# Filesystem image declarations

TARGETS = sntp.conf timeMode.conf
DEST := $(SYSTEMFS)/factory_default
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

.PHONY: default
default: all

.PHONY: all
all:

.PHONY: rebuild
rebuild: clean all

.PHONY: install
install:
	$(Q)cp -f $(TARGETS) $(DEST)/.

.PHONY: uninstall
uninstall:
	$(Q)rm -rf $(INSTALL_TRGTS)

.PHONY: clean
clean:
	$(Q)rm -rf $(INSTALL_TRGTS)

.PHONY: distclean
distclean: uninstall clean
