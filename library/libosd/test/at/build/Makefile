SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../../)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)


INC := -isystem$(JSON_INC) -isystem$(FREETYPE_INC) -isystem$(LIBPNG_INC) -isystem$(ZLIB_INC)
INC += -I$(LIBOSD_INC) -I$(MPP_INC)
LDFLAGS := -l:libosd.a -l:libfreetype.a -l:libpng16.a -lm -lrt -lmpp -lz -ljson-c
LIB := -L$(FREETYPE_LIB) -L$(ZLIB_LIB) -L$(LIBOSD_LIB) -L$(LIBPNG_LIB) $(LDFLAGS)
LIB += -L$(MPP_LIB) -L$(JSON_LIB)
CFLAGS += $(INC)
CFLAGS += -Wno-used-function -pthread

ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
INC += -isystem$(SDL2_INC) -isystem$(SDL2TTF_INC) 
LDFLAGS += -lSDL2 -lSDL2_ttf
LIB += -L$(SDL2_LIB) -L$(SDL2TTF_LIB) 
DEFS += SDL_ENABLE
else
endif

ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
CFLAGS += $(addprefix -D,$(DEFS))
endif


ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
BIN_NAME :=  at_text_ayuv at_text_outline at_rgba2ayuv at_bmp2ayuv at_groupNoprio at_drawLine at_add_del_region
BIN_NAME += at_run_osd_normal at_run_osd_grouping at_run_osd_grouping_overlap
BIN_NAME += at_align_ayuv at_sample_8_bit at_sample_move_mode2 at_src_output
else
BIN_NAME := at_src_output at_unicode_list at_align_ayuv
endif

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



$(BIN_OUT_PATH)/at_unicode_list: $(OBJ_PATH)/at_unicode_list.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)


$(BIN_OUT_PATH)/at_src_output: $(OBJ_PATH)/at_src_output.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_text_ayuv: $(OBJ_PATH)/at_text_ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_rgba2ayuv: $(OBJ_PATH)/at_rgba2ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_run_osd_normal: $(OBJ_PATH)/at_run_osd_normal.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_run_osd_grouping: $(OBJ_PATH)/at_run_osd_grouping.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_run_osd_grouping_overlap: $(OBJ_PATH)/at_run_osd_grouping_overlap.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_bmp2ayuv: $(OBJ_PATH)/at_bmp2ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_groupNoprio: $(OBJ_PATH)/at_groupNoprio.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_drawLine: $(OBJ_PATH)/at_drawLine.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_text_outline: $(OBJ_PATH)/at_text_outline.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_add_del_region: $(OBJ_PATH)/at_add_del_region.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_align_ayuv: $(OBJ_PATH)/at_align_ayuv.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_sample_8_bit: $(OBJ_PATH)/at_sample_8_bit.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/at_sample_move_mode2: $(OBJ_PATH)/at_sample_move_mode2.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)


-include $(DEPS)