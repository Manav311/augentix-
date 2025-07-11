#include <errno.h>

#include "com/amazonaws/kinesis/video/player/AudioPlayer.h"

#include "HC1783SCommon.h"


AudioPlayerHandle audioPlayerCreate(void)
{
    LOG("HC1783S AudioPlayer is not implemented yet");

    return NULL;
}

AudioPlayerStatus audioPlayerGetStatus(const AudioPlayerHandle handle)
{
    return AUD_PLY_STATUS_NOT_READY;
}

int audioPlayerGetCapability(const AudioPlayerHandle handle, AudioCapability* pCapability)
{
    return -EAGAIN;
}

int audioPlayerSetFormat(AudioPlayerHandle handle, const AudioFormat format, const AudioChannel channel, const AudioSampleRate sampleRate,
                         const AudioBitDepth bitDepth)
{
    return -EAGAIN;
}

int audioPlayerGetFormat(const AudioPlayerHandle handle, AudioFormat* pFormat, AudioChannel* pChannel, AudioSampleRate* pSampleRate,
                         AudioBitDepth* pBitDepth)
{
    return -EAGAIN;
}

int audioPlayerAcquireStream(AudioPlayerHandle handle)
{
    return -EAGAIN;
}

int audioPlayerWriteFrame(AudioPlayerHandle handle, void* pData, const size_t size)
{
    return -EAGAIN;
}

int audioPlayerReleaseStream(AudioPlayerHandle handle)
{
    return -EAGAIN;
}

void audioPlayerDestroy(AudioPlayerHandle handle)
{
}