#include "sample_publisher.h"

#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "frame_dumper.h"
#include "mp4_dumper.h"
#include "udp_cast.h"

static pthread_t g_bsb_tid[MPI_MAX_ENC_CHN_NUM] = { 0 };
static char g_bsb_run[MPI_MAX_ENC_CHN_NUM] = { 0 };

typedef struct publisher_info {
	MPI_ECHN encoder_channel;
	MPI_BCHN stream_channel;
	BitStreamSubscriber *subscriber;
} PublisherInfo;

static BitStreamSubscriber *newNullSubscriber(void);
static BitStreamSubscriber *buildTeeSubscriber(BitStreamSubscriber *subscriber1,
                                               BitStreamSubscriber *subscriber2);

/**
 * @brief Thread routine to get bit stream data to publish/delivery.
 * @details The function will create channel to get bit stream at first, then
 * call MPIs to get the parameters of bit stream. With these parameter, we can
 * read bit stream data at the address of stream buffer and deliver to subscriber.
 * After that, the bit stream buffer could be released.
 */
static void *SAMPLE_runStreamPublisher(void *p)
{
	PublisherInfo *info = p;
	MPI_BCHN stream_channel = info->stream_channel;

	uint32_t frame_count = 0;
	uint32_t total_bytes = 0;
	bool has_seen_first_frame = false;
	MPI_STREAM_PARAMS_V2_S params;
	memset(&params, 0, sizeof(params));
	BitStreamSubscriber *subscriber = info->subscriber;
	if (subscriber->deliveryWillStart(subscriber->context)) {
		printf("Stream subscriber is NOT ready! Abort stream delivery.\n");
		goto exit_bit_stream_channel;
	}

	time_t last_time_mark = time(NULL);
	while (g_bsb_run[stream_channel.chn]) {
		int ret = MPI_getBitStreamV2(stream_channel, &params, 1200);
		time_t now = time(NULL);
		if (ret == MPI_SUCCESS) {
			if (!has_seen_first_frame && stream_channel.chn == 0) {
				has_seen_first_frame = true;
				// Info message for QA to test fast boot and get first frame.
				fprintf(stderr, "[Test] [bchn.chn %d] Get first frame success !!!\n",
				        stream_channel.chn);
			}
			for (UINT32 i = 0; i < params.seg_cnt; ++i) {
				total_bytes += params.seg[i].size;
			}
			if (params.seg->type >= MPI_FRAME_TYPE_I) {
				++frame_count;
				if (now != last_time_mark) {
					printf("stream %d, fps %u, bps %u Kbps\n", stream_channel.chn, frame_count,
					       (total_bytes + 64) >> 7);
					last_time_mark = now;
					frame_count = 0;
					total_bytes = 0;
				}
			}
			if (subscriber->receiveFrame(subscriber->context, &params)) {
				g_bsb_run[stream_channel.chn] = 0;
			}
			MPI_releaseBitStreamV2(stream_channel, &params);
		} else if (ret == -ETIMEDOUT || ret == -EAGAIN) {
			/* Request frame again if timeout occurred */
			continue;
		} else if (ret == -ENODATA) {
			/**
			 * Wait encoder run again.
			 * In some scenarios, you might temporary stop encoder to modify some
			 * static configurations (Ex. OSD, codec type), process must not exit
			 * in these cases.
			 */
			frame_count = 0;
			total_bytes = 0;
			continue;
		} else {
			printf("FAILED to get parameters of stream %d.\n", stream_channel.chn);
			g_bsb_run[stream_channel.chn] = 0;
		}
	}
	subscriber->deliveryDidEnd(subscriber->context);

exit_bit_stream_channel:
	MPI_destroyBitStreamChn(stream_channel);
	free(info->subscriber);
	free(info);
	return NULL;
}

/**
 * @brief Start a thread to publish bit stream data to subscriber.
 * @param[in] encoder_channel  Target encoder index
 * @param[in] conf pointer to streaming configuration
 * @return The execution result.
 * @retval 0         success
 * @retval others    unexpected failure
 * @see SAMPLE_shutdownStreamPublisher()
 */
