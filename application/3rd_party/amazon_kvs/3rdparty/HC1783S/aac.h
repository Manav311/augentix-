#ifndef HC1783S_AAC_H_
#define HC1783S_AAC_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "aacenc_lib.h"
#pragma GCC diagnostic push

#define DEBUG_ENABLED 0
#define log_err(fmt, args...) printf("[ERROR] " fmt, ##args)
#define log_warn(fmt, args...) printf("[WARNING] " fmt, ##args)
#define log_notice(fmt, args...) printf("[NOTICE] " fmt, ##args)
#define log_info(fmt, args...) printf("[INFO] " fmt, ##args)

#if DEBUG_ENABLED
#define log_debug(fmt, args...) printf("[DEBUG] " fmt, ##args)
#else
#define log_debug(fmt, args...)
#endif

typedef struct ty_media_aac_handle {
    HANDLE_AACENCODER enc;
    // encorder info
    //
    int frameSize;
    //
    int aot;
    int channels;
    int sampleRate;
    int bitrate;
    char *pcmBuf;
    int pcmLen;
} TyMediaAACHandle;

int AAC_encoderInit(TyMediaAACHandle *phdl, int channels, int sample_rate, int bitrate);
int AAC_encoderGetData(TyMediaAACHandle *phdl, char *pcm, int nb_pcm, int nb_sample, char *aac, int *pnb_aac);
int AAC_encoderUninit(TyMediaAACHandle *phdl);

#endif //HC1783S_AAC_H_
