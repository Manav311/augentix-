/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// A filter that breaks up a H.264 or H.265 Video Elementary Stream into NAL
// units.
// Implementation

#include "H264or5VideoStreamFramer.hh"
#include <assert.h>
#include "BitVector.hh"
#include "MPEGVideoStreamParser.hh"
#include "GroupsockHelper.hh"

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_enc.h"

#define REPLACE_BUFFER_TO_POINTER

typedef struct {
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S params;
#else
	MPI_STREAM_PARAMS_S params;
#endif
	unsigned char *p_sei_nalu;
	int sei_nalu_len;
} VIDEO_STREAM_DATA;

////////// H264or5VideoStreamParser definition //////////

class H264or5VideoStreamParser : public MPEGVideoStreamParser {
    public:
	H264or5VideoStreamParser(int hNumber, H264or5VideoStreamFramer *usingSource, FramedSource *inputSource,
	                         Boolean includeStartCodeInOutput);
	virtual ~H264or5VideoStreamParser();

    private: // redefined virtual functions:
	virtual void flushInput();
	virtual unsigned parse();

    private:
	H264or5VideoStreamFramer *usingSource()
	{
		return (H264or5VideoStreamFramer *)fUsingSource;
	}

	Boolean isVPS(u_int8_t nal_unit_type)
	{
		return usingSource()->isVPS(nal_unit_type);
	}
	Boolean isSPS(u_int8_t nal_unit_type)
	{
		return usingSource()->isSPS(nal_unit_type);
	}
	Boolean isPPS(u_int8_t nal_unit_type)
	{
		return usingSource()->isPPS(nal_unit_type);
	}
	Boolean isVCL(u_int8_t nal_unit_type)
	{
		return usingSource()->isVCL(nal_unit_type);
	}
	Boolean isSEI(u_int8_t nal_unit_type);
	Boolean isEOF(u_int8_t nal_unit_type);
	Boolean usuallyBeginsAccessUnit(u_int8_t nal_unit_type);

	void removeEmulationBytes(u_int8_t *nalUnitCopy, unsigned maxSize, unsigned &nalUnitCopySize);
	void profile_tier_level(BitVector &bv, unsigned max_sub_layers_minus1);

	void analyze_sei_data(u_int8_t nal_unit_type);

    private:
	int fHNumber; // 264 or 265
	unsigned fOutputStartCodeSize;
	Boolean fHaveSeenFirstStartCode, fHaveSeenFirstByteOfNALUnit;
	u_int8_t fFirstByteOfNALUnit;
	double fParsedFrameRate;
	int fFrame_remain_cnt;
	VIDEO_STREAM_DATA *fParam_data_ptr;
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S *fParam_ptr;
#else
	MPI_STREAM_PARAMS_S *fParam_ptr;
#endif
};

////////// H264or5VideoStreamFramer implementation //////////
#define SPS_MAX_SIZE 1000 // larger than the largest possible SPS (Sequence Parameter Set) NAL unit
H264or5VideoStreamFramer::H264or5VideoStreamFramer(int hNumber, UsageEnvironment &env, FramedSource *inputSource,
                                                   Boolean createParser, Boolean includeStartCodeInOutput)
        : MPEGVideoStreamFramer(env, inputSource)
        , fHNumber(hNumber)
        , fLastSeenVPS(NULL)
        , fLastSeenVPSSize(0)
        , fLastSeenSPS(NULL)
        , fLastSeenSPSSize(0)
        , fLastSeenPPS(NULL)
        , fLastSeenPPSSize(0)
        , fProfileLevelId(0)
{
	for (unsigned i = 0; i < 12; ++i)
		fProfileTierLevelHeaderBytes[i] = 0;

	fLastSeenVPS = new u_int8_t[SPS_MAX_SIZE];
	fLastSeenSPS = new u_int8_t[SPS_MAX_SIZE];
	fLastSeenPPS = new u_int8_t[SPS_MAX_SIZE];

	fParser = createParser ? new H264or5VideoStreamParser(hNumber, this, inputSource, includeStartCodeInOutput) :
	                         NULL;
	fNextPresentationTime = fPresentationTimeBase;
	// fFrameRate = 25.0; // We assume a frame rate of 25 fps, unless we learn
	// otherwise (from parsing a VPS or SPS NAL unit)
	fFrameRate = 60.0; // TODO need get frame rate from config
}

