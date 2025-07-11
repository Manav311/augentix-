SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc

INC := -I./ -I./common -I$(APP_INC)
CFLAGS += $(INC)

BIN := ws_server
WSDL_OBJ := ws_process.o
OBJ := ws_server.o stdsoap2.o soapC.o soapServer.o $(WSDL_OBJ) ./common/augentix.o

TARGETS = $(BIN)
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

.PHONY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(Q)$(CC) $(CFLAGS) -o $@ $(OBJ)

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
	$(Q)rm -f *.o ./common/*.o $(BIN)

.PHONY: distclean
distclean: uninstall clean
