SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

TRGTS = ccserver ccclient
LIBTRGT = libccdata.a
DEST := $(CUSTOMFS)/bin
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TRGTS))

INC    := -isystem$(JSON_INC) -I$(LIBSQL_INC) -I$(LIBCM_INC) -I$(APP_INC) -I$(CENTCTRL_PATH)/include
CFLAGS += -MMD $(INC)

LDFLAGS := -L$(ZLIB_LIB) -L$(SQLITE3_LIB) -L$(JSON_LIB) -L$(APP_LIB) -L$(LIBSQL_PATH) -L$(LIBCM_PATH)
LDLIBS := -pthread -lrt -lcm -ljson-c -lsql -lz -lsqlite3

SRCS := $(filter-out cli.c, $(wildcard *.c))
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: default
default:all

.PHONY: all
all: $(TRGTS) $(LIBTRGT)

ccserver: $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ccclient: cli.o cc_data.o
	$(Q)$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIBTRGT): cc_data.o
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@

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
