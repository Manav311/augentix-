mod := $(notdir $(subdir))

# always enable access_mode
app-$(CONFIG_APP_ACCESS_MODE) += access_mode
PHONY += access_mode access_mode-clean access_mode-distclean
PHONY += access_mode-install access_mode-uninstall
access_mode:
	$(Q)$(MAKE) -C $(ACCESS_MODE_PATH) all

access_mode-clean:
	$(Q)$(MAKE) -C $(ACCESS_MODE_PATH) clean

access_mode-distclean:
	$(Q)$(MAKE) -C $(ACCESS_MODE_PATH) distclean

access_mode-install:
	$(Q)$(MAKE) -C $(ACCESS_MODE_PATH) install

access_mode-uninstall:
	$(Q)$(MAKE) -C $(ACCESS_MODE_PATH) uninstall

app-$(CONFIG_APP_ALARM_OUT) += alarm_out
PHONY += alarm_out alarm_out-clean alarm_out-distclean
PHONY += alarm_out-install alarm_out-uninstall
alarm_out:
	$(Q)$(MAKE) -C $(ALARMOUT_PATH) all

alarm_out-clean:
	$(Q)$(MAKE) -C $(ALARMOUT_PATH) clean

alarm_out-distclean:
	$(Q)$(MAKE) -C $(ALARMOUT_PATH) distclean

alarm_out-install:
	$(Q)$(MAKE) -C $(ALARMOUT_PATH) install

alarm_out-uninstall:
	$(Q)$(MAKE) -C $(ALARMOUT_PATH) uninstall

app-$(CONFIG_APP_APPVERIFY) += appverify
PHONY += appverify appverify-clean appverify-distclean
PHONY += appverify-install appverify-uninstall
appverify:
	$(Q)$(MAKE) -C $(APPVERIFY_PATH) all

appverify-clean:
	$(Q)$(MAKE) -C $(APPVERIFY_PATH) clean

appverify-distclean:
	$(Q)$(MAKE) -C $(APPVERIFY_PATH) distclean

appverify-install:
	$(Q)$(MAKE) -C $(APPVERIFY_PATH) install

appverify-uninstall:
	$(Q)$(MAKE) -C $(APPVERIFY_PATH) uninstall

app-$(CONFIG_APP_DAY_NIGHT_MODE) += day_night_mode
PHONY += day_night_mode day_night_mode-clean day_night_mode-distclean
PHONY += day_night_mode-install day_night_mode-uninstall
day_night_mode:
	$(Q)$(MAKE) -C $(DAY_NIGHT_MODE_PATH) all

day_night_mode-clean:
	$(Q)$(MAKE) -C $(DAY_NIGHT_MODE_PATH) clean

day_night_mode-distclean:
	$(Q)$(MAKE) -C $(DAY_NIGHT_MODE_PATH) distclean

day_night_mode-install:
	$(Q)$(MAKE) -C $(DAY_NIGHT_MODE_PATH) install

day_night_mode-uninstall:
	$(Q)$(MAKE) -C $(DAY_NIGHT_MODE_PATH) uninstall

app-$(CONFIG_APP_EVENT_DAEMON) += event_daemon
PHONY += event_daemon event_daemon-clean event_daemon-distclean
PHONY += event_daemon-install event_daemon-uninstall
event_daemon: libpwm libtz
	$(Q)$(MAKE) -C $(EVENTD_PATH) all

event_daemon-clean: libtz-clean libpwm-clean
	$(Q)$(MAKE) -C $(EVENTD_PATH) clean

event_daemon-distclean: libtz-distclean libpwm-distclean
	$(Q)$(MAKE) -C $(EVENTD_PATH) distclean

event_daemon-install: libpwm-install libtz-install
	$(Q)$(MAKE) -C $(EVENTD_PATH) install

event_daemon-uninstall: libtz-uninstall libpwm-uninstall
	$(Q)$(MAKE) -C $(EVENTD_PATH) uninstall

