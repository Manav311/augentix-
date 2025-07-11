#include "tutk_aac.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>

#include "log_define.h"

int aac_encoder_init(ty_media_aac_handle_s *phdl, int channels, int sample_rate, int bitrate)
{
	AACENC_ERROR err = AACENC_OK;

	phdl->aot = 2;
	phdl->channels = channels;
	phdl->sample_rate = sample_rate;
	phdl->bitrate = bitrate;
	CHANNEL_MODE mode = MODE_INVALID;

	switch (channels) {
	case 1:
		mode = MODE_1;
		break;
	case 2:
		mode = MODE_2;
		break;
	default:
		return -1;
	}

	if ((err = aacEncOpen(&phdl->enc, 0, channels)) != AACENC_OK) {
		fprintf(stderr, "Unable to open fdkaac encoder\n");
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_AOT, phdl->aot)) != AACENC_OK) { //aac lc
		fprintf(stderr, "Unable to set the AOT\n");
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK) {
		fprintf(stderr, "Unable to set the AOT\n");
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_CHANNELMODE, mode)) != AACENC_OK) { //2 channle
		fprintf(stderr, "Unable to set the channel mode\n");
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_BITRATE, bitrate)) != AACENC_OK) {
		fprintf(stderr, "Unable to set the bitrate\n");
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_TRANSMUX, 2)) != AACENC_OK) { //0-raw 2-adts
		fprintf(stderr, "Unable to set the ADTS transmux\n");
		return err;
	}

	if ((err = aacEncEncode(phdl->enc, NULL, NULL, NULL, NULL)) != AACENC_OK) {
		fprintf(stderr, "Unable to initialize the encoder\n");
		return err;
	}

	AACENC_InfoStruct info = { 0 };

	if ((err = aacEncInfo(phdl->enc, &info)) != AACENC_OK) {
		fprintf(stderr, "Unable to get the encoder info\n");
		return err;
	}

	phdl->frame_size = info.frameLength;
	tutkservice_log_info("AAC frame_size = %d\n", phdl->frame_size);

	return 0;
}

int aac_encoder_data(ty_media_aac_handle_s *phdl, char *pcm, int nb_pcm, int nb_samples, char *aac, int *pnb_aac)
{
	AACENC_ERROR err = AACENC_OK;
	INT iidentify = IN_AUDIO_DATA;
	INT oidentify = OUT_BITSTREAM_DATA;
	INT ibuffer_element_size = 2; // 16bits.
	INT ibuffer_size = 2 * phdl->channels * nb_samples;
	// The intput pcm must be resampled to fit the encoder,
	// for example, the intput is 2channels but encoder is 1channels,
	// then we should resample the intput pcm to 1channels
	// to make the intput pcm size equals to the encoder calculated size(ibuffer_size).
	// std::cout << ibuffer_size << std::endl;
	if (ibuffer_size != nb_pcm) {
		return -1;
	}

	AACENC_BufDesc ibuf = { 0 };
	if (nb_pcm > 0) {
		ibuf.numBufs = 1;
		ibuf.bufs = (void **)&pcm;
		ibuf.bufferIdentifiers = &iidentify;
		ibuf.bufSizes = &ibuffer_size;
		ibuf.bufElSizes = &ibuffer_element_size;
	}
	AACENC_InArgs iargs = { 0 };
	if (nb_pcm > 0) {
		iargs.numInSamples = phdl->channels * nb_samples;
	} else {
		iargs.numInSamples = -1;
	}
	INT obuffer_element_size = 1;
	INT obuffer_size = *pnb_aac;
	AACENC_BufDesc obuf = { 0 };
	obuf.numBufs = 1;
	obuf.bufs = (void **)&aac;
	obuf.bufferIdentifiers = &oidentify;
	obuf.bufSizes = &obuffer_size;
	obuf.bufElSizes = &obuffer_element_size;
	AACENC_OutArgs oargs = { 0 };
	if ((err = aacEncEncode(phdl->enc, &ibuf, &obuf, &iargs, &oargs)) != AACENC_OK) {
		// Flush ok, no bytes to output anymore.
		if (!pcm && err == AACENC_ENCODE_EOF) {
			*pnb_aac = 0;
			return AACENC_OK;
		}
		return err;
	}

	*pnb_aac = oargs.numOutBytes;

	return err;
}

int aac_encoder_uninit(ty_media_aac_handle_s *phdl)
{
	aacEncClose(&phdl->enc);

	return 0;
}
