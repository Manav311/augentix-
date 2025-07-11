SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc

INC := -I.
CFLAGS += -MMD $(INC)
LDLIBS = -pthread

TARGETS = gpio-alarm gpio-reset
DEST = $(SYSROOT)/usr/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

GPIO_ALARM_SRCS := gpio.c gpio-alarm.c
GPIO_RESET_SRCS := gpio.c gpio-reset.c

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

gpio-reset: $(GPIO_RESET_SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

gpio-alarm: $(GPIO_ALARM_SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: % $(DEST)
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: clean
clean: uninstall
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean
