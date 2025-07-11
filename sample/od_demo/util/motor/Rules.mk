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
SRCS_$(dir) := $(wildcard $(dir)/*.c)
INCS_$(dir) := $(dir) \
               $(MPP_PATH)/include
LIBS_$(dir) := mpp m
LIBDIRS_$(dir) := $(MPP_PATH)/lib

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

TARGET_BIN_$(dir) := $(root)/bin/motor

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
