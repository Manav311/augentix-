mod := $(notdir $(subdir))

app-$(CONFIG_APP_AV_MAIN2) += av_main2
PHONY += av_main2 av_main2-clean av_main2-distclean
PHONY += av_main2-install av_main2-uninstall
av_main2: libado libavftr libcm libeaif libinf libsql libtz
	$(Q)$(MAKE) -C $(AVMAIN2_PATH) all

av_main2-clean: libtz-clean libsql-clean libinf-clean \
	libeaif-clean libcm-clean libavftr-clean libado-clean
	$(Q)$(MAKE) -C $(AVMAIN2_PATH) clean

av_main2-distclean: libtz-distclean libsql-distclean \
	libinf-distclean libeaif-distclean libcm-distclean \
	libavftr-distclean libado-distclean
	$(Q)$(MAKE) -C $(AVMAIN2_PATH) distclean

av_main2-install: libado-install libavftr-install libcm-install \
	libeaif-install libinf-install libsql-install \
	libtz-install
	$(Q)$(MAKE) -C $(AVMAIN2_PATH) install

av_main2-uninstall: libtz-uninstall libsql-uninstall \
	libinf-uninstall libeaif-uninstall libcm-uninstall \
	libavftr-uninstall libado-uninstall
	$(Q)$(MAKE) -C $(AVMAIN2_PATH) uninstall

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
