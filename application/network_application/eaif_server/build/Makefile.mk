USE_TFLITE     ?= 1
USE_ARMNN      ?= 0
USE_OPENCV     ?= 0
USE_C4         ?= 0
USE_MICROLITE  ?= 0

USE_CROW ?= 1
USE_JSON ?= 1

USE_MPI ?= 1

USE_FAKE_MPI ?=0

LOCAL_LIB_PATH:=${HOME}/local/libs

MSG :=
VALID := 1

# MAKEFLAGS += -j

TARGET_ARCH?=HC17X3
ifeq ($(CONFIG_HC1703_1723_1753_1783S), y)
	ifeq ($(CROSS_COMPILE), arm-linux-gnueabi-)
		TARGET_ARCH:=HC17X3
	else ifeq ($(CROSS_COMPILE), arm-linux-gnueabihf-)
		TARGET_ARCH:=HC17X3-hf
	else
		VALID := 0
		MSG := Cannot build eaif-server with HC17X3 using Toolchain: $(CROSS_COMPILE), please check your product config!
	endif
else
	VALID := 0
	MSG := Cannot build eaif-server with HC17X2 using Toolchain: $(CROSS_COMPILE), please check your product config!
endif

HOST?=${TARGET_ARCH}
VERBOSE?=0

SDK_LIB:=0

ifeq ($(HOST), HC17X3)
	TARGET_CROSS_COMPILE?=${CROSS_COMPILE}
	LIB_ENV:=$(PACKAGE_PATH)
	MACHINE_OPT?= -mfloat-abi=soft -mcpu=$(CONFIG_TARGET_CPU)
else ifeq ($(HOST), HC17X3-hf)
	TARGET_CROSS_COMPILE?=${CROSS_COMPILE}
	LIB_ENV:=$(PACKAGE_PATH)
	MACHINE_OPT?= -mcpu=$(CONFIG_TARGET_CPU) -mfpu=neon-vfpv4 -ftree-vectorize -DSTBI_NEON
else ifeq ($(HOST), linux)
	TARGET_CROSS_COMPILE?=
	LIB_ENV:=$(LOCAL_LIB_PATH)/linux
else ifeq ($(HOST), gnueabihf)
	TARGET_CROSS_COMPILE?=arm-linux-gnueabihf-
	LIB_ENV:=$(LOCAL_LIB_PATH)/armnn-devenv
	MACHINE_OPT?= -DSTBI_NEON -march=armv7-a -mfpu=neon-vfpv4 -ftree-vectorize
endif

export PKG_CONFIG_PATH := ${PKG_CONFIG_PATH}:${LIB_ENV}/pkg-config

# temp boost & json library for HC17X3

JSON_PATH=$(LIB_ENV)/json-c
JSON_INC=-I$(JSON_PATH)/include/json-c
JSON_LIB_PATH=-L$(JSON_PATH)/lib
JSON_LIB=-ljson-c

BOOST_PATH=$(LIB_ENV)/boost_1_74_0
BOOST_INC=-I$(BOOST_PATH)/include
BOOST_LIB_PATH=-L$(BOOST_PATH)/lib
BOOST_LIB=-lboost_system -lboost_filesystem -lboost_program_options

LIBTFLITE_PATH=$(LIB_ENV)/tflite
LIBTFLITE_INC=$(LIBTFLITE_PATH)/inc
LIBTFLITE_LIB=$(LIBTFLITE_PATH)/lib

SDK_LIB := 0
ifeq ($(HOST), HC17X3)
	SDK_LIB := 1
else ifeq ($(HOST), HC17X3-hf)
	SDK_LIB := 1
endif

ifeq ($(SDK_LIB), 1)
	JSON_INC:=-I$(JSON_ROOT)
	JSON_LIB_PATH:=-L$(JSON_ROOT)/.libs
	JSON_LIB:=-ljson-c

	BOOST_INC:= -I$(BOOST_ROOT)
	BOOST_LIB_PATH:= -L$(BOOST_ROOT)/stage/lib
	BOOST_LIB:=-lboost_system -lboost_filesystem -lboost_program_options

	LIBTFLITE_INC:=$(TFLITE_INC)
	LIBTFLITE_LIB:=$(TFLITE_LIB)
