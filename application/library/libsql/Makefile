SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

LIB = libsql.a
LIB_DEST := $(APP_LIB)
INSTALL_TRGTS := $(addprefix $(LIB_DEST)/, $(LIB))

INC := -isystem$(SQLITE3_INC)
CFLAGS += -MMD
LFLAGS = -L$(SQLITE3_LIB) -lsqlite3

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

.PHONY: default
default: all

.PHONY: all
all: $(LIB)

$(LIB): $(OBJS)
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(INC)

.PHONY: install
install: $(INSTALL_TRGTS)

$(LIB_DEST):
	$(Q)install -d $@

$(INSTALL_TRGTS): $(LIB) $(LIB_DEST)
	$(Q)install -m 666 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[doa]" -exec rm -f {} \;
	$(Q)rm -f $(LIB)

.PHONY: distclean
distclean: uninstall clean
