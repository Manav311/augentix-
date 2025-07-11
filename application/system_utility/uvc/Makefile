SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

UVC_INC:=$(KERNEL_PATH)/drivers/usb/gadget/function
INC := -I./include -I$(MPP_INC) -I$(UVC_INC) -I$(APP_INC) -I$(GEN_INC) -I$(DEBUG_INC) \
	-I$(LIBCM_INC) -isystem$(JSON_INC) -I$(CENTCTRL_INC) -I$(LIBAGTX_INC)
CFLAGS += -MMD $(INC)
LDFLAGS := -L$(MPP_LIB) -L$(DEBUG_PATH) -L$(LIBCM_PATH) -L$(JSON_LIB)
LDLIBS := -lmpp -lrt -lcm -ljson-c -pthread -lm -lstdc++

BIN = uvcd
SRCS := $(wildcard ./*.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
DEST := $(SYSTEM_BIN)
INSTALL_TRGT := $(addprefix $(DEST)/, $(BIN))

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

sinclude $(DEPS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: install
install: all $(DEST) $(INSTALL_TRGT)

$(DEST):
	$(Q)install -d $@

$(DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGT)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[doa]" -exec rm -f {} \;
	$(Q)rm -f $(BIN)

.PHONY: distclean
distclean: uninstall clean
