mod := $(notdir $(subdir))

# 3rd-party paths
TUYA_SERVICE_PATH := $(APP_PATH)/3rd_party/tuya_service
TUYA_IPC_SDK_DIR := $(CONFIG_APP_TUYA_IPC_SDK_PATH)
export TUYA_IPC_SDK_DIR

app-$(CONFIG_APP_TUYA_SERVICE) += tuya_service
PHONY += tuya_service tuya_service-clean tuya_service-distclean
PHONY += tuya_service-install tuya_service-uninstall
tuya_service: libado libavftr libeaif libcm
	$(Q)$(MAKE) -C $(EVENTD_PATH)/libledevt all
	$(Q)$(MAKE) -C $(TUYA_SERVICE_PATH) all

tuya_service-clean: libcm-clean \
	libavftr-clean libado-clean libeaif-clean
	$(Q)$(MAKE) -C $(TUYA_SERVICE_PATH) clean
	$(Q)$(MAKE) -C $(EVENTD_PATH)/libledevt clean

tuya_service-distclean: libcm-distclean \
	libavftr-distclean libado-distclean libeaif-distclean
	$(Q)$(MAKE) -C $(TUYA_SERVICE_PATH) distclean
	$(Q)$(MAKE) -C $(EVENTD_PATH)/libledevt distclean

tuya_service-install: libado-install libavftr-install libeaif-install \
	libcm-install
	$(Q)$(MAKE) -C $(TUYA_SERVICE_PATH) install

tuya_service-uninstall: libcm-uninstall libeaif-uninstall \
	libavftr-uninstall libado-uninstall
	$(Q)$(MAKE) -C $(TUYA_SERVICE_PATH) uninstall

TUTK_SERVICE_PATH := $(APP_PATH)/3rd_party/tutk_service
TUTK_SDK_DIR := $(CONFIG_APP_TUTK_SDK_PATH)
export TUTK_SDK_DIR

app-$(CONFIG_APP_TUTK_SERVICE) += tutk_service
PHONY += tutk_service tutk_service-clean tutk_service-distclean
PHONY += tutk_service-install tutk_service-uninstall
tutk_service: libado libavftr libeaif libcm
	$(Q)$(MAKE) -C $(TUTK_SERVICE_PATH)/build all

tutk_service-clean: libcm-clean \
	libavftr-clean libado-clean libeaif-clean
	$(Q)$(MAKE) -C $(TUTK_SERVICE_PATH)/build clean

tutk_service-distclean: libcm-distclean \
	libavftr-distclean libado-distclean libeaif-distclean
	$(Q)$(MAKE) -C $(TUTK_SERVICE_PATH)/build distclean

tutk_service-install: libado-install libavftr-install libeaif-install \
	libcm-install
	$(Q)$(MAKE) -C $(TUTK_SERVICE_PATH)/build install

tutk_service-uninstall: libcm-uninstall libeaif-uninstall \
	libavftr-uninstall libado-uninstall
	$(Q)$(MAKE) -C $(TUTK_SERVICE_PATH)/build uninstall


TANGE_CLOUD_PATH := $(APP_PATH)/3rd_party/tange_cloud

app-$(CONFIG_APP_TANGE_CLOUD) += tange_cloud
PHONY += tange_cloud tange_cloud-clean tange_cloud-distclean
PHONY += tange_cloud-install tange_cloud-uninstall
tange_cloud:
	$(Q)$(MAKE) -C $(TANGE_CLOUD_PATH)/build all

tange_cloud-clean:
	$(Q)$(MAKE) -C $(TANGE_CLOUD_PATH)/build clean

tange_cloud-distclean:
	$(Q)$(MAKE) -C $(TANGE_CLOUD_PATH)/build distclean

tange_cloud-install:
	$(Q)$(MAKE) -C $(TANGE_CLOUD_PATH)/build install

tange_cloud-uninstall:
	$(Q)$(MAKE) -C $(TANGE_CLOUD_PATH)/build uninstall

AMAZON_KVS_PATH := $(APP_PATH)/3rd_party/amazon_kvs

app-$(CONFIG_APP_AMAZON_KVS) += amazon_kvs
PHONY += amazon_kvs amazon_kvs-clean amazon_kvs-distclean
PHONY += amazon_kvs-install amazon_kvs-uninstall
amazon_kvs:
	$(Q)$(MAKE) -C $(AMAZON_KVS_PATH) all

amazon_kvs-clean:
	$(Q)$(MAKE) -C $(AMAZON_KVS_PATH) clean

amazon_kvs-distclean:
	$(Q)$(MAKE) -C $(AMAZON_KVS_PATH) distclean

amazon_kvs-install:
	$(Q)$(MAKE) -C $(AMAZON_KVS_PATH) install

amazon_kvs-uninstall:
	$(Q)$(MAKE) -C $(AMAZON_KVS_PATH) uninstall

SE_TRNG_PATH := $(APP_PATH)/3rd_party/se_trng/build

app-$(CONFIG_APP_SE_TRNG_PROVIDER) += se_trng
PHONY += se_trng se_trng-clean se_trng-distclean
PHONY += se_trng-install se_trng-uninstall
se_trng:
	$(Q)$(MAKE) -C $(SE_TRNG_PATH) all

se_trng-clean:
	$(Q)$(MAKE) -C $(SE_TRNG_PATH) clean

se_trng-distclean:
	$(Q)$(MAKE) -C $(SE_TRNG_PATH) distclean

se_trng-install:
	$(Q)$(MAKE) -C $(SE_TRNG_PATH) install

se_trng-uninstall:
	$(Q)$(MAKE) -C $(SE_TRNG_PATH) uninstall

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
