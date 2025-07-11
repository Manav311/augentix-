SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk
include $(CURDIR)/gtk3-pkgconfig.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

CC := $(CROSS_COMPILE)gcc
INC := \
	-isystem$(ATK_HOST_INC) \
	-isystem$(ATK_INC) \
	-isystem$(BUILDROOT_HOST_INC) \
	-isystem$(BUILDROOT_TARGET_LIB)/glib-2.0/include \
	-isystem$(BUILDROOT_USR_INC)/atk-1.0 \
	-isystem$(BUILDROOT_USR_INC)/cairo \
	-isystem$(BUILDROOT_USR_INC)/freetype2 \
	-isystem$(BUILDROOT_USR_INC)/fribidi \
	-isystem$(BUILDROOT_USR_INC)/gdk-pixbuf-2.0 \
	-isystem$(BUILDROOT_USR_INC)/gio-unix-2.0 \
	-isystem$(BUILDROOT_USR_INC)/glib-2.0 \
	-isystem$(BUILDROOT_USR_INC)/gtk-3.0 \
	-isystem$(BUILDROOT_USR_INC)/harfbuzz \
	-isystem$(BUILDROOT_USR_INC)/libdrm \
	-isystem$(BUILDROOT_USR_INC)/libpng16 \
	-isystem$(BUILDROOT_USR_INC)/pango-1.0 \
	-isystem$(BUILDROOT_USR_INC)/pixman-1 \
	-isystem$(BUILDROOT_USR_INC)/uuid \
	-isystem$(CAIRO_INC) \
	-isystem$(FREETYPE_INC) \
	-isystem$(FRIBIDI_INC) \
	-isystem$(GDK_PIXBUF_HOST_INC) \
	-isystem$(GDK_PIXBUF_INC) \
	-isystem$(GIO_UNIX_INC) \
	-isystem$(GLIB_HOST_INC) \
	-isystem$(GLIB_INC) \
	-isystem$(GLIB_MODULE_INC) \
	-isystem$(GLIB_USR_INC) \
	-isystem$(GTK3_INC) \
	-isystem$(HARFBUZZ_INC) \
	-isystem$(LIBDRM_INC) \
	-isystem$(LIBPNG_INC) \
	-isystem$(PANGO_HOST_INC) \
	-isystem$(PANGO_INC) \
	-isystem$(PIXMAN_INC)

LIB := -L$(GTK3_LIB) -L$(GTK3_GDK_LIB) -L$(PANGO_LIB) -L$(GLIB_LIB) \
 -L$(GOBJECT_LIB) -L$(HARFBUZZ_LIB) \
 -L$(FRIBIDI_LIB) -L$(CAIRO_LIB) -L$(CAIRO_GOBJECT_LIB) \
 -L$(PIXMAN_LIB) -L$(LIBDRM_LIB) -L$(GDK_PIXBUF_LIB) -L$(GIO_UNIX_LIB) -L$(ATK_LIB) \
 -L$(LIBPNG_LIB) -L$(FREETYPE_LIB) -L$(BUILDROOT_TARGET_LIB) \
 -lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz \
 -latk-1.0 -lcairo-gobject -lcairo -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lpng16 \
 -lXi -lX11 -lXinerama -lXrandr -lGL -lEGL -lxcb -lxcb-shm -lxcb-render -lXrender -lffi \
 -lXcursor -lXcomposite -lXdamage -lXfixes -lpixman-1 -lXext -lXau -lXdmcp \
 -lepoxy -lfribidi  -lfontconfig -lexpat -luuid -lfreetype \
 -lz -lbrotlidec -lpcre -lglapi -ldrm -lxcb-glx -lX11-xcb -lxcb-dri2 -lXxf86vm \
 -lgbm -lxcb-xfixes -lbrotlicommon -lpangoft2-1.0 -lgmodule-2.0 -lgdk_pixbuf-2.0

CFLAGS += -Wno-extra -pthread $(INC)

GTK3DEMOFS = $(SYSTEM_BIN)/gtk3_demo
ASSETS_TARGET_PATH := $(root)/assets

# Install logo image
LOGO_IMAGE_ASSETS = Augentix_Logo_282x45.png
LOGO_IMAGE_ASSETS_TARGET_PATH := $(ASSETS_TARGET_PATH)/image/
LOGO_IMAGE_ASSETS_PATH := $(GTK3DEMOFS)/image/
LOGO_IMAGE_ASSETS_SRC := $(addprefix $(LOGO_IMAGE_ASSETS_TARGET_PATH)/,$(LOGO_IMAGE_ASSETS))
LOGO_IMAGE_ASSETS_TARGET := $(addprefix $(LOGO_IMAGE_ASSETS_PATH)/,$(LOGO_IMAGE_ASSETS))

# Install number image
NUMBER_IMAGE_ASSETS = 0.jpg 1.jpg 2.jpg 3.jpg 4.jpg 5.jpg 6.jpg 7.jpg 8.jpg 9.jpg
NUMBER_IMAGE_ASSETS_TARGET_PATH := $(ASSETS_TARGET_PATH)/image/number
NUMBER_IMAGE_ASSETS_PATH := $(GTK3DEMOFS)/image/number
NUMBER_IMAGE_ASSETS_SRC := $(addprefix $(NUMBER_IMAGE_ASSETS_TARGET_PATH)/,$(NUMBER_IMAGE_ASSETS))
NUMBER_IMAGE_ASSETS_TARGET := $(addprefix $(NUMBER_IMAGE_ASSETS_PATH)/,$(NUMBER_IMAGE_ASSETS))

# Install Fonts
FONTS_ASSETS = LiberationSans-BoldItalic.ttf
FONTS_ASSETS_TARGET_PATH := $(ASSETS_TARGET_PATH)/liberation/
FONTS_ASSETS_PATH := $(USRDATAFS)/root/.fonts/liberation
FONTS_ASSETS_SRC := $(addprefix $(FONTS_ASSETS_TARGET_PATH)/,$(FONTS_ASSETS))
FONTS_ASSETS_TARGET := $(addprefix $(FONTS_ASSETS_PATH)/,$(FONTS_ASSETS))

#Install .ui
UI_ASSETS = gtk3_demo_gui.ui
UI_ASSETS_TARGET_PATH = $(ASSETS_TARGET_PATH)
UI_ASSETS_PATH = $(GTK3DEMOFS)
UI_ASSETS_SRC := $(addprefix $(UI_ASSETS_TARGET_PATH)/,$(UI_ASSETS))
UI_ASSETS_TARGET := $(addprefix $(UI_ASSETS_PATH)/,$(UI_ASSETS))


BIN_OUT_PATH = $(SDKSRC_DIR)/application/sample/gtk3_demo_gui/bin
BIN_NAME = gtk3-simple-demo car-test gtk3_demo_gui
INSTALL_BIN_NAME :=  gtk3_demo_gui
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(INSTALL_BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: clean uninstall

.PHONY: install
install: all install-assets
	$(Q)install -m 777 $(BIN_OUT_PATH)/$(INSTALL_BIN_NAME) $(SYSTEM_BIN)/$(INSTALL_BIN_NAME)

.PHONY: uninstall
uninstall: uninstall-assets
	$(Q)$(RM) -f $(SYSTEM_BIN)/$(INSTALL_BIN_NAME)

.PHONY: install-assets
install-assets: 
	$(Q)$(MKDIR_P) $(UI_ASSETS_PATH)
	$(Q)$(MKDIR_P) $(LOGO_IMAGE_ASSETS_PATH)
	$(Q)$(MKDIR_P) $(NUMBER_IMAGE_ASSETS_PATH)
	$(Q)$(MKDIR_P) $(FONTS_ASSETS_PATH)
	@printf "  %-8s$(UI_ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(UI_ASSETS_SRC)), \
		cp -r $(f) $(UI_ASSETS_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(LOGO_IMAGE_ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(LOGO_IMAGE_ASSETS_SRC)), \
		cp -r $(f) $(LOGO_IMAGE_ASSETS_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(NUMBER_IMAGE_ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(NUMBER_IMAGE_ASSETS_SRC)), \
		cp -r $(f) $(NUMBER_IMAGE_ASSETS_PATH)/$(notdir $(f)); \
	)
	@printf "  %-8s$(FONTS_ASSETS_PATH)/*\n" "INSTALL"
	$(foreach f, $(wildcard $(FONTS_ASSETS_SRC)), \
		cp -r $(f) $(FONTS_ASSETS_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-assets
uninstall-assets:
	@printf "  %-8s$(UI_ASSETS_TARGET)\n" "RM"
	@printf "  %-8s$(FONTS_ASSETS_TARGET)\n" "RM"
	@printf "  %-8s$(NUMBER_IMAGE_ASSETS_TARGET)\n" "RM"
	@printf "  %-8s$(LOGO_IMAGE_ASSETS_TARGET)\n" "RM"
	$(Q)$(RM) -f $(UI_ASSETS_TARGET)
	$(Q)$(RM) -f $(FONTS_ASSETS_TARGET)
	$(Q)$(RM) -f $(NUMBER_IMAGE_ASSETS_TARGET)
	$(Q)$(RM) -f $(LOGO_IMAGE_ASSETS_TARGET)



$(BIN_OUT_PATH)/gtk3_demo_gui: $(OBJ_PATH)/gtk3_demo_gui.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/gtk3-simple-demo: $(OBJ_PATH)/gtk3-simple-demo.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

$(BIN_OUT_PATH)/car-test: $(OBJ_PATH)/car-test.o
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LIB)

-include $(DEPS)
