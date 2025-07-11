################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/build/sdksrc.mk
include $(SDKSRC_DIR)/application/internal.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

################################################################################
# Build tools and commands
################################################################################

MAKEFLAGS += -j

### First, specify the build tools.

CC := $(CROSS_COMPILE)gcc
CX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
RM := rm
LN := ln
MKDIR := mkdir
RMDIR := rmdir
STRIP := $(CROSS_COMPILE)strip
INSTALL := install

### Then, define global compilation and linking flags.

DEFINE_ALL :=
CFLAGS_ALL := $(CFLAGS) -MMD
LFLAGS_ALL :=

### Finally, define complete commands for all actions.

COMPILE = $(addprefix -D,$(DEFINE_ALL)) $(addprefix -D,$(DEFINE_LOCAL)) $(CFLAGS_ALL) $(CFLAGS_LOCAL) -o $@ -c $<
LINK = -o $@ $^ $(LFLAGS_ALL) $(LFLAGS_LOCAL)

################################################################################
# Build rules
################################################################################

TARGET_CONFIG_PATH = $(SYSTEMFS)/mpp/od_config

### First, define default build target.

.PHONY: all
all: primary

### Second, define variables for collecting outputs and temporarily objects.

TARGET_BINS   :=
TARGET_ASSETS :=
CLEAN_FILES   :=
CLEAN_DIRS    :=
CLEAN_TARGETS :=

### Then, collect the knowledge about how to build all outputs.

root := $(realpath ..)
dir := $(root)/build

subdir := $(root)/src
include $(subdir)/Rules.mk

subdir := $(root)/util
include $(subdir)/Rules.mk

TARGET_ASSETS += $(wildcard $(root)/assets/*)

################################################################################
# Targets (Must be placed after "Subdirectories" section)
################################################################################

.PHONY:	primary
primary: $(TARGET_BINS)

.PHONY: clean
clean:
	@printf "  %-8s$(root)\n" "CLEAN"
	$(Q)$(RM) -f $(CLEAN_FILES)
	$(Q)$(foreach d, $(CLEAN_DIRS), $(RM) -rf $(d); )

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: primary
	$(Q)$(MKDIR) -p $(CUSTOM_BIN)
	$(Q)$(foreach f, $(TARGET_BINS), \
		printf "  %-8s$(CUSTOM_BIN)/$(notdir $(f))\n" "INSTALL"; \
		$(INSTALL) -m 755 -t $(CUSTOM_BIN) $(f); \
	)
	$(Q)$(MKDIR) -p $(TARGET_CONFIG_PATH)
	$(Q)$(foreach f, $(TARGET_ASSETS), \
		printf "  %-8s$(TARGET_CONFIG_PATH)/$(notdir $(f))\n" "INSTALL"; \
		$(INSTALL) -m 644 -t $(TARGET_CONFIG_PATH) $(f); \
	)

.PHONY: uninstall
uninstall:
	$(Q)$(foreach f, $(TARGET_BINS), \
		printf "  %-8s$(CUSTOM_BIN)/$(notdir $(f))\n" "RM"; \
		$(RM) -f $(CUSTOM_BIN)/$(notdir $(f)); \
	)
	$(Q)$(foreach f, $(TARGET_ASSETS), \
		printf "  %-8s$(TARGET_CONFIG_PATH)/$(notdir $(f))\n" "RM"; \
		$(RM) -f $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)
	$(Q)if [ -d $(TARGET_CONFIG_PATH) ]; then \
		$(RMDIR) --ignore-fail-on-non-empty $(TARGET_CONFIG_PATH); \
	fi
	$(Q)if [ -d $(CUSTOM_LIB) ]; then \
		$(RMDIR) --ignore-fail-on-non-empty $(CUSTOM_LIB); \
	fi

.PHONY: help
help:
	@echo ""
	@echo "  Syntax:"
	@echo "    make [options] <target>"
	@echo ""
	@echo "  Options:"
	@echo "    V=0         Hide command lines. (default)"
	@echo "    V=1         Show command lines."
	@echo ""
	@echo "  Targets:"
	@echo "    all         Build primary outputs. (default)"
	@echo "    clean       Clean all outputs."
	@echo "    distclean   Clean all outputs and configurations."
	@echo "    install     Install primary outputs"
	@echo "    uninstall   Uninstall primary outputs"
	@echo "    help        Show help messages."
	@echo ""

################################################################################
# General rules
################################################################################

%.o: %.c
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(COMPILE)

$(root)/bin $(TARGET_CONFIG_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)$(MKDIR) $@

CLEAN_DIRS += $(root)/bin

################################################################################
# Quiet options
################################################################################

V ?= 0
ifeq ($(V),1)
	Q :=
	VOUT :=
else
	Q := @
	VOUT := 2>&1 1>/dev/null
endif

################################################################################
# Prevent make from removing any build targets, including intermediate ones
################################################################################

.SECONDARY: $(CLEAN_FILES) $(CLEAN_DIRS)