endif

AUX_MPI_INC := -I$(MPP_INC)
AUX_MPI_LIB_PATH := -L$(MPP_LIB)
AUX_MPI_LIB := -lmpp -lrt

#####################################################################################

ARMNN_PATH=$(LIB_ENV)/armnn_20_05
ARMNN_INC=-I$(ARMNN_PATH)/include -I$(ARMNN_PATH)/include/armnnUtils
ARMNN_LIB_PATH=-pie -L$(ARMNN_PATH)/lib -Wl,-rpath=$(ARMNN_PATH)/lib
ARMNN_LIB= -larmnn -larmnnTfLiteParser -pthread

ARM_COMPUTE_PATH=$(LIB_ENV)/ComputeLibrary_master
ARM_COMPUTE_INC=-I$(ARM_COMPUTE_PATH) -I$(ARM_COMPUTE_PATH)/include
ARMNN_INC += $(ARM_COMPUTE_INC)

# armnn does not consume arm_compute for scheduler as there is no cpuacc
ifeq ($(HOST), HC17X3)
ARMNN_LIB_PATH+=-L$(ARM_COMPUTE_PATH)/build/share -Wl,-rpath=$(ARM_COMPUTE_PATH)/build/share
ARMNN_LIB+= -larm_compute
endif

OPENCV_PATH=$(LIB_ENV)/opencv
OPENCV_INC=-I$(OPENCV_PATH)/include
OPENCV_LIB_PATH= -L$(OPENCV_PATH)/lib -L$(OPENCV_PATH)/share/OpenCV/3rdparty/lib
FFMPEG_PATH=$(LIB_ENV)/ffmpeg

ifeq ($(HOST), gnueabihf)
	OPENCV_LIB = `pkg-config --static --libs --cflags opencv`
else ifeq ($(SDK_LIB), 1)
	OPENCV_LIB = -Wl,-rpath=$(OPENCV_PATH)/lib -L$(OPENCV_PATH)/lib -lopencv_world
	OPENCV_LIB += -Wl,-rpath=$(FFMPEG_PATH) -L$(FFMPEG_PATH) -lavutil -lavformat -lavdevice -lswscale -lavutil -lswresample -ldl -lm -lpthread -lrt
else
#OPENCV_LIB = -lopencv_world -llibjpeg-turbo -llibjasper -lIlmImf -lzlib -L/usr/lib/x86_64-linux-gnu -ltiff -ldc1394 -ldl -lm -lpthread -lrt
#OPENCV_LIB = -lopencv_world -Wl,-rpath=$(OPENCV_PATH)/lib
	OPENCV_LIB = `pkg-config --static --libs --cflags opencv`
endif


C4_PATH=$(LIB_ENV)/c4
C4_INC=-I$(C4_PATH)/include
C4_LIB_PATH=-Wl,-rpath=$(C4_PATH) -L$(C4_PATH) -lc4_tool

THIRD_PARTY_INC= -I./3rd_party/stb -I./3rd_party/crow

Q?=@
ifeq ($(VERBOSE),1)
Q=
endif

USE_BOOST=0
ifeq ($(USE_ARMNN),1)
	USE_BOOST=1
endif
ifeq ($(USE_CROW),1)
	USE_BOOST=1
endif

VOUT=
ifeq ($(USE_TFLITE),0)
	ifeq ($(USE_ARMNN),0)
$(warning Warning! Both tflite and armnn libraries are disabled!)
	endif
endif

ifeq ($(USE_TFLITE),1)
	ifeq ($(USE_MICRO),1)
		VALID := 0
		MSG := Error! Cannot enable both tflite and microlite library!
	endif
endif

ifeq ($(USE_CROW), 0)
$(warning Warning! http service is not supported)
endif

ifeq ($(USE_JSON), 0)
$(warning Warning! json library is not supported)
endif