app-$(CONFIG_APP_GPIO_DAEMONS) += gpio_daemons
PHONY += gpio_daemons gpio_daemons-clean gpio_daemons-distclean
PHONY += gpio_daemons-install gpio_daemons-uninstall
gpio_daemons:
	$(Q)$(MAKE) -C $(GPIO_DAEMONS_PATH) all

gpio_daemons-clean:
	$(Q)$(MAKE) -C $(GPIO_DAEMONS_PATH) clean

gpio_daemons-distclean:
	$(Q)$(MAKE) -C $(GPIO_DAEMONS_PATH) distclean

gpio_daemons-install:
	$(Q)$(MAKE) -C $(GPIO_DAEMONS_PATH) install

gpio_daemons-uninstall:
	$(Q)$(MAKE) -C $(GPIO_DAEMONS_PATH) uninstall

app-$(CONFIG_APP_NRS) += nrs
PHONY += nrs nrs-clean nrs-distclean
PHONY += nrs-install nrs-uninstall
nrs:
	$(Q)$(MAKE) -C $(NRS_BUILD) all

nrs-clean:
	$(Q)$(MAKE) -C $(NRS_BUILD) clean

nrs-distclean:
	$(Q)$(MAKE) -C $(NRS_BUILD) distclean

nrs-install:
	$(Q)$(MAKE) -C $(NRS_BUILD) install

nrs-uninstall:
	$(Q)$(MAKE) -C $(NRS_BUILD) uninstall

app-$(CONFIG_APP_OTP) += otp
PHONY += otp otp-clean otp-distclean
PHONY += otp-install otp-uninstall
otp:
	$(Q)$(MAKE) -C $(OTP_BUILD) all

otp-clean:
	$(Q)$(MAKE) -C $(OTP_BUILD) clean

otp-distclean:
	$(Q)$(MAKE) -C $(OTP_BUILD) distclean

otp-install:
	$(Q)$(MAKE) -C $(OTP_BUILD) install

otp-uninstall:
	$(Q)$(MAKE) -C $(OTP_BUILD) uninstall

app-$(CONFIG_APP_SNTP) += sntp
PHONY += sntp sntp-clean sntp-distclean
PHONY += sntp-install sntp-uninstall
sntp:
	$(Q)$(MAKE) -C $(SNTP_PATH) all

sntp-clean:
	$(Q)$(MAKE) -C $(SNTP_PATH) clean

sntp-distclean:
	$(Q)$(MAKE) -C $(SNTP_PATH) distclean

sntp-install:
	$(Q)$(MAKE) -C $(SNTP_PATH) install

sntp-uninstall:
	$(Q)$(MAKE) -C $(SNTP_PATH) uninstall

app-$(CONFIG_APP_SYSUPD_TOOLS) += sysupd_tools
PHONY += sysupd_tools sysupd_tools-clean sysupd_tools-distclean
PHONY += sysupd_tools-install sysupd_tools-uninstall
sysupd_tools:
	$(Q)$(MAKE) -C $(SYSUPD_TOOLS_PATH) all

sysupd_tools-clean:
	$(Q)$(MAKE) -C $(SYSUPD_TOOLS_PATH) clean

sysupd_tools-distclean:
	$(Q)$(MAKE) -C $(SYSUPD_TOOLS_PATH) distclean

sysupd_tools-install:
	$(Q)$(MAKE) -C $(SYSUPD_TOOLS_PATH) install

sysupd_tools-uninstall:
	$(Q)$(MAKE) -C $(SYSUPD_TOOLS_PATH) uninstall

app-$(CONFIG_APP_THERMAL_PROTECTION) += thermal_protection
PHONY += thermal_protection thermal_protection-clean
PHONY += thermal_protection-distclean
PHONY += thermal_protection-install thermal_protection-uninstall
thermal_protection:
	$(Q)$(MAKE) -C $(THERMAL_PROTECTION_PATH) all

thermal_protection-clean:
	$(Q)$(MAKE) -C $(THERMAL_PROTECTION_PATH) clean

thermal_protection-distclean:
	$(Q)$(MAKE) -C $(THERMAL_PROTECTION_PATH) distclean

