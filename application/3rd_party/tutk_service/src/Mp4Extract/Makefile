SDKSRC_DIR  = $(realpath $(CURDIR)/../../../../../)

include $(SDKSRC_DIR)/application/internal.mk
include $(APP_PATH)/common.mk

TARGET =extract_avc
SHARE_LIB_TARGET = libextract.so
MAIN_CXX = main.cpp

CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
CXXFLAGS = -std=c++11 -g -O2 -fPIC

SRC = Mp42Aac_extract.cpp Mp42Avc_extract.cpp $(BENTO4_LIB)/libap4.a
INC = extract.h
BENTO4_INCS = $(addprefix -I,$(BENTO4_INC))

# specify targets
.DEFAULT_GOAL := share

.PHONY: clean $(TARGET) 
$(TARGET):
	$(CXX) -o $(TARGET) $(MAIN_CXX) $(SRC) -I$(BENTO4_INCS)

clean :
	rm -f $(TARGET)* $(SHARE_LIB_TARGET)

.PHONY: share
share:
	$(Q)$(CXX) $(CXXFLAGS) -shared -o $(SHARE_LIB_TARGET) $(SRC) -I$(BENTO4_INCS) -s -DNDEBUG

.PHONY: install uninstall
install:
	cp -f $(SHARE_LIB_TARGET) $(SYSTEM_LIB)
uninstall:
	rm -rf $(SYSTEM_LIB)/$(SHARE_LIB_TARGET)


