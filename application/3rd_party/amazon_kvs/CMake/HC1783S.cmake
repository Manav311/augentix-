if(BOARD STREQUAL "HC1783S")
    if($ENV{CROSS_COMPILE} STREQUAL "arm-augentix-linux-uclibcgnueabihf-")
        set(USE_MUCLIBC ON)
        set(BOARD_DESTINATION_PLATFORM arm-unknown-linux-uclibc)
    endif()

    set(BOARD_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/${BOARD})
    set(SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
 
    set(BOARD_SRCS
            ${BOARD_SDK_DIR}/aac.c
            ${BOARD_SDK_DIR}/audio_utils.c
    )
    set(BOARD_INCS_DIR
            ${BOARD_SDK_DIR}
            ${SDK_DIR}/mpp/include
            ${SDK_DIR}/extdrv/audio/include
            ${SDK_DIR}/buildroot/output/build/alsa-lib-1.2.4/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libAACdec/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libAACenc/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libFDK/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libMpegTPDec/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libMpegTPEnc/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libPCMutils/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libSBRdec/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libSBRenc/include
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/libSYS/include
    )
    set(BOARD_LIBS_DIR
            ${SDK_DIR}/mpp/lib
            ${SDK_DIR}/buildroot/output/build/alsa-lib-1.2.4/src/.libs
            ${SDK_DIR}/buildroot/output/build/fdk-aac-2.0.1/.libs
    )
    set(BOARD_LIBS_SHARED
            mpp pthread m rt asound fdk-aac
    )
    set(BOARD_LIBS_STATIC
            mpp pthread m rt asound fdk-aac
    )
endif()
