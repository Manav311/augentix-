SDKSRC_DIR?=$(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
CFLAGS += -MMD $(INC)
LFLAGS = -pthread

INC := -I$(LIBTZ_INC)

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)

TARGETS = testDST


.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SRCS)
	@printf "  %-8s  $@\n" "CC"
	$(Q)$(CC) $^ $(CFLAGS) $(LFLAGS) -o $@


.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)
