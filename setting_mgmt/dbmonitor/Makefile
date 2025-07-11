SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc

BIN=dbmonitor

INC := -I. -isystem$(SQLITE3_INC) -I$(APP_INC)
OBJ := main.o
CFLAGS += -MMD $(INC)
LDFLAGS := -L$(ZLIB_LIB) -L$(SQLITE3_LIB)
LDLIBS := -pthread -lrt -lz -lsqlite3

TARGETS = $(BIN)
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

.PHONY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

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
clean:
	@rm -f *.[doa] $(BIN)

.PHONY: distclean
distclean: uninstall clean
