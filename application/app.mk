PHONY :=

root := $(APP_PATH)

subdir := $(root)/library
-include $(subdir)/app.mk

subdir := $(root)/3rd_party
-include $(subdir)/app.mk

subdir := $(root)/av_daemon
-include $(subdir)/app.mk

subdir := $(root)/network_application
-include $(subdir)/app.mk

subdir := $(root)/sample
-include $(subdir)/app.mk

subdir := $(root)/setting_mgmt
-include $(subdir)/app.mk

subdir := $(root)/streaming_server
-include $(subdir)/app.mk

subdir := $(root)/system_utility
-include $(subdir)/app.mk

PHONY += app-check
app-check:
ifeq ("$(wildcard $(KCONFIG_CONFIG))","")
	$(error Application config not found)
endif

PHONY += app app-clean app-distclean app-install app-uninstall
app: app-check $(APP_BUILD_DEPS)

app-clean: $(APP_CLEAN_DEPS)

app-distclean: $(APP_DISTCLEAN_DEPS)
	$(Q)rm -f $(KCONFIG_CONFIG)
	$(Q)rm -f $(KCONFIG_CONFIG).old
	$(Q)rm -rf $(APP_LIB)
	@echo 'Application config `$(notdir $(KCONFIG_CONFIG))` has been removed.'

app-install: $(APP_INTALL_DEPS)

app-uninstall: $(APP_UNINTALL_DEPS)

.PHONY: $(PHONY)
