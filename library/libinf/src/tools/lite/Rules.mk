########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

D=TOOL_LITE

BENCHMARK_NAME=lite_benchmark
MIN_NAME=lite_minimal
BINS_BENCHMARK := $(root)/bin/$(BENCHMARK_NAME)
BINS_MIN := $(root)/bin/$(MIN_NAME)
TARGET_BINS=$(addprefix $(SYSTEM_BIN)/, $(BENCHMARK_NAME) $(MIN_NAME))

# Copy from tensorflow src path
BENCHMARK_SRCS := \
$(wildcard $(dir)/benchmark/*.cc) \
$(wildcard $(dir)/benchmark/profile_summarizer/*.cc) \
$(wildcard $(dir)/benchmark/cmd_line_tools/*.cc) \
$(wildcard $(dir)/benchmark/delegate/*.cc) \
$(wildcard $(dir)/benchmark/evaluation_utils/*.cc) \
$(dir)/benchmark/main/benchmark_main.cc
BENCHMARK_OBJS := $(patsubst $(root)/src/%.o, $(OBJ_PATH)/%.o, $(BENCHMARK_SRCS:.cc=.o))
#BENCHMARK_DEPS := $(patsubst $(OBJ_PATH)/%.o, $(OBJ_PATH)/%.d, $(BENCHMARK_OBJS))

MIN_SRCS := $(dir)/minimal.cc

LIB_COMMON= \
-L$(CONFIG_INF_LIB_PATH) \
$(CONFIG_INF_LIB) \
-L$(FEATURE_VIDEO_LIB_PATH) -lvftr \
-pthread -ldl -lrt -lm -lstdc++

LIB_$(D)= \
-Wl,--start-group \
-L$(LIBINF_LIB) -linf \
$(LIB_COMMON) \
-Wl,--end-group

ifeq ($(HOST), linux)
LIB_$(D)=\
-Wl,--start-group \
-L$(LIBINF_LIB) -linf -Wl,-rpath=$(LIBINF_LIB)\
$(LIB_COMMON) \
-Wl,-rpath=$(CONFIG_INF_LIB_PATH)\
-Wl,--end-group
endif

tool : tool-min tool-benchmark

tool-min : $(BINS_MIN)

$(BINS_MIN) : CXXFLAGS := -g3 -std=c++11 -Wall $(INCS)
$(BINS_MIN) : LIB_$(D) := $(LIB_COMMON)

$(BINS_MIN) : $(MIN_SRCS)
	@printf "    %-8s $@\n" "LD" $(VOUT)
	$(Q)$(MKDIR_P) $(dir $@)
	$(Q)$(CXX) -o $@ $< $(CXXFLAGS) $(LIB_$(D))

tool-benchmark : $(BINS_BENCHMARK)

DL_PATH := $(TENSORFLOW_PATH)/tensorflow/lite/tools/make/downloads
INCS_$(D) := -I$(DL_PATH) \
	-I$(DL_PATH)/flatbuffers/include \
	-I$(DL_PATH)/absl \
	-I$(DL_PATH)/ruy \
	-I$(DL_PATH)/gemmlowp \
	-I$(TENSORFLOW_PATH)

$(BINS_BENCHMARK) : CXXFLAGS := -std=c++11 -Wno-sign-compare $(CFLAGS_COMMON) $(INCS_$(D)) -DTFLITE_WITHOUT_XNNPACK
$(BINS_BENCHMARK) : LIB_$(D) := $(LIB_COMMON) \
			 -Wl,-rpath=$(CONFIG_INF_LIB_PATH)\

$(BINS_BENCHMARK) : $(BENCHMARK_OBJS)
	@printf "    %-8s $@\n" "LD" $(VOUT)
	$(Q)$(MKDIR_P) $(dir $@)
	$(Q)$(CXX) -o $@ $^ $(CXXFLAGS) $(LIB_$(D))

install-tool: $(TARGET_BINS)
$(bindir)/%: $(root)/bin/%
	$(Q)$(MKDIR_P) $(bindir)
	$(Q)$(INSTALL) -m 755 $< $@

tool-clean:
	$(Q)$(RM) -r $(BINS_MIN) $(BINS_BENCHMARK) $(OBJ_PATH)

uninstall-tool: tool-clean
	$(Q)$(RM) $(TARGET_BINS)

.PHONY: call-tool
call-tool:
	@echo BIN: $(BINS_$(D))
	@echo SRC: $(BENCHMARK_SRCS) $(SRCS_CXX_$(D))
	@echo OBJ: $(BENCHMARK_OBJS)
	@echo $(subdir)
	@echo $(dir)

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
