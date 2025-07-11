mod := $(notdir $(subdir))

app-$(CONFIG_LIBADO) += libado
PHONY += libado libado-clean libado-distclean
PHONY += libado-install libado-uninstall
libado:
	$(Q)$(MAKE) -C $(LIBADO_PATH) all

libado-clean:
	$(Q)$(MAKE) -C $(LIBADO_PATH) clean

libado-distclean:
	$(Q)$(MAKE) -C $(LIBADO_PATH) distclean

libado-install:
	$(Q)$(MAKE) -C $(LIBADO_PATH) install

libado-uninstall:
	$(Q)$(MAKE) -C $(LIBADO_PATH) uninstall

app-$(CONFIG_LIBAGTX) += libagtx
PHONY += libagtx libagtx-clean libagtx-distclean
PHONY += libagtx-install libagtx-uninstall
libagtx:
	$(Q)$(MAKE) -C $(LIBAGTX_PATH)/build all

libagtx-clean:
	$(Q)$(MAKE) -C $(LIBAGTX_PATH)/build clean

libagtx-distclean:
	$(Q)$(MAKE) -C $(LIBAGTX_PATH)/build distclean

libagtx-install:
	$(Q)$(MAKE) -C $(LIBAGTX_PATH)/build install

libagtx-uninstall:
	$(Q)$(MAKE) -C $(LIBAGTX_PATH)/build uninstall

app-$(CONFIG_LIBMETAL) += libmetal
PHONY += libmetal libmetal-clean libmetal-distclean
PHONY += libmetal-install libmetal-uninstall
libmetal:
	$(MAKE) -C $(LIBMETAL_BUILD_PATH) all

libmetal-clean:
	$(MAKE) -C $(LIBMETAL_BUILD_PATH) clean

libmetal-distclean:
	$(MAKE) -C $(LIBMETAL_BUILD_PATH) distclean

libmetal-install:
	$(MAKE) -C $(LIBMETAL_BUILD_PATH) install

libmetal-uninstall:
	$(MAKE) -C $(LIBMETAL_BUILD_PATH) uninstall

app-$(CONFIG_LIBAMPC) += libopen_amp
PHONY += libopen_amp libopen_amp-clean libopen_amp-distclean
PHONY += libopen_amp-install libopen_amp-uninstall
libopen_amp:
	$(MAKE) -C $(LIBOPENAMP_BUILD_PATH) all

libopen_amp-clean:
	$(MAKE) -C $(LIBOPENAMP_BUILD_PATH) clean

libopen_amp-distclean:
	$(MAKE) -C $(LIBOPENAMP_BUILD_PATH) distclean

libopen_amp-install:
	$(MAKE) -C $(LIBOPENAMP_BUILD_PATH) install

libopen_amp-uninstall:
	$(MAKE) -C $(LIBOPENAMP_BUILD_PATH) uninstall

app-$(CONFIG_LIBAMPC) += libampc
PHONY += libampc libampc-clean libampc-distclean
PHONY += libampc-install libampc-uninstall
libampc:
	$(MAKE) -C $(LIBAMPC_BUILD_PATH) all

libampc-clean:
	$(MAKE) -C $(LIBAMPC_BUILD_PATH) clean

libampc-distclean:
	$(MAKE) -C $(LIBAMPC_BUILD_PATH) distclean

libampc-install:
	$(MAKE) -C $(LIBAMPC_BUILD_PATH) install

libampc-uninstall:
	$(MAKE) -C $(LIBAMPC_BUILD_PATH) uninstall

app-$(CONFIG_LIBLPW) += liblpw
PHONY += liblpw liblpw-clean liblpw-distclean
PHONY += liblpw-install liblpw-uninstall
liblpw:
	$(MAKE) -C $(LIBLPW_BUILD_PATH) all

liblpw-clean:
	$(MAKE) -C $(LIBLPW_BUILD_PATH) clean

liblpw-distclean:
	$(MAKE) -C $(LIBLPW_BUILD_PATH) distclean

liblpw-install:
	$(MAKE) -C $(LIBLPW_BUILD_PATH) install

liblpw-uninstall:
	$(MAKE) -C $(LIBLPW_BUILD_PATH) uninstall

app-$(CONFIG_LIBAVFTR) += libavftr
PHONY += libavftr libavftr-clean libavftr-distclean
PHONY += libavftr-install libavftr-uninstall
libavftr: libeaif
	$(MAKE) -C $(LIBAVFTR_BUILD_PATH) all

libavftr-clean: libeaif-clean
	$(MAKE) -C $(LIBAVFTR_BUILD_PATH) clean

libavftr-distclean: libeaif-distclean
	$(MAKE) -C $(LIBAVFTR_BUILD_PATH) distclean

