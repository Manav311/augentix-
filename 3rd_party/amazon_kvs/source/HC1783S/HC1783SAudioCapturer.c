#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "com/amazonaws/kinesis/video/capturer/AudioCapturer.h"

#include "HC1783SCommon.h"
#include "aac.h"
#include "audio_utils.h"


typedef struct {
    AudioCapturerStatus status;
    AudioCapability capability;
    AudioFormat format;
    AudioChannel channel;
    AudioBitDepth bitDepth;
    AudioSampleRate sampleRate;
    TyMediaAACHandle aacEncoder;
    PcmSound *pSound;
} HC1783SAudioCapturer;

#define REINTERPRET_SELF(x) HC1783SAudioCapturer *self = (HC1783SAudioCapturer *) (x)

static int setStatus(AudioCapturerHandle handle, const AudioCapturerStatus newStatus)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    if (newStatus != self->status) {
        self->status = newStatus;
        LOG("AudioCapturer new status[%d]", newStatus);
    }

    return 0;
}

AudioCapturerHandle audioCapturerCreate(void)
{
    HC1783SAudioCapturer *self = NULL;
    if (!(self = malloc(sizeof(*self)))) {
        LOG("OOM");
        return NULL;
    }

    memset(self, 0, sizeof(*self));
#ifdef KVS_PRODUCER
        self->capability.formats = (1 << (AUD_FMT_AAC - 1));
        self->capability.channels = (1 << (AUD_CHN_MONO - 1));
        self->capability.sampleRates = (1 << (AUD_SAM_8K - 1));
        self->capability.bitDepths = (1 << (AUD_BIT_16 - 1));
#else
        self->capability.formats = (1 << (AUD_FMT_G711A - 1));
        self->capability.channels = (1 << (AUD_CHN_MONO - 1));
        self->capability.sampleRates = (1 << (AUD_SAM_8K - 1));
        self->capability.bitDepths = (1 << (AUD_BIT_8 - 1));
#endif

    setStatus((AudioCapturerHandle) self, AUD_CAP_STATUS_STREAM_OFF);

    return (AudioCapturerHandle) self;
}

AudioCapturerStatus audioCapturerGetStatus(const AudioCapturerHandle handle)
{
    if (!handle) {
        return AUD_CAP_STATUS_NOT_READY;
    }

    REINTERPRET_SELF(handle);
    return self->status;
}

int audioCapturerGetCapability(const AudioCapturerHandle handle, AudioCapability* pCapability)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    if (!pCapability) {
        return -EINVAL;
    }

    *pCapability = self->capability;

    return 0;
}

int audioCapturerSetFormat(AudioCapturerHandle handle, const AudioFormat format, const AudioChannel channel, const AudioSampleRate sampleRate,
                           const AudioBitDepth bitDepth)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    HANDLE_STATUS_CHECK(AUD_CAP_STATUS_STREAM_OFF);

    switch (format) {
#ifdef KVS_PRODUCER
        case AUD_FMT_AAC:
#else
        case AUD_FMT_G711A:
#endif
            break;

        default:
            LOG("Unsupported format %d", format);
            return -EINVAL;
    }

    switch (channel) {
        case AUD_CHN_MONO:
            break;

        default:
            LOG("Unsupported channel num %d", channel);
            return -EINVAL;
    }

    switch (sampleRate) {
        case AUD_SAM_8K:
            break;

        default:
            LOG("Unsupported sample rate %d", sampleRate);
            return -EINVAL;
    }

    switch (bitDepth) {
#ifdef KVS_PRODUCER
        case AUD_BIT_16:
#else
        case AUD_BIT_8:
#endif
            break;

        default:
            LOG("Unsupported bit depth %d", bitDepth);
            return -EINVAL;
    }

    self->format = format;
    self->channel = channel;
    self->sampleRate = sampleRate;
    self->bitDepth = bitDepth;

    return 0;
}

int audioCapturerGetFormat(const AudioCapturerHandle handle, AudioFormat* pFormat, AudioChannel* pChannel, AudioSampleRate* pSampleRate,
                           AudioBitDepth* pBitDepth)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    *pFormat = self->format;
    *pChannel = self->channel;
    *pSampleRate = self->sampleRate;
    *pBitDepth = self->bitDepth;

    return 0;
}

