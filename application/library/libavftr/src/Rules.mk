################################################################################
# [DON'T TOUCH] Traverse directory tree
################################################################################

sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

################################################################################
# Subdirectories (Subdirectories can be specified in random order)
################################################################################

subdir := $(dir)/ac
include $(subdir)/Rules.mk

subdir := $(dir)/ac
include $(subdir)/Rules.mk

################################################################################
# Source files and include directories
################################################################################

SRCS_$(dir) := $(wildcard $(dir)/*.c)
INCS_$(dir) := $(dir) $(root)/include \
			   $(MPP_INC) $(DEBUG_INC) \
			   $(AUDIO_INC) $(ALSA_INC) $(LIBADO_INC) $(FEATURE_INC_PATHS) \
			   $(LIBEAIF_INC) $(root)/src/ac

################################################################################
# Corresponding object files and auto-dependencies
################################################################################

OBJS_$(dir) := $(SRCS_$(dir):.c=.o) $(OBJS_$(D))
DEPS_$(dir) := $(SRCS_$(dir):.c=.d) $(OBJS_$(D):.o=.d)

$(OBJS_$(dir)):	CFLAGS_LOCAL := $(addprefix -I,$(INCS_$(dir)))
-include $(DEPS_$(dir))

CLEAN_FILES += $(OBJS_$(dir)) $(DEPS_$(dir))

################################################################################
# Targets (Must be placed after "Subdirectories" section)
################################################################################

TARGET_SLIB_$(dir) := $(root)/lib/libavftr.a

$(TARGET_SLIB_$(dir)): $(OBJS_$(dir)) \
					   $(OBJS_$(dir)/ac)
	@printf "  %-8s$@\n" "AR"
	$(Q)$(MKDIR) -p $(@D)
	$(Q)$(ARCHIVE_STATIC)

TARGET_SLIBS += $(TARGET_SLIB_$(dir))
CLEAN_FILES += $(TARGET_SLIB_$(dir))
CLEAN_DIRS += $(dir $(TARGET_SLIB_$(dir)))

################################################################################
# [DON'T TOUCH] Traverse directory tree
################################################################################

dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
