SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../../../)
include $(SDKSRC_DIR)/application/internal.mk
include $(LIBSAMPLE_PATH)/build/libosd.mk

# include build options
KCONFIG_CONFIG ?= $(APP_PATH)/.config
-include $(KCONFIG_CONFIG)

# local paths
TEST_PATH = $(LIBSAMPLE_PATH)/test
TEST_SRC_PATH = $(TEST_PATH)/src
TEST_INC_PATH =$(TEST_PATH)/src/inc
TEST_OUTPUT_PATH = $(TEST_PATH)/bin
TEST_OBJ_PATH = $(TEST_PATH)/obj
TEST_DEP_PATH = $(TEST_PATH)/.deps

# specify product specific parameters
SNS := $(shell echo $(SENSOR) | tr '[A-Z]' '[a-z]')

DEFS := SNS=$(SNS) SNS0_ID=0
ifneq ($(SENSOR1),)
DEFS += SNS1=$(SENSOR1) SNS1_ID=1
ifneq ($(SENSOR2),)
DEFS += SNS2=$(SENSOR2) SNS2_ID=2
ifneq ($(SENSOR3),)
DEFS += SNS3=$(SENSOR3) SNS3_ID=3
endif
endif
endif

DEPS = $(patsubst $(SRC_PATH)/%.c,$(DEP_PATH)/%.d,$(SRCS))
DEPS := $(addprefix -D,$(DEFS))

SENSOR_LIBS := sensor_0

ifneq ($(SENSOR1),)
    SENSOR_LIBS += sensor_1
ifneq ($(SENSOR2),)
    SENSOR_LIBS += sensor_2
ifneq ($(SENSOR3),)
    SENSOR_LIBS += sensor_3
endif
endif
endif

# specify CFLAGS
TEST_SRCS := \
$(TEST_SRC_PATH)/main.c \
$(TEST_SRC_PATH)/parse_dev.c \
$(TEST_SRC_PATH)/parse_utils.c \
$(TEST_SRC_PATH)/parse_sys.c \
$(TEST_SRC_PATH)/parse_enc.c \
$(TEST_SRC_PATH)/parse_agtx.c 


TEST_OBJS = \
$(patsubst $(TEST_SRC_PATH)/%.o, $(TEST_OBJ_PATH)/%.o, \
$(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(TEST_SRCS))))

TEST_INCS = -iquote$(MPP_INC) -iquote$(LIBSAMPLE_INC) -iquote$(UDPS_INC) -iquote$(SENSOR_PATH) -iquote"$(LIBSAMPLE_PATH)/test/src/inc"
TEST_INCS += -iquote$(ZLIB_INC) -iquote$(APP_INC) -isystem$(JSON_INC) -iquote$(LIBCM_INC)
ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
TEST_INCS += $(LIBOSD_DEPENDS_INCS)
endif

TEST_LDFLAGS := -l:libsample.a -lm -lrt -lmpp -ludps -ljson-c -lcm
TEST_LDFLAGS += $(addprefix -l,$(SENSOR_LIBS)) -Wl,--gc-sections
ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
TEST_LDFLAGS += $(LIBOSD_DEPENDS_LDFLAGS)
endif

TEST_LIBS = -L$(MPP_LIB) -L$(LIBSAMPLE_LIB) -L$(LIBOSD_LIB) -L$(FREETYPE_LIB) -L$(UDPS_LIB) -L$(LIBSENSOR_LIB)
TEST_LIBS += -L$(APP_LIB) -L$(JSON_LIB) -L$(LIBCM_LIB)
ifeq ($(CONFIG_LIBSAMPLE_USE_LIBOSD),y)
TEST_LIBS += $(LIBOSD_DEPENDS_LIBS)
endif
TEST_LIBS += $(TEST_LDFLAGS)
TEST_CFLAGS = $(CFLAGS_COMMON) -pthread $(TEST_INCS) $(addprefix -D,$(DEPS))
TEST_DEPFLAGS = -MT $@ -MMD -MP -MF "$(TEST_DEP_PATH)/$*.d"

# build and install target
TEST_NAME := libsample_test 
TEST_OUTPUT := $(addprefix $(TEST_OUTPUT_PATH)/,$(TEST_NAME))
TEST_INSTALL_TARGET := $(addprefix $(SYSTEM_BIN)/,$(TEST_NAME))

.PHONY: test test-clean test-install test-uninstall

ifeq ($(CONFIG_LIBCM),y)
ifeq ($(CONFIG_LIBFSINK),y)
test: $(TEST_OUTPUT)
test-clean:
	$(Q)$(RM_RF) $(TEST_OUTPUT_PATH) $(TEST_DEP_PATH) $(TEST_OBJ_PATH)
else
test:
test-clean:
endif
endif

ifeq ($(CONFIG_LIBSAMPLE_TEST_PROGRAM),y)
test-install: test
	$(Q)$(INSTALL) -m 755 $(TEST_OUTPUT) $(TEST_INSTALL_TARGET)
test-uninstall:
	$(Q)$(RM) $(TEST_INSTALL_TARGET)
else
test-install:
test-uninstall:
endif

$(TEST_OUTPUT): $(TEST_OBJS)
	$(Q)$(MKDIR_P) $(TEST_OUTPUT_PATH)
	$(Q)$(CC) $(TEST_DEPFLAGS) $(TEST_CFLAGS) -o $@ $^ $(TEST_LIBS)

# objects
$(TEST_OBJ_PATH)/%.o: $(TEST_SRC_PATH)/%.c $(TEST_DEP_PATH)/%.d
	$(Q)$(MKDIR_P) $(TEST_DEP_PATH) $(TEST_OBJ_PATH)
	$(Q)$(CC) $(TEST_DEPFLAGS) $(TEST_CFLAGS) -c -o $@ $<

# dependencies
$(TEST_DEP_PATH)/%.d: ;
.PRECIOUS: $(TEST_DEP_PATH)/%.d