H264or5VideoStreamFramer::~H264or5VideoStreamFramer()
{
	delete[] fLastSeenPPS;
	delete[] fLastSeenSPS;
	delete[] fLastSeenVPS;
}

#define VPS_MAX_SIZE 1000 // larger than the largest possible VPS (Video Parameter Set) NAL unit

void H264or5VideoStreamFramer::saveCopyOfVPS(u_int8_t *from, unsigned size)
{
	if (from == NULL || size > SPS_MAX_SIZE) {
		/*this time MPI ptr == null or MPI ptr not get, happen when ENC restart*/
		return;
	}

	memcpy(fLastSeenVPS, from, size);
	fLastSeenVPSSize = size;
	// We also make another copy - without 'emulation bytes', to extract
	// parameters that we need:
	u_int8_t vps[VPS_MAX_SIZE];
	unsigned vpsSize = removeH264or5EmulationBytes(vps, VPS_MAX_SIZE, fLastSeenVPS, fLastSeenVPSSize);

	// Extract the first 12 'profile_tier_level' bytes:
	if (vpsSize >= 6 /*'profile_tier_level' offset*/ + 12 /*num 'profile_tier_level' bytes*/) {
		memcpy(fProfileTierLevelHeaderBytes, &vps[6], 12);
	}
}

void H264or5VideoStreamFramer::saveCopyOfSPS(u_int8_t *from, unsigned size)
{
	if (size == 0) {
		fprintf(stderr, "sps size == 0\r\n");
		return;
	}

	if (from == NULL || size > SPS_MAX_SIZE) {
		/*this time MPI ptr == null or MPI ptr not get, happen when ENC restart*/
		return;
	}

	memcpy(fLastSeenSPS, from, size);
	fLastSeenSPSSize = size;
	// We also make another copy - without 'emulation bytes', to extract
	// parameters that we need:
	u_int8_t sps[SPS_MAX_SIZE];
	unsigned spsSize = removeH264or5EmulationBytes(sps, SPS_MAX_SIZE, fLastSeenSPS, fLastSeenSPSSize);
	if (fHNumber == 264) {
		// Extract the first 3 bytes of the SPS (after the nal_unit_header byte) as
		// 'profile_level_id'
		if (spsSize >= 1 /*'profile_level_id' offset within SPS*/ + 3 /*num bytes needed*/) {
			fProfileLevelId = (sps[1] << 16) | (sps[2] << 8) | sps[3];
		}
	} else { // 265
		// Extract the first 12 'profile_tier_level' bytes:
		if (spsSize >= 3 /*'profile_tier_level' offset*/ + 12 /*num 'profile_tier_level' bytes*/) {
			memcpy(fProfileTierLevelHeaderBytes, &sps[3], 12);
		}
	}
}

void H264or5VideoStreamFramer::saveCopyOfPPS(u_int8_t *from, unsigned size)
{
	if (from == NULL || size > SPS_MAX_SIZE) {
		/*this time MPI ptr == null or MPI ptr not get, happen when ENC restart*/
		return;
	}

	memcpy(fLastSeenPPS, from, size);
	fLastSeenPPSSize = size;
}

Boolean H264or5VideoStreamFramer::isVPS(u_int8_t nal_unit_type)
{
	// VPS NAL units occur in H.265 only:
	return fHNumber == 265 && nal_unit_type == 32;
}

Boolean H264or5VideoStreamFramer::isSPS(u_int8_t nal_unit_type)
{
	return fHNumber == 264 ? nal_unit_type == 7 : nal_unit_type == 33;
}

Boolean H264or5VideoStreamFramer::isPPS(u_int8_t nal_unit_type)
{
	return fHNumber == 264 ? nal_unit_type == 8 : nal_unit_type == 34;
}

Boolean H264or5VideoStreamFramer::isVCL(u_int8_t nal_unit_type)
{
	return fHNumber == 264 ? (nal_unit_type <= 5 && nal_unit_type > 0) : (nal_unit_type <= 31);
}

