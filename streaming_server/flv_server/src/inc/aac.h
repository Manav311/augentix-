#ifndef AAC_H_
#define AAC_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "aacenc_lib.h"
#pragma GCC diagnostic push

typedef struct ty_media_aac_handle {
	HANDLE_AACENCODER enc;
	// encorder info
	//
	int frame_size;
	//
	int aot;
	int channels;
	int sample_rate;
	int bitrate;
	char *pcm_buf;
	int pcm_len;
} TyMediaAACHandle;

int AAC_encoderInit(TyMediaAACHandle *phdl, int channels, int sample_rate, int bitrate);
int AAC_encoderGetData(TyMediaAACHandle *phdl, const char *pcm, int nb_pcm, int nb_sample, char *aac, int *pnb_aac);
int AAC_encoderUninit(TyMediaAACHandle *phdl);

#endif
