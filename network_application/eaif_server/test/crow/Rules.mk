########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

D=CROW
INCS_$(D) := \
-I$(root)/3rd_party/crow

SRCS_$(D) := $(dir)/main.cc
OBJS_$(D) := $(SRCS_$(D):.cc=.o)
BINS_$(D) := $(dir)/test

test-crow: $(BINS_$(D))

$(OBJS_$(D)): CFLAGS_LOCAL := $(INCS_$(D))

$(BINS_$(D)): $(OBJS_$(D)) $(TARGET_SLIB_C_CORE) $(TARGET_SLIB_CORE)
	@printf "    %-8s $@\n" "CC" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(LIB_PATH) $(CCFLAGS) -lstdc++

.PHONY: call-test-crow
call-test-crow:
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
