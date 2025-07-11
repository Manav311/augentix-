################################################################################
# SDK related definitions
################################################################################

SDKSRC_DIR ?= $(realpath $(CURDIR)/../../..)

include $(SDKSRC_DIR)/application/internal.mk
include $(SDKSRC_DIR)/application/common.mk
include $(ABIVER_MAKE)
include $(LIBMETAL_SRC_PATH)/VERSION

################################################################################
# Build rules
################################################################################

root := $(realpath ./)
lib_gen_dir := $(root)/cmake_build/lib
lib_dir := $(root)/lib

INSTALL_PATH := $(libdir)

LIB_NAME = metal
LIB_REALNAME = lib$(LIB_NAME).so.$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
LIB_SONAME = lib$(LIB_NAME).so.$(VERSION_MAJOR)
LIB_LINKERNAME = lib$(LIB_NAME).so

.DEFAULT_GOAL := default

.PHONY: default
default: all

.PHONY: all
all:
ifeq ("$(wildcard ./cmake_build/Makefile)","")
	cmake $(LIBMETAL_SRC_PATH) -B ./cmake_build -DCMAKE_TOOLCHAIN_FILE=$(root)/hc21xx_linux.cmake
endif
	@echo build libmetal for Linux environment
	$(Q)$(MAKE) -C $(root)/cmake_build all
	$(Q)$(MKDIR_P) $(lib_dir)
	$(Q)$(CP) ${lib_gen_dir}/$(LIB_REALNAME) $(lib_dir)/$(LIB_REALNAME)
	$(Q)$(LN) -sfT $(lib_dir)/$(LIB_REALNAME) $(lib_dir)/$(LIB_SONAME)
	$(Q)$(LN) -sfT $(lib_dir)/$(LIB_REALNAME) $(lib_dir)/$(LIB_LINKERNAME)
	$(Q)$(CP) ${lib_gen_dir}/libmetal.a $(lib_dir)/libmetal.a
	$(Q)$(LN) -sfT ${lib_gen_dir}/include $(root)/include

.PHONY: clean
clean:
ifneq ("$(wildcard ./cmake_build/Makefile)","")
	$(Q)$(MAKE) -C cmake_build -j clean
endif
	$(Q)$(RM) -f $(lib_dir)/*

.PHONY: distclean
distclean: clean
	$(Q)$(RM) -rf ${root}/cmake_build
	$(Q)$(RM) -rf $(root)/include
	$(Q)$(RM) -rf $(lib_dir)

.PHONY: install
install: all
	$(Q)$(CP) $(LIB_OUT_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_REALNAME)
	$(Q)$(LN) -sfr $(INSTALL_PATH)/$(LIB_REALNAME) $(INSTALL_PATH)/$(LIB_SONAME)
	@echo $(LIB_SHARED_TARGET)

.PHONY: uninstall
uninstall: uninstall-so

