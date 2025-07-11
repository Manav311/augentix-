SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/build/sdksrc.mk
include $(SDKSRC_DIR)/application/internal.mk
include Makefile.mk

root   := $(realpath ..)
dir    := $(root)/build
PREFIX ?= $(SYSTEM_BIN)
CONFIG_PREFIX ?= $(SYSTEMFS)/eaif

OPTIM=
LIB_PATH=

OPTIM += ${MACHINE_OPT} -fno-math-errno -fdelete-null-pointer-checks \
-ffunction-sections -fdata-sections

CXX := ${TARGET_CROSS_COMPILE}g++
CC := ${TARGET_CROSS_COMPILE}gcc
STRIP := ${TARGET_CROSS_COMPILE}strip
AR := ${TARGET_CROSS_COMPILE}ar

CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers ${OPTIM}
CXXFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers ${OPTIM} -std=c++14
LDFLAGS += -Wl,--gc-sections

INCS :=
#HEADERS := $(foreach dir,$(INCS_DIRS),$(shell find $(dir) -name *.h))
HEADERS :=
SRCS:=
CLEAN_OBJS:=
DEPS:=
LIB_PREFIX:=$(root)/lib
CONFIG_FILES := $(root)/app/client.ini $(root)/app/config.ini
CONFIG_DIRS := $(root)/templates

TARGET_SLIB:=
TARGET_SLIB_CORE:=
TARGET_SLIB_SERVICE:=
TARGET_SLIB_MIN:=
TARGET_BINS:=
TARGET_CONFIGS:=$(addprefix $(CONFIG_PREFIX)/,$(notdir $(CONFIG_FILES) $(CONFIG_DIRS)))

ifeq ($(USE_MPI), 1)
LIB_PATH += -L$(APP_LIB)
endif

ifeq ($(USE_OPENCV),1)
INCS += $(OPENCV_INC)
LIB_PATH += $(OPENCV_LIB_PATH) $(OPENCV_LIB)
CFLAGS += -DUSE_OPENCV
CXXFLAGS += -DUSE_OPENCV
endif

ifeq ($(USE_TFLITE),1)
INCS += -I$(LIBTFLITE_INC)
LIB_PATH += -L$(LIBTFLITE_LIB) -ltensorflow-lite
CFLAGS += -DUSE_TFLITE
CXXFLAGS += -DUSE_TFLITE
endif

ifeq ($(USE_MICROLITE),1)
INCS += $(MICROLITE_INC)
LIB_PATH += $(MICROLITE_LIB_PATH) $(MICROLITE_LIB)
CFLAGS += -DUSE_MICROLITE -DTF_LITE_STATIC_MEMORY
CXXFLAGS += -DUSE_MICROLITE -DTF_LITE_STATIC_MEMORY
endif

ifeq ($(USE_ARMNN),1)
INCS += $(ARMNN_INC)
LIB_PATH += $(ARMNN_LIB_PATH) $(ARMNN_LIB)
CFLAGS += -DUSE_ARMNN
CXXFLAGS += -DUSE_ARMNN
endif

ifeq ($(USE_BOOST),1)
INCS += $(BOOST_INC)
LIB_PATH += $(BOOST_LIB_PATH) $(BOOST_LIB)
endif

ifeq ($(USE_TF),1)
CFLAGS += -DUSE_TF
CXXFLAGS += -DUSE_TF
endif

ifeq ($(USE_C4), 1)
INCS += $(C4_INC)
LIB_PATH += $(C4_LIB_PATH)
CFLAGS += -DUSE_C4
CXXFLAGS += -DUSE_C4
endif

ifeq ($(USE_CROW), 1)
CFLAGS += -DUSE_CROW
CXXFLAGS += -DUSE_CROW
endif

ifeq ($(USE_JSON), 1)
INCS += $(JSON_INC)
LIB_PATH += $(JSON_LIB_PATH) $(JSON_LIB)
CFLAGS += -DUSE_JSON
CXXFLAGS += -DUSE_JSON
endif

ifeq ($(USE_MPI), 1)
INCS += $(AUX_MPI_INC)
LIB_PATH += $(AUX_MPI_LIB_PATH) $(AUX_MPI_LIB)
CFLAGS += -DUSE_MPI
CXXFLAGS += -DUSE_MPI
endif

ifeq ($(USE_FAKE_MPI), 1)
INCS += -I$(root)/inc/fake
CFLAGS += -DUSE_FAKE_MPI
CXXFLAGS += -DUSE_FAKE_MPI
endif

THIRD_PARTY_INC := $(root)/3rd_party/stb $(root)/3rd_party/crow
INCS += $(addprefix -I,$(THIRD_PARTY_INC))

LIB_PATH += -pthread -lm -ldl

.PHONY: default all

### include directory tree

subdir := $(root)/src
include $(subdir)/Rules.mk

subdir := $(root)/app
include $(subdir)/Rules.mk

subdir := $(root)/test
include $(subdir)/Rules.mk

### define primary make rules
.PHONY: check
check:
ifeq ($(VALID),0)
	$(error $(MSG))
endif

default all: check model lib app client

.PHONY: install
install: check dest install-app install-config

.PHONY: model
ifeq ($(CONFIG_RELEASE),y)
model: ;
else
model:
	$(Q)git lfs pull
endif

.PHONY: install-app

.PHONY: install-demo

.PHONY: install-config
install-config: install-config-files

install-config-files: $(CONFIG_FILES)
	@printf "    %-8s %s\n" "INSTALL" "config" $(VOUT)
	$(Q)install -m 777 $^ $(CONFIG_PREFIX)

install-config-dirs:
	@printf "    %-8s %s\n" "INSTALL" "model-config" $(VOUT)
	$(Q)cp -r $(CONFIG_DIRS) $(CONFIG_PREFIX)

.PHONY: uninstall
uninstall:
	@printf "    %-8s EAIF\n" "UNINSTALL" $(VOUT)
	$(Q)rm -rf $(CONFIG_PREFIX) $(TARGET_BINS)

.PHONY: test

.PHONY: micro

.PHONY: dest
dest:
	$(Q)install -d $(PREFIX)
	$(Q)install -d $(CONFIG_PREFIX)

.PHONY: clean
clean:
	@printf "  %-8s$(root)\n" "CLEAN" $(VOUT)
	$(Q)rm -rf $(DEPS) $(APP) $(CLIENT) $(DEMO) $(CLEAN_OBJS) $(LIB_PREFIX)

.PHONY: distclean
distclean: uninstall clean

-include $(DEPS)

%.o : %.cc
	@printf "  %-8s$@\n" "CX" $(VOUT)
	$(Q)$(CXX) -c $< -o $@ $(INCS) $(CXXFLAGS) $(CFLAGS_LOCAL)

%.o : %.c
	@printf "  %-8s$@\n" "CC" $(VOUT)
	$(Q)$(CC) -c $< -o $@ $(INCS) $(CFLAGS) $(CFLAGS_LOCAL)

.PHONY: call
call:
	@echo SRCS: $(SRCS)
	@echo OBJS: $(OBJS)
	@echo INCS: $(INCS)
	@echo LIBPATH: $(LIB_PATH)
	@echo CXXFLAG: $(CXXFLAGS)
	@echo MACHINEOPT: $(MACHINE_OPT)
	@echo HEADERS: $(HEADERS)
	@echo BINARY: $(APP) $(CLIENT) $(BINARY) $(TARGET_OBJS)
	@echo DEPS: $(DEPS)
	@echo TARGET: $(TARGETS_APP) $(TARGETS_CLIENT)
