SDKSRC_DIR?=$(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

TARGET_BIN_PATH=$(CUSTOMFS)/bin

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
CFLAGS += -MMD

BIN=actl
SRCS:=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,%.d,$(SRCS))

INC=-isystem$(ALSA_INC) -I$(AUDIO_INC)
LIB=-L$(ALSA_LIB) -lasound


.PHONY : all
all: $(BIN)

$(BIN): $(OBJS)
	$(Q)$(CC) $< $(CFLAGS) -o $@ $(INC) $(LIB)

sinclude $(DEPS)

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(INC) $(LIB)

.PHONY: install
install: all
	$(Q)mkdir -p $(TARGET_BIN_PATH)
	$(Q)cp -f $(BIN) $(TARGET_BIN_PATH)

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(TARGET_BIN_PATH)/$(BIN)

.PHONY: clean
clean: uninstall
	$(Q)find . -type f -name "*.[oad]" -exec rm -f {} \;
	$(Q)rm -f $(BIN)

.PHONY: distclean
distclean: uninstall clean