////////// H264or5VideoStreamParser implementation //////////

H264or5VideoStreamParser::H264or5VideoStreamParser(int hNumber, H264or5VideoStreamFramer *usingSource,
                                                   FramedSource *inputSource, Boolean includeStartCodeInOutput)
        : MPEGVideoStreamParser(usingSource, inputSource)
        , fHNumber(hNumber)
        , fOutputStartCodeSize(includeStartCodeInOutput ? 4 : 0)
        , fHaveSeenFirstStartCode(False)
        , fHaveSeenFirstByteOfNALUnit(False)
        , fParsedFrameRate(0.0)
        , fFrame_remain_cnt(0)
        , fParam_data_ptr(NULL)
        , fParam_ptr(NULL)
{
}

H264or5VideoStreamParser::~H264or5VideoStreamParser()
{
}

#define PREFIX_SEI_NUT 39 // for H.265
#define SUFFIX_SEI_NUT 40 // for H.265
Boolean H264or5VideoStreamParser::isSEI(u_int8_t nal_unit_type)
{
	return fHNumber == 264 ? nal_unit_type == 6 :
	                         (nal_unit_type == PREFIX_SEI_NUT || nal_unit_type == SUFFIX_SEI_NUT);
}

Boolean H264or5VideoStreamParser::isEOF(u_int8_t nal_unit_type)
{
	// "end of sequence" or "end of (bit)stream"
	return fHNumber == 264 ? (nal_unit_type == 10 || nal_unit_type == 11) :
	                         (nal_unit_type == 36 || nal_unit_type == 37);
}

Boolean H264or5VideoStreamParser::usuallyBeginsAccessUnit(u_int8_t nal_unit_type)
{
	return fHNumber == 264 ?
	               (nal_unit_type >= 6 && nal_unit_type <= 9) || (nal_unit_type >= 14 && nal_unit_type <= 18) :
	               (nal_unit_type >= 32 && nal_unit_type <= 35) || (nal_unit_type == 39) ||
	                       (nal_unit_type >= 41 && nal_unit_type <= 44) ||
	                       (nal_unit_type >= 48 && nal_unit_type <= 55);
}

void H264or5VideoStreamParser::removeEmulationBytes(u_int8_t *nalUnitCopy, unsigned maxSize, unsigned &nalUnitCopySize)
{
	u_int8_t *nalUnitOrig = fStartOfFrame + fOutputStartCodeSize;
	unsigned const numBytesInNALunit = fTo - nalUnitOrig;
	nalUnitCopySize = removeH264or5EmulationBytes(nalUnitCopy, maxSize, nalUnitOrig, numBytesInNALunit);
}

void H264or5VideoStreamParser::profile_tier_level(BitVector &bv, unsigned max_sub_layers_minus1)
{
	bv.skipBits(96);

	unsigned i;
	Boolean sub_layer_profile_present_flag[7], sub_layer_level_present_flag[7];
	for (i = 0; i < max_sub_layers_minus1; ++i) {
		sub_layer_profile_present_flag[i] = bv.get1BitBoolean();
		sub_layer_level_present_flag[i] = bv.get1BitBoolean();
	}
	if (max_sub_layers_minus1 > 0) {
		bv.skipBits(2 * (8 - max_sub_layers_minus1)); // reserved_zero_2bits
	}
	for (i = 0; i < max_sub_layers_minus1; ++i) {
		if (sub_layer_profile_present_flag[i]) {
			bv.skipBits(88);
		}
		if (sub_layer_level_present_flag[i]) {
			bv.skipBits(8); // sub_layer_level_idc[i]
		}
	}
}

#define SEI_MAX_SIZE 5000 // larger than the largest possible SEI NAL unit

