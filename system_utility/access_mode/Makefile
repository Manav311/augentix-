SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

TARGETS = mode
DEST := $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

mode_file = $(USRDATAFS)/mode

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS)
	$(Q)echo "factory" > $(mode_file)

.PHONY: update
update: clean all install

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS) $(mode_file)

.PHONY: clean
clean: ;

.PHONY: distclean
distclean: uninstall clean

$(TARGETS): ;

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@
