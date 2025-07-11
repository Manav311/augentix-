SDKSRC_DIR = $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

INCLUDES = -Iinclude -I../groupsock/include
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
##### Change the following for your environment:
COMPILE_OPTS =		$(INCLUDES) -I. -Wall -O2 -fPIC -fPIE -fno-omit-frame-pointer -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -mcpu=$(CONFIG_TARGET_CPU) -DALLOW_RTSP_SERVER_PORT_REUSE -I$(APP_INC)

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

NAME = libUsageEnvironment
USAGE_ENVIRONMENT_LIB = $(NAME).$(LIB_SUFFIX)
ALL = $(USAGE_ENVIRONMENT_LIB)
all:	$(ALL)

OBJS = UsageEnvironment.$(OBJ) HashTable.$(OBJ) strDup.$(OBJ)

$(USAGE_ENVIRONMENT_LIB): $(OBJS)
	$(Q)$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) $(OBJS)

.$(C).$(OBJ):
	$(Q)$(C_COMPILER) -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(Q)$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

UsageEnvironment.$(CPP):	include/UsageEnvironment.hh
include/UsageEnvironment.hh:	include/UsageEnvironment_version.hh include/Boolean.hh include/strDup.hh
HashTable.$(CPP):		include/HashTable.hh
include/HashTable.hh:		include/Boolean.hh
strDup.$(CPP):			include/strDup.hh

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: $(USAGE_ENVIRONMENT_LIB)
	  install -d $(DESTDIR)$(PREFIX)/include/UsageEnvironment $(DESTDIR)$(LIBDIR)
	  install -m 644 include/*.hh $(DESTDIR)$(PREFIX)/include/UsageEnvironment
	  install -m 644 $(USAGE_ENVIRONMENT_LIB) $(DESTDIR)$(LIBDIR)
install_shared_libraries: $(USAGE_ENVIRONMENT_LIB)
	  ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).$(SHORT_LIB_SUFFIX)
	  ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).so

##### Any additional, platform-specific rules come here:
