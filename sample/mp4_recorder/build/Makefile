SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk

# specify CFLAGS, LDFLAGS and LDLIBS
INC := -I$(MPP_INC) $(addprefix -I,$(BENTO4_INC)) -I$(AUDIO_INC) -I$(ALSA_INC) $(addprefix -I,$(FDKAAC_INCS))
CFLAGS += $(INC)
CXXFLAGS += $(INC) -std=c++11 -Wno-unused-parameter -Wno-missing-field-initializers
LDFLAGS += -L$(MPP_LIB) -L$(BENTO4_LIB) -L$(ALSA_LIB) -L$(FDKAAC_LIB)
LDLIBS += -lstdc++ -lap4 -lmpp -lrt -lasound -lfdk-aac -pthread

# source codes, objects, and dependencies
SRCS = \
	$(wildcard $(SRC_PATH)/*.c) \
	$(wildcard $(SRC_PATH)/*.cc) \
	$(wildcard $(SRC_PATH)/*.cpp)
OBJS = \
	$(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o, \
	$(patsubst $(SRC_PATH)/%.cc,$(OBJ_PATH)/%.o, \
	$(patsubst $(SRC_PATH)/%.cpp,$(OBJ_PATH)/%.o,$(SRCS))))
DEPS = \
	$(patsubst $(SRC_PATH)/%.c,$(DEP_PATH)/%.d, \
	$(patsubst $(SRC_PATH)/%.cc,$(DEP_PATH)/%.d, \
	$(patsubst $(SRC_PATH)/%.cpp,$(DEP_PATH)/%.d,$(SRCS))))

# specify bin
BIN_NAME := mp4_recorder

# paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

# Delete all files created by building the app.
.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)

# Delete all files created by configuring, installing or building.
.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(BIN_TARGET)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
