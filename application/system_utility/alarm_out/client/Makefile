SDKSRC_DIR?=$(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

TARGET_BIN_PATH=$(CUSTOMFS)/bin

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

INC := -I$(CURDIR)/../include
CFLAGS += -MMD $(INC)
LDLIBS := -pthread

BIN=alarmoutc
SRCS:=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

.PHONY : all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDLIBS)

sinclude $(DEPS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: install
install:
	$(Q)mkdir -p $(TARGET_BIN_PATH)
	$(Q)cp -f $(BIN) $(TARGET_BIN_PATH)

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(TARGET_BIN_PATH)/$(BIN)

.PHONY: clean
clean:
	$(Q)find . -type f -name "*.[oad]" -exec rm -f {} \;
	$(Q)rm -f $(BIN)

.PHONY: distclean
distclean: uninstall clean
