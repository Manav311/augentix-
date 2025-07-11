SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# specify CFLAGS and LDLIBS
INC := -I$(LIBFOO_INC)
CFLAGS += $(INC)
LDLIBS := -lm

# specify bin
BIN_NAME := g726

# bin paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

# data paths
DATA_PATH := $(root)/data
DATA_DEST := $(SYSTEMFS)/share/g726
DATA_SRCS := $(notdir $(wildcard $(DATA_PATH)/*))
DATA_TARGETS := $(addprefix $(DATA_DEST)/,$(DATA_SRCS))

# If you need to install sample data, set the value to y
COPY_SAMPLE_DATA :=

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

# Delete all files created by building the app.
.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all install-bin install-data

.PHONY: install-bin
install-bin: $(BIN_TARGET)

.PHONY: install-data
ifeq ($(COPY_SAMPLE_DATA),y)
install-data: $(DATA_TARGETS)
else
install-data: ;
endif

.PHONY: uninstall
uninstall: uninstall-bin uninstall-data

.PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(BIN_TARGET)

.PHONY: uninstall-data
uninstall-data:
	$(Q)$(RM) $(DATA_TARGETS)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(DATA_DEST):
	$(Q)$(INSTALL) -d $@

$(DATA_DEST)/%: $(DATA_PATH)/% $(DATA_DEST)
	$(Q)$(INSTALL_DATA) $< $@

-include $(DEPS)
