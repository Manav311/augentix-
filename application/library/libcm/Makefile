SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

CODEGEN_DIR = $(SDKSRC_DIR)/application/library/libcm/codegen

LIB := libcm.a
LIB_DEST := $(APP_LIB)
LIB_TRGT := $(addprefix $(LIB_DEST)/, $(LIB))

INC    := -I./include -I$(APP_INC) -isystem$(JSON_INC)
CFLAGS+ = $(INC) -MMD

#LDFLAGS := -pthread -lrt -L$(JSON_LIB) -ljson-c

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))

.PHONY: default
default: all

.PHONY: all
all: $(LIB) iqtool-schema

$(LIB): $(OBJS)
	@printf "  %-8s  $(notdir $^)\n" "AR"
	$(Q)$(AR) rcs $@ $^

%.o: %.c
	$(Q)$(CC) $< -c $(CFLAGS) -o $@ $(INC)

.PHONY: install
install: $(LIB_TRGT) iqtool-schema-install

.PHONY: uninstall
uninstall: iqtool-schema-uninstall
	$(Q)rm -f $(LIB_TRGT)

.PHONY: clean
clean: iqtool-schema-clean
	$(Q)find . -type f -name "*.[doa]" -exec rm -f {} \;
	$(Q)rm -f $(LIB)

.PHONY: distclean
distclean: uninstall clean

$(LIB_TRGT): $(LIB) $(LIB_DEST)
	$(Q)cp -f $< $@

$(LIB_DEST):
	$(Q)install -d $@

.PHONY: iqtool-schema iqtool-schema-clean iqtool-schema-install iqtool-schema-uninstall
iqtool-schema:
	@echo "Run $(CODEGEN_DIR)/gen_schema_for_iq_tuning_tool.sh"
	$(Q)sh $(CODEGEN_DIR)/gen_schema_for_iq_tuning_tool.sh $(CODEGEN_DIR)/iq_tuning_tool_schema

iqtool-schema-clean: iqtool-schema-uninstall
	@echo "Remove $(CODEGEN_DIR)/iq_tuning_tool_schema"
	$(Q)rm -rf $(CODEGEN_DIR)/iq_tuning_tool_schema

iqtool-schema-install:
	$(Q)tar -cvf $(SDKSRC_DIR)/fs/rootfs/output/system/schema.tar -C $(CODEGEN_DIR)/iq_tuning_tool_schema .

iqtool-schema-uninstall:
	$(Q)rm -rf $(SDKSRC_DIR)/fs/rootfs/output/system/schema.tar

