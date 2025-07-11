#ifndef MP4_RECORDER_H26XMP4FILE_H_
#define MP4_RECORDER_H26XMP4FILE_H_

#include <memory>
#include <map>
#include <list>
#include <stdexcept>

#include "fmt/core.h"
#include "Ap4ByteStream.h"
#include "Ap4File.h"
#include "Ap4Movie.h"
#include "Ap4HevcParser.h"
#include "Ap4AvcParser.h"
#include "Ap4SyntheticSampleTable.h"

#define AP4_GUARD(_x)                                                                                                 \
	do {                                                                                                          \
		AP4_Result _result = (_x);                                                                            \
		if (AP4_FAILED(_result)) {                                                                            \
			throw std::runtime_error(fmt::format("{} => {}({})", #_x, _result, AP4_ResultText(_result))); \
		}                                                                                                     \
	} while (0)

class H26xMp4File {
    public:
	static constexpr int VIDEO_TIME_SCALE = 1000;
	static constexpr int AUDIO_TIME_UNIT = 1024;
	static constexpr int SAMPLE_TABLE_CHUNK_SIZE = 10;

	explicit H26xMp4File(AP4_ByteStream &stream);
	virtual ~H26xMp4File();

	AP4_Result prepareStream();
	AP4_Result addH264Track(unsigned int frame_rate, AP4_AvcSequenceParameterSet &sps,
	                        AP4_AvcPictureParameterSet &pps, AP4_UI32 &track_id);
	AP4_Result addH265Track(unsigned int frame_rate, AP4_HevcVideoParameterSet &vps,
	                        AP4_HevcSequenceParameterSet &sps, AP4_HevcPictureParameterSet &pps, bool is_hvc1,
	                        AP4_UI32 &track_id);
	AP4_Result addAacTrack(AP4_UI32 sampling_frequency, unsigned int nums_channel, AP4_UI32 &track_id);
	AP4_Result addSample(AP4_UI32 track_id, AP4_DataBuffer *sample_data, bool is_sync, AP4_UI64 dts = 0,
	                     AP4_UI32 duration = 0); // sample_data ownership transferred
	AP4_Result close();
	AP4_UI32 getMediaTimeScale(AP4_UI32 track_id) const
	{
		auto *track = _movie->GetTrack(track_id);
		if (track) {
			return track->GetMediaTimeScale();
		}
		return 0;
	}

	AP4_Cardinal getTrackCount() const
	{
		return _movie->GetTracks().ItemCount();
	}

	static uint32_t getSampleRateIndex(uint32_t sample_rate);
	static AP4_Atom *buildChunkOffsetTable(AP4_Array<AP4_Position> &offset_array);

    protected:
	static uint32_t _std_sample_rate[];
	static AP4_Result updateChunkOffset(AP4_TrakAtom &trak, AP4_Ordinal chunk_index, AP4_Position offset);
	AP4_Result persistPendingSamplesToTrack(AP4_Ordinal chunk_index, std::list<AP4_DataBuffer *> &pending_samples,
	                                        AP4_UI32 track_id);

    private:
	virtual AP4_Array<AP4_UI32> reportBrands()
	{
		AP4_Array<AP4_UI32> brands;
		for (const auto &item : _brands) {
			brands.Append(item);
		}
		return brands;
	}

	std::list<AP4_UI32> _brands{ AP4_FILE_BRAND_ISOM, AP4_FILE_BRAND_MP42 };
	bool _is_open;
	AP4_ByteStream &_stream;
	AP4_ByteStream *_mdat_stream;
	std::unique_ptr<AP4_File> _file;
	AP4_Movie *_movie;

	AP4_Position _mdat_header_offset;

	std::map<AP4_UI32, AP4_SyntheticSampleTable *> _sample_table;
	// track_id -> (chunk_index, pending_samples)
	std::map<AP4_UI32, std::pair<AP4_Ordinal, std::list<AP4_DataBuffer *> > > _pending_chunk;
	std::map<AP4_UI32, AP4_Array<AP4_Position> > _chunk_offset;
};

#endif //MP4_RECORDER_H26XMP4FILE_H_
