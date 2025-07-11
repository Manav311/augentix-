SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../)
include $(SDKSRC_DIR)/build/sdksrc.mk
include $(SDKSRC_DIR)/application/internal.mk

root = $(realpath $(CURDIR)/..)

TARGET = flv_recorder
MAIN_C = $(root)/src/flv_recorder.c

HTTP_FLV_FILE_TEST = http_flv_file_test
HTTP_FLV_FILE_TEST_C = $(root)/src/http_flv_file_test.c

HTTPS_FLV_FILE_TEST = https_flv_file_test
HTTPS_FLV_FILE_TEST_C = $(root)/src/https_flv_file_test.c

JSON_PLAYER = json_player
JSON_PLAYER_C = $(root)/src/json_player.c

WS_SERVER = ws_server_http
WS_SERVER_C = $(root)/src/ws_server_http.c

CC    = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

CFLAGS =  -Wall -g -D_GNU_SOURCE -O2 -fno-omit-frame-pointer -std=gnu99 -MMD -MP -Wno-unused-variable -Wno-unused-function

TEST_SRC_PATH =  $(realpath $(CURDIR)/../../src)
TEST_SRC = $(TEST_SRC_PATH)/aac.c
TEST_SRC += $(TEST_SRC_PATH)/flv_muxer.c
TEST_SRC += $(TEST_SRC_PATH)/audio.c
TEST_SRC += $(TEST_SRC_PATH)/video.c

TEST_SRC += $(TEST_SRC_PATH)/http_flv.c
TEST_SRC += $(TEST_SRC_PATH)/https_flv.c

TEST_SRC += $(TEST_SRC_PATH)/wss_msg.c
TEST_SRC += $(TEST_SRC_PATH)/wss_server.c

TEST_INC = $(TEST_SRC_PATH)/inc/

CONTENT_SRC = 
CONTENT_INC = $(root)/src/inc


INC =  -I$(MPP_INC) -I$(TEST_INC) -I$(CONTENT_INC) -I$(AUDIO_INC) -I$(ALSA_INC) $(addprefix -I,$(FDKAAC_INCS))
INC += -I$(JSON_INC) -I$(LIBWEBSOCKETS_INC) -I$(LIBEV_INC) -I$(OPENSSL_INC) -I$(ZLIB_INC)
LIB =  -L$(MPP_LIB) -L$(PACKAGE_LIB) -L$(APP_LIB) -L$(ALSA_LIB) -L$(FDKAAC_LIB)
LIB += -L$(JSON_LIB) -L$(LIBWEBSOCKETS_LIB) -L$(LIBEV_LIB) -L$(OPENSSL_LIB) -L$(ZLIB_LIB)
LDFLAGS = -lmpp -lm -pthread -lasound -lfdk-aac -ljson-c -lwebsockets -lev -lssl -lz -lcrypto
SRC = $(TEST_SRC) $(CONTENT_SRC)

all: $(TARGET) $(HTTP_FLV_FILE_TEST) $(HTTPS_FLV_FILE_TEST) $(JSON_PLAYER) $(WS_SERVER)

$(TARGET) :
	$(Q)$(CC) -o $(TARGET) $(MAIN_C) $(SRC) $(CFLAGS) $(LDFLAGS) $(INC) $(LIB)

$(HTTP_FLV_FILE_TEST) :
	$(Q)$(CC) -o $(HTTP_FLV_FILE_TEST) $(HTTP_FLV_FILE_TEST_C) $(SRC) $(CFLAGS) $(LDFLAGS) $(INC) $(LIB)

$(HTTPS_FLV_FILE_TEST) :
	$(Q)$(CC) -o $(HTTPS_FLV_FILE_TEST) $(HTTPS_FLV_FILE_TEST_C) $(SRC) $(CFLAGS) $(LDFLAGS) $(INC) $(LIB)

$(JSON_PLAYER):
	$(Q)$(CC) -o $(JSON_PLAYER) $(JSON_PLAYER_C) $(SRC) $(CFLAGS) $(LDFLAGS) $(INC) $(LIB)

$(WS_SERVER):
	$(Q)$(CC) -o $(WS_SERVER) $(WS_SERVER_C) $(SRC) $(CFLAGS) $(LDFLAGS) $(INC) $(LIB)

clean:
	rm -f $(TARGET)* $(HTTP_FLV_FILE_TEST)* $(HTTPS_FLV_FILE_TEST)* $(JSON_PLAYER)* $(WS_SERVER)*





