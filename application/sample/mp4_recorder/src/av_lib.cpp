#include "av_lib.h"

#include "audio_tape.h"
#include "av_recorder.h"
#include "mp4_log_define.h"


AvAudioTape *AV_createAudioTape(unsigned int sample_rate, int gain, float audio_delay_seconds)
{
	return reinterpret_cast<AvAudioTape *>(new AudioTape(sample_rate, gain, audio_delay_seconds));
}

void AV_AudioTape_runRecording(AvAudioTape *subject, float tape_length_in_seconds,
                               BooleanEvent stop_signal, void *context)
{
	auto *tape = reinterpret_cast<AudioTape *>(subject);
	tape->runRecording(tape_length_in_seconds, stop_signal, context);
}

bool AV_AudioTape_isReady(AvAudioTape *subject)
{
	auto *tape = reinterpret_cast<AudioTape *>(subject);
	return tape->isReady();
}

bool AV_AudioTape_isRecording(AvAudioTape *subject)
{
	auto *tape = reinterpret_cast<AudioTape *>(subject);
	return tape->isRecording();
}

unsigned int AV_AudioTape_getSampleRate(AvAudioTape *subject)
{
	auto *tape = reinterpret_cast<AudioTape *>(subject);
	return tape->getSampleRate();
}

void AV_AudioTape_dispose(AvAudioTape *subject)
{
	auto *tape = reinterpret_cast<AudioTape *>(subject);
	delete tape;
}

AvAvRecorder *AV_createAvRecorder(MPI_ECHN encoder)
{
	return reinterpret_cast<AvAvRecorder *>(new AVRecorder(encoder));
}

bool AV_AvRecorder_isActive(AvAvRecorder *subject)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	return av_recorder->isActive();
}

bool AV_AvRecorder_activate(AvAvRecorder *subject)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	return av_recorder->activate();
}

bool AV_AvRecorder_activateWithAudio(AvAvRecorder *subject, AvAudioTape *tape)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	auto *audio_tape = reinterpret_cast<AudioTape *>(tape);
	try {
		av_recorder->activateWithAudio(*audio_tape);

	} catch (const std::runtime_error& error) {
		mp4_log_err("Runtime error: %s\n", error.what());
		return false;
	}

	return true;
}

bool AV_AvRecorder_createAudioTapeAndActivate(AvAvRecorder *subject, unsigned int sample_rate, int gain,
                                              float buffer_audio_seconds, float audio_delay_seconds)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	try {
		av_recorder->activateWithAudio(sample_rate, gain, buffer_audio_seconds, audio_delay_seconds);

	} catch (const std::runtime_error& error) {
		mp4_log_err("Runtime error: %s\n", error.what());
		return false;
	}

	return true;
}

void AV_AvRecorder_deactivate(AvAvRecorder *subject)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	av_recorder->deactivate();
}

void AV_AvRecorder_exportToFile(AvAvRecorder *subject, const char *file_path, float start_point, bool variable_rate,
                                EndTrigger end_trigger, void *context)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	try {
		av_recorder->exportToFile(file_path, start_point, variable_rate, end_trigger, context);

	} catch (const std::runtime_error& error) {
		mp4_log_err("Runtime error: %s\n", error.what());
	}
}

void AV_AvRecorder_dispose(AvAvRecorder *subject)
{
	auto *av_recorder = reinterpret_cast<AVRecorder *>(subject);
	delete av_recorder;
}
