SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

INC := -I. -I$(SDK_INC) -I$(APP_INC) -I$(CONNSEL_PATH)/include -I$(LIBAGTX_INC)
CFLAGS += -MMD $(INC)
LDFLAGS :=
LDLIBS := -pthread

TRGTS = connsel
DEST = $(CUSTOMFS)/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TRGTS))

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: default
default:all

.PHONY: all
all: $(TRGTS)

$(TRGTS): $(OBJS)
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: install
install: $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[doa]" -exec rm -f {} \;
	$(Q)rm -f $(TRGTS)

.PHONY: distclean
distclean: uninstall clean
