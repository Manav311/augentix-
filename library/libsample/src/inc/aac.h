#ifndef AAC_H_
#define AAC_H_
#ifdef MP4_ENABLE

#include "aacenc_lib.h"

typedef struct ty_media_aac_handle_ {
	HANDLE_AACENCODER enc;
	// encorder info
	//
	int frame_size;

	//
	int aot;
	int channels;
	int sample_rate;
	int bitrate;
	char *aac_buf;
} ty_media_aac_handle_s;

int aac_encoder_init(ty_media_aac_handle_s *phdl, int channels, int sample_rate, int bitrate, int aac_buffer_len);
int aac_encoder_data(ty_media_aac_handle_s *phdl, char *pcm, int nb_pcm, int nb_sample, char *aac, int *pnb_aac);
int aac_encoder_uninit(ty_media_aac_handle_s *phdl);
#endif
#endif
