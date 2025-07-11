#include "aac.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>

#include "log_define.h"
#include "agtx_types.h"

int is_little_endian()
{
	int test_num = 0xff;
	const unsigned char *ptr = (const unsigned char *)&test_num;
	if (ptr[0] == 0xff) {
		return 1;
	}
	return 0;
}

//asdadad



int AAC_encoderInit(TyMediaAACHandle *phdl, int channels, int sample_rate, int bitrate)
{
	AACENC_ERROR err = AACENC_OK;

	phdl->aot = 2;
	phdl->channels = channels;
	phdl->sample_rate = sample_rate;
	phdl->bitrate = bitrate;
	CHANNEL_MODE mode = MODE_INVALID;

	if (!is_little_endian()) {
		return -1;
	}

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
		flv_server_log_err("Unable to open fdkaac encoder: %d", err);
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_AOT, phdl->aot)) != AACENC_OK) { //aac lc
		flv_server_log_err("Unable to set the AOT:%d", err);
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK) {
		flv_server_log_err("Unable to set the sample rate:%d", err);
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_CHANNELMODE, mode)) != AACENC_OK) { //2 channle
		flv_server_log_err("Unable to set the channel mode:%d", err);
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_BITRATE, bitrate)) != AACENC_OK) {
		flv_server_log_err("Unable to set the bitrate:%d", err);
		return err;
	}

	if ((err = aacEncoder_SetParam(phdl->enc, AACENC_TRANSMUX, 2)) != AACENC_OK) { //0-raw 2-adts
		flv_server_log_err("Unable to set the ADTS transmux:%d", err);
		return err;
	}

	if ((err = aacEncEncode(phdl->enc, NULL, NULL, NULL, NULL)) != AACENC_OK) {
		flv_server_log_err("Unable to initialize the encoder:%d", err);
		return err;
	}

	AACENC_InfoStruct info = { 0 };

	if ((err = aacEncInfo(phdl->enc, &info)) != AACENC_OK) {
		flv_server_log_err("Unable to get the encoder info:%d", err);
		return err;
	}

	phdl->pcm_buf = malloc(info.frameLength * 4);
	phdl->frame_size = info.frameLength * 2;
	phdl->pcm_len = 0;

	flv_server_log_notice("AAC frame_size = %d", phdl->frame_size);

	return 0;
}

int AAC_encoderGetData(TyMediaAACHandle *phdl, const char *pcm, int nb_pcm, int nb_samples, char *aac, int *pnb_aac)
{
	AGTX_UNUSED(nb_samples);

	AACENC_ERROR err = AACENC_OK;
	INT iidentify = IN_AUDIO_DATA;
	INT oidentify = OUT_BITSTREAM_DATA;
	INT ibuffer_element_size = 2; // 16bits.
	// The intput pcm must be resampled to fit the encoder,
	// for example, the intput is 2channels but encoder is 1channels,
	// then we should resample the intput pcm to 1channels
	// to make the intput pcm size equals to the encoder calculated size(ibuffer_size).
	// std::cout << ibuffer_size << std::endl;

	memcpy(phdl->pcm_buf + phdl->pcm_len, pcm, nb_pcm);
	phdl->pcm_len += nb_pcm;

	if (phdl->pcm_len < phdl->frame_size) {
		return -1;
	}

	AACENC_BufDesc ibuf = { 0 };
	AACENC_InArgs iargs = { 0 };
	AACENC_OutArgs oargs = { 0 };

	iargs.numInSamples = phdl->frame_size / 2;
	ibuf.numBufs = 1;
	ibuf.bufs = (void **)&phdl->pcm_buf;
	ibuf.bufferIdentifiers = &iidentify;
	ibuf.bufSizes = &phdl->frame_size;
	ibuf.bufElSizes = &ibuffer_element_size;

	INT obuffer_element_size = 1;
	AACENC_BufDesc obuf = { 0 };

	INT obuffer_size = *pnb_aac;
	obuf.numBufs = 1;
	obuf.bufs = (void **)&aac;
	obuf.bufferIdentifiers = &oidentify;
	obuf.bufSizes = &obuffer_size;
	obuf.bufElSizes = &obuffer_element_size;

	if ((err = aacEncEncode(phdl->enc, &ibuf, &obuf, &iargs, &oargs)) != AACENC_OK) {
		// Flush ok, no bytes to output anymore.
		if (err == AACENC_ENCODE_EOF) {
			*pnb_aac = 0;
		}
	}

	*pnb_aac = oargs.numOutBytes;
	int left_len = phdl->pcm_len - phdl->frame_size;

	if (left_len > 0) {
		memmove(phdl->pcm_buf, phdl->pcm_buf + phdl->frame_size, left_len);
		phdl->pcm_len = left_len;
	} else {
		phdl->pcm_len = 0;
	}

	return err;
}

int AAC_encoderUninit(TyMediaAACHandle *phdl)
{
	aacEncClose(&phdl->enc);

	if (phdl->pcm_buf) {
		free(phdl->pcm_buf);
		phdl->pcm_buf = NULL;
	}

	phdl->pcm_len = 0;
	phdl->frame_size = 0;

	return 0;
}
