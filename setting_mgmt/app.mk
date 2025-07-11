mod := $(notdir $(subdir))

app-$(CONFIG_APP_CENTCTRL) += centctrl
PHONY += centctrl centctrl-clean centctrl-distclean
PHONY += centctrl-install centctrl-uninstall
centctrl: libcm libsql
	$(Q)$(MAKE) -C $(CENTCTRL_PATH) all

centctrl-clean: libsql-clean libcm-clean
	$(Q)$(MAKE) -C $(CENTCTRL_PATH) clean

centctrl-distclean: libsql-distclean libcm-distclean
	$(Q)$(MAKE) -C $(CENTCTRL_PATH) distclean

centctrl-install: libcm-install libsql-install
	$(Q)$(MAKE) -C $(CENTCTRL_PATH) install

centctrl-uninstall: libsql-uninstall libcm-uninstall
	$(Q)$(MAKE) -C $(CENTCTRL_PATH) uninstall

app-$(CONFIG_APP_DBMONITOR) += dbmonitor
PHONY += dbmonitor dbmonitor-clean dbmonitor-distclean
PHONY += dbmonitor-install dbmonitor-uninstall
dbmonitor:
	$(Q)$(MAKE) -C $(DBMONITOR_PATH) all

dbmonitor-clean:
	$(Q)$(MAKE) -C $(DBMONITOR_PATH) clean

dbmonitor-distclean:
	$(Q)$(MAKE) -C $(DBMONITOR_PATH) distclean

dbmonitor-install:
	$(Q)$(MAKE) -C $(DBMONITOR_PATH) install

dbmonitor-uninstall:
	$(Q)$(MAKE) -C $(DBMONITOR_PATH) uninstall

app-$(CONFIG_APP_SQLAPP) += sqlapp
PHONY += sqlapp sqlapp-clean sqlapp-distclean
PHONY += sqlapp-install sqlapp-uninstall
sqlapp: libcm libsql
	@echo - Building sql app
	$(Q)$(MAKE) -C $(SQLAPP_PATH) all

sqlapp-clean: libsql-clean libcm-clean
	$(Q)$(MAKE) -C $(SQLAPP_PATH) clean

sqlapp-distclean: libsql-distclean libcm-distclean
	$(Q)$(MAKE) -C $(SQLAPP_PATH) distclean

sqlapp-install: libcm-install libsql-install
	$(Q)$(MAKE) -C $(SQLAPP_PATH) install

sqlapp-uninstall: libsql-uninstall libcm-uninstall
	$(Q)$(MAKE) -C $(SQLAPP_PATH) uninstall

PHONY += $(mod) $(mod)-clean $(mod)-distclean
PHONY += $(mod)-install $(mod)-uninstall
$(mod): $(app-y)
$(mod)-clean: $(addsuffix -clean,$(app-y))
$(mod)-distclean: $(addsuffix -distclean,$(app-y))
$(mod)-install: $(addsuffix -install,$(app-y))
$(mod)-uninstall: $(addsuffix -uninstall,$(app-y))

APP_BUILD_DEPS += $(mod)
APP_CLEAN_DEPS += $(mod)-clean
APP_DISTCLEAN_DEPS += $(mod)-distclean
APP_INTALL_DEPS += $(mod)-install
APP_UNINTALL_DEPS += $(mod)-uninstall
