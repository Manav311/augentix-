SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

BIN_OUT_PATH = $(root)/bin
INC_PATH = $(root)/include
OBJ_PATH = $(root)/obj
SRC_PATH = $(root)/src
DEP_PATH = $(root)/.deps

# specify CFLAGS and LDLIBS
INC := \
	-isystem$(ZLIB_INC) \
	-isystem$(OPENSSL_INC) \
	-isystem$(FFMPEG_INC)

CFLAGS += $(INC) -I$(AUDIO_INC) -I$(ALSA_INC)
CFLAGS += -O2 -Wextra -Werror
LDFLAGS += \
    -L$(ZLIB_LIB) \
	-L$(OPENSSL_LIB) \
    -L$(SDKSRC_DIR)/buildroot/output/build/ffmpeg-4.3.1/libswresample \
    -L$(SDKSRC_DIR)/buildroot/output/build/ffmpeg-4.3.1/libswscale \
    -L$(SDKSRC_DIR)/buildroot/output/build/ffmpeg-4.3.1/libavutil \
	-L$(SDKSRC_DIR)/buildroot/output/build/ffmpeg-4.3.1/libavformat \
	-L$(SDKSRC_DIR)/buildroot/output/build/ffmpeg-4.3.1/libavcodec \
	-L$(ALSA_LIB)
	

LDLIBS += -lavformat -lavcodec -lswresample -lswscale -lavutil -lm -lz -lssl -lcrypto -lasound

BIN_NAME = dd_demo

BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

# objects
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(Q)$(MKDIR) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(Q)$(MKDIR) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CXX) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(OBJ_PATH) $(DEP_PATH) $(BIN_OUT_PATH) 

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all install-bin install-data

.PHONY: install-bin
install-bin: $(BIN_TARGET)

.PHONY: install-data
install-data:
	$(Q)mkdir -p $(USRDATAFS)/root
	$(Q)cp -f $(SRC_PATH)/target_ip_config $(USRDATAFS)/root/target_ip_config

$(BIN_TARGET): $(BIN_OUTPUT)
	$(Q)install -c $< $@

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

.PHONY: uninstall-bin
uninstall-bin:
	$(Q)$(RM) $(BIN_TARGET)

-include $(DEPS)
