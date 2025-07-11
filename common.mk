# install
INSTALL := install -c
INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644

# utils
CP := cp
LN := ln
MKDIR := mkdir
MKDIR_P := mkdir -p
MV := mv
RM := rm -f
RM_RF := rm -rf

# some useful pre-defined variables
blank :=
space := $(blank) $(blank)
comma := $(blank),$(blank)

# toolchain
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

# paths
root = $(realpath $(CURDIR)/..)
bindir = $(SYSTEM_BIN)
libdir = $(SYSTEM_LIB)
BIN_OUT_PATH = $(root)/bin
BUILD_PATH = $(root)/build
INC_PATH = $(root)/include
LIB_OUT_PATH = $(root)/lib
OBJ_PATH = $(root)/obj
SRC_PATH = $(root)/src
SRC_INC_PATH = $(root)/src/inc
DEP_PATH = $(root)/.deps

# compiler/linker flags
CFLAGS += -I"$(INC_PATH)" -iquote"$(SRC_INC_PATH)"
CXXFLAGS += -I"$(INC_PATH)" -iquote"$(SRC_INC_PATH)"

# dependency flags
DEPFLAGS = -MT $@ -MMD -MP -MF "$(DEP_PATH)/$*.d"

# source codes, objects, and dependencies
SRCS = $(wildcard $(SRC_PATH)/*.c)
OBJS = $(patsubst $(SRC_PATH)/%.c,$(OBJ_PATH)/%.o,$(SRCS))
DEPS = $(patsubst $(SRC_PATH)/%.c,$(DEP_PATH)/%.d,$(SRCS))

# specify default lib name and its version
LIB_NAME = foo
LIB_MAJOR = 1
LIB_MINOR = 0
LIB_PATCH = 0

# application library definitions
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)
LIB_ARNAME = lib$(LIB_NAME).a
LIB_REALNAME = lib$(LIB_NAME).so.$(LIB_VERSION)
LIB_SONAME = lib$(LIB_NAME).so.$(LIB_MAJOR)
LIB_LINKERNAME = lib$(LIB_NAME).so

LIB_STATIC = $(LIB_ARNAME)
LIB_STATIC_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_STATIC))

LIB_SHARED = $(LIB_LINKERNAME) $(LIB_SONAME) $(LIB_REALNAME)
LIB_SHARED_OUTPUT = $(addprefix $(LIB_OUT_PATH)/,$(LIB_SHARED))
LIB_SHARED_TARGET = $(addprefix $(libdir)/,$(LIB_SHARED))

# static/shared lib targets
.PHONY: install-so
install-so: $(LIB_SHARED_TARGET)

.PHONY: uninstall-so
uninstall-so:
	$(Q)$(RM) $(LIB_SHARED_TARGET) $(LIB_LINKERNAME).*

.PHONY: clean-a
clean-a:
	$(Q)$(RM) $(LIB_STATIC_OUTPUT)

.PHONY: clean-so
clean-so:
	$(Q)$(RM) $(LIB_SHARED_OUTPUT)

# toggle verbose output
V ?= 0
ifeq ($(V),1)
	Q :=
	VOUT :=
else
	Q := @
	VOUT := 2>&1 1>/dev/null
endif

# install targets
$(bindir)/%: $(BIN_OUT_PATH)/%
	$(Q)$(INSTALL_PROGRAM) $< $@

$(libdir)/%: $(LIB_OUT_PATH)/%
	$(Q)$(INSTALL_DATA) $< $@

# objects
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cc $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp $(DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(DEP_PATH) $(OBJ_PATH)
	$(Q)$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

# dependencies
$(DEP_PATH)/%.d: ;
.PRECIOUS: $(DEP_PATH)/%.d
