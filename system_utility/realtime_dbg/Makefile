SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I.
CFLAGS += -MMD $(INC)
LDLIBS := -pthread

TARGETS = spi_dbg uart_dbg
DEST = $(SYSROOT)/usr/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

SPI_DBG := spi_demo.c
UART_DBG := uart_demo.c

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

spi_dbg: $(SPI_DBG)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

uart_dbg: $(UART_DBG)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -f $(TARGETS)

.PHONY: distclean
distclean: clean

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