void H264or5VideoStreamParser::analyze_sei_data(u_int8_t nal_unit_type)
{
	// Begin by making a copy of the NAL unit data, removing any 'emulation
	// prevention' bytes:
	u_int8_t sei[SEI_MAX_SIZE];
	unsigned seiSize;
	removeEmulationBytes(sei, sizeof sei, seiSize);

	unsigned j = 1; // skip the initial byte (forbidden_zero_bit; nal_ref_idc;
	        // nal_unit_type); we've already seen it
	while (j < seiSize) {
		unsigned payloadType = 0;
		do {
			payloadType += sei[j];
		} while (sei[j++] == 255 && j < seiSize);
		if (j >= seiSize)
			break;

		unsigned payloadSize = 0;
		do {
			payloadSize += sei[j];
		} while (sei[j++] == 255 && j < seiSize);
		if (j >= seiSize)
			break;
		j += payloadSize;
	}
}

void H264or5VideoStreamParser::flushInput()
{
	fHaveSeenFirstStartCode = False;
	fHaveSeenFirstByteOfNALUnit = False;

	StreamParserByPtr::flushInput();
}

#define NUM_NEXT_SLICE_HEADER_BYTES_TO_ANALYZE 12
int sps_size = 0;
int pps_addr = 0;
unsigned H264or5VideoStreamParser::parse()
{
	try {
		int frame_size = 0;
		int nalu_start_len = 0;

		if (fHNumber == 264) {
			if (fFrame_remain_cnt == 0) {
				fParam_data_ptr = (VIDEO_STREAM_DATA *)testNBytes(1);
				fParam_ptr = &(fParam_data_ptr->params);
				assert(fParam_ptr != NULL);
				assert(fParam_data_ptr != NULL);

				if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_P) {
					fFrame_remain_cnt = 1;
				} else {
					if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_SPS) {
						fFrame_remain_cnt = 3;
					} else if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_I) {
						fprintf(stderr, "seg[0] IDR,segcnt: %d\r\n", fParam_ptr->seg_cnt);
						fFrame_remain_cnt = 1;
					} else {
						fprintf(stderr, "Unknown seg[0] type:%d\r\n", fParam_ptr->seg[0].type);
						assert(0);
					}
				}

				if (fParam_data_ptr->sei_nalu_len != 0) {
					/*send SEI here and ret*/
					saveNBytes(fParam_data_ptr->p_sei_nalu + 4, fParam_data_ptr->sei_nalu_len - 4);
					frame_size = fParam_data_ptr->sei_nalu_len - 4;
					keepFrameBytes(frame_size + 4);
					fHaveSeenFirstByteOfNALUnit = False; // for the next NAL unit that we'll parse
#ifdef HC1703_1723_1753_1783S
					usingSource()->fPresentationTime.tv_sec = fParam_ptr->timestamp.tv_sec;
					usingSource()->fPresentationTime.tv_usec = fParam_ptr->timestamp.tv_nsec / 1000;
#else
					gettimeofdayMonotonic(&usingSource()->fPresentationTime, NULL);
#endif
					usingSource()->fPictureEndMarker = True;
					++usingSource()->fPictureCount;
					setParseState();
					return frame_size;
				}
			}
			if (fFrame_remain_cnt == 1) { /*I, P frame case*/
				if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_P) {
					for (unsigned int i = 0; i < fParam_ptr->seg_cnt;
					     i++) { /* merge all P seg together*/
						if (fParam_ptr->seg[i].size == 0) {
							continue;
						}
						/*P frame should start with 0x00 00 00 01*/
						if ((fParam_ptr->seg[i].uaddr[0] == 0x00) &&
						    (fParam_ptr->seg[i].uaddr[1] == 0x00) &&
						    (fParam_ptr->seg[i].uaddr[2] == 0x00) &&
						    (fParam_ptr->seg[i].uaddr[3] == 0x1) &&
						    (fParam_ptr->seg[i].uaddr[4] == 0x41)) {
							saveNBytes(fParam_ptr->seg[i].uaddr + 4,
							           fParam_ptr->seg[i].size - 4);
							frame_size += fParam_ptr->seg[i].size - 4;
							nalu_start_len = 4;
						} else {
							saveNBytes(fParam_ptr->seg[i].uaddr, fParam_ptr->seg[i].size);
							frame_size += fParam_ptr->seg[i].size;
						}
					}
				} else {
					int idrStartSeg = 2; /* seg[0] SPS*/
					if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_I) { /*FIXME*/
						idrStartSeg = 0;
						fprintf(stderr, "H264 seg[0] IDR, parse fFrame_remain_cnt = 1\r\n");
					}

					for (unsigned int i = 0; i < fParam_ptr->seg_cnt - idrStartSeg;
					     i++) { /* merge all I seg together*/
						if (fParam_ptr->seg[idrStartSeg + i].size == 0) {
							continue;
						}

						/*start of IDR may 0x00 00 00 01, or 0x00 00 01*/
						int NALU_start = 0; /*has no NALU start*/
						if ((fParam_ptr->seg[idrStartSeg + i].uaddr[0] == 0x00) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[1] == 0x00) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[2] == 0x01) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[3] == 0x65)) {
							NALU_start = 3;
							nalu_start_len = NALU_start;
						} else if ((fParam_ptr->seg[idrStartSeg + i].uaddr[0] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[1] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[2] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[3] == 0x01) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[4] == 0x65)) {
							NALU_start = 4;
							nalu_start_len = NALU_start;
						}

						if (NALU_start > 0) {
							saveNBytes(fParam_ptr->seg[idrStartSeg + i].uaddr + NALU_start,
							           fParam_ptr->seg[idrStartSeg + i].size - NALU_start);
							frame_size +=
							        fParam_ptr->seg[idrStartSeg + i].size - NALU_start;
						} else {
							saveNBytes(fParam_ptr->seg[idrStartSeg + i].uaddr,
							           fParam_ptr->seg[idrStartSeg + i].size);
							frame_size += fParam_ptr->seg[idrStartSeg + i].size;
						}
					}
				}

			} else if (fFrame_remain_cnt == 3) { /*SPS case*/
				if (fParam_ptr->seg[0].type != MPI_FRAME_TYPE_SPS) {
					fprintf(stderr, "[%s]may copy seg[0] not SPS, %d\r\n", __func__,
					        fParam_ptr->seg[0].type);
				}

				if ((fParam_ptr->seg[0].size - 4) > SPS_MAX_SIZE) {
					fprintf(stderr, "Invalid sps len: %d\r\n", fParam_ptr->seg[0].size);
					for (unsigned int i = 0; i < fParam_ptr->seg[0].size; i++) {
						printf("%0x ", fParam_ptr->seg[0].uaddr[i]);
					}
					printf("\r\n");
				}

				usingSource()->saveCopyOfSPS(fParam_ptr->seg[0].uaddr + 4, fParam_ptr->seg[0].size - 4);
				saveNBytes(fParam_ptr->seg[0].uaddr + 4, fParam_ptr->seg[0].size - 4);
				frame_size = fParam_ptr->seg[0].size - 4;
				nalu_start_len = 4;

			} else if (fFrame_remain_cnt == 2) { /*PPS case*/
				if (fParam_ptr->seg[1].type != MPI_FRAME_TYPE_PPS) {
					fprintf(stderr, "[%s]may copy seg[0] not PPS\r\n", __func__);
				}
				usingSource()->saveCopyOfPPS(fParam_ptr->seg[1].uaddr + 4, fParam_ptr->seg[1].size - 4);
				saveNBytes(fParam_ptr->seg[1].uaddr + 4, fParam_ptr->seg[1].size - 4);
				frame_size = fParam_ptr->seg[1].size - 4;
				nalu_start_len = 4;
			}
		} else if (fHNumber == 265) {
			if (fFrame_remain_cnt == 0) {
				fParam_data_ptr = (VIDEO_STREAM_DATA *)testNBytes(1);
				fParam_ptr = &(fParam_data_ptr->params);

				if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_P) {
					fFrame_remain_cnt = 1;
				} else {
					if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_SPS) {
						fFrame_remain_cnt = 4;
					} else if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_I) {
						fprintf(stderr, "seg[0] IDR,segcnt: %d\r\n", fParam_ptr->seg_cnt);
						fFrame_remain_cnt = 1;
					} else {
						fprintf(stderr, "Unknown seg[0] type:%d\r\n", fParam_ptr->seg[0].type);
						assert(0);
					}
				}

				if (fParam_data_ptr->sei_nalu_len != 0) {
					/*send SEI here and ret*/
					/*start of SEI always 0x00 00 00 01*/
					//printf("parse SEI\r\n");
					saveNBytes(fParam_data_ptr->p_sei_nalu + 4, fParam_data_ptr->sei_nalu_len - 4);
					frame_size = fParam_data_ptr->sei_nalu_len - 4;
					keepFrameBytes(frame_size + 4);
					fHaveSeenFirstByteOfNALUnit = False;

#ifdef HC1703_1723_1753_1783S
					usingSource()->fPresentationTime.tv_sec = fParam_ptr->timestamp.tv_sec;
					usingSource()->fPresentationTime.tv_usec = fParam_ptr->timestamp.tv_nsec / 1000;
#else
					gettimeofdayMonotonic(&usingSource()->fPresentationTime, NULL);
#endif
					usingSource()->fPictureEndMarker = True;
					++usingSource()->fPictureCount;

					setParseState();
					return frame_size;
				}
			}

			if (fFrame_remain_cnt == 1) { /*I, P frame case*/
				if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_P) {
					for (unsigned int i = 0; i < fParam_ptr->seg_cnt;
					     i++) { /* merge all P seg together*/
						if (fParam_ptr->seg[i].size == 0) {
							continue;
						}

						/*start of P frame may 0x00 00 00 01, or 0x00 00 01*/
						int NALU_start = 0; /*has no NALU start*/
						if ((fParam_ptr->seg[i].uaddr[0] == 0x00) &&
						    (fParam_ptr->seg[i].uaddr[1] == 0x00) &&
						    (fParam_ptr->seg[i].uaddr[2] == 0x01) &&
						    (fParam_ptr->seg[i].uaddr[3] == 0x02)) {
							NALU_start = 3;
							nalu_start_len = NALU_start;
						} else if ((fParam_ptr->seg[i].uaddr[0] == 0x00) &&
						           (fParam_ptr->seg[i].uaddr[1] == 0x00) &&
						           (fParam_ptr->seg[i].uaddr[2] == 0x00) &&
						           (fParam_ptr->seg[i].uaddr[3] == 0x01) &&
						           (fParam_ptr->seg[i].uaddr[4] == 0x02)) {
							NALU_start = 4;
							nalu_start_len = NALU_start;
						}

						if (NALU_start > 0) {
							saveNBytes(fParam_ptr->seg[i].uaddr + NALU_start,
							           fParam_ptr->seg[i].size - NALU_start);
							frame_size += fParam_ptr->seg[i].size - NALU_start;

						} else {
							saveNBytes(fParam_ptr->seg[i].uaddr, fParam_ptr->seg[i].size);
							frame_size += fParam_ptr->seg[i].size;
						}
					}
				} else { /* merge all I seg together*/
					int idrStartSeg = 1; /* seg[0] VPS + SPS + PPS*/
					if (fParam_ptr->seg[0].type == MPI_FRAME_TYPE_I) { /*FIXME*/
						fprintf(stderr, "H265 seg[0] IDR, parse fFrame_remain_cnt = 1\r\n");
						idrStartSeg = 0;
					}

					for (unsigned int i = 0; i < fParam_ptr->seg_cnt - idrStartSeg; i++) {
						if (fParam_ptr->seg[idrStartSeg + i].size == 0) {
							continue;
						}

						/*start of IDR may 0x00 00 00 01, or 0x00 00 01*/
						int NALU_start = 0; /*has no NALU start*/
						if ((fParam_ptr->seg[idrStartSeg + i].uaddr[0] == 0x00) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[1] == 0x00) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[2] == 0x01) &&
						    (fParam_ptr->seg[idrStartSeg + i].uaddr[3] == 0x26)) {
							NALU_start = 3;
							nalu_start_len = NALU_start;
						} else if ((fParam_ptr->seg[idrStartSeg + i].uaddr[0] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[1] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[2] == 0x00) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[3] == 0x01) &&
						           (fParam_ptr->seg[idrStartSeg + i].uaddr[4] == 0x26)) {
							NALU_start = 4;
							nalu_start_len = NALU_start;
						}

						if (NALU_start > 0) {
							saveNBytes(fParam_ptr->seg[idrStartSeg + i].uaddr + NALU_start,
							           fParam_ptr->seg[idrStartSeg + i].size - NALU_start);
							frame_size +=
							        fParam_ptr->seg[idrStartSeg + i].size - NALU_start;
						} else {
							saveNBytes(fParam_ptr->seg[idrStartSeg + i].uaddr,
							           fParam_ptr->seg[idrStartSeg + i].size);
							frame_size += fParam_ptr->seg[idrStartSeg + i].size;
						}
					}
				}
			} else if (fFrame_remain_cnt == 4) {
				/*start of VPS always 0x00 00 00 01*/
				usingSource()->saveCopyOfVPS(fParam_ptr->seg[0].uaddr + 4, 24);
				saveNBytes(fParam_ptr->seg[0].uaddr + 4, 24);
				frame_size = 24;
			} else if (fFrame_remain_cnt == 3) {
				if (fParam_ptr->seg[0].size != 75) {
					sps_size = fParam_ptr->seg[0].size - (4 + 24 + 4 + 4 + 7);
				} else {
					sps_size = 32;
				}

				pps_addr = 36 + sps_size;

				if (fParam_ptr->seg[0].type != MPI_FRAME_TYPE_SPS) {
					fprintf(stderr, "[%s]may copy seg[0] not SPS\r\n", __func__);
				}

				if (sps_size > SPS_MAX_SIZE) {
					fprintf(stderr, "Invalid sps len:\r\n");
					for (unsigned int i = 0; i < fParam_ptr->seg[0].size; i++) {
						printf("%0x ", fParam_ptr->seg[0].uaddr[i]);
					}
					printf("\r\n");
				}
				/*start of SPS always 0x00 00 00 01*/
				usingSource()->saveCopyOfSPS(fParam_ptr->seg[0].uaddr + 4 + 24 + 4, sps_size);
				saveNBytes(fParam_ptr->seg[0].uaddr + 4 + 24 + 4, sps_size);
				frame_size = sps_size;

			} else if (fFrame_remain_cnt == 2) {
				/*start of PPS always 0x00 00 00 01*/
				usingSource()->saveCopyOfPPS(fParam_ptr->seg[0].uaddr + pps_addr, 7);
				saveNBytes(fParam_ptr->seg[0].uaddr + pps_addr, 7);
				frame_size = 7;
			}
		}

		fFrame_remain_cnt -= 1;	
		if (fHNumber == 264) {
			keepFrameBytes(frame_size + 4);
		} else if (fHNumber == 265) {
			if (fFrame_remain_cnt == 0) {
				keepFrameBytes(frame_size + nalu_start_len);
			} else {
				keepFrameBytes(frame_size + 4);
			}
		}
		fHaveSeenFirstByteOfNALUnit = False; // for the next NAL unit that we'll parse
#ifdef HC1703_1723_1753_1783S
		usingSource()->fPresentationTime.tv_sec = fParam_ptr->timestamp.tv_sec;
		usingSource()->fPresentationTime.tv_usec = fParam_ptr->timestamp.tv_nsec / 1000;
#else
		gettimeofdayMonotonic(&usingSource()->fPresentationTime, NULL);
#endif
		usingSource()->fPictureEndMarker = True;
		++usingSource()->fPictureCount;

		setParseState();
		return frame_size;
	} catch (int /*e*/) {
		return 0; // the parsing got interrupted
	}
}

unsigned removeH264or5EmulationBytes(u_int8_t *to, unsigned toMaxSize, u_int8_t *from, unsigned fromSize)
{
	unsigned toSize = 0;
	unsigned i = 0;
	while (i < fromSize && toSize + 1 < toMaxSize) {
		if (i + 2 < fromSize && from[i] == 0 && from[i + 1] == 0 && from[i + 2] == 3) {
			to[toSize] = to[toSize + 1] = 0;
			toSize += 2;
			i += 3;
		} else {
			to[toSize] = from[i];
			toSize += 1;
			i += 1;
		}
	}

	return toSize;
}
