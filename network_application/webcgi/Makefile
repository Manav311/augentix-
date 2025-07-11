SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

ifndef CONFIG_PROD
CONFIG_PROD := AGT800_20
endif

PROD_NAME := $(CONFIG_PROD)
export PROD_NAME

# install
INSTALL := install -c
INSTALL_PROGRAM := $(INSTALL)

# utils
CP := cp
MKDIR := mkdir
MKDIR_P := $(MKDIR) -p
RM := rm -f
SED := sed
MV := mv
RS := rsync -av

# toolchain
CC := $(CROSS_COMPILE)gcc

# paths
root := $(CURDIR)
FCGI_BIN_DIR := $(root)/bin
FCGI_DEP_DIR := $(root)/.deps
FCGI_OBJ_DIR := $(root)/obj
FCGI_SH_DIR := $(root)/scripts
FCGI_SRC_DIR := $(root)/src
TARGET_DIR := $(root)/www/cgi-bin
TARGET_NGINX_DIR := $(SYSROOT)/etc/nginx
TARGET_WWW_DIR := $(SYSTEMFS)/www
TARGET_DEMO_DIR := $(TARGET_WWW_DIR)/demo

# compiler/linker flags
INC := -isystem$(LIBFCGI_INC) -isystem$(JSON_INC)
CFLAGS += $(INC)
DEPFLAGS = -MT $@ -MMD -MP -MF "$(FCGI_DEP_DIR)/$*.d"
LDFLAGS := -L$(LIBFCGI_LIB) -L$(JSON_LIB)
LDLIBS := -lfcgi -ljson-c

BINS := msg.cgi web.cgi

