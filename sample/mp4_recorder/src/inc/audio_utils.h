#ifndef MP4_RECORDER_AUDIO_UTILS_H_
#define MP4_RECORDER_AUDIO_UTILS_H_

#include "alsa/asoundlib.h"
#include "alsa/error.h"
#include "pcm_interfaces.h"

class PcmSound {
    public:
	PcmSound(unsigned int sample_rate, int gain, bool enable_timestamp = false);
	~PcmSound();

	int setGain(int gain);

	long takeSamples(void *buffer, unsigned int frames);
	bool reportTimestamp(snd_htimestamp_t &timestamp, snd_htimestamp_t &trigger_timestamp,
	                     snd_htimestamp_t &audio_timestamp, snd_pcm_uframes_t &available, snd_pcm_sframes_t &delay);

	bool isReady() const
	{
		return _ready;
	}

	bool supportTimestamp() const
	{
		return _timestamp_enabled;
	}

	unsigned int getSampleRate() const
	{
		return _sample_rate;
	}

	unsigned int convertToFrames(unsigned int sample_byte_size) const
	{
		return sample_byte_size / _bytes_per_frame / _channels;
	}

	unsigned int bytesInPeriod(float seconds) const
	{
		return static_cast<unsigned int>(seconds * _sample_rate) * _channels * _bytes_per_frame;
	}

	float bytesToPeriod(unsigned int bytes) const
	{
		return static_cast<float>(bytes) / _sample_rate / _channels / _bytes_per_frame;
	}

	unsigned int getChunkSize(unsigned int frames) const
	{
		return frames * _bytes_per_frame * _channels;
	}

	static constexpr unsigned int getBytesPerFrame()
	{
		return _bytes_per_frame;
	}

	static constexpr unsigned int getChannels()
	{
		return _channels;
	}

    private:
	snd_pcm_t *_handle;
	const char *_device = "default";
	snd_pcm_hw_params_t *_params;
	unsigned int _sample_rate;
	snd_pcm_format_t _format = SND_PCM_FORMAT_S16_LE;
	static constexpr snd_pcm_uframes_t _frames = 1024;
	static constexpr unsigned int _channels = 1;
	static constexpr unsigned int _bytes_per_frame = 2; /* this SHOULD match with _format */
	static constexpr unsigned int _sample_size = _frames * _bytes_per_frame * _channels;

	bool _ready;
	bool _timestamp_enabled;
};

#endif //MP4_RECORDER_AUDIO_UTILS_H_
