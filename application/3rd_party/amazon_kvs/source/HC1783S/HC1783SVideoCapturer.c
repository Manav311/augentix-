#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "mpi_sys.h"
#include "mpi_enc.h"
#include "mpi_errno.h"
#include "com/amazonaws/kinesis/video/capturer/VideoCapturer.h"

#include "HC1783SCommon.h"


typedef struct {
    VideoCapturerStatus status;
    VideoCapability capability;
    VideoFormat format;
    VideoResolution resolution;
    MPI_BCHN bitstream;
} HC1783SVideoCapturer;

#define REINTERPRET_SELF(x) HC1783SVideoCapturer *self = (HC1783SVideoCapturer *) (x)

static int setStatus(VideoCapturerHandle handle, const VideoCapturerStatus newStatus)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    if (newStatus != self->status) {
        self->status = newStatus;
        LOG("VideoCapturer new status[%d]", newStatus);
    }

    return 0;
}

VideoCapturerHandle videoCapturerCreate(void)
{
    int err = MPI_SYS_init();
    if (err != MPI_SUCCESS) {
        LOG("Initialize MPP system FAILED, err=%d", err);
        return NULL;
    }

    err = MPI_initBitStreamSystem();
    if (err != MPI_SUCCESS) {
        if (err == MPI_ERR_ENC_NOT_INIT) {
            LOG("Video encoder is NOT running?!");
        } else {
            LOG("Initialize bit-stream system FAILED, err=%d!", err);
        }
        MPI_SYS_exit();
        return NULL;
    }

    HC1783SVideoCapturer *self = NULL;
    if (!(self = malloc(sizeof(*self)))) {
        LOG("OOM");
        return NULL;
    }
    memset(self, 0, sizeof(*self));

    /* Note:
     * kvsWebRTCClientMaster.c is hardcoded setting format=H264, resolution=1080p,
     * no matter what capability is supported
     */
    self->capability.formats = (1 << (VID_FMT_H264 - 1));
    self->capability.resolutions = (1 << (VID_RES_1080P - 1));
    setStatus((VideoCapturerHandle) self, VID_CAP_STATUS_STREAM_OFF);

    self->bitstream = MPI_INVALID_ENC_BCHN;
    return (VideoCapturerHandle) self;
}

VideoCapturerStatus videoCapturerGetStatus(const VideoCapturerHandle handle)
{
    if (!handle) {
        return VID_CAP_STATUS_NOT_READY;
    }

    REINTERPRET_SELF(handle);
    return self->status;
}

int videoCapturerGetCapability(const VideoCapturerHandle handle, VideoCapability* pCapability)
{
    HANDLE_NULL_CHECK(handle);

    if (!pCapability) {
        return -EAGAIN;
    }

    REINTERPRET_SELF(handle);
    *pCapability = self->capability;

    return 0;
}

int videoCapturerSetFormat(VideoCapturerHandle handle, const VideoFormat format, const VideoResolution resolution)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    HANDLE_STATUS_CHECK(VID_CAP_STATUS_STREAM_OFF);

    switch (format) {
        case VID_FMT_H264:
            break;

        default:
            LOG("Unsupported format %d", format);
            return -EINVAL;
    }

    switch (resolution) {
        case VID_RES_1080P:
            break;

        default:
            LOG("Unsupported resolution %d", resolution);
            return -EINVAL;
    }

    self->format = format;
    self->resolution = resolution;

    return 0;
}

int videoCapturerGetFormat(const VideoCapturerHandle handle, VideoFormat* pFormat, VideoResolution* pResolution)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    *pFormat = self->format;
    *pResolution = self->resolution;

    return 0;
}

int videoCapturerAcquireStream(VideoCapturerHandle handle)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    const UINT8 encoderId = 0;  /* specify video source from which encoder */
    /* consume video bit-stream from this bit-stream channel */
    self->bitstream = MPI_createBitStreamChn(MPI_ENC_CHN(encoderId));
    if (!VALID_MPI_ENC_BCHN(self->bitstream)) {
        LOG("Invalid encoder id: %d!", encoderId);
        return -EINVAL;
    }

    return setStatus(handle, VID_CAP_STATUS_STREAM_ON);
}

int videoCapturerGetFrame(VideoCapturerHandle handle, void *pFrameDataBuffer, const size_t frameDataBufferSize,
                          uint64_t *pTimestamp, size_t *pFrameSize)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);
    HANDLE_STATUS_CHECK(VID_CAP_STATUS_STREAM_ON);

    if (!pFrameDataBuffer || !pTimestamp || !pFrameSize) {
        return -EINVAL;
    }

    MPI_STREAM_PARAMS_V2_S param;
    memset(&param, 0, sizeof(param));
    int err = MPI_getBitStreamV2(self->bitstream, &param, 1000);
    if (err != MPI_SUCCESS) {
        return err;
    }

    int ret = 0;
    size_t frameSize = 0;
    for (int i = 0; i < param.seg_cnt; ++i) {
        frameSize += param.seg[i].size;
    }

    if (frameSize > frameDataBufferSize) {
        *pFrameSize = 0;
        LOG("FrameDataBufferSize(%ld) < frameSize(%ld), frame dropped", frameDataBufferSize, frameSize);
        ret = -ENOMEM;
    } else {
        char *buf = pFrameDataBuffer;
        for (int i = 0; i < param.seg_cnt; ++i) {
            memcpy(buf, param.seg[i].uaddr, param.seg[i].size);
            buf += param.seg[i].size;
        }
        *pFrameSize = frameSize;
        struct timespec cur_time;
        clock_gettime(CLOCK_REALTIME, &cur_time);
        *pTimestamp = (uint64_t) cur_time.tv_sec * 1000 * 1000 + (uint64_t) cur_time.tv_nsec / 1000;
//        *pTimestamp = (uint64_t) param.timestamp.tv_sec * 1000 * 1000 + (uint64_t) param.timestamp.tv_nsec / 1000;
        LOG("[V:%llu] frame size: %lu", *pTimestamp, *pFrameSize);
    }

    err = MPI_releaseBitStreamV2(self->bitstream, &param);
    if (ret) {
        return ret;
    }

    return err;
}

int videoCapturerReleaseStream(VideoCapturerHandle handle)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    HANDLE_STATUS_CHECK(VID_CAP_STATUS_STREAM_ON);

    if (VALID_MPI_ENC_BCHN(self->bitstream)) {
        MPI_destroyBitStreamChn(self->bitstream);
    }

    return setStatus(handle, VID_CAP_STATUS_STREAM_OFF);
}

void videoCapturerDestroy(VideoCapturerHandle handle)
{
    if (!handle) {
        return;
    }

    REINTERPRET_SELF(handle);
    if (self->status == VID_CAP_STATUS_STREAM_ON) {
        videoCapturerReleaseStream(handle);
    }

    setStatus(handle, VID_CAP_STATUS_NOT_READY);

    free(handle);
}
