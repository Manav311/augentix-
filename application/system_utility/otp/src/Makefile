SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc

INC := -I. -I$(OTP_INC) -I$(NRS_DRV_INC)
CFLAGS += -MMD $(INC)

TARGETS = otprw
DEST = $(CUSTOMFS)/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

SRCS := $(wildcard ./*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SRCS)
	$(Q)$(CC) $(CFLAGS) -o $@ $^

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
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean

TARGETS_TEST = otp-test

.PHONY: ut-otp
ut-otp: clean $(TARGETS_TEST)

$(TARGETS_TEST): $(SRCS)
	$(Q)$(CC) $(CFLAGS) -DOTP_DEBUG -o $@ $^
