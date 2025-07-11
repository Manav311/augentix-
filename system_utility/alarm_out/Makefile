SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

DAEMON_PATH=$(CURDIR)/daemon
CLIENT_PATH=$(CURDIR)/client

.PHONY: all
all:
	$(Q)$(MAKE) -C $(DAEMON_PATH)
	$(Q)$(MAKE) -C $(CLIENT_PATH)

.PHONY: install
install: all
	$(Q)$(MAKE) -C $(DAEMON_PATH) install
	$(Q)$(MAKE) -C $(CLIENT_PATH) install

.PHONY: uninstall
uninstall:
	$(Q)$(MAKE) -C $(DAEMON_PATH) uninstall
	$(Q)$(MAKE) -C $(CLIENT_PATH) uninstall

.PHONY: clean
clean:
	$(Q)$(MAKE) -C $(DAEMON_PATH) clean
	$(Q)$(MAKE) -C $(CLIENT_PATH) clean

.PHONY: distclean
distclean: uninstall clean
