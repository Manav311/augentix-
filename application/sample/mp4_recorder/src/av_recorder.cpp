#include "av_recorder.h"

#include <cassert>

#include "Ap4FileByteStream.h"
#include "H26xMp4File.h"
#include "video_utils.h"
#include "mp4_log_define.h"


int is_little_endian()
{
	int test_num = 0xff;
	auto *ptr = reinterpret_cast<unsigned char *>(&test_num);
	if (ptr[0] == 0xff) {
		return 1;
	}
	return 0;
}

AacEncoder::AacEncoder(unsigned int sample_rate, unsigned int channels)
        : _sample_rate(sample_rate)
        , _channels(channels)
{
	static_assert(sizeof(INT_PCM) == PcmSound::getBytesPerFrame(), "PCM sample size MISMATCH AAC sample size");

	if (!is_little_endian()) {
		_error = AACENC_INIT_ERROR;
		mp4_log_err("This machine is NOT little-endian?!\n");
		return;
	}

	CHANNEL_MODE mode = MODE_INVALID;
	/* Currently we only consider 1 and 2 channels. */
	switch (channels) {
	case 1:
		mode = MODE_1;
		break;
	case 2:
		mode = MODE_2;
		break;
	default:
		_error = AACENC_INIT_ERROR;
		return;
	}

	if ((_error = aacEncOpen(&_aac_encoder, 0, channels)) != AACENC_OK) {
		mp4_log_err("Unable to open AAC encoder: %d\n", _error);
		return;
	}

	constexpr UINT aot = AOT_AAC_LC;
	if ((_error = aacEncoder_SetParam(_aac_encoder, AACENC_AOT, aot)) != AACENC_OK) {
		mp4_log_err("Unable to set the AOT(%u): %d\n", aot, _error);
		return;
	}

	if ((_error = aacEncoder_SetParam(_aac_encoder, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK) {
		mp4_log_err("Unable to set the sample rate(%u): %d\n", sample_rate, _error);
		return;
	}

	if ((_error = aacEncoder_SetParam(_aac_encoder, AACENC_CHANNELMODE, mode)) != AACENC_OK) { //2 channle
		mp4_log_err("Unable to set the channel mode(%u): %d\n", mode, _error);
		return;
	}

	if ((_error = aacEncoder_SetParam(_aac_encoder, AACENC_BITRATE, sample_rate)) != AACENC_OK) {
		mp4_log_err("Unable to set the bitrate(%u): %d\n", sample_rate, _error);
		return;
	}

	constexpr UINT transmux = TT_MP4_ADTS;
	if ((_error = aacEncoder_SetParam(_aac_encoder, AACENC_TRANSMUX, transmux)) != AACENC_OK) {
		mp4_log_err("Unable to set the ADTS transmux(%u): %d\n", transmux, _error);
		return;
	}

	if ((_error = aacEncEncode(_aac_encoder, nullptr, nullptr, nullptr, nullptr)) != AACENC_OK) {
		mp4_log_err("Unable to initialize AAC encoder: %d\n", _error);
		return;
	}

	AACENC_InfoStruct info{ 0 };
	if ((_error = aacEncInfo(_aac_encoder, &info)) != AACENC_OK) {
		mp4_log_err("Unable to get the AAC encoder info: %d\n", _error);
		return;
	}

	_max_frame_size = info.maxOutBufBytes;
	mp4_log_notice("AAC encoder: frame-length=%u\n", info.frameLength);
	mp4_log_notice("AAC encoder: delay=%u\n", info.nDelay);
	assert((info.nDelay == 2048) && "AAC encoder delay is NOT 2048");
}

AacEncoder::~AacEncoder()
{
	if (isReady()) {
		aacEncClose(&_aac_encoder);
	}
}

INT AacEncoder::digest(void *buffer, INT buffer_size, UCHAR *& aac_frame, INT& aac_frame_size) const
{
	std::unique_ptr<UCHAR> frame{ new UCHAR[_max_frame_size] };
	INT in_buffer_id = IN_AUDIO_DATA;
	INT in_element_size = sizeof(INT_PCM);
	AACENC_BufDesc in_buf{
		.numBufs = 1,
		.bufs = &buffer,
		.bufferIdentifiers = &in_buffer_id,
		.bufSizes = &buffer_size,
		.bufElSizes = &in_element_size,
	};

	INT out_buffer_id = OUT_BITSTREAM_DATA;
	INT out_element_size = sizeof(UCHAR);
	void *output = frame.get();
	INT output_size = static_cast<INT>(_max_frame_size);
	AACENC_BufDesc out_buf{
		.numBufs = 1,
		.bufs = &output,
		.bufferIdentifiers = &out_buffer_id,
		.bufSizes = &output_size,
		.bufElSizes = &out_element_size,
	};

	AACENC_InArgs in_args{ 0 };
	in_args.numInSamples = buffer_size / in_element_size;

	AACENC_OutArgs out_args{ 0 };
	auto error = aacEncEncode(_aac_encoder, &in_buf, &out_buf, &in_args, &out_args);
	if (error != AACENC_OK) {
		mp4_log_err("aacEncEncode FAILED, err=%d\n", error);
		aac_frame = nullptr;
		aac_frame_size = 0;
		return 0;
	}

	mp4_log_debug("%d frames consumed, %d bytes frame generated.\n", out_args.numInSamples, out_args.numOutBytes);
	aac_frame_size = out_args.numOutBytes;
	if (aac_frame_size > 0) {
		aac_frame = frame.release();
	} else {
		aac_frame = nullptr;
	}

	return out_args.numInSamples * in_element_size;
}

AVRecorder::AVRecorder(MPI_ECHN encoder)
        : _bitstream(MPI_INVALID_ENC_BCHN)
        , _encoder(encoder)
        , _tape(nullptr)
        , _activated(false)
        , _stop_tape(false)
{
}

bool AVRecorder::activate()
{
	makeSureNotActive();
	_bitstream = MPI_createBitStreamChn(_encoder);
	if (VALID_MPI_ENC_BCHN(_bitstream)) {
		_activated.store(true);
		return true;
	}

	return false;
}

void AVRecorder::activateWithAudio(AudioTape& tape)
{
	makeSureNotActive();
	if (!tape.isRecording()) {
		throw std::runtime_error("AudioTape is NOT recording!");
	}

	_aac_encoder.reset(new AacEncoder(tape.getSampleRate(), 1));
	if (!_aac_encoder->isReady()) {
		throw std::runtime_error("AAC encoder is NOT ready!");
	}
	if (!activate()) {
		throw std::runtime_error(fmt::format("{}, NOT a valid encoder!?", _encoder.chn));
	}
	_tape = &tape;
}

void AVRecorder::activateWithAudio(unsigned int sample_rate, int gain, float buffer_audio_seconds,
                                   float audio_delay_seconds)
{
	makeSureNotActive();

	std::unique_ptr<AudioTape> tape(new AudioTape(sample_rate, gain, audio_delay_seconds));
	tape->makeSureReady();

	_local_tape = std::move(tape);
	_audio_task = std::thread{ [this, buffer_audio_seconds]() {
		_local_tape->runRecording(buffer_audio_seconds, [this]() { return _stop_tape.load(); });
	} };

	while (!_local_tape->isRecording()) {
		usleep(1000);
	}

	try {
		activateWithAudio(*_local_tape);

	} catch (...) {
		_stop_tape.store(true);
		if (_audio_task.joinable()) {
			_audio_task.join();
		}
		_local_tape = nullptr;
		_aac_encoder = nullptr;
		throw;
	}
}

void AVRecorder::deactivate()
{
	if (!isActive()) {
		return;
	}

	if (_local_tape) {
		_stop_tape.store(true);
		if (_audio_task.joinable()) {
			_audio_task.join();
		}
		_local_tape = nullptr;
	}

	_aac_encoder = nullptr;

	MPI_destroyBitStreamChn(_bitstream);
	_bitstream = MPI_INVALID_ENC_BCHN;
	_activated.store(false);
}

struct VideoFrame {
	char *data;
	size_t data_size;
	float timestamp;
	bool is_keyframe;

	VideoFrame(MPI_STREAM_PARAMS_V2_S& frame, float timestamp)
	        : timestamp(timestamp)
	{
		auto& data_seg = frame.seg[frame.seg_cnt - 1];
		data_size = data_seg.size - 4;
		data = new char[data_size];
		memcpy(data, data_seg.uaddr + 4, data_size);
		is_keyframe = data_seg.type == MPI_FRAME_TYPE_I;
	}

	AP4_DataBuffer *build() const
	{
		auto *sample_buffer = new AP4_DataBuffer(data_size + 4);
		auto *buffer_stream = new AP4_MemoryByteStream(*sample_buffer);
		AP4_GUARD(buffer_stream->WriteUI32(data_size));
		AP4_GUARD(buffer_stream->Write(data, data_size));
		buffer_stream->Release();
		return sample_buffer;
	}

	~VideoFrame()
	{
		delete[] data;
	}
};

void visitFirstKeyFrameAfter(float timestamp, MPI_BCHN bs_channel,
                             const std::function<void(MPI_STREAM_PARAMS_V2_S&, std::list<VideoFrame>&)>& visitor)
{
	std::list<VideoFrame> previous_gop;
	mp4_log_notice("Searching first I frame after T:%.3f.\n", timestamp);
	while (true) {
		RequestFrame request(bs_channel);
		if (!request.isReady()) {
			mp4_log_warn("Cannot get frame from bit-stream, encoder=%d.\n", bs_channel.chn);
			continue;
		}
		auto& frame = request.useFrame();
		float frame_ts = timespec_to_float(frame.timestamp);
		auto& data_seg = frame.seg[frame.seg_cnt - 1];
		if (data_seg.type == MPI_FRAME_TYPE_I) {
			if (frame_ts < timestamp) {
				previous_gop.clear();
				previous_gop.emplace_back(frame, frame_ts);
			} else {
				if (previous_gop.empty()) {
					mp4_log_notice("Seek to I frame@%.3f.\n", frame_ts);
				} else {
					mp4_log_notice("Seek to I frame@%.3f with %u frames in previous GOP@%.3f.\n",
					               frame_ts, previous_gop.size(), previous_gop.front().timestamp);
				}

				visitor(frame, previous_gop);
				break;
			}
		} else {
			previous_gop.emplace_back(frame, frame_ts);
		}
	}
}

void AVRecorder::exportToFile(const std::string& file_path, float start_point, bool variable_rate,
                              EndTrigger end_trigger, void *context) const
{
	AP4_ByteStream *output = nullptr;
	AP4_GUARD(AP4_FileByteStream::Create(file_path.c_str(), AP4_FileByteStream::STREAM_MODE_WRITE, output));

	std::mutex mp4_mutex;
	H26xMp4File mp4_file{ *output };
	/* write necessary header to mp4 file */
	AP4_GUARD(mp4_file.prepareStream());

	MPI_VENC_TYPE_E encoder_type = MPI_VENC_TYPE_NUM;
	int ret = queryEncoderType(_encoder.chn, encoder_type);
	if (ret != MPI_SUCCESS) {
		throw std::runtime_error(fmt::format("Cannot determine video encoder type, err={}!", ret));
	}

	if (encoder_type != MPI_VENC_TYPE_H264 && encoder_type != MPI_VENC_TYPE_H265) {
		throw std::runtime_error("Encoder is neither H.264 nor H.265!");
	}

	int frame_rate = -1;
	ret = queryEncoderFrameRate(_encoder.chn, frame_rate);
	if (ret != MPI_SUCCESS) {
		throw std::runtime_error(fmt::format("Cannot determine encoder frame rate, err={}!", ret));
	}

	const bool is_h265 = encoder_type == MPI_VENC_TYPE_H265;
	AP4_UI32 video_track_id = 0;
	float first_frame_ts = 0;
	float track_timestamp = 0;
	unsigned int nums_frame = 0;
	visitFirstKeyFrameAfter(
	        start_point, _bitstream,
	        [is_h265, frame_rate, &mp4_file, &mp4_mutex, &video_track_id, &nums_frame, &first_frame_ts,
	         &track_timestamp, variable_rate](MPI_STREAM_PARAMS_V2_S& frame, std::list<VideoFrame>& previous_gop) {
			if (is_h265) {
				AP4_HevcVideoParameterSet vps;
				AP4_HevcSequenceParameterSet sps;
				AP4_HevcPictureParameterSet pps;
				AP4_GUARD(decodeH265XPS(frame, vps, sps, pps));
				AP4_GUARD(mp4_file.addH265Track(frame_rate, vps, sps, pps, true, video_track_id));
			} else {
				AP4_AvcSequenceParameterSet sps;
				AP4_AvcPictureParameterSet pps;
				AP4_GUARD(decodeH264XPS(frame, sps, pps));
				AP4_GUARD(mp4_file.addH264Track(frame_rate, sps, pps, video_track_id));
			}
			previous_gop.emplace_back(frame, timespec_to_float(frame.timestamp));
			nums_frame = previous_gop.size();
			first_frame_ts = previous_gop.front().timestamp;
			track_timestamp = previous_gop.back().timestamp - first_frame_ts;
			std::unique_lock<std::mutex> lock{ mp4_mutex };
			for (auto&& vframe : previous_gop) {
				AP4_UI64 dts = 0;
				if (variable_rate) {
					dts = (vframe.timestamp - first_frame_ts)
					      * mp4_file.getMediaTimeScale(video_track_id);
				}
				mp4_file.addSample(video_track_id, vframe.build(), vframe.is_keyframe, dts);
			}
		});

	const AP4_UI32 media_time_scale = mp4_file.getMediaTimeScale(video_track_id);
	AP4_UI32 audio_track_id = 0;
	AudioTape::iterator audio_cursor;
	bool cursor_is_done = false;
	float audio_start = -1;
	float audio_timestamp = -1;
	/*
	 * FDK-AAC encoder add 2048 silence PCM samples at the start of stream.
	 * 2048 PCM samples encoded to 2 AAC frames.
	 * See:https://github.com/mstorsjo/fdk-aac/issues/24#issuecomment-68954054
	 */
	int aac_padding_skip = 2;
	if (_tape) {
		AP4_GUARD(mp4_file.addAacTrack(_tape->getSampleRate(), 1, audio_track_id));
	}
	// mux all frames into mp4 file
	while (!end_trigger(context, nums_frame, track_timestamp)) {
		RequestFrame request(_bitstream);
		if (request.isReady()) {
			mp4_log_debug("Frame#%u@%.3f\n", nums_frame, timespec_to_float(request.useFrame().timestamp));
			++nums_frame;
			auto& frame = request.useFrame();
			track_timestamp = timespec_to_float(frame.timestamp) - first_frame_ts;
			auto& data_seg = frame.seg[frame.seg_cnt - 1];
			auto *sample_buffer = new AP4_DataBuffer(data_seg.size);
			auto *buffer_stream = new AP4_MemoryByteStream(*sample_buffer);
			AP4_GUARD(buffer_stream->WriteUI32(data_seg.size - 4));
			AP4_GUARD(buffer_stream->Write(data_seg.uaddr + 4, data_seg.size - 4));
			buffer_stream->Release();
			AP4_UI64 dts = 0;
			if (variable_rate) {
				dts = static_cast<AP4_UI64>(track_timestamp * media_time_scale);
			}
			// std::unique_lock<std::mutex> lock{ mp4_mutex };
			AP4_GUARD(mp4_file.addSample(video_track_id, sample_buffer, data_seg.type == MPI_FRAME_TYPE_I,
			                             dts));
		} else {
			mp4_log_warn("Cannot get frame from bit-stream, encoder=%d.\n", _bitstream.chn);
		}
		// mux audio samples
		if (_tape) {
			if (audio_start < 0) {
				audio_cursor = _tape->getSampleAfter(first_frame_ts);
				if (!_tape->isValid(audio_cursor)) {
					continue;
				}
				audio_start = audio_cursor->timestamp;
				audio_timestamp = audio_cursor->timestamp;
				cursor_is_done = false;
			}

			while (_tape->isValid(audio_cursor)) {
				if (!cursor_is_done) {
					mp4_log_debug("A:%.3f\n", audio_cursor->timestamp);
					char *head = audio_cursor->data;
					char *end = head + audio_cursor->data_size;
					UCHAR* aac_frame;
					INT aac_frame_size;
					while (head < end) {
						head += _aac_encoder->digest(head, end - head, aac_frame, aac_frame_size);
						if (aac_frame) {
							if (aac_padding_skip > 0) {
								--aac_padding_skip;
								if (aac_padding_skip == 0) {
									/* AAC silence padding had skipped, adjust audio track start time */
									audio_start =
									        audio_cursor->timestamp +
									        _tape->bytesToPeriod(
									                head - audio_cursor->data);
								}
							} else {
								auto *sample_buffer =
								        new AP4_DataBuffer(aac_frame, aac_frame_size);
								delete[] aac_frame;
#define AUDIO_VARIABLE_RATE 1
#if AUDIO_VARIABLE_RATE
								auto dts = static_cast<AP4_UI64>(
								        (audio_timestamp - audio_start) *
								        _tape->getSampleRate());
								mp4_log_debug("AAC dts: %llu (%.3f)\n", dts,
								              audio_timestamp);
								mp4_file.addSample(audio_track_id, sample_buffer, true,
								                   dts);
#else
								(void)audio_timestamp;
								mp4_file.addSample(audio_track_id, sample_buffer, true,
								                   0, 1024);
#endif
								audio_timestamp =
								        audio_cursor->timestamp +
								        _tape->bytesToPeriod(head - audio_cursor->data);
							}
						}
						// mp4_log_info("AT: %.3f\n", audio_timestamp);
					}
					cursor_is_done = true;
				}
				auto next = _tape->toNextSample(audio_cursor);
				if (!_tape->isValid(next)) {
					break;
				}
				audio_cursor = next;
				cursor_is_done = false;
			}
		}
	}
}
