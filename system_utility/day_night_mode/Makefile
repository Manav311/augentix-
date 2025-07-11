SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I$(MPP_INC) -I$(APP_INC) -I$(DEBUG_INC) -isystem$(JSON_INC)
CFLAGS += -MMD $(INC)
LDFLAGS := -L$(MPP_LIB) -L$(JSON_LIB) -L$(DEBUG_PATH)
LDLIBS := -pthread -lm -ljson-c -lrt -lmpp

IR_CUT_CTRL_DIR := $(CURDIR)/ir_cut.sh
IR_LED_CTRL_DIR := $(CURDIR)/ir_led.sh
TARGETS = switch_day_night_mode
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))
TARGET_SCRIPT_DIR := $(SYSTEMFS)/mpp/script

SRCS=$(shell find -name '*.c')

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
	$(Q)mkdir -p $(TARGET_SCRIPT_DIR)

$(DEST)/%: %
	$(Q)install -m 777 $< $@
	$(Q)cp -f $(IR_CUT_CTRL_DIR) $(TARGET_SCRIPT_DIR)/ir_cut.sh
	$(Q)cp -f $(IR_LED_CTRL_DIR) $(TARGET_SCRIPT_DIR)/ir_led.sh

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)
	$(Q)rm -f $(TARGET_SCRIPT_DIR)/ir_cut.sh
	$(Q)rm -f $(TARGET_SCRIPT_DIR)/ir_led.sh

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean
