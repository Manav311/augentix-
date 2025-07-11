#include "udp_cast.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "udp_socket.h"

#define UDP_STREAM_NAME "udp_stream"
#define UDP_XMT_LEN 8192
#define DESCRIPTOR_MAX 64

typedef struct udp_live_stream {
	BitStreamSubscriber base_part;
	char descriptor[DESCRIPTOR_MAX];
	SinkUdp *sink;
} UdpLiveStream;

static bool UDPLIVESTREAM_deliveryWillStart(void *context)
{
	UdpLiveStream *stream = context;
	if (stream->sink == NULL) {
		return true;
	}

	if (stream->sink->ops->open(stream->sink->info, 0) < 0) {
		printf("%s: CANNOT open UDP socket!\n", stream->descriptor);
		releaseUdpSink(stream->sink);
		stream->sink = NULL;
		return true;
	}

	return false;
}

static bool UDPLIVESTREAM_receiveFrame(void *context, const MPI_STREAM_PARAMS_V2_S *frame)
{
	UdpLiveStream *stream = context;
	SinkUdp *sink = stream->sink;
	if (sink == NULL) {
		return true;
	}

	for (UINT32 i = 0; i < frame->seg_cnt; ++i) {
		uint8_t *anchor = frame->seg[i].uaddr;
		for (ssize_t chunk_size = frame->seg[i].size; chunk_size > 0; chunk_size -= UDP_XMT_LEN) {
			if (chunk_size <= UDP_XMT_LEN) {
				if (sink->ops->write(sink->info, anchor, chunk_size) < 0) {
					printf("%s: stream write error(%d-%d).\n",
					       stream->descriptor, frame->frame_id, i);
				}
			} else {
				if (sink->ops->write(sink->info, anchor, UDP_XMT_LEN) < 0) {
					printf("%s: stream write error(%d-%d).\n",
					       stream->descriptor, frame->frame_id, i);
				}
				anchor += UDP_XMT_LEN;
			}
		}
	}
	return false;
}

static void UDPLIVESTREAM_deliveryDidEnd(void *context)
{
	UdpLiveStream *stream = context;
	if (stream->sink != NULL) {
		releaseUdpSink(stream->sink);
		stream->sink = NULL;
	}
}

BitStreamSubscriber *newUdpLiveStream(const char *dest_ip, int dest_port)
{
	UdpLiveStream *stream = calloc(1, sizeof(*stream));

	snprintf(stream->descriptor, sizeof(stream->descriptor),
	         "UdpLiveStream[%s:%d]", dest_ip, dest_port);
	stream->sink = createUdpSink(UDP_STREAM_NAME, dest_ip, dest_port);
	if (stream->sink == NULL) {
		printf("FAILED to create UDP sink to %s:%d.\n", dest_ip, dest_port);
	}
	BitStreamSubscriber *base = &stream->base_part;
	base->context = stream;
	base->deliveryWillStart = UDPLIVESTREAM_deliveryWillStart;
	base->receiveFrame = UDPLIVESTREAM_receiveFrame;
	base->deliveryDidEnd = UDPLIVESTREAM_deliveryDidEnd;
	return base;
}