SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

INSTALL_PROGRAM := install -m 700
INSTALL_DIR := install -d
RM_RF := rm -rf

APPVERIFY := appverify

BIN_DIR := $(CURDIR)
TARGET_BIN_DIR := $(SYSTEM_BIN)

.PHONY: all
all: ;

.PHONY: distclean
distclean: uninstall clean

.PHONY: clean
clean: ;

.PHONY: install
install: $(TARGET_BIN_DIR)/$(APPVERIFY)

.PHONY: uninstall
uninstall:
	$(Q)$(RM_RF) $(TARGET_BIN_DIR)/$(APPVERIFY)

$(TARGET_BIN_DIR)/$(APPVERIFY): $(BIN_DIR)/$(APPVERIFY) | $(TARGET_BIN_DIR)
	$(Q)$(INSTALL_PROGRAM) "$<" "$(dir $@)"

$(TARGET_BIN_DIR):
	$(Q)$(INSTALL_DIR) "$@"
