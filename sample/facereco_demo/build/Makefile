SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)
include eaif.mk
include inf.mk

# specify CFLAGS and LDLIBS
INC := \
	-isystem$(JSON_INC) \
	$(addprefix -isystem,$(CONFIG_INCS)) \
	-I$(LIBAVFTR_INC) \
	-I$(LIBEAIF_INC) \
	-I$(LIBINF_INC) \
	-I$(MPP_INC) \
	-I$(FEATURE_AUDIO_INC_PATH) \
	-I$(FEATURE_VIDEO_INC_PATH)

SRCS := \
	$(SRC_PATH)/avftr_conn.c \
	$(SRC_PATH)/facereco_demo.c \
	$(SRC_PATH)/main.c

CFLAGS += $(CONFIG_CFLAGS) $(INC) $(FRUTILS_INC)
LDFLAGS += \
	-L$(CONFIG_INF_LIBS) \
	-L$(JSON_LIB) \
	-L$(LIBEAIF_LIB) \
	-L$(LIBINF_LIB) \
	-L$(MPP_LIB) \
	-L$(FEATURE_VIDEO_LIB_PATH) \
	-Wl,--gc-sections \
	-Wl,-rpath=$(LIBBROLTI_LIB)

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDLIBS := -l:libeaif.a -l:libinf.a
else
LDLIBS := -leaif -linf
endif

LDLIBS += \
	$(CONFIG_LIBS)\
	$(CONFIG_INF_LIB) \
	-lstdc++ \
	-ldl \
	-ljson-c \
	-lm \
	-lmpp \
	-lvftr \
	-lrt \
	-pthread

ifeq ($(CONFIG_APP_FACERECO_SUPPORT_SEI),y)
CPPFLAGS += -DCONFIG_APP_FACERECO_SUPPORT_SEI -DSEI_DRAW_ROI
endif

FRUTILS_INC := \
	$(addprefix -isystem,$(CONFIG_INF_INCS)) \
	-I$(LIBINF_INC) \
	-I$(MPP_INC)

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
FRUTILS_LIB := -L$(LIBINF_LIB) -l:libinf.a
else
FRUTILS_LIB := -L$(LIBINF_LIB) -linf
endif

FRUTILS_LIB += -L$(CONFIG_INF_LIBS) -lstdc++ -ldl -L$(JSON_LIB) -ljson-c -lm -lrt -L$(FEATURE_VIDEO_LIB_PATH) -lvftr -L$(MPP_LIB) -lmpp
FRUTILS_OBJS := $(OBJ_PATH)/facereco_utils.o
FRUTILS_DEPS := $(DEP_PATH)/facereco_utils.d
DEPS += $(FRUTILS_DEPS)

# specify bins
BIN_NAME := facereco_demo
BIN_FRUTILS_NAME := facereco_utils

# paths
SRC_CONFIG_PATH := $(root)/assets
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/facereco_config
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))
BIN_FRUTILS_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_FRUTILS_NAME))
BIN_FRUTILS_TARGET := $(addprefix $(bindir)/,$(BIN_FRUTILS_NAME))

# sepcify targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT) $(BIN_FRUTILS_OUTPUT)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: clean

.PHONY: install
install: all install-bin install-config

.PHONY: install-bin
install-bin: $(BIN_TARGET) $(BIN_FRUTILS_TARGET)

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

$(BIN_FRUTILS_OUTPUT): $(INF_OBJS) $(FRUTILS_OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(FRUTILS_LIB)

.PHONY: check
check:
	@echo $(BIN_SCORE_OUTPUT)  src: $(SCORE_SRC) inf: $(INF_SRCS)
	@echo $(OBJ_PATH) $(SRC_PATH)
	@echo CFLAGS $(CFLAGS)
	@echo CPPFLAGS $(CPPFLAGS)
	@echo SRCS $(SRCS)
	@echo OBJS $(OBJS)
	@echo DEPS $(DEPS)

-include $(DEPS)
