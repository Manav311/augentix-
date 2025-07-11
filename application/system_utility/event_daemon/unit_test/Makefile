SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I$(APP_INC) -I$(EXTDRV_PATH)/eintc_pir/include -I$(LIBTZ_INC)
CFLAGS += -MMD $(INC)
LDFLAGS := -L$(APP_LIB) -L$(LIBTZ_PATH)
LDLIBS := -pthread -ltz

TARGETS = test_eirq_pir
DEST = $(SYSROOT)/system/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

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
clean: uninstall
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)
