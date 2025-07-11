########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

D=C_API
INCS_$(D) := \
-I$(root)/inc \
-I$(root)/inc/c_api

SRCS_$(D) := $(dir)/main.c
OBJS_$(D) := $(SRCS_$(D):.c=.o)
BINS_$(D) := $(dir)/test

test-c: $(BINS_$(D))

$(OBJS_$(D)): CFLAGS_LOCAL := $(INCS_$(D))

$(BINS_$(D)): $(OBJS_$(D)) $(TARGET_SLIB_C_CORE) $(TARGET_SLIB_CORE)
	@printf "    %-8s $@\n" "CC" $(VOUT)
	$(Q)$(CC) -o $@ $^ $(LIB_PATH) $(CCFLAGS) -lstdc++

.PHONY: call-test-c
call-test-c:
	echo $(BINS_$(D))
	echo $(OBJS_$(D))
	echo $(subdir)
	echo $(dir)

CLEAN_OBJS += $(OBJS_$(D)) $(BINS_$(D))

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
