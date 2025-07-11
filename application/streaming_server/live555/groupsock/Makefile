SDKSRC_DIR = $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

INCLUDES = -Iinclude -I../UsageEnvironment/include
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
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

NAME = libgroupsock
ALL = $(NAME).$(LIB_SUFFIX)
all:	$(ALL)

.$(C).$(OBJ):
	$(Q)$(C_COMPILER) -c $(C_FLAGS) $<
.$(CPP).$(OBJ):
	$(Q)$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

GROUPSOCK_LIB_OBJS = GroupsockHelper.$(OBJ) GroupEId.$(OBJ) inet.$(OBJ) Groupsock.$(OBJ) NetInterface.$(OBJ) NetAddress.$(OBJ) IOHandlers.$(OBJ)

GroupsockHelper.$(CPP):	include/GroupsockHelper.hh
include/GroupsockHelper.hh:	include/NetAddress.hh
include/NetAddress.hh:	include/NetCommon.h
GroupEId.$(CPP):	include/GroupEId.hh
include/GroupEId.hh:	include/NetAddress.hh
inet.$(C):		include/NetCommon.h
Groupsock.$(CPP):	include/Groupsock.hh include/GroupsockHelper.hh include/TunnelEncaps.hh
include/Groupsock.hh:	include/groupsock_version.hh include/NetInterface.hh include/GroupEId.hh
include/NetInterface.hh:	include/NetAddress.hh
include/TunnelEncaps.hh:	include/NetAddress.hh
NetInterface.$(CPP):	include/NetInterface.hh include/GroupsockHelper.hh
NetAddress.$(CPP):	include/NetAddress.hh include/GroupsockHelper.hh
IOHandlers.$(CPP):	include/IOHandlers.hh include/TunnelEncaps.hh

libgroupsock.$(LIB_SUFFIX): $(GROUPSOCK_LIB_OBJS) \
    $(PLATFORM_SPECIFIC_LIB_OBJS)
	$(Q)$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) \
		$(GROUPSOCK_LIB_OBJS)

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: libgroupsock.$(LIB_SUFFIX)
	  install -d $(DESTDIR)$(PREFIX)/include/groupsock $(DESTDIR)$(LIBDIR)
	  install -m 644 include/*.hh include/*.h $(DESTDIR)$(PREFIX)/include/groupsock
	  install -m 644 libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)
install_shared_libraries: libgroupsock.$(LIB_SUFFIX)
	  ln -s libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/libgroupsock.$(SHORT_LIB_SUFFIX)
	  ln -s libgroupsock.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/libgroupsock.so

##### Any additional, platform-specific rules come here:
