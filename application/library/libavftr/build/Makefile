################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/.config

################################################################################
# Build tools and commands
################################################################################

MAKEFLAGS += -j

### First, specify the build tools.

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
CP := cp
RM := rm
LN := ln
MKDIR := mkdir
RMDIR := rmdir
INSTALL := install

### Then, define global compilation and linking flags.

CFLAGS_ALL += $(CFLAGS) -Wno-missing-field-initializers -MMD -ffunction-sections -fdata-sections
CPPFLAGS += -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DBSD=1
LFLAGS_ALL :=


### Finally, define complete commands for all actions.

COMPILE     = $(CC) $(CFLAGS_ALL) $(CFLAGS_LOCAL) $(CPPFLAGS) -o $@ -c $<
LINK        = $(CC) -o $@ -Wl,--start-group $^ -Wl,--end-group $(LFLAGS_ALL) $(LFLAGS_LOCAL)
ARCHIVE_STATIC = $(AR) $(ARFLAGS) $@ $^

# ARCHIVE_SHARED = $(CC) -shared -Wl,--build-id -Wl,--soname,$(basename $(basename $(notdir $@))) -o $@ \
# 		-Wl,--whole-archive $^ -Wl,--no-whole-archive

################################################################################
# Build rules
################################################################################

### First, define default build target.

all: primary

### Second, define variables for collecting outputs and temporarily objects.

TARGET_SLIBS :=
TARGET_DLIBS :=
TARGET_TESTS :=
CLEAN_FILES :=
CLEAN_DIRS  :=
DEP_SLIBS :=
TARGET_MODELS :=

### Then, collect the knowledge about how to build all outputs.

root := $(realpath ..)
dir := $(root)/build

subdir := $(root)/src
include $(subdir)/Rules.mk

### Finally, define essential build targets.

.PHONY: primary
primary: $(TARGET_SLIBS)

.PHONY: test
test: all $(TARGET_TESTS)

.PHONY: doc
doc: $(TARGET_DOCS)

.PHONY: clean
clean:
	@printf "  %-8s$(root)\n" "CLEAN"
	$(Q)$(RM) -f $(CLEAN_FILES)
	$(Q)$(foreach d, $(CLEAN_DIRS), $(RM) -rf $(d); )
	$(Q)cd $(root); find . -name "*.[oa]" -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: ;

.PHONY: uninstall
uninstall: ;

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
	@echo "    test        Build test programs."
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
	$(Q)$(COMPILE)

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

