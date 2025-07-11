SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

TRGTS = rtspDebug
DEST := $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TRGTS))

SRCS := $(wildcard $(CURDIR)/*.c)
OBJS := $(SRCS:.c=.o)

INC    := 
CFLAGS := -Wall -g -std=gnu99 -MMD
LDFLAGS :=

.PHONY: default
default:all

.PHONY: all
all: $(TRGTS)

$(TRGTS): $(OBJS)
	$(Q)$(CC) $^ -o $@

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(INC)

.PHONY: install
install: $(DEST) $(INSTALL_TRGTS)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

clean: uninstall
	@find . -type f -name "*.[doa]" -exec rm -f {} \;
	@rm -f $(TRGTS)
