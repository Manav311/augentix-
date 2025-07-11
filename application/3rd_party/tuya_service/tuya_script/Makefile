SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/build/sdksrc.mk

PROD_NAME ?= HC1892_EM1

SCRIPTS = time.sh DSTConf.sh tuya_unpair.sh
DEST := $(SYSTEMFS)/script
INSTALL_TRGT := $(addprefix $(DEST)/, $(SCRIPTS))

.PHONY: default
default: all

.PHONY: all
all: install

.PHONY: install
install: $(INSTALL_TRGT)

$(INSTALL_TRGT):
	$(Q)install -d $(DEST)
	$(Q)install -m 777 $(SCRIPTS) $(DEST)

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGT)

.PHONY: clean
clean: uninstall
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean
