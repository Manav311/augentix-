SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)
include inf.mk

# specify CFLAGS, LDFLAGS and LDLIBS
INC := \
	-isystem$(JSON_INC) \
	-I$(LIBAVFTR_INC) \
	-I$(LIBEAIF_INC) \
	-I$(LIBINF_INC) \
	-I$(MPP_INC) \
	-I$(FEATURE_AUDIO_INC_PATH) \
	-I$(FEATURE_VIDEO_INC_PATH)

SRCS := \
	$(SRC_PATH)/avftr_conn.c \
	$(SRC_PATH)/facedet_demo.c \
	$(SRC_PATH)/main.c \
	$(SRC_PATH)/utils.c

CFLAGS += $(INC) $(FACEDET_IMG_INC)
LDFLAGS += \
	-L$(CONFIG_INF_LIB_PATH) \
	-L$(JSON_LIB) \
	-L$(LIBINF_LIB) \
	-L$(MPP_LIB) \
	-L$(FEATURE_VIDEO_LIB_PATH) \
	-Wl,--gc-sections

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDLIBS += -l:libinf.a
else
LDLIBS += -linf
endif

LDLIBS += \
	-lstdc++ \
	$(CONFIG_INF_LIB) \
	-ldl \
	-ljson-c \
	-lm \
	-lmpp \
	-lrt \
	-lvftr \
	-pthread

ifeq ($(CONFIG_APP_FACEDET_SUPPORT_SEI),y)
CPPFLAGS += -DCONFIG_APP_FACEDET_SUPPORT_SEI
endif

FACEDET_IMG_INC := \
	$(addprefix -isystem,$(CONFIG_INF_INCS)) \
	-I$(LIBINF_INC) \
	-I$(MPP_INC)
FACEDET_IMG_LIB := $(LDFLAGS) $(LDLIBS)
FACEDET_IMG_OBJS := $(OBJ_PATH)/facedet_image.o
FACEDET_IMG_DEPS := $(DEP_PATH)/facedet_image.d
DEPS += $(FACEDET_IMG_DEPS)

# specify bins
BIN_NAME := facedet_demo
BIN_FACEDET_IMG_NAME := facedet_image

# paths
TEST_PATH := $(root)/test
SRC_CONFIG_PATH := $(root)/assets
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/facedet_config
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))
BIN_FACEDET_IMG_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_FACEDET_IMG_NAME))
BIN_FACEDET_IMG_TARGET := $(addprefix $(bindir)/,$(BIN_FACEDET_IMG_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT) $(BIN_FACEDET_IMG_OUTPUT)

# Delete all files created by building the app.
.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all install-bin install-config

.PHONY: install-bin
install-bin: $(BIN_TARGET) $(BIN_FACEDET_IMG_TARGET)

.PHONY: install-config
install-config: 
	@printf "  %-8s$(TARGET_CONFIG_PATH)/*\n" "INSTALL"
	$(Q)$(MKDIR_P) $(TARGET_CONFIG_PATH)
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIG_PATH)/*), \
		cp -r $(f) $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall
uninstall: uninstall-bin uninstall-config

PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(BIN_TARGET)

.PHONY: uninstall-config
uninstall-config:
	@printf "  %-8s$(TARGET_CONFIG_PATH)\n" "RM"
	$(Q)$(RM_RF) $(TARGET_CONFIG_PATH)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BIN_FACEDET_IMG_OUTPUT): $(FACEDET_IMG_OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(FACEDET_IMG_LIB)

.PHONY: check
check:
	@echo $(OBJ_PATH) $(SRC_PATH)
	@echo CFLAGS $(CFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo SRCS $(SRCS)
	@echo OBJS $(OBJS)
	@echo DEPS $(DEPS)
	@echo FACEDET_IMG_LIB $(FACEDET_IMG_LIB)
	@echo LDLIBS $(LDLIBS)

-include $(TEST_PATH)/Rule.mk
-include $(DEPS)
