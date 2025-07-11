SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

INC := -I"$(INC_PATH)" -iquote"$(SRC_INC_PATH)"
INC += -I$(MPP_INC) -I$(AUDIO_INC) -isystem$(ALSA_INC) -I$(APP_INC) \
	   $(addprefix -isystem,$(FDKAAC_INCS)) -isystem$(JSON_INC) -isystem$(LIBEV_INC) -isystem$(OPENSSL_INC) -isystem$(ZLIB_INC)

# specify CFLAGS, LDFLAGS and LDLIBS
CFLAGS += -D_GNU_SOURCE -Wl,-rpath=$(LIBPCRE_LIB) -Wl,-rpath=$(LIBGLIB2_LIB) -pthread
LDFLAGS += -L$(MPP_LIB) -L$(PACKAGE_LIB) -L$(APP_LIB) -L$(ALSA_LIB) -L$(FDKAAC_LIB) -L$(JSON_LIB) -L$(LIBEV_LIB) -L$(OPENSSL_LIB) -L$(ZLIB_LIB)
LDLIBS += -lmpp -lm -lasound -lfdk-aac -lpthread -ljson-c -lev -lssl -lz -lcrypto

SRC_DIR := $(realpath $(CURDIR)/../src)  
SRC_FILES := $(wildcard $(CURDIR)/../src/*.c)

ifeq ($(CONFIG_APP_SUPPORT_HTTPS), y)
CFLAGS += -DSUPPORT_HTTPS
SRC_DIR += $(realpath $(CURDIR)/../src/https)
SRC_FILES += $(wildcard $(CURDIR)/../src/https/*.c)

ifeq ($(CONFIG_APP_WEBSOCKET_AUDIO_PLAYBACK), y)
CFLAGS += -DWEBSOCKET_AUDIO_PLAYBACK
INC += -isystem$(LIBWEBSOCKETS_INC)
LDFLAGS += -L$(LIBWEBSOCKETS_LIB)
LDLIBS += -lwebsockets
SRC_DIR += $(realpath $(CURDIR)/../src/wss_audio)
SRC_FILES += $(wildcard $(CURDIR)/../src/wss_audio/*.c)
endif

endif

OBJS := $(patsubst %.c,%.o,$(SRC_FILES))

CFLAGS += $(INC)

# specify bin
BIN_NAME := flv_server

# paths
BIN_OUTPUT := $(addprefix $(BIN_OUT_PATH)/,$(BIN_NAME))
BIN_TARGET := $(addprefix $(bindir)/,$(BIN_NAME))

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: $(BIN_OUTPUT)

.PHONY: clean
clean:
	$(Q)$(RM) -r $(BIN_OUT_PATH) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(RM) $(OBJS)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(BIN_TARGET)

.PHONY: uninstall
uninstall:
	$(Q)$(RM) $(BIN_TARGET)

$(BIN_OUTPUT): $(OBJS)
	$(Q)$(MKDIR_P) $(BIN_OUT_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS)
