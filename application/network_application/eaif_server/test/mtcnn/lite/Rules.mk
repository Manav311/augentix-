########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

######## unit-test for python vs c++ ##

D=MTCNN
INCS_$(D) := \
$(INCS) \
-I$(root)/inc \
-I$(root)/src/facereco

SRCS_$(D) := $(dir)/main.cc
OBJS_$(D) := $(SRCS_$(D):.cc=.o)
BINS_$(D) := $(dir)/test

test-mtcnn: $(BINS_$(D))

$(OBJS_$(D)): CFLAGS_LOCAL := $(INCS_$(D))

$(BINS_$(D)): $(OBJS_$(D)) $(TARGET_SLIB)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(LIB_PATH) $(CXXFLAGS)

.PHONY: call-mtcnn
call-mtcnn:
	echo $(BINS_$(D))
	echo $(OBJS_$(D))
	echo $(subdir)
	echo $(dir)

CLEAN_OBJS += $(OBJS_$(D)) $(BINS_$(D))

######### test for mtcnn / facenet memory

E=FACERECO_MEM
INCS_$(E) := \
$(INCS) \
-I$(root)/inc \
-I$(root)/src/facereco

SRCS_$(E) := $(dir)/face.cc
OBJS_$(E) := $(SRCS_$(E):.cc=.o)
BINS_$(E) := $(dir)/face

test-face: $(BINS_$(E))

$(OBJS_$(E)): CFLAGS_LOCAL := $(INCS_$(E))

$(BINS_$(E)): $(OBJS_$(E)) $(TARGET_SLIB)
	@printf "    %-8s $@\n" "CXX" $(VOUT)
	$(Q)$(CXX) -o $@ $^ $(LIB_PATH) $(CXXFLAGS)

.PHONY: call-face
call-face:
	echo $(BINS_$(D))
	echo $(OBJS_$(D))
	echo $(subdir)
	echo $(dir)

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