int audioCapturerAcquireStream(AudioCapturerHandle handle)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    int gain = 25;
    char *env_gain = getenv("SOUND_GAIN");
    if (env_gain) {
        gain = atoi(env_gain);
    }

    LOG("audio capture gain=%d", gain);
#ifdef KVS_PRODUCER
    self->pSound = PcmSound_new(SND_PCM_FORMAT_S16_LE, 2, 8000, gain, true);
#else
    self->pSound = PcmSound_new(SND_PCM_FORMAT_A_LAW, 1, 8000, gain, true);
#endif
    if (self->pSound == NULL) {
        LOG("FAIL to initialize ALSA PCM sound!");
        return errno;
    }

#ifdef KVS_PRODUCER
    int err = AAC_encoderInit(&self->aacEncoder, 1, 8000, 8000);
    if (err) {
        LOG("FAIL to prepare ACC encoder, err=%d", err);
        return err;
    }
#endif

    return setStatus(handle, AUD_CAP_STATUS_STREAM_ON);
}

int audioCapturerGetFrame(AudioCapturerHandle handle, void* pFrameDataBuffer, const size_t frameDataBufferSize, uint64_t* pTimestamp,
                          size_t* pFrameSize)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);
    HANDLE_STATUS_CHECK(AUD_CAP_STATUS_STREAM_ON);

    if (!pFrameDataBuffer || !pTimestamp || !pFrameSize) {
        return -EINVAL;
    }

    unsigned int pcmSize = 0;
    char *pcmData = PcmSound_takeSample(self->pSound, &pcmSize);
    if (pcmData == NULL) {
        return -EAGAIN;
    }

    struct timespec cur_time;
    clock_gettime(CLOCK_REALTIME, &cur_time);
    snd_htimestamp_t timestamp = {0};
    snd_htimestamp_t trigger = {0};
    snd_htimestamp_t audio = {0};
    PcmSound_reportTimestamp(self->pSound, &timestamp, &trigger, &audio, NULL, NULL);
    *pTimestamp = (uint64_t) cur_time.tv_sec * 1000 * 1000 + (uint64_t) cur_time.tv_nsec / 1000;
    LOG("[A:%llu] pcm size: %u", *pTimestamp, pcmSize);


#ifdef KVS_PRODUCER
    *pFrameSize = frameDataBufferSize;
    int err = AAC_encoderGetData(&self->aacEncoder, pcmData, pcmSize, 0,
                                 pFrameDataBuffer, pFrameSize);
    if (err != AACENC_OK) {
        LOG("AAC encoder error, err=%d", err);
        *pFrameSize = 0;
        return -EAGAIN;
    }
    LOG("[A:%llu] pcm size: %u, aac size: %lu", *pTimestamp, pcmSize, *pFrameSize);
#else
    *pFrameSize = pcmSize;
    memcpy(pFrameDataBuffer, pcmData, pcmSize);
    LOG("[A:%llu] pcm size: %u", *pTimestamp, pcmSize);
#endif

    LOG("timestamp, trigger, audio: %ld.%09lu, %ld.%09lu, %ld.%09lu",
        timestamp.tv_sec, timestamp.tv_nsec,
        trigger.tv_sec, trigger.tv_nsec,
        audio.tv_sec, audio.tv_nsec);

    return 0;
}

int audioCapturerReleaseStream(AudioCapturerHandle handle)
{
    HANDLE_NULL_CHECK(handle);
    REINTERPRET_SELF(handle);

    HANDLE_STATUS_CHECK(AUD_CAP_STATUS_STREAM_ON);

    return setStatus(handle, AUD_CAP_STATUS_STREAM_OFF);
}

void audioCapturerDestroy(AudioCapturerHandle handle)
{
    if (!handle) {
        return;
    }

    REINTERPRET_SELF(handle);

    if (self->status == AUD_CAP_STATUS_STREAM_ON) {
        audioCapturerReleaseStream(handle);
    }

#ifdef KVS_PRODUCER
    AAC_encoderUninit(&self->aacEncoder);
#endif
    PcmSound_dispose(self->pSound);

    setStatus(handle, AUD_CAP_STATUS_NOT_READY);

    free(handle);
}
