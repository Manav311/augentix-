########################################
# [DON'T TOUCH] Traverse directory tree
########################################
sp             := $(sp).x
dirstack_$(sp) := $(dir)
dir            := $(subdir)

########################################
# Define Node in directory tree
########################################

subdir := $(dir)/mtcnn/lite
include $(subdir)/Rules.mk

subdir := $(dir)/image
include $(subdir)/Rules.mk

subdir := $(dir)/c_api
include $(subdir)/Rules.mk

subdir := $(dir)/crow
include $(subdir)/Rules.mk

subdir := $(dir)/minimum
include $(subdir)/Rules.mk

test: test-mtcnn test-c

########################################
# [DON'T TOUCH] Traverse directory tree
########################################
dir		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