ifeq ($(CONFIG_SECURE_WEB_PAGE), y)
SCRIPTS := passwd_secure.sh setToDefault_secure.sh 
else
SCRIPTS := passwd.sh setToDefault.sh 
endif
SCRIPTS += time.sh setDNS.sh setGateway.sh setIP.sh
SCRIPTS += setMask.sh SntpConf.sh
SCRIPTS += DSTConf.sh TimeSwitch.sh updateCA.sh resetCA.sh
SCRIPTS += removeFile.sh validateFaceModel.sh registerFaceModel.sh
SCRIPTS += unregisterFaceModel.sh setWPA.sh packNginxLog.sh
ifeq ($(CONFIG_SECURE_WEB_PAGE), y)
SCRIPTS += installCrt.sh 
ifeq ($(CONFIG_SE_SQ7131S), y)
SCRIPTS += encryptedDeviceSecret_sq7131s.sh
else
SCRIPTS += encryptedDeviceSecret.sh
endif
endif
SRCS := $(wildcard $(FCGI_SRC_DIR)/*.c)
DEPS := $(patsubst $(FCGI_SRC_DIR)/%.c,$(FCGI_DEP_DIR)/%.d,$(SRCS))

INSTALL_BIN_TRGT := $(addprefix $(TARGET_DIR)/,$(BINS))
INSTALL_SH_TRGT := $(addprefix $(TARGET_DIR)/,$(SCRIPTS))

.DEFAULT_GOAL := default

ifeq ($(CONFIG_SECURE_WEB_PAGE), y)
CFLAGS += -DSECURE_WEB_PAGE
endif

.PHONY: default
default: all

.PHONY: all
all: $(addprefix $(FCGI_BIN_DIR)/,$(BINS))

.PHONY: clean
clean: clean-fcgi

.PHONY: clean-fcgi
clean-fcgi:
	$(Q)$(RM) -r $(FCGI_BIN_DIR) $(FCGI_DEP_DIR) $(FCGI_OBJ_DIR)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all install-fcgi install-www

.PHONY: install-fcgi
install-fcgi: install-bin install-sh

.PHONY: install-bin
install-bin: $(TARGET_DIR) $(INSTALL_BIN_TRGT)

.PHONY: install-sh
install-sh: $(TARGET_DIR) $(INSTALL_SH_TRGT)

.PHONY: install-www
install-www:
	$(Q)$(MKDIR_P) $(TARGET_WWW_DIR)
	$(Q)$(MKDIR_P) $(TARGET_NGINX_DIR)
ifneq ($(CONFIG_SECURE_WEB_PAGE), y)
	$(Q)$(RS) --exclude='www_secure' $(root)/www/. $(TARGET_WWW_DIR)
	$(Q)$(RS) --exclude='nginx.conf.secure' $(root)/webserver/. $(TARGET_NGINX_DIR)
	$(Q)$(RS) --exclude='agtx_web.conf.secure' $(root)/webserver/. $(TARGET_NGINX_DIR)
	$(Q)$(RS) --exclude='.htpasswd.secure' $(root)/webserver/. $(TARGET_NGINX_DIR)
else
	$(Q)$(RS) --exclude='/html' --exclude='/img' --exclude='/js' $(root)/www/. $(TARGET_WWW_DIR)
	$(Q)$(MV) $(TARGET_WWW_DIR)/www_secure/* $(TARGET_WWW_DIR)/.
	$(Q)$(RM) -r $(TARGET_WWW_DIR)/www_secure
	$(Q)$(RS) --exclude='nginx.conf' $(root)/webserver/. $(TARGET_NGINX_DIR)
	$(Q)$(RS) --exclude='.htpasswd' $(root)/webserver/. $(TARGET_NGINX_DIR)
	$(Q)$(MV) $(TARGET_NGINX_DIR)/nginx.conf.secure $(TARGET_NGINX_DIR)/nginx.conf
	$(Q)$(MV) $(TARGET_NGINX_DIR)/agtx_web.conf.secure $(TARGET_NGINX_DIR)/agtx_web.conf
	$(Q)$(MV) $(TARGET_NGINX_DIR)/.htpasswd.secure $(TARGET_NGINX_DIR)/.htpasswd
endif # CONFIG_SECURE_WEB_PAGE
ifeq ($(CONFIG_APP_WEBCGI_DEMO),y)
	$(Q)$(MKDIR_P) $(TARGET_DEMO_DIR)
	$(Q)$(CP) -a $(root)/demo/. $(TARGET_DEMO_DIR)
endif
	$(Q)$(SED) -i -e 's/PROD_NAME/$(CONFIG_PROD)/g' $(TARGET_WWW_DIR)/html/system.html
	@echo "PROD_NAME: $(PROD_NAME)"

.PHONY: uninstall
uninstall: uninstall-fcgi uninstall-www
	$(Q)$(RM) -r $(TARGET_DIR)

.PHONY: uninstall-fcgi
uninstall-fcgi: uninstall-bin uninstall-sh

.PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(INSTALL_BIN_TRGT)

.PHONY: uninstall-sh
uninstall-sh:
	$(Q)$(RM) $(INSTALL_SH_TRGT)

.PHONY: uninstall-www
uninstall-www:
	$(Q)$(RM) -r $(TARGET_WWW_DIR)

# cgi-bin
$(FCGI_BIN_DIR)/msg.cgi: $(FCGI_OBJ_DIR)/msg.o
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)
$(FCGI_BIN_DIR)/web.cgi: $(FCGI_OBJ_DIR)/web.o $(FCGI_OBJ_DIR)/fcgi.o $(FCGI_OBJ_DIR)/utils.o
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(FCGI_BIN_DIR)/%.exe: $(FCGI_OBJ_DIR)/%.o
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LDLIBS)

# objects
$(FCGI_OBJ_DIR)/%.o: $(FCGI_SRC_DIR)/%.c $(FCGI_DEP_DIR)/%.d $(FCGI_BIN_DIR) $(FCGI_DEP_DIR) $(FCGI_OBJ_DIR)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
.PRECIOUS: $(FCGI_OBJ_DIR)/%.o

# install targets
$(TARGET_DIR)/%.exe: $(FCGI_BIN_DIR)/%.exe
	$(Q)$(INSTALL_PROGRAM) $< $@

$(TARGET_DIR)/%.cgi: $(FCGI_BIN_DIR)/%.cgi
	$(Q)$(INSTALL_PROGRAM) $< $@

$(FCGI_BIN_DIR) $(FCGI_DEP_DIR) $(FCGI_OBJ_DIR) $(TARGET_DIR):
	$(Q)$(INSTALL) -d $@

$(TARGET_DIR)/%.sh: $(FCGI_SH_DIR)/%.sh
	$(Q)$(INSTALL_PROGRAM) $< $@

# dependencies
$(FCGI_DEP_DIR)/%.d: ;
.PRECIOUS: $(FCGI_DEP_DIR)/%.d

-include $(DEPS)
