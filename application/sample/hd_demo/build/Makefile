SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)
include eaif.mk
include inf.mk

# specify CFLAGS and LDLIBS
INCS_PATH := \
	$(LIBAVFTR_INC) \
	$(LIBEAIF_INC) \
	$(MPP_INC) \
	$(FEATURE_AUDIO_INC_PATH) \
	$(FEATURE_VIDEO_INC_PATH)

INC := -isystem$(JSON_INC) $(addprefix -isystem,$(CONFIG_INCS))
INC += $(addprefix -I,$(INCS_PATH))

SRCS := \
$(root)/src/avftr_conn.c \
$(root)/src/hd_demo.c \
$(root)/src/main.c

OBJS = \
$(patsubst $(SRC_PATH)/%.o, $(OBJ_PATH)/%.o, \
$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SRCS))))

DEPS = $(OBJS:.o=.d)

LD_FLAGS = -Wl,--gc-sections -Wl,-rpath=$(LIBBROLTI_LIB)
#LDLIBS := \
#$(CONFIG_LIBS) -L$(CONFIG_INF_LIBS)

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDLIBS := -L$(LIBEAIF_LIB) -l:libeaif.a \
-L$(LIBINF_LIB) -l:libinf.a \
-L$(CONFIG_INF_LIBS) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-L$(ZLIB_LIB) -L$(MPP_LIB) -lmpp -lrt -ldl \
-lstdc++ \
-L$(JSON_LIB) -ljson-c -lm -pthread
else
LDLIBS := -L$(LIBEAIF_LIB) -leaif \
-L$(LIBINF_LIB) -linf \
-L$(CONFIG_INF_LIBS) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-L$(ZLIB_LIB) -L$(MPP_LIB) -lmpp -lrt -ldl \
-lstdc++ \
-L$(JSON_LIB) -ljson-c -lm -pthread
endif

LDLIBS += $(CONFIG_LIBS)

CFLAGS += $(CONFIG_CFLAGS) $(INC)
CXXFLAGS += $(CONFIG_CFLAGS) $(INC)

ifeq ($(CONFIG_APP_HD_SUPPORT_SEI),y)
CPPFLAGS += -DCONFIG_APP_HD_SUPPORT_SEI
endif

##################################################
# specify bin
BIN_NAME = hd_demo

# bin paths
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/hd_config
SRC_CONFIG_PATH := $(root)/assets
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

#############################################

# specify score bin, hd_score only need inference src from libeaif

BIN_SCORE_NAME = hd_score

SCORE_SRC := $(root)/src/hd_score.c
SCORE_OBJ := $(root)/obj/hd_score.o
SCORE_INC := \
	-isystem$(JSON_INC) \
	$(addprefix -isystem,$(CONFIG_INF_INCS)) \
	-I$(MPP_INC) \
	-I$(LIBINF_INC)

SCORE_CFLAGS := $(CFLAGS) $(SCORE_INC)

SCORE_LIB := -L$(FEATURE_VIDEO_LIB_PATH) -lvftr

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
SCORE_LIB += -L$(LIBINF_LIB) -l:libinf.a
else
SCORE_LIB += -L$(LIBINF_LIB) -linf
endif

SCORE_LIB += -L$(MPP_LIB) -lmpp -ldl -lrt -lstdc++ -lm -pthread

BIN_SCORE_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_SCORE_NAME))
BIN_SCORE_TARGET := $(addprefix $(bindir)/,$(BIN_SCORE_NAME))

DEPS += $(SCORE_OBJ:.o=.d)

#############################################

## testing rule
include ../test/Rule.mk

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cc $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<


#############################################

# sepcify targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT) score

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

.PHONY: distclean
distclean: clean

.PHONY: install-config
install-config: 
	@printf "  %-8s$(TARGET_CONFIG_PATH)/*\n" "INSTALL"
	$(Q)$(MKDIR_P) $(TARGET_CONFIG_PATH)
	$(Q)$(foreach f, $(wildcard $(SRC_CONFIG_PATH)/*), \
		cp -r $(f) $(TARGET_CONFIG_PATH)/$(notdir $(f)); \
	)

.PHONY: uninstall-config
uninstall-config:
	@printf "  %-8s$(TARGET_CONFIG_PATH)\n" "RM"
	$(Q)rm -rf $(TARGET_CONFIG_PATH)

.PHONY: install 
install: all $(BIN_TARGET) $(BIN_SCORE_TARGET) install-config

.PHONY: uninstall
uninstall: uninstall-config
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LD_FLAGS) -Wl,--start-group $(LDLIBS)  -Wl,--end-group

.PHONY: score
score: $(BIN_SCORE_OUTPUT)

$(BIN_SCORE_OUTPUT): CFLAGS := $(SCORE_CFLAGS) $(SCORE_LIB)

$(BIN_SCORE_OUTPUT): $(INF_OBJS) $(SCORE_OBJ)
	$(Q)$(MKDIR_P) $(dir $@)
	$(Q)$(CC) -o $@ $^ $(CFLAGS) $(CPPFLAGS) -L$(JSON_LIB) -ljson-c -L$(CONFIG_INF_LIBS)

.PHONY: check
check:
	@echo $(BIN_SCORE_OUTPUT)  src: $(SCORE_SRC) inf: $(INF_SRCS)
	@echo $(OBJ_PATH) $(SRC_PATH)
	@echo OBJS $(OBJS)
	@echo SRCS $(SRCS)
	@echo LDLIBS $(LDLIBS)

.PHONY: test

-include $(DEPS)
