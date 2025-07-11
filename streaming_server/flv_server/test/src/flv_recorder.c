#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <errno.h>
#include "time.h"

#include "http_flv.h"
#include "flv_muxer.h"
#include "log_define.h"

#include "audio.h"
#include "video.h"


#define FLV_DEBUG
#ifdef FLV_DEBUG
#define LOG(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);
#else
#define LOG(format, args...)
#endif
#define ERR(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);
#define INFO(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);

static const char *g_device[] = { "default", "hw:0,0" }; /* sound device */
int g_run_flag = 0;
FILE *g_fp;

int writeFLVOutput(void *src, uint32_t len, int fd)
{
	if (g_fp == NULL) {
		ERR("fp is null\r\n");
		return -EIO;
	}

	fwrite(src, len, 1, g_fp);
	return 0;
}

int openFLVOutput(char *filename)
{
	if (NULL == filename) {
		return -EIO;
	}

	g_fp = fopen(filename, "wb");
	if (g_fp == NULL) {
		fprintf(stderr, "failed to open flv\r\n");
	}

	return 0;
}

int closeFLVOutput()
{
	fclose(g_fp);
	return 0;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		INFO("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		INFO("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		INFO("Caught SIGPIPE!\n");
		ERR("pls re-start flv-recorder\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
	g_run_flag = 0;
}

void help()
{
	printf("[Usage]:\r\n");
	printf("-i save .flv path, dft ./save_flv.flv\r\n");
	printf("-c video chn idx\r\n");
	printf("-a has audio or not 1 or 0\r\n");
	printf("-h help()\r\n");
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int c;
	uint8_t chn_num = 0;
	int isAudio = 1;
	char savePath[128];
	snprintf(&savePath[0], 128, "%s", "./save_flv.flv");

	while ((c = getopt(argc, argv, "hc:a:i:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'c':
			chn_num = atoi(argv[optind - 1]);
			break;
		case 'i':
			snprintf(&savePath[0], 128, "%s", argv[optind - 1]);
			break;
		case 'a':
			isAudio = atoi(argv[optind - 1]);
			break;
		default:
			help();
			exit(1);
		}
	}

	INFO("flv mux video chn[%d], audio: %s, save to: %s\r\n", chn_num, (isAudio == 1) ? "yes" : "no", savePath);

	/*init bistream*/
	int ret;
	uint8_t aac_ret;

	MPI_BCHN Bchn;
	VideoStreamData StreamData;
	MPI_VENC_TYPE_E type;
	bool is_first_frame = true;

	if (MPI_SYS_init() != MPI_SUCCESS) {
		ERR("Failed to MPI_SYS_init!");
		return -1;
	}

	ret = MPI_initBitStreamSystem(); /*only once, first clients,  destroy if no client*/
	if (ret != MPI_SUCCESS) {
		ERR("MPI_initBitStreamSystem_failed...\n");
		g_run_flag = 0;
		return -EIO;
	}

	ret = initStream(chn_num, &Bchn);
	if (ret != 0) {
		ERR("failed to init bitstream\n");
		return -EIO;
	}

	/*audio init*/
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	unsigned int rate = 8000;
	int dev_id = 0;
	snd_pcm_uframes_t frame = 1024;
	int channel = 1;
	int buf_len = 1024 * 2 * 1;
	char aac_buf[buf_len];
	char pcm_buf[buf_len];
	TyMediaAACHandle hdl;
	int frames = 1024;
	bool is_first_aac = true;
	uint32_t audio_ts = 0;
	snd_pcm_t *p_capture;

	if (isAudio) {
		ret = agtxPcmInit(&p_capture, g_device[dev_id], stream, format, frame, rate, channel);
		if (ret < 0) {
			ERR("failed to init pcm, %d\n", ret);
			isAudio = 0;
		} else {
			if (AAC_encoderInit(&hdl, channel, rate, rate)) {
				ERR("Failed to init AAC encoder.\n");
				isAudio = 0;
			} else {
				snd_pcm_nonblock(p_capture, 1);
			}
		}
	}

	g_run_flag = 1;

	MediaSrcInfo src_info;
	src_info.chn_num = chn_num;
	src_info.output_fd = 0;
	src_info.fCheckCodecChange = HTTP_checkCodecInvalid;
	src_info.fWriteFlv = writeFLVOutput;
	src_info.fFlvOpen = openFLVOutput;
	src_info.fFlvClose = closeFLVOutput;

	src_info.fFlvOpen(&savePath[0]);
	FLV_writeFLVHeader(&src_info, isAudio, true);

	unsigned int time_diff = 0;
	unsigned int start_time_jiff = 0;

	uint8_t vps_offset = 0;
	uint8_t sps_offset = 0;
	uint8_t pps_offset = 0;
	uint32_t size = 0;
	uint8_t *vps = NULL;
	uint8_t *sps = NULL;
	uint8_t *pps = NULL;
	uint8_t *idr = NULL;

	while (g_run_flag) {
		ret = readFrame(&Bchn, &StreamData);
		if (ret != 0) {
			g_run_flag = 0;
			break;
		}

		/*need to wait until first Idr*/
		if ((is_first_frame) && (StreamData.params.seg[0].type != MPI_FRAME_TYPE_SPS)) {
			LOG("Wait for first I frame\r\n");
			releaseFrame(&Bchn, &StreamData);
			continue;
		}

		if (false == HTTP_checkCodecInvalid(chn_num)) {
			/*change codec, getbistream error handling*/
			ERR("no data\r\n");
			break;
		}

		if (HTTP_checkCodecId(chn_num, &type) != 0) {
			ERR("failed to get codec\n");
		}
		if (src_info.video_codec != type && is_first_frame == false) {
			ERR("change codec, need err handling !\n");
		}
		src_info.video_codec = type;
		/*Mux video tag + send*/
		if (is_first_frame) {
			start_time_jiff = timespec_to_ms(&stream_data.params.timestamp);
			time_diff = 0;
			if (src_info.video_codec == MPI_VENC_TYPE_H264) {
                LOG("enter H264\n");
#if 1
				FLV_writeAVCSeqHeaderTag(&src_info, NULL, 0, StreamData.params.seg[0].uaddr + 4,
				                         StreamData.params.seg[0].size - 4,
				                         StreamData.params.seg[1].uaddr + 4,
				                         StreamData.params.seg[1].size - 4, time_diff);
#endif
			} else if (src_info.video_codec == MPI_VENC_TYPE_H265) {
				sps_offset = 32;
				if (StreamData.params.seg[0].size != 75) {
					sps_offset = StreamData.params.seg[0].size - (4 + 24 + 4 + 4 + 7);
				}
				pps_offset = 4 + 24 + 4 + sps_offset + 4;

				FLV_writeAVCSeqHeaderTag(&src_info, StreamData.params.seg[0].uaddr + 4, 24,
				                         StreamData.params.seg[0].uaddr + 4 + 24 + 4, sps_offset,
				                         StreamData.params.seg[0].uaddr + pps_offset, 7, time_diff);
			}

			is_first_frame = false;
		} else {
			time_diff = (timespec_to_ms(&stream_data.params.timestamp) - start_time_jiff);
		}

#if 1
		if (StreamData.params.seg[0].type == MPI_FRAME_TYPE_SPS) {
			if (src_info.video_codec == MPI_VENC_TYPE_H264) {
				LOG("MPI_VENC_TYPE_H264\n");
				sps_offset = StreamData.params.seg[0].size - 4;
				pps_offset = StreamData.params.seg[1].size - 4;
				size = StreamData.params.seg[2].size - 4;
				LOG("MPI_VENC_TYPE_H264, %u\n", size);
				if (StreamData.params.seg_cnt > 2) {
					for (int i = 3; i < StreamData.params.seg_cnt; i++) {
						size += StreamData.params.seg[i].size;
						LOG("MPI_VENC_TYPE_H264, %u\n", size);
					}
				}
				vps = NULL;
				sps = StreamData.params.seg[0].uaddr + 4;
				pps = StreamData.params.seg[1].uaddr + 4;
				idr = StreamData.params.seg[2].uaddr + 4;

			} else if (src_info.video_codec == MPI_VENC_TYPE_H265) {
				vps_offset = 24;
				sps_offset = StreamData.params.seg[0].size - (4 + 24 + 4 + 4 + 7);
				pps_offset = 7;
				size = StreamData.params.seg[1].size - 4;

				vps = StreamData.params.seg[0].uaddr + 4;
				sps = StreamData.params.seg[0].uaddr + 4 + 24 + 4;
				pps = StreamData.params.seg[0].uaddr + 4 + 24 + 4 + sps_offset + 4;
				idr = StreamData.params.seg[1].uaddr + 4;
				if (StreamData.params.seg_cnt > 1) {
					for (int i = 2; i < StreamData.params.seg_cnt; i++) {
						size += StreamData.params.seg[i].size;
					}
				}
			}

			LOG("I size %u, %d, %d\r\n", size, StreamData.params.seg[2].size,
			    StreamData.params.seg[1].size);
			ret = FLV_writeAVCDataTag(&src_info, vps, vps_offset, sps, sps_offset, pps, pps_offset, idr, size,
			                          time_diff, 1);
			if (ret == -EACCES) {
				releaseFrame(&Bchn, &StreamData);
				g_run_flag = 0;
				ERR("change codec\r\n");
				break;
			} else if (ret == -EIO) { /*change profile*/
				FLV_writeAVCSeqHeaderTag(&src_info, NULL, 0, StreamData.params.seg[0].uaddr + 4,
				                         StreamData.params.seg[0].size - 4,
				                         StreamData.params.seg[1].uaddr + 4,
				                         StreamData.params.seg[1].size - 4, time_diff);
				FLV_writeAVCDataTag(&src_info, NULL, 0, StreamData.params.seg[0].uaddr + 4, sps_offset,
				                    StreamData.params.seg[1].uaddr + 4, pps_offset,
				                    StreamData.params.seg[2].uaddr + 4, size, time_diff, 1);
				audio_ts = time_diff;
			}
		} else {
			uint32_t size = StreamData.params.seg[0].size - 4;
			if (StreamData.params.seg_cnt > 1) {
				for (int i = 1; i < StreamData.params.seg_cnt; i++) {
					size += StreamData.params.seg[i].size;
				}
			}

			FLV_writeAVCDataTag(&src_info,  NULL, 0, NULL, 0, NULL, 0, StreamData.params.seg[0].uaddr + 4, size,
			                    time_diff, 0);
		}
#endif
		releaseFrame(&Bchn, &StreamData);
#if 1
		if (isAudio) {
			while ((g_run_flag == 1) && (ret != -EAGAIN)) {
				ret = snd_pcm_readi(p_capture, pcm_buf, frames);
				if (ret == -EPIPE) {
					snd_pcm_prepare(p_capture);
					flv_server_log_err("-EPIPE");
					continue;
				} else if (ret == -EAGAIN) {
					/* means there is no data, break for loop */
					break;
				} else if (ret < 0) {
					flv_server_log_err("snd pcm unknown err: %d", ret);
					break;
				}
				int aac_recv = ret * 2;
				aac_ret = AAC_encoderGetData(&hdl, pcm_buf, ret * 2, ret, aac_buf, &aac_recv);
				if (aac_ret != AACENC_OK) {
					flv_server_log_err("aac_ret == %d", aac_ret);
					break;
				}

				audio_ts += 128;
				if (is_first_aac == true) {
					FLV_writeAACSeqHeaderTag(&src_info, rate, channel, 0);

					is_first_aac = false;
				}

				FLV_writeAACDataTag(&src_info, (uint8_t *)&aac_buf[7], aac_recv - 7, audio_ts);
			}
		}
#endif
	}

	if (isAudio) {
		ret = agtxPcmUninit(p_capture);
		if (ret < 0) {
			ERR("failed to uninit pcm, %d\n", ret);
			isAudio = 0;
		}
	}

	src_info.fFlvClose();
	uninitStream(&Bchn);

	return 0;
}