#ifndef AGTX_AUDIO_CONF_H_
#define AGTX_AUDIO_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_AUDIO_CODEC_NONE,
	AGTX_AUDIO_CODEC_PCM,
	AGTX_AUDIO_CODEC_ULAW,
	AGTX_AUDIO_CODEC_ALAW,
	AGTX_AUDIO_CODEC_G726
} AGTX_AUDIO_CODEC_E;


typedef struct {
	AGTX_AUDIO_CODEC_E codec;
	AGTX_INT32 enabled;
	AGTX_INT32 gain;
	AGTX_INT32 sampling_bit;
	AGTX_INT32 sampling_frequency;
} AGTX_AUDIO_CONF_S;


#endif /* AGTX_AUDIO_CONF_H_ */