int SAMPLE_startStreamPublisher(MPI_ECHN encoder_channel, const CONF_BITSTREAM_PARAM_S *conf,
                                INT32 reservation_level, INT32 recycle_level)
{
	PublisherInfo *info = malloc(sizeof(*info));
	info->subscriber = NULL;
	info->encoder_channel = encoder_channel;
	int ret = 0;

	if (conf->record.enable) {
		if (strstr(conf->record.fname, ".mp4") != NULL) {
#ifdef MP4_ENABLE
			MPI_ENC_CHN_ATTR_S enc_attr;
			MPI_VENC_ATTR_S venc_attr;
			MPI_ENC_getChnAttr(encoder_channel, &enc_attr);
			MPI_ENC_getVencAttr(encoder_channel, &venc_attr);

			if (venc_attr.type != MPI_VENC_TYPE_H264) {
				printf("MP4 dumper only support h264 format, venc type: %d\n", venc_attr.type);
				ret = -EINVAL;
				goto exit;
			}

			info->subscriber = buildTeeSubscriber(
			        info->subscriber,
			        newMp4Dumper(encoder_channel, conf->record.fname, abs(conf->record.frame_num),
			                     conf->record.frame_num < 0, reservation_level, recycle_level,
			                     conf->record.max_dumped_files, enc_attr.res.width, enc_attr.res.height,
			                     venc_attr.h264.rc.frm_rate_o));
#endif
		} else {
			info->subscriber = buildTeeSubscriber(
			        info->subscriber,
			        newFrameDumper(encoder_channel, conf->record.fname, abs(conf->record.frame_num),
			                       conf->record.frame_num < 0, reservation_level, recycle_level,
			                       conf->record.max_dumped_files));
		}
	}
	if (conf->stream.enable) {
		info->subscriber = buildTeeSubscriber(
		        newUdpLiveStream(conf->stream.client_ip, conf->stream.client_port), info->subscriber);
	}

	/* Create BCHN outside the thread to avoid race conditions */
	info->stream_channel = MPI_createBitStreamChn(info->encoder_channel);
	if (!VALID_MPI_ENC_BCHN(info->stream_channel)) {
		printf("FAILED to create bit-stream channel.\n");
		ret = -EINVAL;
		goto exit;
	}

	/* We want publisher always doing its job, so attach a NullSubscriber (it never unsubscribe). */
	info->subscriber = buildTeeSubscriber(info->subscriber, newNullSubscriber());
	g_bsb_run[encoder_channel.chn] = 1;
	ret = pthread_create(&g_bsb_tid[encoder_channel.chn], NULL, SAMPLE_runStreamPublisher, info);
	if (ret) {
		printf("FAILED to create thread to get stream data of channel %d! err: %d\n", encoder_channel.chn, ret);
		g_bsb_run[encoder_channel.chn] = 0;
		goto exit_bit_stream_channel;
	}

	return 0;

exit_bit_stream_channel:
	MPI_destroyBitStreamChn(info->stream_channel);
	free(info->subscriber);
exit:
	free(info);
	return ret;
}

/**
 * @brief Stop publisher thread(thus stop consuming bit stream data).
 * @see SAMPLE_startStreamPublisher()
 */
void SAMPLE_shutdownStreamPublisher(MPI_ECHN idx)
{
	g_bsb_run[MPI_GET_ENC_CHN(idx)] = 0;
	pthread_join(g_bsb_tid[MPI_GET_ENC_CHN(idx)], NULL);
}

bool SAMPLE_hasAnyPublisherThreadActive(void)
{
	for (int i = 0; i < MPI_MAX_ENC_CHN_NUM; ++i) {
		if (g_bsb_run[i]) {
			return true;
		}
	}
	return false;
}

