SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

LAYOUT_FW_CONFIG := $(patsubst "%.ini",%.config,$(CONFIG_NVSP_LAYOUT))
NAND_B2_FW_CONFIG := nand_b2_fw_env.config

#===============================================================================
# SDKSRC rules
#===============================================================================

.PHONY: default
default: all

.PHONY: all
all: ;

.PHONY: install
install:
ifeq ($(CONFIG_HC1702_1752_1772_1782), y)
	$(Q)cp -f product/$(NAND_B2_FW_CONFIG) fw_env.config
else
	$(Q)cp -f product/$(LAYOUT_FW_CONFIG) fw_env.config
endif
	$(Q)cp -f fw_env.config $(SYSROOT)/etc/fw_env.config

.PHONY: uninstall
uninstall:
	$(Q)rm -rf $(SYSROOT)/etc/fw_env.config
	$(Q)rm -rf fw_env.config

.PHONY: clean
clean: ;

.PHONY: distclean
distclean: clean
