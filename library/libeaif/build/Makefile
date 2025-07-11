################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/application/internal.mk

KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)
include config.mk

MAKEFLAGS += -j

################################################################################
# Build tools and commands
################################################################################

### First, specify the build tools.
AR := $(CROSS_COMPILE)ar
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
ARFLAGS = rcsD
MKDIR := mkdir
MKDIR_P := $(MKDIR) -p
RMDIR := rmdir
LN := ln
CP := cp
MV := mv
RM := rm -f
INSTALL = install
ARCHIVE_SHARED = -shared -Wl,--build-id -Wl,--soname,$(basename $(basename $(notdir $@))) -o $@ \
                 -Wl,--whole-archive $^ -Wl,--no-whole-archive
### paths

root = $(realpath ..)
bindir = $(SYSTEM_BIN)
libdir = $(SYSTEM_LIB)
LIB_OUT_PATH = $(root)/lib
BIN_OUT_PATH = $(root)/bin
BUILD_PATH = $(root)/build
OBJ_PATH = $(root)/obj
SRC_PATH = $(root)/src

### Then, collect the knowledge about how to build all outputs.

INC_PATHS := \
$(root)/include \
$(root)/src \
$(MPP_INC) \
$(SRC_PATH) \
$(LIBINF_INC) \
$(FEATURE_VIDEO_INC_PATH) \
$(CONFIG_INCS)

INCS = $(addprefix -iquote, $(INC_PATHS))

SRCS = $(wildcard $(root)/src/*.c)

OBJS := $(patsubst $(SRC_PATH)/%.o, $(OBJ_PATH)/%.o, $(patsubst %.c,%.o,$(SRCS)))
DEPS := $(patsubst $(SRC_PATH)/%.d, $(OBJ_PATH)/%.d, $(patsubst %.c,%.d,$(SRCS)))

### Then, define global compilation and linking flags.

CFLAGS_COMMON := $(INCS) -L$(FEATURE_VIDEO_LIB_PATH) -lvftr
CFLAGS_COMMON += -D_GNU_SOURCE -MMD -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DBSD=1
CFLAGS_COMMON += -ffunction-sections -fdata-sections 

CFLAGS += $(CONFIG_CFLAGS) $(CFLAGS_COMMON) 
CPPFLAGS += $(CONFIG_CPPFLAGS) $(CFLAGS_COMMON)


################################################################################
# Build rules
################################################################################

.PHONY: all

LIB_NAME = eaif
LIB_MAJOR = 1
LIB_MINOR = 0
LIB_PATCH = 0

LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)
LIB_ARNAME = lib$(LIB_NAME).a
LIB_REALNAME = lib$(LIB_NAME).so.$(LIB_VERSION)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_MAJOR)
LIB_LINKERNAME = lib$(LIB_NAME).so

LIB_STATIC = $(LIB_ARNAME)
LIB_STATIC_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_STATIC))

LIB_SHARED = $(LIB_REALNAME)
LIB_SHARED_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_SHARED) $(LIB_LINKERNAME))
LIB_SHARED_TARGET = $(addprefix $(libdir)/,$(LIB_SHARED) $(LIB_SONAME))

### Finally, define essential build targets.
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c | $(OBJ_PATH)
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: all
all: $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

.PHONY: clean
clean:
	@printf "  %-8s$(root)\n" "CLEAN"
	$(Q)$(RM) -r $(OBJ_PATH) $(LIB_OUT_PATH)

.PHONY: distclean
distclean: uninstall clean

.PHONY: install
install: all $(LIB_SHARED_TARGET)

.PHONY: uninstall
uninstall:
	$(Q)$(foreach f, $(LIB_SHARED_TARGET), \
		printf "  %-8s$(f)\n" "RM"; \
		$(RM) -f $(f); \
	)
	$(Q)if [ -d $(libdir) ]; then \
		$(RMDIR) --ignore-fail-on-non-empty $(libdir); \
	fi

.PHONY: check
check:
	@echo obj: $(OBJ_PATH) $(OBJS)
	@echo src: $(SRCS) $(SRC_PATH)
	@echo inc: $(INC_PATHS)
	@echo cflags: $(CFLAGS)
	@echo cppflags: $(CPPFLAGS)
	@echo tar: $(LIB_STATIC_OUTPUT)

$(LIB_STATIC_OUTPUT): $(OBJS) | $(LIB_OUT_PATH)
	@printf "  %-8s$@\n" "AR"
	$(Q)$(AR) $(ARFLAGS) $@ $^

$(LIB_OUT_PATH)/$(LIB_REALNAME): $(OBJS) | $(LIB_OUT_PATH)
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(ARCHIVE_SHARED)

$(LIB_OUT_PATH)/$(LIB_LINKERNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME)
	@printf "  %-8s$@\n" "LN"
	$(Q)$(LN) -sfT $(notdir $<) $(LIB_OUT_PATH)/$(basename $(basename $(basename $(notdir $<))))

$(libdir)/$(LIB_REALNAME): $(LIB_OUT_PATH)/$(LIB_REALNAME) | $(libdir)
	@printf "  %-8s$@\n" "INSTALL"
	$(Q)$(INSTALL) -m 644 -t $(libdir) $<

$(libdir)/$(LIB_SONAME): $(libdir)/$(LIB_REALNAME)
	@printf "  %-8s$@\n" "LN"
	$(Q)$(LN) -sfT $(notdir $<) $(libdir)/$(basename $(basename $(notdir $<)))

$(OBJ_PATH) $(LIB_OUT_PATH) $(libdir):
	@printf "  %-8s$@\n" "MKDIR"
	$(Q)$(MKDIR_P) $@

################################################################################
# General rules
################################################################################

-include $(DEPS)
