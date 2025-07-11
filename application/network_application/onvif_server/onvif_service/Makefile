SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk
GSOAP=../gsoap
GSOAP_INC=$(GSOAP)/include

ifeq ($(CONFIG_PROD), MT800_1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), HC1782_EM1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), HC1892_EM1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), GT804_1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), GT804_2)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), GT804_3)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), GT804_4)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), MT800_2)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), MT801_1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), GS901_1)
	PROD_NAME = $(CONFIG_PROD)
else ifeq ($(CONFIG_PROD), MS701_1)
	PROD_NAME = MT800_1
else
	PROD_NAME = MT801_1
endif

export PROD_NAME

TARGETS = fcgi spawn-fcgi
TARGETS_INSTALL := $(addsuffix -install, $(TARGETS))
TARGETS_CLEAN :=  $(addsuffix -clean, $(TARGETS))
INC := -I$(GSOAP_INC) -isystem$(JSON_INC) -I$(APP_INC) -I$(LIBCM_INC) -I$(LIBTZ_INC) \
       -I$(GSOAP)/plugin -isystem$(OPENSSL_INC)
LDFLAGS := -L$(LIBCM_PATH) -L$(GSOAP) -L$(APP_LIB) -L$(OPENSSL_LIB) -L$(LIBTZ_PATH) -L$(JSON_LIB)
LDLIBS := -pthread -lm -lcm -lcrypto -lssl -lgsoap -ltz -lstdc++ -ljson-c
export AR := $(CROSS_COMPILE)ar
export CC := $(CROSS_COMPILE)gcc
export CFLAGS += -Wno-unused-parameter -funwind-tables -rdynamic $(INC)
export CPPFLAGS += -D_GNU_SOURCE -D$(PROD_NAME)

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)
BIN=onvif_server

TARGETS = $(BIN)
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))


.PHONY: default
default: all

.PHONY: all
all: lib onvif_server

.PHONY: lib
lib:
	$(MAKE) -C $(GSOAP) all

onvif_server: lib $(OBJS)
	$(Q)$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: clean
clean:
	$(MAKE) -C $(GSOAP) clean
	$(Q)rm -f $(OBJS) $(BIN)

.PHONY: distclean
distclean: uninstall clean
