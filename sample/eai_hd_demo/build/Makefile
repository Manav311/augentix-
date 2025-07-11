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
	$(LIBINF_INC) \
	$(MPP_INC) \
	$(FEATURE_AUDIO_INC_PATH) \
	$(FEATURE_VIDEO_INC_PATH)

INC := -isystem$(JSON_INC) $(addprefix -isystem,$(CONFIG_INCS))
INC += $(addprefix -I,$(INCS_PATH))

SRCS := \
$(root)/src/avftr_conn.c \
$(root)/src/main.c

OBJS = \
$(patsubst $(SRC_PATH)/%.o, $(OBJ_PATH)/%.o, \
$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(SRCS))))

DEPS = $(OBJS:.o=.d)

LD_FLAGS = -Wl,--gc-sections -Wl,-rpath=$(LIBBROLTI_LIB)

ifeq ($(CONFIG_LIBINF_NCNN_MINIMAL), y)
LDLIBS := \
-L$(LIBINF_LIB) -l:libinf.a \
-L$(LIBTBB_LIB) -ltbb \
-L$(LIBCPPREACT_LIB) -lCppReact \
-L$(CONFIG_INF_LIBS) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-L$(ZLIB_LIB) -L$(MPP_LIB) -lmpp -lrt -ldl \
-lstdc++ \
-L$(JSON_LIB) -ljson-c -lm -pthread
else
LDLIBS := \
-L$(LIBINF_LIB) -linf \
-L$(CONFIG_INF_LIBS) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-L$(ZLIB_LIB) -L$(MPP_LIB) -lmpp -lrt -ldl \
-lstdc++ \
-L$(JSON_LIB) -ljson-c -lm  -pthread
endif

CFLAGS += $(CONFIG_CFLAGS) $(INC)
CXXFLAGS += $(CONFIG_CFLAGS) $(INC) -Wno-unused-parameter -Wno-missing-field-initializers

ifeq ($(CONFIG_APP_EAI_HD_SUPPORT_SEI),y)
CPPFLAGS += -DCONFIG_APP_HD_SUPPORT_SEI
endif

CPPFLAGS += -DLIBEAI

##################################################
# specify bin
BIN_NAME = eai_hd_demo

# bin paths
TARGET_CONFIG_PATH := $(SYSTEMFS)/mpp/hd_config
SRC_CONFIG_PATH := $(root)/assets
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

#############################################


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
all: $(BIN_OUTPUT)

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
install: all $(BIN_TARGET) install-config

.PHONY: uninstall
uninstall: uninstall-config
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LD_FLAGS) -ldl -Wl,--as-needed -Wl,--start-group $(LDLIBS)  -Wl,--end-group

.PHONY: check
check:
	@echo inf: $(INF_SRCS)
	@echo $(OBJ_PATH)
	@echo OBJS $(OBJS)
	@echo SRCS $(SRCS)
	@echo LDLIBS $(LDLIBS)

-include $(DEPS)
