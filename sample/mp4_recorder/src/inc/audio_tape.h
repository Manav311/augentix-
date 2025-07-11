#ifndef AUDIOTAPE_H_
#define AUDIOTAPE_H_

#include <list>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <functional>

#include "audio_utils.h"
#include "av_lib.h"

struct AudioSample {
	AudioSample(const void *sample_data, size_t sample_size, float timestamp_in_seconds)
	        : data(nullptr)
	        , data_size(sample_size)
	        , timestamp(timestamp_in_seconds)
	{
		data = new char[sample_size];
		memcpy(data, sample_data, sample_size);
	}
	~AudioSample()
	{
		delete[] data;
	}

	char *data;
	size_t data_size;
	float timestamp;
	std::atomic_bool pined{ false };

	static AudioSample *muteAt(float when, size_t sample_size)
	{
		return new AudioSample(when, sample_size);
	}

    private:
	AudioSample(float timestamp_in_seconds, size_t sample_size)
	        : data(nullptr)
	        , data_size(sample_size)
	        , timestamp(timestamp_in_seconds)
	{
		data = new char[data_size];
		memset(data, 0, data_size);
	}
};

class AudioTape {
    public:
	using const_iterator = std::list<AudioSample>::const_iterator;
	using iterator = std::list<AudioSample>::iterator;

	AudioTape(unsigned int sample_rate, int gain, float audio_delay);

	void runRecording(float tape_length_in_seconds, const std::function<bool()> &stop_signal);
	void runRecording(float tape_length_in_seconds, BooleanEvent stop_signal, void *context);

	iterator getSampleAfter(float time);

	void returnSample(iterator it) const;

	iterator toNextSample(iterator it);

	bool isValid(iterator it) const;

	bool isReady() const
	{
		return _sound.isReady();
	}

	void makeSureReady() const
	{
		if (!isReady())
			throw std::runtime_error("PCM sound is NOT ready!");
	}

	bool isRecording() const
	{
		return _recording.load();
	}

	unsigned int getSampleRate() const
	{
		return _sound.getSampleRate();
	}

	unsigned int bytesInPeriod(float seconds) const
	{
		return _sound.bytesInPeriod(seconds);
	}

	float bytesToPeriod(unsigned int bytes) const
	{
		return _sound.bytesToPeriod(bytes);
	}

    private:
	PcmSound _sound;
	std::list<AudioSample> _tape;
	std::mutex _tape_mutex;
	std::atomic_bool _recording;
	unsigned int _frames_in_record;
	float _audio_delay_seconds;
};

float timespec_to_float(const struct timespec &time);

#endif //AUDIOTAPE_H_
