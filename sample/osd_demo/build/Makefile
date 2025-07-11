SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INC := -isystem$(FREETYPE_INC) -isystem$(JSON_INC) -isystem$(ZLIB_INC)
INC += -I$(LIBOSD_INC) -I$(MPP_INC)

LDFLAGS := -losd -lz -lfreetype -lm -lrt -lmpp -ljson-c -Wl,--gc-sections

ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
INC += -isystem$(SDL2_INC) -isystem$(SDL2TTF_INC) -isystem$(LIBPNG_INC)
LDFLAGS += lSDL2_ttf -lSDL2 -lpng16
endif


LIB := -L$(FREETYPE_LIB) -L$(ZLIB_LIB)  -L$(LIBOSD_LIB) $(LDFLAGS)
ifeq ($(CONFIG_LIBOSD_USE_FREETYPE), n)
LIB += -L$(SDL2_LIB) -L$(SDL2TTF_LIB) -L$(LIBPNG_LIB)
endif

LIB += -L$(MPP_LIB) -L$(JSON_LIB)
CFLAGS += -pthread $(INC)

BIN_NAME := osd_demo
INSTALL_BIN_NAME := osd_demo

ifeq ($(CONFIG_APP_OSD_DEMO_RGBA2AYUV),y)
INSTALL_BIN_NAME += osd_rgba2ayuv
endif
ifeq ($(CONFIG_APP_OSD_DEMO_LINE_DYNAMIC),y)
INSTALL_BIN_NAME += osd_drawLine_dynamic
endif

BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(INSTALL_BIN_NAME))

ASSETS_TARGET_PATH := $(root)/assets
ASSETS_PATH := $(SYSTEMFS)/mpp/font
ASSETS := LOGO_Augentix_v2.imgayuv
ifeq ($(CONFIG_APP_OSD_DEMO_INSTALL_SIMPLIFIED_CHINESE), y)
ASSETS += SourceHanSansCN-Regular_1.otf
else
ASSETS += SourceHanSansTC-Normal.otf
endif


ASSETS_SRC := $(addprefix $(ASSETS_TARGET_PATH)/,$(ASSETS))
ASSETS_TARGET := $(addprefix $(ASSETS_PATH)/,$(ASSETS))

CONFIG_TARGET_PATH := $(root)/config
CONFIG_PATH := $(SYSTEMFS)/mpp/osd_config
CONFIGS := \
	normal_fhd.json normal_fhd_chn1.json text_outline_fhd.json \
	grouping_overlap.json drawLine_fhd.json grouping_chn0.json \
	sample_move.json sample_8bit.json text_outline_fhd_CN.json
CONFIG_TARGET := $(addprefix $(CONFIG_PATH)/,$(CONFIGS))

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

.PHONY: install
install: all $(BIN_TARGET) install-assets install-configs
	@printf "  %-8s$(bindir)/*\n" "INSTALL"

.PHONY: uninstall
uninstall: uninstall-assets uninstall-configs
	$(Q)$(RM) -f $(BIN_TARGET)

.PHONY: install-assets
install-assets: 
	@printf "  %-8s$(ASSETS_PATH)/*\n" "INSTALL"
	$(Q)$(MKDIR_P) $(ASSETS_PATH)
	$(foreach f, $(wildcard $(ASSETS_SRC)), \
		cp -r $(f) $(ASSETS_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-assets
uninstall-assets:
	@printf "  %-8s$(ASSETS_TARGET)\n" "RM"
	$(Q)$(RM) -f $(ASSETS_TARGET)

.PHONY: install-configs
install-configs:
	@printf "  %-8s$(CONFIG_PATH)/\n" "INSTALL"
	$(Q)$(MKDIR_P) $(CONFIG_PATH)
	$(Q)$(foreach f, $(CONFIGS), \
		cp -r $(CONFIG_TARGET_PATH)/$(f) $(CONFIG_PATH)/$(f); \
	)


.PHONY: uninstall-configs
uninstall-configs:
	@printf "  %-8s$(CONFIG_TARGET)\n" "RM"
	$(Q)$(RM) -f $(CONFIG_TARGET)


$(BIN_OUT_PATH)/osd_text_ayuv: $(OBJ_PATH)/osd_text_ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_rgba2ayuv: $(OBJ_PATH)/osd_rgba2ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_bmp2ayuv: $(OBJ_PATH)/osd_bmp2ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_groupNoprio: $(OBJ_PATH)/osd_groupNoprio.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_drawLine_dynamic: $(OBJ_PATH)/osd_drawLine_dynamic.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_normal: $(OBJ_PATH)/osd_normal.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_demo: $(OBJ_PATH)/osd_demo.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/osd_merge_array: $(OBJ_PATH)/osd_merge_array.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

-include $(DEPS)
