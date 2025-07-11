mod := $(notdir $(subdir))

app-$(CONFIG_APP_CONNSEL) += connsel
PHONY += connsel connsel-clean connsel-distclean
PHONY += connsel-install connsel-uninstall
connsel:
	$(Q)$(MAKE) -C $(CONNSEL_PATH) all

connsel-clean:
	$(Q)$(MAKE) -C $(CONNSEL_PATH) clean

connsel-distclean:
	$(Q)$(MAKE) -C $(CONNSEL_PATH) distclean

connsel-install:
	$(Q)$(MAKE) -C $(CONNSEL_PATH) install

connsel-uninstall:
	$(Q)$(MAKE) -C $(CONNSEL_PATH) uninstall

app-$(CONFIG_APP_EAIF_SERVER) += eaif_server
PHONY += eaif_server eaif_server-clean eaif_server-distclean
PHONY += eaif_server-install eaif_server-uninstall
eaif_server:
	$(Q)$(MAKE) -C $(EAIF_SERV_BUILD_PATH) all

eaif_server-clean:
	$(Q)$(MAKE) -C $(EAIF_SERV_BUILD_PATH) clean

eaif_server-distclean:
	$(Q)$(MAKE) -C $(EAIF_SERV_BUILD_PATH) distclean

eaif_server-install:
	$(Q)$(MAKE) -C $(EAIF_SERV_BUILD_PATH) install

eaif_server-uninstall:
	$(Q)$(MAKE) -C $(EAIF_SERV_BUILD_PATH) uninstall

app-$(CONFIG_APP_IP_ASSIGN) += ip_assign
PHONY += ip_assign ip_assign-clean ip_assign-distclean
PHONY += ip_assign-install ip_assign-uninstall
ip_assign:
	$(Q)$(MAKE) -C $(IPASSIGN_PATH) all

ip_assign-clean:
	$(Q)$(MAKE) -C $(IPASSIGN_PATH) clean

ip_assign-distclean:
	$(Q)$(MAKE) -C $(IPASSIGN_PATH) distclean

ip_assign-install:
	$(Q)$(MAKE) -C $(IPASSIGN_PATH) install

ip_assign-uninstall:
	$(Q)$(MAKE) -C $(IPASSIGN_PATH) uninstall

app-$(CONFIG_APP_ONVIF_SERVER) += onvif_server
PHONY += onvif_server onvif_server-clean onvif_server-distclean
PHONY += onvif_server-install onvif_server-uninstall
onvif_server: libcm libtz
	$(Q)$(MAKE) -C $(ONVIF_SERVER_PATH) all

onvif_server-clean: libtz-clean libcm-clean
	$(Q)$(MAKE) -C $(ONVIF_SERVER_PATH) clean

onvif_server-distclean: libtz-distclean \
	libcm-distclean
	$(Q)$(MAKE) -C $(ONVIF_SERVER_PATH) distclean

onvif_server-install: libcm-install libtz-install
	$(Q)$(MAKE) -C $(ONVIF_SERVER_PATH) install

onvif_server-uninstall: libtz-uninstall libcm-uninstall
	$(Q)$(MAKE) -C $(ONVIF_SERVER_PATH) uninstall

app-$(CONFIG_APP_STAMEN) += stamen
PHONY += stamen stamen-clean stamen-distclean
PHONY += stamen-install stamen-uninstall
stamen:
	$(Q)$(MAKE) -C $(STAMEN_PATH)/build all

stamen-clean:
	$(Q)$(MAKE) -C $(STAMEN_PATH)/build clean

stamen-distclean:
	$(Q)$(MAKE) -C $(STAMEN_PATH)/build distclean

stamen-install:
	$(Q)$(MAKE) -C $(STAMEN_PATH)/build install

stamen-uninstall:
	$(Q)$(MAKE) -C $(STAMEN_PATH)/build uninstall

