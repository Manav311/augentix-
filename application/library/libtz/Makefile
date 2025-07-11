SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
CFLAGS += -MMD

BIN=libtz.a
SRCS:=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

IFLAGS=-I$(CURDIR)/include -I$(LIBAGTX_INC)
LFLAGS=-pthread

SH_FILE = setDST_en.sh
SH_DEST = $(CUSTOMFS)/bin


.PHONY : all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^

sinclude $(DEPS)

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(IFLAGS) $(LFLAGS)

.PHONY : install
install: sh-install all
	$(Q)mkdir -p $(APP_LIB)
	$(Q)cp -f $(BIN) $(APP_LIB)

.PHONY : sh-install
sh-install: $(SH_DEST)/$(SH_FILE)

$(SH_DEST)/$(SH_FILE):
	$(Q)install -d $(SH_DEST)
	$(Q)install -m 777 $(SH_FILE) $@

.PHONY : uninstall
uninstall:
	$(Q)rm -f $(APP_LIB)/$(BIN)
	@rm -f $(SH_DEST)/$(SH_FILE)

.PHONY : clean
clean:
	$(Q)find . -type f -name "*.[oad]" -exec rm -f {} \;
	$(Q)rm -f $(BIN)

.PHONY: distclean
distclean: uninstall clean
