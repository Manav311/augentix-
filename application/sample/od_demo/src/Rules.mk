################################################################################
# [DON'T TOUCH] Traverse directory tree
################################################################################

sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

################################################################################
# Subdirectories, in random order
################################################################################

################################################################################
# [EDIT] Specify source files and include directories
################################################################################

DEFS_$(dir) :=

ifeq ($(CONFIG_APP_OD_SUPPORT_SEI),y)
    DEFS_$(dir) += CONFIG_APP_OD_SUPPORT_SEI
endif

SRCS_$(dir) := $(wildcard $(dir)/*.c)
INCS_$(dir) := $(dir) \
               $(LIBAVFTR_INC) \
               $(LIBEAIF_INC) \
               $(FEATURE_VIDEO_INC_PATH) \
               $(FEATURE_AUDIO_INC_PATH) \
               $(JSON_INC) \
               $(MPP_PATH)/include
LIBS_$(dir) := json-c mpp m rt
LIBDIRS_$(dir) := $(MPP_PATH)/lib \
                  $(JSON_LIB)

################################################################################
# [DON'T TOUCH] Calculate corresponding object files and auto-dependencies
################################################################################

OBJS_$(dir) := $(SRCS_$(dir):.c=.o)
DEPS_$(dir) := $(SRCS_$(dir):.c=.d)

$(OBJS_$(dir)):	CFLAGS_LOCAL := -pthread $(addprefix -D,$(DEFS_$(dir))) $(addprefix -iquote,$(INCS_$(dir)))
-include $(DEPS_$(dir))

CLEAN_FILES += $(OBJS_$(dir)) $(DEPS_$(dir))

################################################################################
# Targets
################################################################################

TARGET_BIN_$(dir) := $(root)/bin/od_demo

$(TARGET_BIN_$(dir)): LFLAGS_LOCAL := -pthread $(addprefix -L,$(LIBDIRS_$(dir))) $(addprefix -l,$(LIBS_$(dir)))
$(TARGET_BIN_$(dir)): $(OBJS_$(dir)) | $(root)/bin
	@printf "  %-8s$@\n" "CC"
	$(Q)$(CC) $(LINK)

TARGET_BINS += $(TARGET_BIN_$(dir))
CLEAN_FILES += $(TARGET_BIN_$(dir))

################################################################################
# Traverse directory tree
################################################################################

dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