app-$(CONFIG_APP_UNICORN) += unicorn
PHONY += unicorn unicorn-clean unicorn-distclean
PHONY += unicorn-install unicorn-uninstall
unicorn:
	$(Q)$(MAKE) -C $(UNICORN_PATH)/build all

unicorn-clean:
	$(Q)$(MAKE) -C $(UNICORN_PATH)/build clean

unicorn-distclean:
	$(Q)$(MAKE) -C $(UNICORN_PATH)/build distclean

unicorn-install:
	$(Q)$(MAKE) -C $(UNICORN_PATH)/build install

unicorn-uninstall:
	$(Q)$(MAKE) -C $(UNICORN_PATH)/build uninstall

app-$(CONFIG_APP_WEBCGI) += webcgi
PHONY += webcgi webcgi-clean webcgi-distclean
PHONY += webcgi-install webcgi-uninstall
webcgi:
	$(Q)$(MAKE) -C $(WEBCGI_PATH) all

webcgi-clean:
	$(Q)$(MAKE) -C $(WEBCGI_PATH) clean

webcgi-distclean:
	$(Q)$(MAKE) -C $(WEBCGI_PATH) distclean

webcgi-install:
	$(Q)$(MAKE) -C $(WEBCGI_PATH) install

webcgi-uninstall:
	$(Q)$(MAKE) -C $(WEBCGI_PATH) uninstall

app-$(CONFIG_APP_SE_AUTH) += se_auth
PHONY += se_auth se_auth-clean se_auth-distclean
PHONY += se_auth-install se_auth-uninstall
se_auth:
	$(Q)$(MAKE) -C $(SE_AUTH_PATH) all

se_auth-clean:
	$(Q)$(MAKE) -C $(SE_AUTH_PATH) clean

se_auth-distclean:
	$(Q)$(MAKE) -C $(SE_AUTH_PATH) distclean

se_auth-install:
	$(Q)$(MAKE) -C $(SE_AUTH_PATH) install

se_auth-uninstall:
	$(Q)$(MAKE) -C $(SE_AUTH_PATH) uninstall

app-$(CONFIG_APP_WIFI) += wifi
PHONY += wifi wifi-clean wifi-distclean
PHONY += wifi-install wifi-uninstall
wifi:
	$(Q)$(MAKE) -C $(WIFI_PATH) all

wifi-clean:
	$(Q)$(MAKE) -C $(WIFI_PATH) clean

wifi-distclean:
	$(Q)$(MAKE) -C $(WIFI_PATH) distclean

wifi-install:
	$(Q)$(MAKE) -C $(WIFI_PATH) install

wifi-uninstall:
	$(Q)$(MAKE) -C $(WIFI_PATH) uninstall

####################
# CONFIG_APP_ONVIF_DISCOVERY is deprecated.
# Please use CONFIG_APP_WS_DISCOVERY instead.
ifeq ($(CONFIG_APP_ONVIF_DISCOVERY),y)
ifneq ($(CONFIG_APP_WS_DISCOVERY),y)
app-$(CONFIG_APP_ONVIF_DISCOVERY) += ws_discovery
endif
endif
####################
app-$(CONFIG_APP_WS_DISCOVERY) += ws_discovery
PHONY += ws_discovery ws_discovery-clean ws_discovery-distclean
PHONY += ws_discovery-install ws_discovery-uninstall
ws_discovery:
	$(Q)$(MAKE) -C $(WS_DISCOVERY_PATH) all

ws_discovery-clean:
	$(Q)$(MAKE) -C $(WS_DISCOVERY_PATH) clean

ws_discovery-distclean:
	$(Q)$(MAKE) -C $(WS_DISCOVERY_PATH) distclean

ws_discovery-install:
	$(Q)$(MAKE) -C $(WS_DISCOVERY_PATH) install

ws_discovery-uninstall:
	$(Q)$(MAKE) -C $(WS_DISCOVERY_PATH) uninstall

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
