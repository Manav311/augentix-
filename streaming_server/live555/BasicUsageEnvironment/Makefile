SDKSRC_DIR = $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

INCLUDES = -Iinclude -I../UsageEnvironment/include -I../groupsock/include -I$(MPP_INC)
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
##### Change the following for your environment:
COMPILE_OPTS = $(INCLUDES) -I. -Wall -O2 -fPIC -fPIE -fno-omit-frame-pointer -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -mcpu=$(CONFIG_TARGET_CPU) -DALLOW_RTSP_SERVER_PORT_REUSE

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

NAME = libBasicUsageEnvironment
LIB = $(NAME).$(LIB_SUFFIX)
ALL = $(LIB)
all:	$(ALL)

OBJS = BasicUsageEnvironment0.$(OBJ) BasicUsageEnvironment.$(OBJ) \
	BasicTaskScheduler0.$(OBJ) BasicTaskScheduler.$(OBJ) \
	DelayQueue.$(OBJ) BasicHashTable.$(OBJ)

libBasicUsageEnvironment.$(LIB_SUFFIX): $(OBJS)
	$(Q)$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) \
		$(OBJS)

.$(C).$(OBJ):
	$(Q)$(C_COMPILER) -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(Q)$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

BasicUsageEnvironment0.$(CPP):	include/BasicUsageEnvironment0.hh
include/BasicUsageEnvironment0.hh:	include/BasicUsageEnvironment_version.hh include/DelayQueue.hh
BasicUsageEnvironment.$(CPP):	include/BasicUsageEnvironment.hh
include/BasicUsageEnvironment.hh:	include/BasicUsageEnvironment0.hh
BasicTaskScheduler0.$(CPP):	include/BasicUsageEnvironment0.hh include/HandlerSet.hh
BasicTaskScheduler.$(CPP):	include/BasicUsageEnvironment.hh include/HandlerSet.hh
DelayQueue.$(CPP):		include/DelayQueue.hh
BasicHashTable.$(CPP):		include/BasicHashTable.hh

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: libBasicUsageEnvironment.$(LIB_SUFFIX)
	  install -d $(DESTDIR)$(PREFIX)/include/BasicUsageEnvironment $(DESTDIR)$(LIBDIR)
	  install -m 644 include/*.hh $(DESTDIR)$(PREFIX)/include/BasicUsageEnvironment
	  install -m 644 libBasicUsageEnvironment.$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)
install_shared_libraries: libBasicUsageEnvironment.$(LIB_SUFFIX)
	  ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).$(SHORT_LIB_SUFFIX)
	  ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).so

##### Any additional, platform-specific rules come here:
