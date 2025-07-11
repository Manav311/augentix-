#include <algorithm>
#include <ctime>
#include "H26xMp4File.h"

#include "Ap4FtypAtom.h"
#include "Ap4TrakAtom.h"
#include "Ap4TkhdAtom.h"
#include "Ap4SyntheticSampleTable.h"
#include "Ap4StcoAtom.h"
#include "Ap4Co64Atom.h"

uint32_t H26xMp4File::_std_sample_rate[] = { 96000, 88200, 64000, 48000, 44100, 32000,
	                                     24000, 22050, 16000, 12000, 11025, 8000 };

H26xMp4File::H26xMp4File(AP4_ByteStream& stream)
        : _is_open(false)
        , _stream{ stream }
        , _mdat_stream(nullptr)
        , _mdat_header_offset(0)
{
	_stream.AddReference();

	AP4_UI64 creation_time = 0;
	time_t now = time(nullptr);
	if (now != static_cast<time_t>(-1)) {
		// adjust the time based on the MPEG time origin
		creation_time = static_cast<AP4_UI64>(now) + 0x7C25B080;
	}

	_movie = new AP4_Movie(0, 0, creation_time, creation_time);
	_file = std::unique_ptr<AP4_File>(new AP4_File(_movie));
}

H26xMp4File::~H26xMp4File()
{
	close();
	_stream.Release();
}