thermal_protection-install:
	$(Q)$(MAKE) -C $(THERMAL_PROTECTION_PATH) install

thermal_protection-uninstall:
	$(Q)$(MAKE) -C $(THERMAL_PROTECTION_PATH) uninstall

app-$(CONFIG_APP_UVC) += uvc
PHONY += uvc uvc-clean uvc-distclean
PHONY += uvc-install uvc-uninstall
uvc: libcm
	$(Q)$(MAKE) -C $(UVC_PATH) all

uvc-clean: libcm-clean
	$(Q)$(MAKE) -C $(UVC_PATH) clean

uvc-distclean: libcm-distclean
	$(Q)$(MAKE) -C $(UVC_PATH) distclean

uvc-install: libcm-install
	$(Q)$(MAKE) -C $(UVC_PATH) install

uvc-uninstall: libcm-uninstall
	$(Q)$(MAKE) -C $(UVC_PATH) uninstall

app-$(CONFIG_APP_REALTIME_DBG) += realtime_dbg
PHONY += realtime_dbg realtime_dbg-clean realtime_dbg-distclean
PHONY += realtime_dbg-install realtime_dbg-uninstall
realtime_dbg: 
	$(Q)$(MAKE) -C $(REALTIME_DBG_PATH) all

realtime_dbg-clean: 
	$(Q)$(MAKE) -C $(REALTIME_DBG_PATH) clean

realtime_dbg-distclean:
	$(Q)$(MAKE) -C $(REALTIME_DBG_PATH) distclean

realtime_dbg-install:
	$(Q)$(MAKE) -C $(REALTIME_DBG_PATH) install

realtime_dbg-uninstall:
	$(Q)$(MAKE) -C $(REALTIME_DBG_PATH) uninstall

app-$(CONFIG_APP_WATCHDOG) += watchdog
PHONY += watchdog watchdog-clean watchdog-distclean
PHONY += watchdog-install watchdog-uninstall
watchdog:
	$(Q)$(MAKE) -C $(WATCHDOG_PATH) all

watchdog-clean:
	$(Q)$(MAKE) -C $(WATCHDOG_PATH) clean

watchdog-distclean:
	$(Q)$(MAKE) -C $(WATCHDOG_PATH) distclean

watchdog-install:
	$(Q)$(MAKE) -C $(WATCHDOG_PATH) install

watchdog-uninstall:
	$(Q)$(MAKE) -C $(WATCHDOG_PATH) uninstall

app-$(CONFIG_APP_SYSTEM_LEVEL_TEST) += system_level_test
PHONY += system_level_test system_level_test-clean system_level_test-distclean
PHONY += system_level_test-install system_level_test-uninstall
system_level_test:
	$(Q)$(MAKE) -C $(SYSTEM_LEVEL_TEST_PATH) all

system_level_test-clean:
	$(Q)$(MAKE) -C $(SYSTEM_LEVEL_TEST_PATH) clean

system_level_test-distclean:
	$(Q)$(MAKE) -C $(SYSTEM_LEVEL_TEST_PATH) distclean

system_level_test-install:
	$(Q)$(MAKE) -C $(SYSTEM_LEVEL_TEST_PATH) install

system_level_test-uninstall:
	$(Q)$(MAKE) -C $(SYSTEM_LEVEL_TEST_PATH) uninstall

app-y += fw_env
PHONY += fw_env fw_env-clean fw_env-distclean
PHONY += fw_env-install fw_env-uninstall
fw_env:
	$(Q)$(MAKE) -C $(FW_ENV_PATH) all

fw_env-clean:
	$(Q)$(MAKE) -C $(FW_ENV_PATH) clean

fw_env-distclean:
	$(Q)$(MAKE) -C $(FW_ENV_PATH) distclean

fw_env-install:
	$(Q)$(MAKE) -C $(FW_ENV_PATH) install

fw_env-uninstall:
	$(Q)$(MAKE) -C $(FW_ENV_PATH) uninstall

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
