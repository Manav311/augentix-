################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../)
include $(SDKSRC_DIR)/application/internal.mk

################################################################################
# Build tools
################################################################################

AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc

################################################################################
# Build flags
################################################################################

CFLAGS += -MMD

################################################################################
# Default target
################################################################################

LIB = libfile.a

.PHONY: all
all: $(LIB)

################################################################################
# [EDIT] Specify source files and include directories
################################################################################

SRCS = $(wildcard *.c)
INCS = $(CURDIR)/../inc

################################################################################
# [DON'T TOUCH] Calculate corresponding object files and auto-dependencies
################################################################################

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

-include $(DEPS)

################################################################################
# Targets
################################################################################

$(LIB): $(OBJS)
	@printf "  %-8s $@\n" "AR"
	$(Q)$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	@printf "  %-8s $<\n" "CC"
	$(Q)$(CC) -c $< -o $@ $(CFLAGS) $(addprefix -iquote,$(INCS))

.PHONY: install
install: $(APP_LIB)/$(LIB)

$(APP_LIB)/$(LIB): $(LIB) | $(APP_LIB)
	@printf "  %-8s $@\n" "INSTALL"
	$(Q)install -m 644 $< $(dir $@)

$(APP_LIB):
	@printf "  %-8s $@\n" "MKDIR"
	$(Q)mkdir -p $@

.PHONY: uninstall
uninstall:
	@printf "  %-8s $(APP_LIB)/$(LIB)\n" "RM"
	$(Q)rm -f $(APP_LIB)/$(LIB)

.PHONY: clean
clean:
	@printf "  %-8s $(CURDIR)/*.[ado]\n" "CLEAN"
	$(Q)find . -type f -name "*.[ado]" -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean
