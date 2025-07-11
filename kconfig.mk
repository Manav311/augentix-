Kconfig := Kconfig

CP := cp

CONFIG_PATH := $(APP_PATH)/configs
DEFCONFIG ?= $(APP_CONFIG)

COMMON_CONFIG_ENV := \
	LD_LIBRARY_PATH=$(KCONFIG_PATH)

define config_written
	@echo \#
	@echo \# configuration written to $(1)
	@echo \#
endef

.PHONY: config
config: $(KCONFIG_PATH)/kconfig-conf
	$(Q)$(COMMON_CONFIG_ENV) $< $(Kconfig)

.PHONY: nconfig
nconfig: $(KCONFIG_PATH)/kconfig-nconf
	$(Q)$(COMMON_CONFIG_ENV) $< $(Kconfig)

.PHONY: menuconfig
menuconfig: $(KCONFIG_PATH)/kconfig-mconf
	$(Q)$(COMMON_CONFIG_ENV) $< $(Kconfig)

.PHONY: oldconfig
oldconfig: $(KCONFIG_PATH)/kconfig-conf
	$(Q)$(COMMON_CONFIG_ENV) $< --$@ $(Kconfig)

.PHONY: defconfig
defconfig: $(KCONFIG_PATH)/kconfig-conf
	@echo "*** Default configuration is based on '$(DEFCONFIG)'"
	$(Q)$(COMMON_CONFIG_ENV) $< --$@=$(CONFIG_PATH)/$(DEFCONFIG) $(Kconfig)

.PHONY: savedefconfig
savedefconfig:
	$(Q)$(CP) -f $(KCONFIG_CONFIG) $(CONFIG_PATH)/$(DEFCONFIG)
	$(call config_written,$(DEFCONFIG))

.PHONY: defconfig-list
defconfig-list:
	$(Q)$(foreach b, $(sort $(notdir $(wildcard $(CONFIG_PATH)/*_defconfig))), \
		printf "  %-30s - Build for %s\\n" $(b) $(b:_defconfig=);)

%_defconfig: DEFCONFIG=$@
%_defconfig: $(CONFIG_PATH)/%_defconfig
	$(Q)DEFCONFIG=$(DEFCONFIG) $(MAKE) V=1 --no-print-directory defconfig
