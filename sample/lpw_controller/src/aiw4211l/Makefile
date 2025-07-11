
CURDIR ?= $(shell pwd)
CC := $(CROSS_COMPILE)gcc

SAMPLE_IMAGES := lpw_controller

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

CFLAGS = -Wno-declaration-after-statement -funsigned-char -freg-struct-return -fno-strict-aliasing -Wtrampolines -Wformat=2 -Wshadow -Wall -fpic -fpie -fno-common -Wfloat-equal -Wextra -fvisibility=hidden
CFLAGS += -D_FORTIFY_SOURCE=2 -O2 -pipe
#规范建议：在Debug版本中实施，Release版本不实施
#CFLAGS += -ftrapv

ifeq ($(CONFIG_LPW_AIW4211L),y)
CFLAGS+=-DLPW_AIW4211L
else
$(error No LPW module selected)
endif


CFLAGS += -I$(CURDIR)/ -I$(CURDIR)/../
CFLAGS += -D_GNU_SOURCE
$(warning $(CURDIR))
LDFLAGS := -Wl,-z,relro,-z,now -Wl,-z,noexecstack -pie -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined

SRCS = $(wildcard $(CURDIR)/../augentix/*.c)
SAMPLE_OBJ := socchannel_main.o socchannel_host.o socchannel_host_comm.o
SAMPLE_OBJ += $(SRCS:.c=.o)

DEPEND_LIBS := -lpthread
.PHONY: all clean

all: $(SAMPLE_OBJ)
	$(CC) $(LDFLAGS) -o $(SAMPLE_IMAGES) $^ $(LIBPATH) $(DEPEND_LIBS)
	-@mkdir -p ../../bin/
	-@cp -f $(SAMPLE_IMAGES) ../../bin/$(SAMPLE_IMAGES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	-@rm -f $(SAMPLE_IMAGES)
	-@rm -f $(SAMPLE_OBJ)
	-@rm -rf ../../bin
