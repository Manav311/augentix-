SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc

INC := -I. -I./../include -I$(APP_INC)
CFLAGS += -MMD $(INC)

TARGETS = libgpio.a

SRCS=$(shell find -name '*.c')
OBJS=$(SRCS:.c=.o)

.PHONY: default
default: all

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(OBJS)
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@

.PHONY : install
install:
	$(Q)mkdir -p $(APP_LIB)
	$(Q)cp -f $(TARGETS) $(APP_LIB)

.PHONY : uninstall
uninstall:
	$(Q)rm -f $(APP_LIB)/$(TARGETS)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[do]" -exec rm -f {} \;
	$(Q)rm -rf $(TARGETS)

.PHONY: distclean
distclean: uninstall clean
