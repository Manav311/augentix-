SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)

include $(SDKSRC_DIR)/application/internal.mk

# architecture dependent variables
ifeq ($(CONFIG_HC1703_1723_1753_1783S),y)
    LOAD_SCRIPT := $(CURDIR)/load_mpp_hc17x3.sh
else
    LOAD_SCRIPT := $(CURDIR)/load_mpp_hc17x2.sh
endif

# specify build / install target
TARGET_PATH := $(SYSTEMFS)/mpp/script
SOURCES := $(filter-out load_mpp_%.sh, $(wildcard *.sh))
TARGETS := $(addprefix $(TARGET_PATH)/, load_mpp.sh $(SOURCES))

# targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: ;

.PHONY: clean
clean: ;

.PHONY: install
install: all $(TARGETS)

.PHONY: distclean
distclean: uninstall clean

$(TARGET_PATH)/%.sh: %.sh | $(TARGET_PATH)
	@printf "  %-8s$@\n" "INSTALL"
	$(Q)install -m 755 $< $@

$(TARGET_PATH)/load_mpp.sh: $(LOAD_SCRIPT) | $(TARGET_PATH)
	@printf "  %-8s$@\n" "INSTALL"
	$(Q)install -m 755 $< $@

$(TARGET_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)mkdir -p $(TARGET_PATH)

.PHONY: uninstall
uninstall:
	@printf "  %-8s$(TARGET_PATH)\n" "RMDIR"
	$(Q)rm -f $(TARGETS)
	$(Q)if [ -d $(TARGET_PATH) ]; then \
		rmdir --ignore-fail-on-non-empty $(TARGET_PATH); \
	fi
