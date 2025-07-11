SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I$(APP_INC)
CFLAGS += -MMD $(INC)
LDLIBS := -pthread

TARGETS = watchd
DEST = $(SYSROOT)/usr/sbin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

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
