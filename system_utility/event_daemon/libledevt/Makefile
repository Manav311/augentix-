SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I../led -I$(APP_INC)
CFLAGS += -MMD $(INC)

TARGETS = libledevt.a

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS) ledapp

.PHONY: install
install: ledapp-install

.PHONY: uninstall
uninstall: ledapp-uninstall

$(TARGETS): $(OBJS)
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@

.PHONY: clean
clean: ledapp-clean
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: clean ledapp-distclean

.PHONY: ledapp
ledapp:
	 $(Q)$(MAKE) -C ./ledapp all

.PHONY: ledapp-install
ledapp-install:
	 $(Q)$(MAKE) -C ./ledapp install

.PHONY: ledapp-uninstall
ledapp-uninstall:
	 $(Q)$(MAKE) -C ./ledapp uninstall

.PHONY: ledapp-clean
ledapp-clean:
	$(Q)$(MAKE) -C ./ledapp clean

.PHONY: ledapp-distclean
ledapp-distclean:
	$(Q)$(MAKE) -C ./ledapp distclean
