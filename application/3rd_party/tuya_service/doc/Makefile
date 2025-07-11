#####################
# Setup Environment #
#####################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../../..)
include $(SDKSRC_DIR)/build/sdksrc.mk

#################
# Build Targets #
#################

OUTPUT := output

.PHONY: doc
doc:
	@printf "  %-8s$(OUTPUT)\n" "DOXYGEN"
	$(Q)(cat $(DOXYGEN_PATH)/global_c_lang.conf ./local.conf; echo OUTPUT_DIRECTORY=./$(OUTPUT)) | doxygen - > /dev/null 2>&1
	$(Q)cp $(DOXYGEN_PATH)/augentix_logo.ico $(OUTPUT)/html/augentix_logo.ico

.PHONY: clean
clean:
	@printf "  %-8s$(OUTPUT)\n" "CLEAN"
	$(Q)rm -rf $(OUTPUT)

.PHONY: distclean
distclean: clean
