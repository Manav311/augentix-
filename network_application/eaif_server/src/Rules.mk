########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

SLIB_CORE := $(LIB_PREFIX)/libeaif_core.a
SLIB_SERVICE := $(LIB_PREFIX)/libeaif_service.a
SLIB_C_CORE := $(LIB_PREFIX)/libeaif_c_core.a
SLIB_MIN := $(LIB_PREFIX)/libeaif_minimum.a

TARGET_SLIB_CORE += $(SLIB_CORE)
TARGET_SLIB_SERVICE += $(SLIB_SERVICE)
TARGET_SLIB_C_CORE += $(SLIB_C_CORE)
TARGET_SLIB_MIN += $(SLIB_MIN)

TARGET_SLIB += $(SLIB_CORE) $(SLIB_SERVICE)

### define rule for libeaif_core.a

INCS_CORE := \
$(INCS) \
-I$(root)/inc \
-I$(root)/src/classify \
-I$(root)/src/detect \
-I$(root)/src/facereco \
-I$(root)/src/human_classify \
-I$(root)/src/service

SRCS_CORE := \
$(wildcard $(dir)/*.cc) \
$(wildcard $(dir)/classify/*.cc) \
$(wildcard $(dir)/detect/*.cc) \
$(wildcard $(dir)/facereco/*.cc) \
$(wildcard $(dir)/human_classify/*.cc)

OBJS_CORE := $(SRCS_CORE:.cc=.o)

$(OBJS_CORE): CFLAGS_LOCAL := $(INCS_CORE)

$(SLIB_CORE): $(OBJS_CORE)
	@printf "  %-8s$@\n" "AR" $(VOUT)
	$(Q)install -d $(LIB_PREFIX)
	$(Q)$(AR) rcsD $@ $^

### define rule for libeaif_service.a

INCS_SERVICE := \
$(INCS) \
-I$(root)/inc \
-I$(root)/src/service

SRCS_SERVICE := $(wildcard $(dir)/service/*.cc)
OBJS_SERVICE := $(SRCS_SERVICE:.cc=.o)

$(OBJS_SERVICE): CFLAGS_LOCAL := $(INCS_SERVICE)

# for libeaif_service.a
$(SLIB_SERVICE): $(OBJS_SERVICE)
	@printf "    %-8s $@\n" "AR" $(VOUT)
	$(Q)install -d $(LIB_PREFIX)
	$(Q)$(AR) rcsD $@ $^


### define rule for libeaif_c_core.a

INCS_C_CORE := \
$(INCS) \
-I$(root)/inc/ \
-I$(root)/inc/c_api

SRCS_CXX_CORE := $(wildcard $(dir)/c_api/*.cc)
SRCS_C_CORE := $(wildcard $(dir)/c_api/*.c)
OBJS_C_CORE := $(SRCS_C_CORE:.c=.o) $(SRCS_CXX_CORE:.cc=.o)

$(OBJS_C_CORE): CFLAGS_LOCAL := $(INCS_C_CORE)

$(SLIB_C_CORE): $(OBJS_C_CORE) $(SLIB_CORE)
	@printf "    %-8s $@\n" "AR" $(VOUT)
	$(Q)install -d $(LIB_PREFIX)
	$(Q)$(AR) rcsD $@ $^

.PHONY: call-c
call-c:
	@echo $(OBJS_C_CORE)
	@echo $(SRCS_C_CORE)

### define rule for libeaif_minimum.a

INCS_MIN := \
-I$(MPP_INC) \
-I$(root)/inc/c_api/minimum

SRCS_MIN := $(wildcard $(dir)/minimum/*.c)
OBJS_MIN := $(SRCS_MIN:.c=.o)
SRCS_MIN := $(wildcard $(dir)/minimum/*.cc)
OBJS_MIN += $(SRCS_MIN:.cc=.o)

$(OBJS_MIN): CFLAGS_LOCAL := $(INCS_MIN) $(FAKE_INC)

$(SLIB_MIN): $(OBJS_MIN)
	@printf "    %-8s $@\n" "AR" $(VOUT)
	$(Q)install -d $(LIB_PREFIX)
	$(Q)$(AR) rcsD $@ $^

micro: $(SLIB_MIN)

lib: $(SLIB_CORE) $(SLIB_SERVICE) $(SLIB_C_CORE)

lib-clean:
	@printf "  CLEAN \t%s\n" $(TARGET_SLIB)
	$(Q)rm -rf $(TARGET_SLIB)

CLEAN_OBJS += $(OBJS_CORE) $(OBJS_SERVICE) $(OBJS_C_CORE) $(OBJS_MIN)

.PHONY: call-min
call-min:
	@echo $(OBJS_MIN)

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
