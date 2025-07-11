SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)
include $(SDKSRC_DIR)/application/internal.mk

# specify build tools
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

# specify CFLAGS
CFLAGS_COMMON = -MMD -MP

ifeq ($(CROSS_COMPILE),arm-linux-gnueabihf-)
CFLAGS_COMMON += -mfloat-abi=hard
endif

ifneq ($(TFLITE_STATIC_MEMORY),)
CPPFLAGS += -DTF_LITE_STATIC_MEMORY
endif

CFLAGS += $(CFLAGS_COMMON)
CXXFLAGS += $(CFLAGS_COMMON)

# paths
root = $(realpath $(CURDIR))
TFLITE_DIR = $(BUILDROOT_OUTPUT_BUILD_PATH)/tensorflow-2.6.0/build_tflite
MICROLITE_DIR = $(BUILDROOT_OUTPUT_BUILD_PATH)/tensorflow-2.6.0/build_microlite
FREERTOS_DIR = $(SDKSRC_DIR)/remote/freertos/Demo/HC1703_1723_1753_1783S_GCC
AMP_DIR = $(SDKSRC_DIR)/remote/amp
AMPI_DIR = $(SDKSRC_DIR)/application/library/libamp/ampc

# specify source files and include directories
SRCS := $(wildcard *.c) $(wildcard *.cpp)
#SRCS +=	$(wildcard $(AMPI_DIR)/src/linux/*.[cS])
MATH_SRCS := $(wildcard math/*.c) $(wildcard math/*.cc)

INCS = .
TFLITE_INCS := $(TFLITE_DIR)/inc
MICROLITE_INCS := $(MICROLITE_DIR)/include
FREERTOS_INCS := $(FREERTOS_DIR) $(AMPI_DIR)/include
FREERTOS_INCS += $(AMP_DIR)/libmetal/build/linux/lib/include
FREERTOS_INCS += $(AMP_DIR)/openamp/lib/include
FREERTOS_INCS += $(BUILDROOT_HOST_INC)/sysfs

LIBS = pthread m rt dl
TFLITE_LIBS = $(LIBS) tensorflow-lite
MICROLITE_LIBS = $(LIBS) tensorflow-microlite
FREERTOS_LIBS = $(LIBS) sysfs metal open_amp ampc_linux

LIBDIRS := $(BUILDROOT_TARGET_LIB)
LIBDIRS += $(AMP_DIR)/libmetal/build/linux/lib
LIBDIRS += $(AMP_DIR)/openamp/build/linux/lib
LIBDIRS += $(AMPI_DIR)/lib
#LIBDIRS += $(BUILDROOT_OUTPUT_PATH)/host/usr/arm-buildroot-linux-$(TOOLCHAIN_SUFFIX)/sysroot/usr/lib

# [DON'T TOUCH] calculate corresponding object files and auto-dependencies
#OBJS = $(SRCS:.cpp=.o)
#DEPS = $(SRCS:.cpp=.d)
OBJS := $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRCS)))
#DEPS = $(OBJS:.o=.d)

#MATH_OBJS := $(patsubst %.c, %.o, $(patsubst %.cc, %.o, $(MATH_SRCS)))

# specify bin
TFLITE_BIN = tflite_runner
MICROLITE_BIN = microlite_runner
FREERTOS_BIN = freertos_runner

OBJS := $(filter-out $(TFLITE_BIN).o $(MICROLITE_BIN).o $(FREERTOS_BIN).o, $(OBJS))
DEPS := $(OBJS:.o=.d)

# specify targets
.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all: tflite microlite

.PHONY: tflite
tflite: $(TFLITE_BIN)

.PHONY: microlite
microlite: $(MICROLITE_BIN)

.PHONY: freertos
freertos: $(FREERTOS_BIN)

.PHONY: tflm
tflm: $(TFLM_BIN)


$(TFLITE_BIN): $(OBJS) $(TFLITE_BIN).o
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $(CXXFLAGS) $^ -o $@ $(addprefix -L,$(LIBDIRS) $(TFLITE_DIR)/lib) $(addprefix -l,$(TFLITE_LIBS))

$(MICROLITE_BIN): $(OBJS) $(MICROLITE_BIN).o
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $(CXXFLAGS) -static $^ -o $@ $(addprefix -L,$(LIBDIRS) $(MICROLITE_DIR)/lib) $(addprefix -l,$(MICROLITE_LIBS))

$(FREERTOS_BIN): $(OBJS) $(FREERTOS_BIN).o
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $(CXXFLAGS) $^ -o $@ $(addprefix -L,$(LIBDIRS) $(MICROLITE_DIR)/lib) $(addprefix -l,$(FREERTOS_LIBS))


.PHONY: clean
clean:
	@printf "  %-8s$(CURDIR)\n" "CLEAN"
	$(Q)rm -f $(TFLITE_BIN) $(MICROLITE_BIN) $(TFLM_BIN) *.[ado] math/*.[ado]

$(TFLITE_BIN).o: $(TFLITE_BIN).cpp
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $< -c $(CXXFLAGS) -o $@ $(addprefix -iquote,$(INCS) $(TFLITE_INCS))

$(MICROLITE_BIN).o: $(MICROLITE_BIN).cpp
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $< -c $(CXXFLAGS) -o $@ $(addprefix -iquote,$(INCS) $(MICROLITE_INCS))

$(FREERTOS_BIN).o: $(FREERTOS_BIN).cpp
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $< -c $(CXXFLAGS) -o $@ $(addprefix -iquote,$(INCS) $(MICROLITE_INCS) $(FREERTOS_INCS))

#math/%.o: math/%.c
#	@printf "  %-8s$@\n" "CC"
#	$(Q)$(CC) $< -c $(CFLAGS_TFLM) -o $@ $(addprefix -iquote,$(INCS) $(TFLM_INCS))
#
#math/%.o: math/%.cc
#	@printf "  %-8s$@\n" "CXX"
#	$(Q)$(CXX) $< -c $(CXXFLAGS_TFLM) -o $@ $(addprefix -iquote,$(INCS) $(TFLM_INCS))


# general directory independent targets
%.o: %.c
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $< -c $(CFLAGS) $(CPPFLAGS) -o $@ $(addprefix -iquote,$(INCS) $(MICROLITE_INCS)) $(addprefix -isystem,$(FREERTOS_INCS))

%.o: %.cpp
	@printf "  %-8s$@\n" "CXX"
	$(Q)$(CXX) $< -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $(addprefix -iquote,$(INCS) $(MICROLITE_INCS)) $(addprefix -isystem,$(FREERTOS_INCS))


# Autodependencies
-include $(DEPS)
