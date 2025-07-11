SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

INSTALL := install -c

DSTDIR = $(SYSROOT)/usr/sbin
TARGET_ELF := $(wildcard ../bin/*)

DST_TARGET_ELF := $(subst ../bin,$(DSTDIR),$(TARGET_ELF))

INC :=
LIB := -lpthread

CFLAGS += $(addprefix -I,$(INC))
LIB_LPW := -I$(LIBLPW_INC) -L$(LIBLPW_LIB) -llpw

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all:
	-@mkdir -p ../bin/
	$(Q)$(CC) ../src/*.c $(CFLAGS) $(CPPFLAGS) $(LIB) $(LIB_LPW) -o ../bin/lpw_supplicant

.PHONY: clean
clean:
	$(Q)$(RM) -r ../bin

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all
	$(Q)$(INSTALL) -d $(DSTDIR)
	$(Q)$(INSTALL) -m 755 $(TARGET_ELF) $(DSTDIR)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(DST_TARGET_ELF)

-include $(DEPS)
