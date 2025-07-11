SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)


LIBOSD_DEPENDS_INC := $(LIBOSD_INC) \
	   $(ZLIB_INC) \
	   $(LIBPNG_INC) \
	   $(SDL2_INC) \
	   $(SDL2TTF_INC) \
	   $(FREETYPE_INC)
LIBOSD_DEPENDS_LIB := $(LIBOSD_LIB) $(FREETYPE_LIB) $(ZLIB_LIB)
LIBOSD_DEPENDS_LDFLAG = osd z freetype

ifeq ($(CONFIG_LIBOSD_USE_FREETYPE),n)
LIBOSD_DEPENDS_INC += -isystem$(SDL2_INC) -isystem$(SDL2TTF_INC) -isystem$(LIBPNG_INC)
LIBOSD_DEPENDS_LIB += $(SDL2_LIB) $(SDL2TTF_LIB) $(LIBPNG_LIB)
LIBOSD_DEPENDS_LDFLAG += SDL2 SDL2_ttf png16
endif

LIBOSD_DEPENDS_INCS := $(addprefix -I,$(LIBOSD_DEPENDS_INC))
LIBOSD_DEPENDS_LIBS := $(addprefix -L,$(LIBOSD_DEPENDS_LIB))
LIBOSD_DEPENDS_LDFLAGS := $(addprefix -l,$(LIBOSD_DEPENDS_LDFLAG))