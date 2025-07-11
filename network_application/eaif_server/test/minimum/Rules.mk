########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

D=TMIN
INCS_$(D) := \
-I$(root)/inc \
-I$(root)/inc/c_api \
-I$(root)/inc/c_api/minimum \
-I$(root)/inc/fake

SRCS_CC_$(D)  := $(dir)/main.c
SRCS_CXX_$(D) :=  $(dir)/test.cc $(root)/src/c_api/eaif_c_test.cc
OBJS_$(D) := $(SRCS_CC_$(D):.c=.o) $(SRCS_CXX_$(D):.cc=.o)
BINS_$(D) := $(dir)/test

test-min: $(BINS_$(D))

$(OBJS_$(D)): CFLAGS_LOCAL := $(INCS_$(D))

$(BINS_$(D)): $(OBJS_$(D)) $(TARGET_SLIB_MIN)
	@printf "    %-8s $@\n" "CC" $(VOUT)
	$(Q)$(CC) -o $@ $^ $(LIB_PATH) $(CCFLAGS) $(LD_FLAGS) -lstdc++

.PHONY: call-test-min
call-test-min:
	@echo BIN: $(BINS_$(D))
	@echo SRC: $(SRCS_CC_$(D)) $(SRCS_CXX_$(D))
	@echo OBJ: $(OBJS_$(D))
	@echo $(subdir)
	@echo $(dir)

CLEAN_OBJS += $(OBJS_$(D)) $(BINS_$(D))

BINS_SPEED_$(D) := $(dir)/speed
SRCS_CC_SPEED_$(D)  := $(dir)/speed.c

$(BINS_SPEED_$(D)): $(TARGET_SLIB_MIN)
	@printf "    %-8s $@\n" "CC" $(VOUT)
	$(Q)$(CC) -o $@ $(SRCS_CC_SPEED_$(D)) $^ $(INCS_$(D)) $(CCFLAGS) $(LD_FLAGS) $(LIB_PATH) -lstdc++

test-min-speed: $(BINS_SPEED_$(D))

CLEAN_OBJS += $(BINS_SPEED_$(D))

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