void SAMPLE_signalAllStreamThreadToShutdown(void)
{
	for (int i = 0; i < MPI_MAX_ENC_CHN_NUM; ++i) {
		g_bsb_run[i] = 0;
	}
}

static bool NULLSUBSCRIBER_deliveryWillStart(__attribute__((unused)) void *context)
{
	return false;
}

static bool NULLSUBSCRIBER_receiveFrame(__attribute__((unused)) void *context,
                                        __attribute__((unused)) const MPI_STREAM_PARAMS_V2_S *frame)
{
	return false;
}

static void NULLSUBSCRIBER_deliveryDidEnd(__attribute__((unused)) void *context)
{
}

static BitStreamSubscriber *newNullSubscriber(void)
{
	BitStreamSubscriber *subscriber = malloc(sizeof(*subscriber));
	subscriber->context = NULL;
	subscriber->deliveryWillStart = NULLSUBSCRIBER_deliveryWillStart;
	subscriber->receiveFrame = NULLSUBSCRIBER_receiveFrame;
	subscriber->deliveryDidEnd = NULLSUBSCRIBER_deliveryDidEnd;
	return subscriber;
}

typedef struct subscriber_tee {
	BitStreamSubscriber base_part;
	BitStreamSubscriber *left_branch;
	BitStreamSubscriber *right_branch;
} SubscriberTee;

static bool SUBSCRIBERTEE_deliveryWillStart(void *context)
{
	SubscriberTee *T = context;
	bool left_result = T->left_branch->deliveryWillStart(T->left_branch->context);
	bool right_result = T->right_branch->deliveryWillStart(T->right_branch->context);
	if (left_result) {
		free(T->left_branch);
		T->left_branch = NULL;
	}
	if (right_result) {
		free(T->right_branch);
		T->right_branch = NULL;
	}
	return left_result && right_result;
}

static bool SUBSCRIBERTEE_receiveFrame(void *context, const MPI_STREAM_PARAMS_V2_S *frame)
{
	SubscriberTee *T = context;
	BitStreamSubscriber *left_branch = T->left_branch;
	bool left_result = true;
	if (left_branch != NULL) {
		left_result = left_branch->receiveFrame(left_branch->context, frame);
		if (left_result) {
			T->left_branch = NULL;
			left_branch->deliveryDidEnd(left_branch->context);
			free(left_branch);
		}
	}

	BitStreamSubscriber *right_branch = T->right_branch;
	bool right_result = true;
	if (right_branch != NULL) {
		right_result = right_branch->receiveFrame(right_branch->context, frame);
		if (right_result) {
			T->right_branch = NULL;
			right_branch->deliveryDidEnd(right_branch->context);
			free(right_branch);
		}
	}
	return left_result && right_result;
}

static void SUBSCRIBERTEE_deliveryDidEnd(void *context)
{
	SubscriberTee *T = context;
	if (T->left_branch != NULL) {
		T->left_branch->deliveryDidEnd(T->left_branch->context);
		free(T->left_branch);
		T->left_branch = NULL;
	}
	if (T->right_branch != NULL) {
		T->right_branch->deliveryDidEnd(T->right_branch->context);
		free(T->right_branch);
		T->right_branch = NULL;
	}
}

static BitStreamSubscriber *buildTeeSubscriber(BitStreamSubscriber *subscriber1,
                                               BitStreamSubscriber *subscriber2)
{
	if (subscriber1 == NULL) {
		return subscriber2;
	} else if (subscriber2 == NULL) {
		return subscriber1;
	} else {
		SubscriberTee *tee = malloc(sizeof(*tee));
		tee->left_branch = subscriber1;
		tee->right_branch = subscriber2;
		BitStreamSubscriber *base = &tee->base_part;
		base->context = tee;
		base->deliveryWillStart = SUBSCRIBERTEE_deliveryWillStart;
		base->receiveFrame = SUBSCRIBERTEE_receiveFrame;
		base->deliveryDidEnd = SUBSCRIBERTEE_deliveryDidEnd;

		return base;
	}
}
