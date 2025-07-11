mod := $(notdir $(subdir))


app-$(CONFIG_APP_LIVE555) += live555
PHONY += live555 live555-clean live555-distclean
PHONY += live555-install live555-uninstall
ifeq ($(CONFIG_APP_RTSP_SERVER_ENABLE_IVA),y)
live555: libavftr

live555-clean: libavftr-clean

live555-distclean: libavftr-distclean

live555-install: libavftr-install

live555-uninstall: libavftr-uninstall
endif
live555: libado libcm
	$(Q)$(MAKE) -C $(LIVE555_PATH) all

live555-clean: libavftr-clean libeaif-clean libcm-clean \
	libado-clean
live555-clean: libcm-clean libado-clean
	$(Q)$(MAKE) -C $(LIVE555_PATH) clean

live555-distclean: libcm-distclean libado-distclean
	$(Q)$(MAKE) -C $(LIVE555_PATH) distclean

live555-install: libado-install libcm-install
	$(Q)$(MAKE) -C $(LIVE555_PATH) install

live555-uninstall: libcm-uninstall libado-uninstall
	$(Q)$(MAKE) -C $(LIVE555_PATH) uninstall

app-$(CONFIG_APP_FLV_SERVER) += flv_server
PHONY += flv_server flv_server-clean flv_server-distclean
PHONY += flv_server-install flv_server-uninstall
flv_server:
	$(Q)$(MAKE) -C $(FLV_PATH)/build all

flv_server-clean:
	$(Q)$(MAKE) -C $(FLV_PATH)/build clean

flv_server-distclean:
	$(Q)$(MAKE) -C $(FLV_PATH)/build distclean

flv_server-install:
	$(Q)$(MAKE) -C $(FLV_PATH)/build install

flv_server-uninstall:
	$(Q)$(MAKE) -C $(FLV_PATH)/build uninstall

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
