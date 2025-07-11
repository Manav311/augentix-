########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

APP := $(dir)/eaif_server
CLIENT := $(dir)/eaif_client
DEMO := $(dir)/eaif_server_demo

### Define rule for eaif_server ###

INCS_APP := -I$(root)/inc
SRCS_APP := $(dir)/eaif_server.cc
OBJS_APP := $(SRCS_APP:.cc=.o)
$(OBJS_APP) : CFLAGS_LOCAL := $(INCS_APP)

$(APP)_g: $(TARGET_SLIB)

$(APP)_g: $(OBJS_APP)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(CXXFLAGS) $(LD_FLAGS) -Wl,--start-group $(LIB_PATH) $(TARGET_SLIB) -Wl,--end-group

### Define rule for eaif_client ###

INCS_CLIENT := -I$(root)/inc
SRCS_CLIENT := $(dir)/eaif_client.cc
OBJS_CLIENT := $(SRCS_CLIENT:.cc=.o)
$(OBJS_CLIENT) : CFLAGS_LOCAL := $(INCS_CLIENT)


$(CLIENT)_g: $(OBJS_CLIENT)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(CXXFLAGS) $(LD_FLAGS) -pthread -lm -ldl

### Define rule for eaif_server_demo ###

INCS_DEMO := \
-I$(root)/inc \
-I$(root)/src/classify \
-I$(root)/src/detect \
-I$(root)/src/facereco \
-I$(root)/src/human_classify \
-I$(root)/src/demo

SRCS_DEMO := $(dir)/eaif_server_demo.cc \
$(wildcard $(root)/src/demo/*.cc)
OBJS_DEMO := $(SRCS_DEMO:.cc=.o)
$(OBJS_DEMO) : CFLAGS_LOCAL := $(INCS_DEMO)

$(DEMO)_g: $(TARGET_SLIB_CORE)

$(DEMO)_g: $(OBJS_DEMO)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(CXXFLAGS) $(LD_FLAGS) -Wl,--start-group $(LIB_PATH) $(TARGET_SLIB_CORE) -Wl,--end-group

### Define rule for strip & primary make rule ###

$(APP) $(CLIENT) $(DEMO) : % : %_g
	@printf "    %-8s $@\n" "STRIP" $(VOUT)
	$(Q)$(STRIP) -p $< -o $@

CLEAN_OBJS += $(OBJS_APP) $(OBJS_CLIENT) $(OBJS_DEMO) $(APP) $(CLIENT) $(DEMO) $(APP)_g $(CLIENT)_g $(DEMO)_g
TARGET_BINS += $(addprefix $(PREFIX)/, $(notdir $(APP) $(CLIENT) $(DEMO)))

app: $(APP)
client: $(CLIENT)
demo : $(DEMO)

install-app: install-server install-client

install-server: $(APP)
	@printf "    %-8s %s\n" "INSTALL" $< $(VOUT)
	$(Q)install -m 755 $< $(PREFIX)

install-client: $(CLIENT)
	@printf "    %-8s %s\n" "INSTALL" $< $(VOUT)
	$(Q)install -m 755 $< $(PREFIX)

install-demo: $(DEMO)
	@printf "    %-8s %s\n" "INSTALL" $< $(VOUT)
	$(Q)install -m 755 $< $(PREFIX)

app-clean:
	@printf "    %-8s APP\n" "CLEAN" $(VOUT)
	$(Q)rm -rf $(TARGET_BINS)

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
