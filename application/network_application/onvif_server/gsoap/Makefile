SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

MAKEFLAGS += -j5

CC=$(CROSS_COMPILE)gcc

INC := -isystem./include -isystem./plugin -isystem$(OPENSSL_INC)
CFLAGS += -Wno-deprecated-declarations -MMD $(INC)
CPPFLAGS := -DWITH_OPENSSL

BIN=libgsoap.a

SRCS=$(wildcard *.c) $(wildcard plugin/*.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

.PHONY: default
default: all

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: install
install: all

.PHONY: uninstall
uninstall: ;

.PHONY: clean
clean:
	@find . -type f -name "*.[doa]" -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean
