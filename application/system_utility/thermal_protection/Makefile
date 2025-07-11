SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

TARGET_BIN_PATH=$(CUSTOMFS)/bin

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

SNS := $(shell echo $(SENSOR) | tr '[A-Z]' '[a-z]')
MB := $(shell echo $(MOTHERBOARD) | tr '[A-Z]' '[a-z]')
SNS_DIR := $(SENSOR_PATH)/$(SNS)/$(SNS)_$(MB)

BIN=thermal_protd
SRCS:=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

INC := -I$(CURDIR)/../include -I$(GEN_INC) -I$(SNS_DIR) -I$(APP_INC)
CFLAGS += -MMD $(INC)

.PHONY: all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(CC) -o $@ $^

sinclude $(DEPS)

%.o: %.c
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: install
install: all
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
