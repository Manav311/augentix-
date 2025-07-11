SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../)
include $(SDKSRC_DIR)/application/internal.mk

.PHONY: all
all: 
	$(Q)$(MAKE) -C $(CURDIR)/file all
	$(Q)$(MAKE) -C $(CURDIR)/udps all

.PHONY: install
install:
	$(Q)$(MAKE) -C $(CURDIR)/udps install
	$(Q)$(MAKE) -C $(CURDIR)/file install

.PHONY: uninstall
uninstall:
	$(Q)$(MAKE) -C $(CURDIR)/udps uninstall
	$(Q)$(MAKE) -C $(CURDIR)/file uninstall

.PHONY: clean
clean:
	$(Q)$(MAKE) -C $(CURDIR)/udps clean
	$(Q)$(MAKE) -C $(CURDIR)/file clean

.PHONY: distclean
distclean: uninstall clean
