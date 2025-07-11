#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "media/ty_media_aac_codec.h"
#include "ty_media_aac_codec.h"
//dmp file for debug
//FILE *out;
#include "tuya_utils.h"


int ty_media_aac_encoder_init(ty_media_aac_handle_s* phdl, int channels, int sampleRate, int bitRate)
{
	AACENC_InfoStruct info = {0};
	//out = fopen("/tmp/live.aac", "wb");
	//if ( IS_LITTLE_ENDIAN() != 1) {
	if (is_little_endian() != 1) {
		printf("unsupported Endian\n");
	}

	if(aacEncOpen(&phdl->aacEncHandle, 0, channels) != AACENC_OK) {
		printf("Unable to open fdkaac encoder\n");
		return -1;
	}
 
	if(aacEncoder_SetParam(phdl->aacEncHandle, AACENC_AOT, 2) != AACENC_OK) {  //aac lc
		printf("Unable to set the AOT\n");
		return -1;
	}
 
	if(aacEncoder_SetParam(phdl->aacEncHandle, AACENC_SAMPLERATE, sampleRate) != AACENC_OK) {
		printf("Unable to set the AOT\n");
		return -1;
	}

	if(aacEncoder_SetParam(phdl->aacEncHandle, AACENC_CHANNELMODE, MODE_1) != AACENC_OK) {  //2 channle
		printf("Unable to set the channel mode\n");
		return -1;
	}

	if(aacEncoder_SetParam(phdl->aacEncHandle, AACENC_BITRATE, bitRate) != AACENC_OK) {
		printf("Unable to set the bitrate\n");
		return -1;
	}

	if(aacEncoder_SetParam(phdl->aacEncHandle, AACENC_TRANSMUX, 2) != AACENC_OK) { //0-raw 2-adts
		printf("Unable to set the ADTS transmux\n");
		return -1;
	}
 
	if(aacEncEncode(phdl->aacEncHandle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		printf("Unable to initialize the encoder\n");
		return -1;
	}

	if(aacEncInfo(phdl->aacEncHandle, &info) != AACENC_OK) {
		printf("Unable to get the encoder info\n");
		return -1;
	}

	phdl->pcmBuf = (char*)malloc(info.frameLength * 4);
	phdl->pcmLen = 0;
	phdl->sampleLen = info.frameLength * 2;

	printf("maxAncBytes: %d, maxOutBufBytes: %d, frameLength: %d\n", 
			info.maxAncBytes, info.maxOutBufBytes, info.frameLength);

	return 0;
}

int ty_media_aac_encoder_data(ty_media_aac_handle_s* phdl, char* pData, int size,
								char* aacOutbuf, int* outlen)
{
	int ret = -1;

	memcpy(phdl->pcmBuf + phdl->pcmLen, pData, size);
	phdl->pcmLen += size;

	if(phdl->pcmLen < phdl->sampleLen) {
		return -1;
	}

	AACENC_BufDesc in_buf = {0}, out_buf = {0};
	AACENC_InArgs in_args = {0};
	AACENC_OutArgs out_args = {0};
	int in_identifier = IN_AUDIO_DATA;
	int in_elem_size = 2;

	in_args.numInSamples = phdl->sampleLen/2;  // pcm字节数
	in_buf.numBufs = 1;
	in_buf.bufs = (void**)&phdl->pcmBuf;  // pcm数据指针
	in_buf.bufferIdentifiers = &in_identifier;
	in_buf.bufSizes = &phdl->sampleLen;
	in_buf.bufElSizes = &in_elem_size;

	int out_identifier = OUT_BITSTREAM_DATA;
	void *out_ptr = aacOutbuf;
	int out_size = *outlen;
	int out_elem_size = 1;
	out_buf.numBufs = 1;
	out_buf.bufs = &out_ptr;
	out_buf.bufferIdentifiers = &out_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &out_elem_size;

	if((aacEncEncode(phdl->aacEncHandle, &in_buf, &out_buf, &in_args, &out_args)) == AACENC_OK) {
		if(out_args.numOutBytes > 0) {
			*outlen = out_args.numOutBytes;
            //fprintf(stderr,"AAC outlength  %d  <-----\n",out_args.numOutBytes);
			ret = 0;
		}
	} else {
		fprintf(stderr, "Encoding aac failed\n");
	}

	int left_len = phdl->pcmLen - phdl->sampleLen;
	if(left_len > 0) {
		char temp[2048] = {0};
		memcpy(temp, phdl->pcmBuf + phdl->sampleLen, left_len);
		memcpy(phdl->pcmBuf, temp, left_len);
		phdl->pcmLen = left_len;
	} else {
		phdl->pcmLen = 0;
	}

	return ret;
}

int ty_media_aac_encoder_uninit(ty_media_aac_handle_s* phdl)
{
	aacEncClose(&phdl->aacEncHandle);

	if(phdl->pcmBuf != NULL) {
		free(phdl->pcmBuf);
		phdl->pcmBuf = NULL;
	}

    phdl->pcmLen = 0;
    phdl->sampleLen = 0;

	return 0;
}

