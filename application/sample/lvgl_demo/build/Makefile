SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk


# specify CFLAGS and LDLIBS
INC := \
	-isystem$(ZLIB_INC) \
	-isystem$(LIBPNG_INC) \
	-isystem$(FREETYPE_INC) \
	-isystem$(LVGL_INC) \
	-isystem$(LV_DRIVERS_INC) \
	-isystem$(BUILDROOT_OUTPUT_STAGING_PATH)/usr/include \
	-isystem$(BUILDROOT_OUTPUT_STAGING_PATH)/usr/include/lvgl
CFLAGS += $(INC)
CFLAGS += -O2 -Wextra -Werror -DLV_USE_DEMO_WIDGETS=1
CFLAGS := $(filter-out -Wextra, $(CFLAGS))
LDFLAGS += \
	-L$(ZLIB_LIB) \
	-L$(LIBPNG_LIB) \
	-L$(FREETYPE_LIB) \
	-L$(LVGL_LIB) \
	-L$(LV_DRIVERS_LIB)
LDLIBS += -lm -lz -lpng16 -lfreetype -llvgl -llv_drivers

BIN_NAME = lvgl_demo
BMP_NAME = Augentix32C_TD.bmp

BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))
BMP_TARGET := $(addprefix $(bindir)/,$(BMP_NAME))

root = $(realpath $(CURDIR)/..)
SRC_ASSETS_PATH    := $(root)/assets
LFB_PATH_BUILD := $(SRC_ASSETS_PATH)/linux_frame_buffer/build

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)
	make -C $(LFB_PATH_BUILD)

.PHONY: clean
clean:
	$(Q)echo -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)
	make -C $(LFB_PATH_BUILD) clean

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(BIN_TARGET) install-assets
	make -C $(LFB_PATH_BUILD) install

.PHONY: install-assets
install-assets:
	cp -af $(SRC_ASSETS_PATH)/$(BMP_NAME) $(BMP_TARGET)


.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET) $(BMP_TARGET)
	make -C $(LFB_PATH_BUILD) uninstall
	
$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
