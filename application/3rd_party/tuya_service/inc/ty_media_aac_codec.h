#ifndef _TY_MEDIA_AAC_CODEC_H_
#define _TY_MEDIA_AAC_CODEC_H_

// #include <stdlib.h>

//#include "fdk-aac/aacenc_lib.h"
//#include "libAACenc/include/aacenc_lib.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "aacenc_lib.h"
#pragma GCC diagnostic push

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef struct ty_media_aac_handle_ {
    HANDLE_AACENCODER   aacEncHandle;
    char*               pcmBuf;
    int                 pcmLen;
    int                 sampleLen;
} ty_media_aac_handle_s;


int ty_media_aac_encoder_init(ty_media_aac_handle_s* phdl, int channels, int sampleRate, int bitRate);


int ty_media_aac_encoder_data(ty_media_aac_handle_s* phdl, char* pData, int size,
                                char* aacOutbuf, int* outlen);


int ty_media_aac_encoder_uninit(ty_media_aac_handle_s* phdl);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
