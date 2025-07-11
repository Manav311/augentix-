SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)

include $(SDKSRC_DIR)/application/internal.mk

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

INC = -I. -I./acodec -isystem$(ALSA_INC) -I$(LIBADO_INC) -I$(AUDIO_INC)
LIB = -L$(ALSA_LIB) -lasound -L$(MPP_LIB) -lacodec
CFLAGS += -MMD $(INC) $(LIB)
CFLAGS += -DCODECS='"$(AUDIO_CODEC)"'

BIN = libado.a
DEST := $(APP_LIB)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(BIN))

SRCS = $(shell find . -name '*.c')
OBJS = $(patsubst %.c,%.o,$(SRCS))
DEPS = $(patsubst %.c,%.d,$(SRCS))

.PHONY : all
all: $(BIN)

$(BIN): $(OBJS)
	@printf "  %-6s  $@\n" "AR"
	$(Q)$(AR) rcs $@ $^

sinclude $(DEPS)

# compile and generate dependency info
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed: strip the target (everything before colon)
#   sed: remove any continuation backslashes
#   fmt -1: list words one per line
#   sed: strip leading spaces
#   sed: add trailing colons
#

%.o: %.c
	@printf "  %-6s  $@\n" "CC"
	$(Q)$(CC) $< -c $(CFLAGS) -o $@
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

.PHONY: install
install: $(DEST) $(INSTALL_TRGTS)

$(DEST):
	@printf "  %-6s  $@\n" "INSTALL"
	$(Q)install -d $@

$(DEST)/%: %
	@printf "  %-6s  $@\n" "INSTALL"
	$(Q)install -m 666 $< $@

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: clean
clean:
	@printf "  %-6s  *.[ado]\n" "CLEAN"
	$(Q)find . -type f -name "*.[ado]" -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean
