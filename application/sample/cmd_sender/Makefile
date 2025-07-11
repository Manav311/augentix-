SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

MAKEFLAGS += -j

# specify build tools
CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

# specify CFLAGS
CFLAGS += -MMD -MP

# paths
root = $(realpath $(CURDIR))
TARGET_BIN_PATH = $(SYSTEM_BIN)

# specify source files and include directories
SRCS = $(addprefix $(root)/,$(wildcard *.c) $(wildcard cmds/*.c))
INCS = . $(CURDIR)/cmds $(MPP_INC)
LIBS = mpp pthread m rt
LIBDIRS = $(MPP_LIB)

# [DON'T TOUCH] calculate corresponding object files and auto-dependencies
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

# specify bin
BIN = cmdsender

# specify targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(root)/$(BIN)

$(root)/$(BIN): $(OBJS)
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(CFLAGS) $^ -o $@ $(addprefix -L,$(LIBDIRS)) $(addprefix -l,$(LIBS))

.PHONY: install
install: $(TARGET_BIN_PATH)/$(BIN)

$(TARGET_BIN_PATH)/$(BIN): $(root)/$(BIN) | $(TARGET_BIN_PATH)
	@printf "  %-8s$@\n" "INSTALL"
	$(Q)install -m 755 $< $(dir $@)

$(TARGET_BIN_PATH):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)install -d $@

.PHONY: uninstall
uninstall:
	@printf "  %-8s$(TARGET_BIN_PATH)/$(BIN)\n" "RM"
	$(Q)rm -f $(TARGET_BIN_PATH)/$(BIN)

.PHONY: clean
clean:
	@printf "  %-8s$(CURDIR)\n" "CLEAN"
	$(Q)rm -rf $(BIN) $(OBJS) $(DEPS)
	$(Q)find . -name *.[do] -exec rm -f {} \;

.PHONY: distclean
distclean: uninstall clean

# general directory independent targets
%.o: %.c
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(addprefix -iquote,$(INCS))

# Autodependencies
-include $(DEPS)
