################################################################################
# [DON'T TOUCH] Traverse directory tree
################################################################################

sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

################################################################################
# [EDIT] Specify source files and include directories
################################################################################

DT=TTEST

# Consider replacing the following targets by facereco_utils
# sample/facereco_demo/bin/facereco_utils
# - at_registerface.elf
# - at_testface.elf
# - at_testfaceV2.elf
# - ut_facedet.elf

SRCS_$(DT) := $(wildcard $(dir)/*.c)
BINS_$(DT) := $(SRCS_$(DT):.c=.elf)

SRCS_CXX_$(DT) := $(wildcard $(dir)/*.cc)
BINS_CXX_$(DT) := $(SRCS_CXX_$(DT):.cc=.elf)

MIN_SRCS := $(dir)/minimal.cc
BINS_MIN := $(dir)/minimal

LIB_COMMON := \
-L$(CONFIG_INF_LIB_PATH) \
$(CONFIG_INF_LIB) \
-pthread -ldl -lrt -lm -lstdc++

LIB_$(DT) := \
-Wl,--start-group \
-L$(LIBINF_LIB) -linf \
$(LIB_COMMON) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-Wl,--end-group

ifeq ($(HOST), linux)
LIB_$(DT) :=\
-Wl,--start-group \
-L$(LIBINF_LIB) -linf -Wl,-rpath=$(LIBINF_LIB)\
$(LIB_COMMON) \
-Wl,-rpath=$(CONFIG_INF_LIB_PATH)\
-Wl,--end-group
endif

$(BINS_$(DT)): CFLAGS := -g3 -std=gnu99 -Wall $(INCS)
$(BINS_$(DT)): $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

$(BINS_CXX_$(DT)): CXXFLAGS := -g3 -std=c++11 -Wall $(INCS)
$(BINS_CXX_$(DT)): $(LIB_STATIC_OUTPUT) $(LIB_SHARED_OUTPUT)

$(BINS_MIN) : CXXFLAGS := -g3 -std=c++11 -Wall $(INCS)
$(BINS_MIN) : LIB_$(DT) := $(LIB_COMMON) \
			 -Wl,-rpath=$(CONFIG_INF_LIB_PATH)

ifeq ($(CONFIG_LIBINF_NCNN), y)
	CFLAGS+=-DUSE_NCNN
endif

$(BINS_MIN): $(MIN_SRCS)
	@printf "  %-8s$@\n" "LD"
	$(Q)$(CXX) -o $@ $< $(CXXFLAGS) $(LIB_$(DT))

%.elf: %.c
	@printf "  %-8s$@\n" "LD"
	$(Q)$(CC) -o $@ $< $(CFLAGS) $(LIB_$(DT))

%.elf: %.cc
	@printf "  %-8s$@\n" "LD"
	$(Q)$(CXX) -o $@ $< $(CXXFLAGS) $(LIB_$(DT))

test: $(BINS_$(DT)) $(BINS_CXX_$(DT)) $(BINS_MIN)

.PHONY: test-clean
test-clean:
	$(Q)$(RM) $(BINS_$(DT)) $(BINS_CXX_$(DT)) $(BINS_MIN)

################################################################################
# [DON'T TOUCH] Traverse directory tree
################################################################################

dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
