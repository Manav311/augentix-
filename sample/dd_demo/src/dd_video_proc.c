/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <stdbool.h>
#include <pthread.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include "dd_video_proc.h"
#include "dd_disp_utils.h"

static AVFormatContext *format_ctx = NULL;
static AVCodecContext *codec_ctx = NULL;
static AVCodec *codec = NULL;
static AVFrame *frame = NULL, *frame_rgba = NULL;
static AVPacket packet;
static struct SwsContext *sws_ctx = NULL;
static int video_stream_index = -1;
static uint8_t *buffer = NULL;
static int num_bytes = 0;
static bool g_runVideo = false;

int video_mem_release(void);

int video_process_initial(void *arg)
{
	const char *input_url = (const char *)arg;

	if (display_process_initial()) {
		fprintf(stderr, "disp_initial fail.\n");
		return -1;
	}

	// Do global initialization of network libraries.
	avformat_network_init();

	// open input stream
	if (avformat_open_input(&format_ctx, input_url, NULL, NULL) != 0) {
		fprintf(stderr, "Could not open input stream.\n");
		return -1;
	}

	// Find flow information
	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information.\n");
		return -1;
	}

	// Find video stream
	for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
		if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}
	if (video_stream_index == -1) {
		fprintf(stderr, "Could not find video stream.\n");
		return -1;
	}

	// Get decoder
	codec = avcodec_find_decoder(format_ctx->streams[video_stream_index]->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Unsupported codec.\n");
		return -1;
	}

	return 0;
}

int video_process_deinitial(void)
{
	display_process_deinitial();
	return video_mem_release();
}

int startVideoRun(void)
{
	// ffmpeg
	// Initialize decoder context
	// Allocate an AVCodecContext and set its fields to default values.
	// The resulting struct should be freed with avcodec_free_context().
	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		fprintf(stderr, "Could not allocate codec context.\n");
		return -1;
	}
	// Fill the codec context based on the values from the supplied codec parameters.
	avcodec_parameters_to_context(codec_ctx, format_ctx->streams[video_stream_index]->codecpar);

	// open decoder
	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec.\n");
		return -1;
	}

	// Initialize conversion context
	sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width,
	                         codec_ctx->height, AV_PIX_FMT_BGRA, SWS_BILINEAR, NULL, NULL, NULL);

	// Allocate frame memory
	frame = av_frame_alloc();
	frame_rgba = av_frame_alloc();
	num_bytes = av_image_get_buffer_size(AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);
	buffer = (uint8_t *)av_malloc(num_bytes * sizeof(uint8_t));
	av_image_fill_arrays(frame_rgba->data, frame_rgba->linesize, buffer, AV_PIX_FMT_BGRA, codec_ctx->width,
	                     codec_ctx->height, 32);

	g_runVideo = true;
	return 0;
}

int stopVideoRun(void)
{
	g_runVideo = false;
	return 0;
}

void *getVideoContext(void)
{
	return (void *)format_ctx;
}

void *thread_VideoFrameData(void *arg)
{
	(void)(arg);

	// Read frames and convert
	while (g_runVideo && ((av_read_frame(format_ctx, &packet)) >= 0)) {
		if (packet.stream_index == video_stream_index) {
			if (avcodec_send_packet(codec_ctx, &packet) == 0) {
				while (avcodec_receive_frame(codec_ctx, frame) == 0) {
					sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
					          codec_ctx->height, frame_rgba->data, frame_rgba->linesize);

					av_image_fill_arrays(frame_rgba->data, frame_rgba->linesize, buffer,
					                     AV_PIX_FMT_BGRA, codec_ctx->width, codec_ctx->height, 32);

					display_image_update(buffer, num_bytes);
				}
			}
		}
		av_packet_unref(&packet);
	}
	pthread_exit(0);
}

int video_mem_release(void)
{
	// ffmpeg
	// 釋放資源
	av_free(buffer);
	av_frame_free(&frame);
	av_frame_free(&frame_rgba);
	sws_freeContext(sws_ctx);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&format_ctx);
	return 0;
}
