SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# append CFLAGS
INC := -I$(LIBSAMPLE_INC) -I$(LIBMOTOR_INC) -isystem$(JSON_INC)
LIB := -L$(LIBSAMPLE_LIB) -l:libsample.a -L$(LIBMOTOR_LIB) -l:libmotor.a -L$(JSON_LIB) -ljson-c -lsample
CFLAGS += $(INC) 


BIN_NAME := motor_test

BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: clean

$(BIN_OUT_PATH)/motor_test: $(OBJ_PATH)/motor_test.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

	
-include $(DEPS)
