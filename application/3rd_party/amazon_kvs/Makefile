SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
DEBUG := 0
OPTIMIZE := 0
WARNING := 0
include $(SDKSRC_DIR)/application/internal.mk

INSTALL := install -c
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644

AR := $(CROSS_COMPILE)ar
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip

BIN_NAME := kvswebrtcmaster-static
BIN_TARGET := $(addprefix $(SYSTEM_BIN)/,$(BIN_NAME))

DATA_NAME := rootca.pem
DATA_DIR := $(SYSTEMFS)/share/amazon_kvs
DATA_TARGET := $(addprefix $(DATA_DIR)/,$(DATA_NAME))

CMAKE := cmake
RM := rm -f
RM_RF := rm -rf

.PHONY: default
default: all

.PHONY: all
all: build

.PHONY: build
build:
ifeq ("$(wildcard $(CURDIR)/build/Makefile)","")
	$(Q)$(CMAKE) $(CURDIR) -B $(CURDIR)/build -DCMAKE_BUILD_TYPE=Release -DBOARD=HC1783S -DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/toolchain.cmake
endif
	$(Q)$(MAKE) -C build

.PHONY: install
install: build install-bin install-data

.PHONY: uninstall
uninstall: uninstall-bin uninstall-data

.PHONY: clean
clean:
ifneq ("$(wildcard $(CURDIR)/build/Makefile)","")
	$(Q)$(MAKE) -C build clean
endif

.PHONY: distclean
distclean: uninstall clean
	$(Q)$(RM_RF) $(CURDIR)/build

.PHONY: install-bin
install-bin:
	$(Q)$(INSTALL) -d $(SYSTEM_BIN)
	$(Q)$(INSTALL_PROGRAM) \
		$(CURDIR)/build/samples/webrtc/$(BIN_NAME) \
		$(BIN_TARGET)

.PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(BIN_TARGET)

.PHONY: install-data
install-data:
	$(Q)$(INSTALL) -d $(DATA_DIR)
	$(Q)$(INSTALL_DATA) \
		$(CURDIR)/resources/certs/$(DATA_NAME) \
		$(DATA_TARGET)

.PHONY: uninstall-data
uninstall-data:
	$(Q)$(RM) $(DATA_TARGET)

# toggle verbose output
V ?= 0
ifeq ($(V),1)
	Q :=
	VOUT :=
else
	Q := @
	VOUT := 2>&1 1>/dev/null
endif
