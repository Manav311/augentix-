#include <string.h>
#include <assert.h>

#include <linux/videodev2.h>

#include "webcam.h"
#include "mpi_sys.h"
#include "mpi_enc.h"
#include "uvcd_common.h"
#include "uvcd_stream.h"

#ifdef CONFIG_WEBCAM_CAP_FIXED
#include "agtx_webcam_cap.h"
#else
extern volatile int streaming;
extern AgtxWebcamCap g_webcam_cap;
#endif

static MPI_BCHN g_bchn;

int initBitstreamSystem()
{
	int ret = MPI_FAILURE;
	MPI_ECHN chn = MPI_ENC_CHN(0);

	trace("%s=>\n", __func__);

	ret = MPI_initBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		error("Bitstream system initialisation failed\n");
		assert(ret == MPI_SUCCESS);
		return ret;
	}

	g_bchn = MPI_createBitStreamChn(chn);
	if (!VALID_MPI_ENC_BCHN(g_bchn)) {
		error("Failed to create bitstream channel 0.\n");
		destroyBitstreamSystem();
		return MPI_FAILURE;
	}

	trace("%s=<\n", __func__);
	return MPI_SUCCESS;
}

void destroyBitstreamSystem()
{
	int ret = MPI_FAILURE;
	trace("%s=>\n", __func__);

	ret = MPI_destroyBitStreamChn(g_bchn);
	if (ret != MPI_SUCCESS) {
		error("Failed to exit bitstream system.\n");
		assert(ret == MPI_SUCCESS);
	}

	g_bchn = MPI_INVALID_ENC_BCHN;

	trace("%s=<\n", __func__);
}

void fillVideoBuffer(uvc_device *dev, struct v4l2_buffer *buf)
{
	int i;
	MPI_STREAM_PARAMS_S params;
	MPI_BUF_SEG_S *seg = NULL;
	UINT32 offset = 0;
	struct agtx_webcam_cap_format *formats = g_webcam_cap.formats;
	trace("%s=>\n", __func__);

fetch_next:
	if (MPI_getBitStream(g_bchn, &params, -1) != MPI_SUCCESS) {
		error("Get video data failed\n");
		buf->bytesused = 0;
		return;
	}
	switch (formats[dev->bFormatIndex - 1].codec) {
	case AGTX_VENC_TYPE_MJPEG:
		if (params.seg != NULL) {
			for (i = 0; (unsigned)i < params.seg_cnt; ++i) {
				seg = &params.seg[i];
				memcpy((dev->buf[buf->index] + offset), seg->uaddr, seg->size);
				offset = offset + seg->size;
			}
			buf->bytesused = offset;
		}
		break;
	case AGTX_VENC_TYPE_H264:
		if (params.seg != NULL) {
			for (i = 0; (unsigned)i < params.seg_cnt; ++i) {
				seg = &params.seg[i];
				memcpy((dev->buf[buf->index] + offset), seg->uaddr, seg->size);
				offset += seg->size;
			}
			if (params.seg[0].type == MPI_FRAME_TYPE_PPS || params.seg[0].type == MPI_FRAME_TYPE_SPS) {
				MPI_releaseBitStream(g_bchn, &params);
				goto fetch_next;
			}
			buf->bytesused = offset;
		}
		break;
	case AGTX_VENC_TYPE_H265:
	case AGTX_VENC_TYPE_NONE:
	default:
		error("Unknown codec type\n");
		break;
	}
	MPI_releaseBitStream(g_bchn, &params);
	trace("%s=<\n", __func__);
}
