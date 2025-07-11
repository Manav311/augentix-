SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
ifeq ("$(wildcard $(SDKSRC_DIR)/build/sdksrc.mk)","")
SDKSRC_DIR := $(realpath $(CURDIR)/../..)
endif
include $(SDKSRC_DIR)/application/internal.mk

TARGETS = otprw
DEST = $(OTP_LIB)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))
TARGETS_TEST = otp-test

.PHONY: all
all:
	$(MAKE) -C $(OTP_PATH)/src all

.PHONY: clean
clean: 
	$(MAKE) -C $(OTP_PATH)/src clean

.PHONY: distclean
distclean:
	$(MAKE) -C $(OTP_PATH)/src distclean

.PHONY: install
install:
	$(MAKE) -C $(OTP_PATH)/src install

.PHONY: uninstall
uninstall:
	$(MAKE) -C $(OTP_PATH)/src uninstall

.PHONY: ut-otp
ut-otp: clean $(TARGETS_TEST)

$(TARGETS_TEST): $(SRCS)
	$(Q)$(CC) $(CFLAGS) -DNRS_DEBUG -o $@ $^
