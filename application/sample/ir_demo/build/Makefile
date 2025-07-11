SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

CC := $(CROSS_COMPILE)gcc

LIBIR_CONTROL_PATH = $(SDKSRC_DIR)/feature/ir_control
LIBIR_CONTROL_INC = $(LIBIR_CONTROL_PATH)/include
LIBIR_CONTROL_LIB = $(LIBIR_CONTROL_PATH)/lib
INC := -I$(LIBSAMPLE_INC) -I$(MPIAPP_PATH) -I$(LIBIR_CONTROL_INC)
LIB := -L$(ZLIB_LIB) -lc -pthread -lm -lrt 
MPP_FLAG := -L$(MPP_LIB) -I$(MPP_INC) -lmpp

ATFLAGS = -I$(LIBIR_CONTROL_INC) -L$(LIBIR_CONTROL_LIB) -lir_control
AT_PATH = $(SDKSRC_DIR)/application/sample/ir_demo/src
AT_SRC = $(AT_PATH)/pwm.* $(AT_PATH)/light.* $(AT_PATH)/gpio.*
AT_BIN = $(BIN_PATH)/ir_demo $(BIN_PATH)/ir_demo_extra $(BIN_PATH)/ir_demo_get_scene_luma

BIN_PATH = $(SDKSRC_DIR)/application/sample/ir_demo/bin

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: clean
clean:
	$(Q)rm -rf $(BIN_PATH)

.PHONY: distclean
distclean: uninstall clean
	
.PHONY: all
all: $(AT_BIN)

$(BIN_PATH)/ir_demo:
	$(Q) mkdir -p $(BIN_PATH)
	$(Q)$(CC) $(CFLAGS) $(ATFLAGS) -o $@ $(AT_PATH)/ir_demo.c $(AT_SRC) $(INC) $(LIB) $(MPP_FLAG)

$(BIN_PATH)/ir_demo_extra:
	$(Q) mkdir -p $(BIN_PATH)
	$(Q)$(CC) $(CFLAGS) $(ATFLAGS) -o $@ $(AT_PATH)/ir_demo_extra.c $(AT_SRC) $(INC) $(LIB) $(MPP_FLAG)

$(BIN_PATH)/ir_demo_get_scene_luma:
	$(Q)$(CC) $(CFLAGS) $(ATFLAGS) -o $@ $(AT_PATH)/ir_demo_get_scene_luma.c $(AT_SRC) $(INC) $(LIB) $(MPP_FLAG)

.PHONY: install
install: all
	$(Q)install -m 777 $(BIN_PATH)/ir_demo $(SYSTEM_BIN)/ir_demo
	$(Q)install -m 777 $(BIN_PATH)/ir_demo_extra $(SYSTEM_BIN)/ir_demo_extra

.PHONY: uninstall
uninstall:
	$(Q)rm -f $(SYSTEM_BIN)/ir_demo
	$(Q)rm -f $(SYSTEM_BIN)/ir_demo_extra

