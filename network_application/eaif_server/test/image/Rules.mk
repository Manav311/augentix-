########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

D=IMAGE
INCS_$(D) := \
-I$(root)/inc

SRCS_$(D) := $(dir)/main.cc
OBJS_$(D) := $(SRCS_$(D):.cc=.o)
BINS_$(D) := $(dir)/test

test-img: $(BINS_$(D))

$(OBJS_$(D)): CFLAGS_LOCAL := $(INCS_$(D))

$(BINS_$(D)): $(OBJS_$(D)) $(TARGET_SLIB_CORE)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(LIB_PATH) $(CXXFLAGS)

.PHONY: call-test-img
call-test-img:
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
