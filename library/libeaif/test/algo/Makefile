SDK_PATH=../../../../..
APP_PATH=$(SDK_PATH)/application
EAIF_PATH=$(SDK_PATH)/application/library/libeaif
INF_PATH=$(SDK_PATH)/application/library/libinf
CC=gcc

INCS=\
-I$(INF_PATH)/src/inc \
-I$(EAIF_PATH)/src \
-I$(EAIF_PATH)/include \
-I$(SDK_PATH)/mpp/include


BINS=test
SRCS=ut.c \
$(wildcard $(EAIF_PATH)/src/*.c) \
$(wildcard $(EAIF_PATH)/test/src/fake/*.c)

CFLAGS=-Wall -g $(INCS) -std=gnu99 -D_GNU_SOURCE -pthread

.PHONY: all
all:
	$(CC) -o $(BINS) $(SRCS) $(CFLAGS) -Wno-unused

.PHONY: clean
clean:
	rm -rf $(BINS)
