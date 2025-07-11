SDKSRC_DIR = $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/.config

INCLUDES := -I../UsageEnvironment/include -I../groupsock/include -I../liveMedia/include -I../BasicUsageEnvironment/include
INCLUDES += -I$(MPP_INC) -I$(SENSOR_PATH) -I$(AUDIO_INC) -I$(ALSA_INC) -I$(UAPP_INC) \
            -I$(LIBCURL_INC) -I$(CENTCTRL_PATH)/include -I$(APP_INC) -I$(JSON_INC)
ifeq ($(CONFIG_APP_RTSP_SERVER_ENABLE_IVA),y)
INCLUDES += -I$(LIBAVFTR_INC) -I$(LIBEAIF_INC) -I$(FEATURE_AUDIO_INC_PATH) -I$(FEATURE_VIDEO_INC_PATH)
else
endif

# Default library filename suffixes for each library that we link with.  The "config.*" file might redefine these later.
libliveMedia_LIB_SUFFIX = $(LIB_SUFFIX)
libBasicUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libUsageEnvironment_LIB_SUFFIX = $(LIB_SUFFIX)
libgroupsock_LIB_SUFFIX = $(LIB_SUFFIX)
##### Change the following for your environment:
COMPILE_OPTS = $(INCLUDES) -I. -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DALLOW_RTSP_SERVER_PORT_REUSE
COMPILE_OPTS += -Wl,--gc-sections

C =			c
C_COMPILER =		$(CROSS_COMPILE)gcc
C_FLAGS =		$(COMPILE_OPTS) $(CFLAGS_COMMON) $(CPPFLAGS) -Wl,-unresolved-symbols=ignore-in-shared-libs
CPP =			cpp
CPLUSPLUS_COMPILER =	$(CROSS_COMPILE)g++ -Wl,-unresolved-symbols=ignore-in-shared-libs -Wl,--gc-sections
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) $(CXXFLAGS_COMMON) $(CPPFLAGS) -Werror -Wno-int-to-pointer-cast -Wno-conversion-null -DBSD=1
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

UNICAST_STREAMER_APPS = testOnDemandRTSPServer$(EXE)
UNICAST_RECEIVER_APPS = openRTSP$(EXE)
UNICAST_APPS = $(UNICAST_STREAMER_APPS) $(UNICAST_RECEIVER_APPS)

PREFIX = /usr/local
ALL = $(UNICAST_APPS)
all: $(ALL)

.$(C).$(OBJ):
	$(Q)$(C_COMPILER) -c $(C_FLAGS) $< 
.$(CPP).$(OBJ):
	$(Q)$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

ON_DEMAND_RTSP_SERVER_OBJS	= testOnDemandRTSPServer.$(OBJ)
OPEN_RTSP_OBJS    = openRTSP.$(OBJ) playCommon.$(OBJ)


openRTSP.$(CPP):	playCommon.hh
playCommon.$(CPP):	playCommon.hh
playSIP.$(CPP):		playCommon.hh

USAGE_ENVIRONMENT_DIR = ../UsageEnvironment
USAGE_ENVIRONMENT_LIB = $(USAGE_ENVIRONMENT_DIR)/libUsageEnvironment.$(libUsageEnvironment_LIB_SUFFIX)
BASIC_USAGE_ENVIRONMENT_DIR = ../BasicUsageEnvironment
BASIC_USAGE_ENVIRONMENT_LIB = $(BASIC_USAGE_ENVIRONMENT_DIR)/libBasicUsageEnvironment.$(libBasicUsageEnvironment_LIB_SUFFIX)
LIVEMEDIA_DIR = ../liveMedia
LIVEMEDIA_LIB = $(LIVEMEDIA_DIR)/libliveMedia.$(libliveMedia_LIB_SUFFIX)
GROUPSOCK_DIR = ../groupsock
GROUPSOCK_LIB = $(GROUPSOCK_DIR)/libgroupsock.$(libgroupsock_LIB_SUFFIX)
LOCAL_LIBS =	$(LIVEMEDIA_LIB) $(GROUPSOCK_LIB) \
		$(BASIC_USAGE_ENVIRONMENT_LIB) $(USAGE_ENVIRONMENT_LIB)

LIB_INC = -L$(MPP_LIB) -L$(LIBADO_LIB) -L$(JSON_LIB) \
	-L$(APP_LIB) -L$(ALSA_LIB)



LIBS := $(LOCAL_LIBS) $(LIBS_FOR_CONSOLE_APPLICATION)
ifeq ($(CONFIG_APP_RTSP_SERVER_ENABLE_IVA),y)
LIBS += -l:libavftr.a -l:libaftr.a -l:libvftr.a
LIB_INC += -L$(LIBAVFTR_LIB)  -L$(FEATURE_AUDIO_LIB_PATH) -L$(FEATURE_VIDEO_LIB_PATH)
else
endif
LIBS += -lmpp -ljson-c
LIBS += -lrt -lasound -lado
LIBS += $(LIB_INC)


testOnDemandRTSPServer$(EXE):	$(ON_DEMAND_RTSP_SERVER_OBJS) $(LOCAL_LIBS)
	$(Q)$(LINK) $@ $(CONSOLE_LINK_OPTS) $(ON_DEMAND_RTSP_SERVER_OBJS) $(LIBS)
openRTSP$(EXE):	$(OPEN_RTSP_OBJS) $(LOCAL_LIBS)
	$(Q)$(LINK) $@ $(CONSOLE_LINK_OPTS) $(OPEN_RTSP_OBJS) $(LIBS)

clean:
	$(Q)-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: $(ALL)
	  install -d $(DESTDIR)$(PREFIX)/bin
	  install -m 755 $(ALL) $(DESTDIR)$(PREFIX)/bin

##### Any additional, platform-specific rules come here:
