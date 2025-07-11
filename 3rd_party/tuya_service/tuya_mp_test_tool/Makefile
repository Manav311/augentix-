###########################
# General SDK Environment #
###########################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC := $(CROSS_COMPILE)gcc

###############################
# Include Paths and Libraries #
###############################

INC := -isystem$(JSON_INC) -isystem$(LIBINIPARSER_PATH)/src -iquote./include

#################
# Compile Flags #
#################

CFLAGS += -MMD --static $(INC)
LDFLAGS := -L$(JSON_LIB) -Llib
LDLIBS := -ljson-c -liniparser -pthread

###########################
# Source and Output Files #
###########################

TARGETS = tuya_mptt
SRCS := $(shell find ./src -name '*.c')
OBJS := $(SRCS:.c=.o)
DEPS := $(TARGETS).d

#########
# Paths #
#########

DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

#########
# Sound #
#########

SOUND_FILE = test.wav
SOUND_DEST := $(CALIBFS)
SOUND_INSTALL_TRGTS := $(addprefix $(SOUND_DEST)/, $(SOUND_FILE))

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: install
install: all $(DEST) $(INSTALL_TRGTS) $(SOUND_DEST) $(SOUND_INSTALL_TRGTS)

$(DEST) $(SOUND_DEST):
	$(Q)install -d $@

$(DEST)/% $(SOUND_DEST)/%: %
	$(Q)install -m 777 $< $@

.PHONY: uninstall
uninstall:
	@rm -f $(INSTALL_TRGTS) $(SOUND_INSTALL_TRGTS)

.PHONY: clean
clean:
	@find . -type f -name "*.[do]" -exec rm -f {} \;
	@rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean

-include $(DEPS)
