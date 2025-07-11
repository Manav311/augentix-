SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

#===============================================================================
# SDKSRC rules
#===============================================================================

.PHONY: default
default: all

.PHONY: all
all: host target

.PHONY: install
install: host-install target-install

.PHONY: uninstall
uninstall: host-uninstall target-uninstall

.PHONY: clean
clean: host-clean target-clean

.PHONY: distclean
distclean: host-distclean target-distclean

#===============================================================================
# Host
#===============================================================================

.PHONY: host
host:
	$(Q)for f in $(shell ls $@); \
		do make -C $@/$$f all; \
	done

.PHONY: host-clean
host-clean:
	$(Q)for f in $(shell ls $(@:-clean=)); do \
		[ -f Makefile ] && make -C $(@:-clean=)/$$f clean; \
	done

.PHONY: host-distclean
host-distclean: host-uninstall host-clean

.PHONY: host-install
host-install:
	$(Q)for f in $(shell ls $(@:-install=)); do \
		[ -f Makefile ] && make -C $(@:-install=)/$$f install; \
	done

.PHONY: host-uninstall
host-uninstall:
	$(Q)for f in $(shell ls $(@:-uninstall=)); do \
		[ -f Makefile ] && make -C $(@:-uninstall=)/$$f uninstall; \
	done

#===============================================================================
# Target
#===============================================================================

# Name of product fw_env.config file
LAYOUT_FW_CONFIG := $(patsubst "%.ini",%.config,$(CONFIG_NVSP_LAYOUT))
NAND_B2_FW_CONFIG := nand_b2_fw_env.config

.PHONY: target
target: ;

.PHONY: target-clean
target-clean: ;

.PHONY: target-distclean
target-distclean: target-uninstall target-clean

.PHONY: target-install
target-install:
	$(Q)mkdir -p $(SYSROOT)/usr/sbin/
	$(Q)rsync -ar target/usr/sbin/sysupd-recover $(SYSROOT)/usr/sbin/sysupd-recover
ifeq ($(CONFIG_APP_SYSUPD_ANTI_ROLLBACK_SE_TOOL),y)
	$(Q)rsync -ar target/usr/sbin/sysupd-anti-rollback-se $(SYSROOT)/usr/sbin/sysupd
else # normal
	$(Q)rsync -ar target/usr/sbin/sysupd $(SYSROOT)/usr/sbin/sysupd
endif

.PHONY: target-uninstall
target-uninstall:
	$(Q)rm -rf $(SYSROOT)/usr/sbin/sysupd
	$(Q)rm -rf $(SYSROOT)/usr/sbin/sysupd-recover
