SDKSRC_DIR ?= $(realpath $(CURDIR)/..)
include $(SDKSRC_DIR)/application/internal.mk

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all:
	@echo application CROSS_COMPILE: $(CROSS_COMPILE)
	$(MAKE) app

KCONFIG_CONFIG ?= $(APP_PATH)/.config
export KCONFIG_CONFIG
include kconfig.mk

-include $(KCONFIG_CONFIG)
include app.mk

.PHONY: clean
clean: app-clean

.PHONY: distclean
distclean: app-distclean

.PHONY: install
install: app-install

.PHONY: uninstall
uninstall: app-uninstall

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG): ;

.PHONY: help
help:
	@echo 'Cleaning targets:'
	@echo '  clean                  - delete all files created by build'
	@echo '  distclean              - delete all non-source files (including .config)'
	@echo ''
	@echo 'Configuration targets:'
	@echo '  config                 - Update current config utilising a line-oriented program'
	@echo '  nconfig                - Update current config utilising a ncurses menu based'
	@echo '                           program'
	@echo '  menuconfig             - Update current config utilising a menu based program'
	@echo '  oldconfig              - Update current config utilising a provided .config as base'
	@echo '  defconfig              - New config with default defconfig'
	@echo '  <%_defconfig>          - New config with specified defconfig'
	@echo '  savedefconfig          - Save current config as default defconfig'
	@echo '  savedefconfig DEFCONFIG=<path_to_target_defconfig>'
	@echo '                         - Save current config as specified defconfig'
	@echo ''
	@echo 'Generic targets:'
	@echo '  all                    - build specified applications depending on configuration'
	@echo '  install                - install applications to rootfs'
	@echo '  uninstall              - uninstall applications to rootfs'
	@echo '  defconfig-list         - list all defconfigs'
	@echo ''
	@echo '  make V=0|1 [targets]   - 0 => quiet build (default), 1 => verbose build'
	@echo ''
	@echo 'Execute "make" or "make all" to build all targets marked with [*]'