AP4_Result H26xMp4File::prepareStream()
{
	if (_is_open) {
		return AP4_ERROR_INVALID_STATE;
	}

	AP4_Array<AP4_UI32> brands = reportBrands();
	_file->SetFileType(AP4_FILE_BRAND_MP42, 1, &brands[0], brands.ItemCount());

	AP4_FtypAtom *file_type = _file->GetFileType();
	if (file_type) {
		file_type->Write(_stream);
	}

	for (AP4_List<AP4_Atom>::Item *atom_item = _file->GetChildren().FirstItem(); atom_item;
	     atom_item = atom_item->GetNext()) {
		AP4_Atom *atom = atom_item->GetData();
		if (atom->GetType() == AP4_ATOM_TYPE_MDAT || atom->GetType() == AP4_ATOM_TYPE_FTYP ||
		    atom->GetType() == AP4_ATOM_TYPE_MOOV) {
			continue;
		}

		AP4_CHECK(atom->Write(_stream));
	}

	AP4_CHECK(_stream.Tell(_mdat_header_offset));
	_mdat_stream = new AP4_SubStream(_stream, _mdat_header_offset, 1ULL << 62);
	AP4_CHECK(_mdat_stream->WriteUI32(0));
	AP4_CHECK(_mdat_stream->WriteUI32(AP4_ATOM_TYPE_MDAT));

	_stream.Flush();

	_is_open = true;
	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::close()
{
	if (_is_open) {
		printf("persist cached chunk\n");
		for (auto& item : _pending_chunk) {
			AP4_UI32 track_id = item.first;
			std::list<AP4_DataBuffer *>& pending_samples = item.second.second;
			printf("Track: %d, chunk items: %zu\n", track_id, pending_samples.size());
			persistPendingSamplesToTrack(item.second.first, pending_samples, track_id);
			std::for_each(pending_samples.begin(), pending_samples.end(),
			              [](AP4_DataBuffer *buffer) { delete buffer; });
		}
		printf("persist cached chunk done!\n");
		AP4_List<AP4_Track>& all_tracks = _movie->GetTracks();
		for (auto *item = all_tracks.FirstItem(); item; item = item->GetNext()) {
			AP4_Track *track = item->GetData();
			AP4_TrakAtom *trak_atom = track->UseTrakAtom();

			AP4_Atom *atom = nullptr;
			AP4_ContainerAtom *stbl = nullptr;
			AP4_CHECK(track->GetSampleTable()->GenerateStblAtom(stbl));
			atom = stbl->FindChild("stco");
			if (atom) {
				stbl->RemoveChild(atom);
			}
			atom = stbl->FindChild("co64");
			if (atom) {
				stbl->RemoveChild(atom);
			}
			stbl->AddChild(buildChunkOffsetTable(_chunk_offset[track->GetId()]));
			atom = trak_atom->FindChild("mdia/minf", true, true);
			auto minf = AP4_DYNAMIC_CAST(AP4_AtomParent, atom);
			AP4_Atom *old_stbl = trak_atom->FindChild("mdia/minf/stbl");
			if (old_stbl) {
				minf->RemoveChild(old_stbl);
			}
			minf->AddChild(stbl);

			// MAYBE these steps are NOT necessary.
			AP4_Sample last_sample;
			AP4_CHECK(track->GetSample(track->GetSampleCount() - 1, last_sample));
			if (track->GetType() == AP4_Track::TYPE_VIDEO) {
				AP4_UI64 duration = last_sample.GetDuration();
				if (duration == 0) {
					duration = VIDEO_TIME_SCALE;
				}
				trak_atom->SetDuration(last_sample.GetDts() + duration);
				trak_atom->SetMediaDuration(last_sample.GetDts() + duration);
			} else if (track->GetType() == AP4_Track::TYPE_AUDIO) {
				AP4_UI64 duration = last_sample.GetDuration();
				if (duration == 0) {
					duration = AUDIO_TIME_UNIT;
				}
				trak_atom->SetDuration(last_sample.GetDts() + duration);
				trak_atom->SetMediaDuration(last_sample.GetDts() + duration);
			}

			if (_movie->GetDurationMs() < track->GetDurationMs()) {
				// movie duration align the longer track
				_movie->GetMvhdAtom()->SetTimeScale(track->GetMediaTimeScale());
				_movie->GetMvhdAtom()->SetDuration(track->GetDuration());
			}
		}
		AP4_Position mdat_size;
		AP4_CHECK(_mdat_stream->Tell(mdat_size));

		_movie->GetMvhdAtom()->SetNextTrackId(_movie->GetTracks().ItemCount() + 1);
		AP4_CHECK(_movie->GetMoovAtom()->Write(_stream));
		AP4_CHECK(_stream.Seek(_mdat_header_offset));
		AP4_CHECK(_stream.WriteUI32(mdat_size));
		_stream.Flush();
		_mdat_stream->Release();
		_is_open = false;
	}
	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::addH264Track(unsigned int frame_rate, AP4_AvcSequenceParameterSet& sps,
                                     AP4_AvcPictureParameterSet& pps, AP4_UI32& track_id)
{
	auto *sample_table = new AP4_SyntheticSampleTable(SAMPLE_TABLE_CHUNK_SIZE);

	unsigned int width = 0;
	unsigned int height = 0;
	sps.GetInfo(width, height);
	AP4_Array<AP4_DataBuffer> sps_array(&sps.raw_bytes, 1);
	AP4_Array<AP4_DataBuffer> pps_array(&pps.raw_bytes, 1);
	auto *sample_description = new AP4_AvcSampleDescription(
	        AP4_SAMPLE_FORMAT_AVC1, width, height, 24, "AVC Coding", sps.profile_idc, sps.level_idc,
	        sps.constraint_set0_flag << 7 | sps.constraint_set1_flag << 6 | sps.constraint_set2_flag << 5 |
	                sps.constraint_set3_flag << 4,
	        4, sps.chroma_format_idc, sps.bit_depth_luma_minus8, sps.bit_depth_chroma_minus8, sps_array, pps_array);
	AP4_CHECK(sample_table->AddSampleDescription(sample_description));
	auto *track = new AP4_Track(AP4_Track::TYPE_VIDEO, sample_table, 0, VIDEO_TIME_SCALE, 0,
	                            frame_rate * VIDEO_TIME_SCALE, 0, "und", width << 16, height << 16);
	AP4_CHECK(_movie->AddTrack(track));
	track_id = track->GetId();
	_sample_table[track->GetId()] = sample_table;
	if (std::none_of(_brands.begin(), _brands.end(), [](AP4_UI32 n) { return n == AP4_FILE_BRAND_AVC1; })) {
		_brands.emplace_back(AP4_FILE_BRAND_AVC1);
	}
	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::addH265Track(unsigned int frame_rate, AP4_HevcVideoParameterSet& vps,
                                     AP4_HevcSequenceParameterSet& sps, AP4_HevcPictureParameterSet& pps, bool is_hvc1,
                                     AP4_UI32& track_id)
{
	auto *sample_table = new AP4_SyntheticSampleTable(SAMPLE_TABLE_CHUNK_SIZE);
	unsigned int width = 0;
	unsigned int height = 0;
	sps.GetInfo(width, height);
	AP4_Array<AP4_DataBuffer> vps_array(&vps.raw_bytes, 1);
	AP4_Array<AP4_DataBuffer> sps_array(&sps.raw_bytes, 1);
	AP4_Array<AP4_DataBuffer> pps_array(&pps.raw_bytes, 1);
	AP4_UI08 parameters_completeness = is_hvc1 ? 1 : 0;

	auto *sample_description = new AP4_HevcSampleDescription(
	        is_hvc1 ? AP4_SAMPLE_FORMAT_HVC1 : AP4_SAMPLE_FORMAT_HEV1, width, height, 24, "HEVC Coding",
	        sps.profile_tier_level.general_profile_space, sps.profile_tier_level.general_tier_flag,
	        sps.profile_tier_level.general_profile_idc, sps.profile_tier_level.general_profile_compatibility_flags,
	        sps.profile_tier_level.general_constraint_indicator_flags, sps.profile_tier_level.general_level_idc,
	        0, // min_spatial_segmentation
	        0, // parallelism_type
	        sps.chroma_format_idc,
	        8, // luma_bit_depth
	        8, // chroma_bit_depth
	        0, // average_frame_rate
	        0, // constant_frame_rate
	        0, // num_temporal_layers
	        0, // temporal_id_nested
	        4, // nalu_length_size
	        vps_array, parameters_completeness, sps_array, parameters_completeness, pps_array,
	        parameters_completeness);
	AP4_CHECK(sample_table->AddSampleDescription(sample_description));

	auto *track = new AP4_Track(AP4_Track::TYPE_VIDEO, sample_table,
	                            0, // auto-select track id
	                            VIDEO_TIME_SCALE, 0, frame_rate * VIDEO_TIME_SCALE, 0, "und", width << 16,
	                            height << 16);
	AP4_CHECK(_movie->AddTrack(track));
	track_id = track->GetId();
	_sample_table[track->GetId()] = sample_table;
	if (std::none_of(_brands.begin(), _brands.end(), [](AP4_UI32 n) { return n == AP4_FILE_BRAND_HVC1; })) {
		_brands.emplace_back(AP4_FILE_BRAND_HVC1);
	}
	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::addAacTrack(AP4_UI32 sampling_frequency, unsigned int nums_channel, AP4_UI32& track_id)
{
	uint32_t sample_rate_index = getSampleRateIndex(sampling_frequency);
	AP4_UI32 sample_rate = _std_sample_rate[sample_rate_index];
	auto *sample_table = new AP4_SyntheticSampleTable(SAMPLE_TABLE_CHUNK_SIZE);

	unsigned int object_type = 2; // AAC LC
	AP4_DataBuffer decoder_info{ 2 };
	decoder_info.SetDataSize(2);
	decoder_info.UseData()[0] = (object_type << 3) | (sample_rate_index >> 1);
	decoder_info.UseData()[1] = ((sample_rate_index & 0x01) << 7) | (nums_channel << 3);

	auto *sample_description = new AP4_MpegAudioSampleDescription(AP4_OTI_MPEG4_AUDIO, // object type
	                                                              sample_rate, 16, nums_channel, &decoder_info,
	                                                              6144, 128000, 128000);
	AP4_CHECK(sample_table->AddSampleDescription(sample_description));

	auto *track = new AP4_Track(AP4_Track::TYPE_AUDIO, sample_table,
	                            0, // auto-select id
	                            sample_rate, 0, sample_rate, 0, "und", 0, 0);
	AP4_CHECK(_movie->AddTrack(track));
	track_id = track->GetId();
	_sample_table[track->GetId()] = sample_table;

	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::updateChunkOffset(AP4_TrakAtom& trak, AP4_Ordinal chunk_index, AP4_Position offset)
{
	AP4_Atom *atom;
	AP4_Ordinal chunk_id = chunk_index + 1;
	if ((atom = trak.FindChild("mdia/minf/stbl/stco", true, true)) != nullptr) {
		AP4_StcoAtom *stco = AP4_DYNAMIC_CAST(AP4_StcoAtom, atom);
		if (stco == nullptr)
			return AP4_ERROR_INTERNAL;
		printf("update chunk %d:%d offset to %llu\n", trak.GetId(), chunk_id, offset);
		stco->SetChunkOffset(chunk_id, offset);
		return AP4_SUCCESS;
	} else if ((atom = trak.FindChild("mdia/minf/stbl/co64", true, true)) != nullptr) {
		AP4_Co64Atom *co64 = AP4_DYNAMIC_CAST(AP4_Co64Atom, atom);
		if (co64 == nullptr)
			return AP4_ERROR_INTERNAL;
		printf("update chunk %d:%d offset to %llu\n", trak.GetId(), chunk_id, offset);
		co64->SetChunkOffset(chunk_id, offset);
		return AP4_SUCCESS;
	}
	return AP4_ERROR_INVALID_STATE;
}

AP4_Result H26xMp4File::addSample(AP4_UI32 track_id, AP4_DataBuffer *sample_data, bool is_sync, AP4_UI64 dts,
                                  AP4_UI32 duration)
{
	if (!_is_open) {
		return AP4_ERROR_INVALID_STATE;
	}

	auto cursor = _sample_table.find(track_id);
	if (cursor == _sample_table.end()) {
		return AP4_ERROR_NO_SUCH_ITEM;
	}

	AP4_Track *track = _movie->GetTrack(track_id);
	AP4_SyntheticSampleTable *sample_table = cursor->second;
	AP4_UI32 average_duration = track->GetType() == AP4_Track::TYPE_VIDEO ? VIDEO_TIME_SCALE : AUDIO_TIME_UNIT;
	//    AP4_CHECK(sample_table->AddSample(*new AP4_MemoryByteStream(*sample_data), 0, sample_data->GetDataSize(),
	//                                      duration, 0,duration * sample_table->GetSampleCount(), 0,is_sync));
	// we never touch stream data in sample, so here we don't care offset
	if (dts == 0 && sample_table->GetSampleCount() > 0) {
		const AP4_Sample& last_sample = sample_table->UseSample(sample_table->GetSampleCount() - 1);
		if (last_sample.GetDuration() != 0) {
			dts = last_sample.GetDts() + last_sample.GetDuration();
		} else {
			dts = last_sample.GetDts() + average_duration;
		}
	}
	AP4_CHECK(sample_table->AddSample(_stream, 0, sample_data->GetDataSize(), duration, 0, dts, 0, is_sync));

	// test if a new chunk?
	AP4_Ordinal chunk_index = 0;
	AP4_Ordinal position_in_chunk = 0;
	AP4_CHECK(sample_table->GetSampleChunkPosition(sample_table->GetSampleCount() - 1, chunk_index,
	                                               position_in_chunk));
	if (position_in_chunk == 0) {
		// a new chunk, persist last chunk to file
		auto pending_chunk_record = _pending_chunk[track_id];
		AP4_CHECK(persistPendingSamplesToTrack(pending_chunk_record.first, pending_chunk_record.second,
		                                       track_id));
		std::for_each(pending_chunk_record.second.begin(), pending_chunk_record.second.end(),
		              [](AP4_DataBuffer *buffer) { delete buffer; });

		_pending_chunk[track_id] = std::make_pair(chunk_index, std::list<AP4_DataBuffer *>{});
	}
	std::list<AP4_DataBuffer *>& chunk = _pending_chunk[track_id].second;
	chunk.emplace_back(sample_data);

	return AP4_SUCCESS;
}

AP4_Result H26xMp4File::persistPendingSamplesToTrack(AP4_Ordinal chunk_index,
                                                     std::list<AP4_DataBuffer *>& pending_samples, AP4_UI32 track_id)
{
	if (!pending_samples.empty()) {
		AP4_Position chunk_offset = 0;
		AP4_CHECK(_stream.Tell(chunk_offset));

		//        AP4_CHECK(updateChunkOffset(*_movie->GetTrack(track_id)->UseTrakAtom(), chunk_index, chunk_offset));
		_chunk_offset[track_id].Append(chunk_offset);
		for (auto *item : pending_samples) {
			AP4_CHECK(_mdat_stream->Write(item->UseData(), item->GetDataSize()));
		}
		_stream.Flush();
	}
	return AP4_SUCCESS;
}

AP4_Atom *H26xMp4File::buildChunkOffsetTable(AP4_Array<AP4_Position>& offset_array)
{
	if (offset_array[offset_array.ItemCount() - 1] <= 0xFFFFFFFF) {
		auto chunk_offsets_32 = new AP4_UI32[offset_array.ItemCount()];
		for (AP4_Cardinal i = 0; i < offset_array.ItemCount(); ++i) {
			chunk_offsets_32[i] = static_cast<AP4_UI32>(offset_array[i]);
		}
		return new AP4_StcoAtom(chunk_offsets_32, offset_array.ItemCount());
	} else {
		return new AP4_Co64Atom(&offset_array[0], offset_array.ItemCount());
	}
}

uint32_t H26xMp4File::getSampleRateIndex(uint32_t sample_rate)
{
	if (92017 <= sample_rate)
		return 0;
	if (75132 <= sample_rate)
		return 1;
	if (55426 <= sample_rate)
		return 2;
	if (46009 <= sample_rate)
		return 3;
	if (37566 <= sample_rate)
		return 4;
	if (27713 <= sample_rate)
		return 5;
	if (23004 <= sample_rate)
		return 6;
	if (18783 <= sample_rate)
		return 7;
	if (13856 <= sample_rate)
		return 8;
	if (11502 <= sample_rate)
		return 9;
	if (9391 <= sample_rate)
		return 10;

	return 11;
}
