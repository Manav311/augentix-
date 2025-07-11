SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I./gpio -I./event -I./sw -I./adc -I./led -I./libledevt -I./pwm -I./mpi -I$(APP_INC) -I$(MPP_INC) -I$(EXTDRV_PATH)/eintc_pir/include
CFLAGS += -MMD $(INC)
LDFLAGS := -L$(LIBLEDEVT_PATH) $(LIB) -L$(MPP_LIB)
LDLIBS := -pthread -lm -lledevt -lrt -lmpp

TARGETS = eventd
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

SRCS=$(shell find . \( -path './libledevt' -o -path './unit_test' \) -prune -o -name "*.c" -print)
OBJS=$(SRCS:.c=.o)

CONF_FILE = TimeSwitch.conf
CONF_DEST := $(SYSTEMFS)/factory_default

SHELL_SCRIPT = setLEDevt.sh button_mode.sh
SHELL_DEST := $(SYSTEM_BIN)
SHELL_TRGTS := $(addprefix $(SHELL_DEST)/, $(SHELL_SCRIPT))

.PHONY: default
default: all

.PHONY: all
all: lib unit_test $(TARGETS)

.PHONY: lib
lib:
	$(Q)make -C libledevt all

.PHONY: lib-clean
lib-clean:
	$(Q)make -C libledevt clean

.PHONY: lib-distclean
lib-distclean:
	$(Q)make -C libledevt distclean

.PHONY: lib-install
lib-install:
	$(Q)make -C libledevt install

.PHONY: lib-uninstall
lib-uninstall:
	$(Q)make -C libledevt uninstall

.PHONY: unit_test
unit_test:
	$(Q)make -C unit_test all install

.PHONY: unit_test-clean
unit_test-clean:
	$(Q)make -C unit_test clean

$(TARGETS): $(SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS) lib-install conf-install script-install

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: conf-install
conf-install: $(CONF_DEST)/$(CONF_FILE)

$(CONF_DEST)/$(CONF_FILE):
	$(Q)install -d $(CONF_DEST)
	$(Q)install -m 666 $(CONF_FILE) $@

.PHONY: script-install
script-install: $(SHELL_DEST)/$(SHELL_SCRIPT)

$(SHELL_DEST)/$(SHELL_SCRIPT):
	$(Q)install -d $(SHELL_DEST)
	$(Q)install -m 777 $(SHELL_SCRIPT) $(SHELL_DEST)

.PHONY: uninstall
uninstall: lib-uninstall
	$(Q)rm -f $(INSTALL_TRGTS)
	$(Q)rm -f $(CONF_DEST)/$(CONF_FILE)
	$(Q)rm -f $(SHELL_TRGTS)

.PHONY: clean
clean: lib-clean unit_test-clean
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean lib-distclean