libavftr-install: libeaif-install
	$(MAKE) -C $(LIBAVFTR_BUILD_PATH) install

libavftr-uninstall: libeaif-uninstall
	$(MAKE) -C $(LIBAVFTR_BUILD_PATH) uninstall

app-$(CONFIG_LIBCM) += libcm
PHONY += libcm libcm-clean libcm-distclean
PHONY += libcm-install libcm-uninstall
libcm:
	$(Q)$(MAKE) -C $(LIBCM_PATH) all

libcm-clean:
	$(Q)$(MAKE) -C $(LIBCM_PATH) clean

libcm-distclean:
	$(Q)$(MAKE) -C $(LIBCM_PATH) distclean

libcm-install:
	$(Q)$(MAKE) -C $(LIBCM_PATH) install

libcm-uninstall:
	$(Q)$(MAKE) -C $(LIBCM_PATH) uninstall

app-$(CONFIG_FOO) += libfoo
PHONY += libfoo libfoo-clean libfoo-distclean
PHONY += libfoo-install libfoo-uninstall
libfoo:
	$(Q)$(MAKE) -C $(LIBFOO_PATH)/build all

libfoo-clean:
	$(Q)$(MAKE) -C $(LIBFOO_PATH)/build clean

libfoo-distclean:
	$(Q)$(MAKE) -C $(LIBFOO_PATH)/build distclean

libfoo-install:
	$(Q)$(MAKE) -C $(LIBFOO_PATH)/build install

libfoo-uninstall:
	$(Q)$(MAKE) -C $(LIBFOO_PATH)/build uninstall

app-$(CONFIG_LIBFSINK) += libfsink
PHONY += libfsink libfsink-clean libfsink-distclean
PHONY += libfsink-install libfsink-uninstall
libfsink:
	$(Q)$(MAKE) -C $(FILE_PATH) all
	$(Q)$(MAKE) -C $(UDPS_PATH) all

libfsink-clean:
	$(Q)$(MAKE) -C $(UDPS_PATH) clean
	$(Q)$(MAKE) -C $(FILE_PATH) clean

libfsink-distclean:
	$(Q)$(MAKE) -C $(UDPS_PATH) distclean
	$(Q)$(MAKE) -C $(FILE_PATH) distclean

libfsink-install:
	$(Q)$(MAKE) -C $(FILE_PATH) install
	$(Q)$(MAKE) -C $(UDPS_PATH) install

libfsink-uninstall:
	$(Q)$(MAKE) -C $(UDPS_PATH) uninstall
	$(Q)$(MAKE) -C $(FILE_PATH) uninstall

app-$(CONFIG_LIBGPIO) += libgpio
PHONY += libgpio libgpio-clean libgpio-distclean
PHONY += libgpio-install libgpio-uninstall
libgpio:
	$(Q)$(MAKE) -C $(LIBGPIO_PATH) all

libgpio-clean:
	$(Q)$(MAKE) -C $(LIBGPIO_PATH) clean

libgpio-distclean:
	$(Q)$(MAKE) -C $(LIBGPIO_PATH) distclean

libgpio-install:
	$(Q)$(MAKE) -C $(LIBGPIO_PATH) install

libgpio-uninstall:
	$(Q)$(MAKE) -C $(LIBGPIO_PATH) uninstall

app-$(CONFIG_LIBOSD) += libosd
PHONY += libosd libosd-clean libosd-distclean
PHONY += libosd-install libosd-uninstall
libosd:
	$(Q)$(MAKE) -C $(LIBOSD_PATH)/build all

libosd-clean:
	$(Q)$(MAKE) -C $(LIBOSD_PATH)/build clean

libosd-distclean:
	$(Q)$(MAKE) -C $(LIBOSD_PATH)/build distclean

libosd-install:
	$(Q)$(MAKE) -C $(LIBOSD_PATH)/build install

libosd-uninstall:
	$(Q)$(MAKE) -C $(LIBOSD_PATH)/build uninstall



app-$(CONFIG_LIBUTILS) += libutils
PHONY += libutils libutils-clean libutils-distclean
PHONY += libutils-install libutils-uninstall
libutils:
	$(Q)$(MAKE) -C $(LIBUTILS_PATH)/build all

libutils-clean:
	$(Q)$(MAKE) -C $(LIBUTILS_PATH)/build clean

libutils-distclean:
	$(Q)$(MAKE) -C $(LIBUTILS_PATH)/build clean

libutils-install:
	$(Q)$(MAKE) -C $(LIBUTILS_PATH)/build install

libutils-uninstall:
	$(Q)$(MAKE) -C $(LIBUTILS_PATH)/build uninstall

PHONY += libpwm libpwm-clean libpwm-distclean
PHONY += libpwm-install libpwm-uninstall
app-$(CONFIG_LIBPWM) += libpwm
libpwm:
	$(Q)$(MAKE) -C $(LIBPWM_PATH) all

libpwm-clean:
	$(Q)$(MAKE) -C $(LIBPWM_PATH) clean

libpwm-distclean:
	$(Q)$(MAKE) -C $(LIBPWM_PATH) distclean

libpwm-install:
	$(Q)$(MAKE) -C $(LIBPWM_PATH) install

libpwm-uninstall:
	$(Q)$(MAKE) -C $(LIBPWM_PATH) uninstall

PHONY += libmotor libmotor-clean libmotor-distclean
PHONY += libmotor-install libmotor-uninstall
app-$(CONFIG_LIBMOTOR) += libmotor
libmotor:
	$(Q)$(MAKE) -C $(LIBMOTOR_PATH)/build all

libmotor-clean:
	$(Q)$(MAKE) -C $(LIBMOTOR_PATH)/build clean

libmotor-distclean:
	$(Q)$(MAKE) -C $(LIBMOTOR_PATH)/build distclean

libmotor-install:
	$(Q)$(MAKE) -C $(LIBMOTOR_PATH)/build install

libmotor-uninstall:
	$(Q)$(MAKE) -C $(LIBMOTOR_PATH)/build uninstall

PHONY += libsample libsample-clean libsample-distclean
PHONY += libsample-install libsample-uninstall
app-$(CONFIG_LIBSAMPLE) += libsample
libsample:
	$(Q)$(MAKE) -C $(LIBSAMPLE_PATH)/build all

libsample-clean:
	$(Q)$(MAKE) -C $(LIBSAMPLE_PATH)/build clean

libsample-distclean:
	$(Q)$(MAKE) -C $(LIBSAMPLE_PATH)/build distclean

libsample-install:
	$(Q)$(MAKE) -C $(LIBSAMPLE_PATH)/build install

libsample-uninstall:
	$(Q)$(MAKE) -C $(LIBSAMPLE_PATH)/build uninstall

app-$(CONFIG_LIBSQL) += libsql
PHONY += libsql libsql-clean libsql-distclean
PHONY += libsql-install libsql-uninstall
libsql:
	$(Q)$(MAKE) -C $(LIBSQL_PATH) all

libsql-clean:
	$(Q)$(MAKE) -C $(LIBSQL_PATH) clean

libsql-distclean:
	$(Q)$(MAKE) -C $(LIBSQL_PATH) distclean

libsql-install:
	$(Q)$(MAKE) -C $(LIBSQL_PATH) install

libsql-uninstall:
	$(Q)$(MAKE) -C $(LIBSQL_PATH) uninstall

app-$(CONFIG_LIBTZ) += libtz
PHONY += libtz libtz-clean libtz-distclean
PHONY += libtz-install libtz-uninstall
libtz:
	$(Q)$(MAKE) -C $(LIBTZ_PATH) all

libtz-clean:
	$(Q)$(MAKE) -C $(LIBTZ_PATH) clean

libtz-distclean:
	$(Q)$(MAKE) -C $(LIBTZ_PATH) distclean

libtz-install:
	$(Q)$(MAKE) -C $(LIBTZ_PATH) install

libtz-uninstall:
	$(Q)$(MAKE) -C $(LIBTZ_PATH) uninstall

app-$(CONFIG_LIBEAIF) += libeaif
PHONY += libeaif libeaif-clean libeaif-distclean
PHONY += libeaif-install libeaif-uninstall
libeaif: libinf
	$(Q)$(MAKE) -C $(LIBEAIF_PATH)/build all

libeaif-clean: libinf-clean
	$(Q)$(MAKE) -C $(LIBEAIF_PATH)/build clean

libeaif-install: libinf-install
	$(Q)$(MAKE) -C $(LIBEAIF_PATH)/build install

libeaif-uninstall: libinf-uninstall
	$(Q)$(MAKE) -C $(LIBEAIF_PATH)/build uninstall

libeaif-distclean: libinf-distclean
	$(Q)$(MAKE) -C $(LIBEAIF_PATH)/build distclean

app-$(CONFIG_LIBINF) += libinf
PHONY += libinf libinf-clean libinf-distclean
PHONY += libinf-install libinf-uninstall

libinf:
	$(Q)$(MAKE) -C $(LIBINF_PATH)/build all

libinf-clean:
	$(Q)$(MAKE) -C $(LIBINF_PATH)/build clean

libinf-install:
	$(Q)$(MAKE) -C $(LIBINF_PATH)/build install

libinf-uninstall:
	$(Q)$(MAKE) -C $(LIBINF_PATH)/build uninstall

libinf-distclean:
	$(Q)$(MAKE) -C $(LIBINF_PATH)/build distclean

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
