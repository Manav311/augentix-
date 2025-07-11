#include "audio_tape.h"

#include <ctime>
#include <memory>

#include "mp4_log_define.h"


AudioTape::AudioTape(unsigned int sample_rate, int gain, float audio_delay)
        : _sound(sample_rate, gain, true)
        , _recording(false)
	, _frames_in_record(128)
	, _audio_delay_seconds(audio_delay)
{
	if (sample_rate > 8000) {
		_frames_in_record *= sample_rate / 8000;
	}
}

static float current_time()
{
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	return cur_time.tv_nsec / 1000000000.0 + cur_time.tv_sec;
}

#define PROBING 0
void AudioTape::runRecording(float tape_length_in_seconds, const std::function<bool()>& stop_signal)
{
	makeSureReady();

	std::unique_ptr<char[]> sample_data{ new char[_sound.getChunkSize(_frames_in_record)] };
	float last_timestamp = -1;
#if PROBING
	float last_pcm_timestamp = -1;
#endif
	_recording.store(true);
	mp4_log_info("Begin of AudioTape recording: %.3f(audio delay=%.3f).\n", current_time(), _audio_delay_seconds);
	while (!stop_signal()) {
		long frames = _sound.takeSamples(sample_data.get(), _frames_in_record);
		// mp4_log_info("*a: %.3f\n", current_time());
		if (frames > 0) {
			unsigned int sample_size = _sound.getChunkSize(frames);
			if (static_cast<unsigned int>(frames) < _frames_in_record) {
				mp4_log_notice("AudioTape::runRecording: Not enough samples to record (%ld < %u).\n",
					frames, _frames_in_record);
			}

			snd_htimestamp_t timestamp;
			snd_htimestamp_t trigger;
			snd_htimestamp_t audio;
			snd_pcm_uframes_t available;
			snd_pcm_sframes_t delay;
			if (!_sound.reportTimestamp(timestamp, trigger, audio, available, delay)) {
				throw std::runtime_error("FAILED to get PCM sound timestamp!");
			}
			mp4_log_debug("timestamp, trigger, audio: %ld.%09lu, %ld.%09lu, %ld.%09lu\n",
			              timestamp.tv_sec, timestamp.tv_nsec,
			              trigger.tv_sec, trigger.tv_nsec,
			              audio.tv_sec, audio.tv_nsec);
			mp4_log_debug("available, delay: %lu, %ld\n", available, delay);
#if PROBING
			float timestamp_seconds = timespec_to_float(timestamp);
			if (timestamp_seconds != last_pcm_timestamp) {
				mp4_log_info("a:%.3f\n", timestamp_seconds);
				last_pcm_timestamp = timestamp_seconds;
			}
#endif
			/* assume timestamp is samples batch end time */
			/* This 0.128 should be adjusted according to period_size/rate in /etc/asound.conf */
			float seconds = timespec_to_float(timestamp) - 1024.0f/8000 - _audio_delay_seconds;
			if (seconds <= last_timestamp) {
				seconds = last_timestamp + 1.0 / _sound.getSampleRate() * frames;
			}
			{
				std::unique_lock<std::mutex> lock{ _tape_mutex };
				mp4_log_debug("+ audio@%.3f\n", seconds);
				_tape.emplace_back(sample_data.get(), sample_size, seconds);
			}
			last_timestamp = seconds;
		}

		const float start_seconds = last_timestamp - tape_length_in_seconds;
		{
			std::unique_lock<std::mutex> lock{ _tape_mutex };
			while (!_tape.empty()) {
				auto& sample = _tape.front();
				if (sample.pined.load()) {
					break;
				}

				if (sample.timestamp >= start_seconds) {
					break;
				}

				mp4_log_debug("- audio@%.3f\n", sample.timestamp);
				_tape.pop_front();
			}
		}
	}
	mp4_log_info("End of AudioTape recording.\n");
	_recording.store(false);
}

void AudioTape::runRecording(float tape_length_in_seconds, BooleanEvent stop_signal, void *context)
{
	runRecording(tape_length_in_seconds, [&stop_signal, context]() {
		return stop_signal(context);
	});
}

AudioTape::iterator AudioTape::getSampleAfter(float time)
{
	std::unique_lock<std::mutex> lock{ _tape_mutex };
	for (auto it = _tape.begin(); it != _tape.end(); ++it) {
		if (it->timestamp >= time) {
			it->pined.store(true);
			return it;
		}
	}

	return _tape.end();
}

void AudioTape::returnSample(iterator it) const
{
	if (isValid(it)) {
		it->pined.store(false);
	}
}

AudioTape::iterator AudioTape::toNextSample(iterator it)
{
	if (!isValid(it)) {
		return _tape.end();
	}

	std::unique_lock<std::mutex> lock{ _tape_mutex };
	auto keep = it++;
	if (isValid(it)) {
		it->pined.store(true);
		keep->pined.store(false);
	}
	return it;
}

bool AudioTape::isValid(iterator it) const
{
	return it != _tape.end();
}

float timespec_to_float(const timespec& time)
{
	return time.tv_nsec / 1000000000.0f + time.tv_sec;
}

