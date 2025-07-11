#ifndef AV_RECORDER_H_
#define AV_RECORDER_H_

#include <atomic>
#include <memory>
#include <thread>

#include "mpi_enc.h"
#include "audio_tape.h"
#include "aacenc_lib.h"
#include "av_lib.h"

class AacEncoder {
    public:
	AacEncoder(UINT sample_rate, UINT channels);

	~AacEncoder();

	INT digest(void *buffer, INT buffer_size, UCHAR *&aac_frame, INT &aac_frame_size) const;

	bool isReady() const
	{
		return _error == AACENC_OK;
	}

    private:
	AACENC_ERROR _error{ AACENC_OK };
	HANDLE_AACENCODER _aac_encoder{ nullptr };
	const UINT _sample_rate;
	const UINT _channels;
	UINT _max_frame_size;
};

class AVRecorder {
    public:
	explicit AVRecorder(MPI_ECHN encoder);

	bool activate();
	void activateWithAudio(AudioTape &tape);
	void activateWithAudio(unsigned int sample_rate, int gain, float buffer_audio_seconds,
	                       float audio_delay_seconds);
	void deactivate();

	void exportToFile(const std::string &file_path, float start_point, bool variable_rate, EndTrigger end_trigger,
	                  void *context) const;
	bool isActive() const
	{
		return _activated.load();
	}

    protected:
	void makeSureNotActive() const
	{
		if (isActive()) {
			throw std::runtime_error("still activated");
		}
	}

    private:
	MPI_BCHN _bitstream;
	const MPI_ECHN _encoder;
	std::unique_ptr<AudioTape> _local_tape;
	std::unique_ptr<AacEncoder> _aac_encoder;
	AudioTape *_tape;
	std::atomic_bool _activated;
	std::thread _audio_task;
	std::atomic_bool _stop_tape;
};

#endif //AV_RECORDER_H_
