SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/.config

##### Change the following for your environment:
COMPILE_OPTS = $(INCLUDES) -I. -O2 -fPIC -fPIE -fno-omit-frame-pointer -Wall -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -mcpu=$(CONFIG_TARGET_CPU) -DALLOW_RTSP_SERVER_PORT_REUSE

### Disable -Werror for uClibc toolchain
ifneq ($(CROSS_COMPILE),arm-augentix-linux-uclibcgnueabihf-)
CFLAGS += -Werror
endif

# debugging flags
DEBUG ?= 0
ifeq ($(DEBUG),1)
COMPILE_OPTS += -g3
endif
C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
C_FLAGS =		$(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS)
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -DBSD=1 $(CPPFLAGS) $(CXXFLAGS)
ifeq ($(CONFIG_APP_RTSP_SERVER_ENABLE_IVA),y)
CPLUSPLUS_FLAGS += -DRTSP_SERVER_ENABLE_IVA
else
endif

OBJ =			o
LINK =			$(CPLUSPLUS_COMPILER) -o 
LINK_OPTS =		$(LDFLAGS) -L. -pthread
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		ar cr 
LIBRARY_LINK_OPTS = 
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
##### End of variables to change

LIVEMEDIA_DIR = liveMedia
GROUPSOCK_DIR = groupsock
USAGE_ENVIRONMENT_DIR = UsageEnvironment
BASIC_USAGE_ENVIRONMENT_DIR = BasicUsageEnvironment
TESTPROGS_DIR = testProgs
MEDIA_SERVER_DIR = mediaServer
PROXY_SERVER_DIR = proxyServer
AUX_DEBUG_UTILS = utils

.PHONY: all
all:
	cd $(LIVEMEDIA_DIR) ; $(MAKE)
	cd $(GROUPSOCK_DIR) ; $(MAKE)
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE)
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE)
	cd $(TESTPROGS_DIR) ; $(MAKE)
	cd $(MEDIA_SERVER_DIR) ; $(MAKE)
	cd $(PROXY_SERVER_DIR) ; $(MAKE)
	cd $(AUX_DEBUG_UTILS) ; $(MAKE)

.PHONY: install
install: all OnDemandRTSPServer-install rtspDebug-install

.PHONY: uninstall
uninstall: OnDemandRTSPServer-uninstall rtspDebug-uninstall

.PHONY: clean
clean:
	cd $(LIVEMEDIA_DIR) ; $(MAKE) clean
	cd $(GROUPSOCK_DIR) ; $(MAKE) clean
	cd $(USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(BASIC_USAGE_ENVIRONMENT_DIR) ; $(MAKE) clean
	cd $(TESTPROGS_DIR) ; $(MAKE) clean
	cd $(MEDIA_SERVER_DIR) ; $(MAKE) clean
	cd $(PROXY_SERVER_DIR) ; $(MAKE) clean
	cd $(AUX_DEBUG_UTILS) ; $(MAKE) clean

.PHONY: distclean
distclean: uninstall clean

TARGETS = testOnDemandRTSPServer
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS := $(addprefix $(DEST)/, $(TARGETS))

.PHONY: OnDemandRTSPServer-install
OnDemandRTSPServer-install:
	$(Q)install -d $(DEST)
	$(Q)install -m 777 $(LIVE555_PATH)/testProgs/testOnDemandRTSPServer $(INSTALL_TRGTS)

TARGET_DEBUG = rtspDebug
DEST = $(SYSTEM_BIN)
INSTALL_TRGTS_DEBUG := $(addprefix $(DEST)/, $(TARGET_DEBUG))


.PHONY: rtspDebug-install
rtspDebug-install:
	$(Q)install -d $(DEST)
	$(Q)install -m 777 $(LIVE555_PATH)/utils/rtspDebug $(INSTALL_TRGTS_DEBUG)

.PHONY: OnDemandRTSPServer-uninstall
OnDemandRTSPServer-uninstall:
	$(Q)rm -f $(INSTALL_TRGTS)

.PHONY: rtspDebug-uninstall
rtspDebug-uninstall:
	$(Q)rm -f $(INSTALL_TRGTS_DEBUG)


