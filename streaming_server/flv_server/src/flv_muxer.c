#include <stdio.h>
#include <stdlib.h>

#include "flv_muxer.h"
#include "amf_byte_stream.h"
#include "log_define.h"

struct flv_tag {
	uint8_t type;
	uint8_t data_size[3];
	uint8_t timestamp[3];
	uint8_t timestamp_ex;
	uint8_t streamid[3];
} __attribute__((__packed__));

typedef struct flv_tag FlvTag;


int FLV_writeFLVHeader(MediaSrcInfo *info, bool is_have_audio, bool is_have_video)
{
	char flv_file_header[] = "FLV\x1\x5\0\0\0\x9\0\0\0\0"; // have audio and have video

	if (is_have_audio && is_have_video) {
		flv_file_header[4] = 0x05;
	} else if (is_have_audio && !is_have_video) {
		flv_file_header[4] = 0x04;
	} else if (!is_have_audio && is_have_video) {
		flv_file_header[4] = 0x01;
	} else {
		flv_file_header[4] = 0x00;
	}

	return info->fWriteFlv(flv_file_header, 13, info->output_fd);
}

/*
* @brief write video tag
* @param[in] buf:
* @param[in] buf_len: flv tag body size
* @param[in] timestamp: flv tag timestamp
*/

int write_tag_ts(MediaSrcInfo *info, uint32_t buf_len, uint32_t timestamp, int type)
{
	FlvTag flvtag;

	memset(&flvtag, 0, sizeof(flvtag));

	flvtag.type = type;
	ui24_to_bytes(flvtag.data_size, buf_len);
	flvtag.timestamp_ex = (uint8_t)((timestamp >> 24) & 0xff);
	flvtag.timestamp[0] = (uint8_t)((timestamp >> 16) & 0xff);
	flvtag.timestamp[1] = (uint8_t)((timestamp >> 8) & 0xff);
	flvtag.timestamp[2] = (uint8_t)((timestamp)&0xff);

	return info->fWriteFlv(&flvtag, sizeof(flvtag), info->output_fd);
}

int write_tag_src(MediaSrcInfo *info, uint8_t *buf, uint32_t buf_len)
{
	return info->fWriteFlv(buf, buf_len, info->output_fd);
}

int write_tag_prevsize(MediaSrcInfo *info, uint32_t buf_len)
{
	uint8_t prev_size[4] = { 0 };
	ui32_to_bytes(prev_size, buf_len + (uint32_t)sizeof(FlvTag));

	return info->fWriteFlv(prev_size, 4, info->output_fd);
}

static int getFlag(int is_keyframe, MPI_VENC_TYPE_E codec, uint8_t *flag)
{
	if (codec == MPI_VENC_TYPE_H264) {
		if (is_keyframe) {
			*flag = (1 << 4) | 0x07;
		} else {
			*flag = (2 << 4) | 0x07;
		}

	} else if (codec == MPI_VENC_TYPE_H265) {
		if (is_keyframe) {
			*flag = (1 << 4) | 0x0c;
		} else {
			*flag = (2 << 4) | 0x0c;
		}
	} else {
		flv_server_log_err("unknown codec: %d", (int)codec);
		return -EINVAL;
	}

	return 0;
}

/*
* @brief write header of video tag data part, fixed 5 bytes
*
*/
int FLV_writeAVCDataTag(MediaSrcInfo *info, const uint8_t *vps, uint32_t vps_len, const uint8_t *sps, uint32_t sps_len,
                        const uint8_t *pps, uint32_t pps_len, const uint8_t *data, uint32_t data_len,
                        uint32_t timestamp, int is_keyframe)
{
	uint8_t buf = 1;
	uint8_t buf32[4] = { 0 };
	uint8_t buf24[3] = { 0 };
	uint8_t flag;
	uint32_t sum_len = 5 + 4 + data_len;

	if (info->video_codec == MPI_VENC_TYPE_H265) {
		if (vps_len > 0) {
			sum_len += 4 + vps_len;
		}
	}

	if (sps_len > 0) {
		if (((sps[1] != info->video_profile) || (sps[3] != info->video_level)) &&
		    (info->video_codec == MPI_VENC_TYPE_H264)) {
			flv_server_log_err("Need resend seq");
			info->video_profile = sps[1];
			info->video_level = sps[3];
			return -EIO;
		}

		sum_len += 4 + sps_len;
	}

	if (pps_len > 0) {
		sum_len += 4 + pps_len;
	}

	int ret = 0;

	ret = write_tag_ts(info, sum_len, timestamp, FLV_TAG_TYPE_VIDEO);
	if (ret < 0) {
		return ret;
	}

	/* (FrameType << 4) | CodecID, 1 - keyframe, 2 - inner frame, 7 - AVC(h264), c (12) AVC(hevc)*/
	ret = getFlag(is_keyframe, info->video_codec, &flag);
	if (ret != 0) {
		return ret;
	}

	ret = write_tag_src(info, &flag, 1);
	if (ret < 0) {
		return ret;
	}
	/* AVCPacketType: 0x00 - AVC sequence header; 0x01 - AVC NALU */
	ret = write_tag_src(info, &buf, 1);
	if (ret < 0) {
		return ret;
	}

	/* composition time */
	ret = write_tag_src(info, &buf24[0], 3);
	if (ret < 0) {
		return ret;
	}

	if (info->video_codec == MPI_VENC_TYPE_H265) {
		if (vps_len > 0) {
			ui32_to_bytes(&buf32[0], vps_len);
			ret = write_tag_src(info, &buf32[0], 4);
			if (ret < 0) {
				return ret;
			}

			ret = write_tag_src(info, (uint8_t *)vps, vps_len);
			if (ret < 0) {
				return ret;
			}
		}
	}

	if (sps_len > 0) {
		ui32_to_bytes(&buf32[0], sps_len);
		ret = write_tag_src(info, &buf32[0], 4);
		if (ret < 0) {
			return ret;
		}

		ret = write_tag_src(info, (uint8_t *)sps, sps_len);
		if (ret < 0) {
			return ret;
		}
	}

	if (pps_len > 0) {
		ui32_to_bytes(&buf32[0], pps_len);
		ret = write_tag_src(info, &buf32[0], 4);
		if (ret < 0) {
			return ret;
		}

		ret = write_tag_src(info, (uint8_t *)pps, pps_len);
		if (ret < 0) {
			return ret;
		}
	}

	ui32_to_bytes(&buf32[0], data_len);
	ret = write_tag_src(info, &buf32[0], 4);
	if (ret < 0) {
		return ret;
	}
	ret = write_tag_src(info, (uint8_t *)data, data_len);
	if (ret < 0) {
		return ret;
	}

	ret = write_tag_prevsize(info, sum_len);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

/*
* @brief write AVC sequence header in header of video tag data part, the first video tag
*/
int FLV_writeAVCSeqHeaderTag(MediaSrcInfo *info, const uint8_t *vps, uint32_t vps_len, const uint8_t *sps,
                             uint32_t sps_len, const uint8_t *pps, uint32_t pps_len, uint32_t timestamp)
{
	uint8_t flag = 0;
	//uint32_t timestamp = 0;
	int ret = 0;
	uint8_t buf = 0;
	uint8_t buf16[2] = { 0 };
	uint8_t buf24[3] = { 0 };
	uint8_t buf32[4] = { 0 };

	const uint8_t k_tag_header_size = 5;
	const uint8_t k_hvcc_size = 23;
	const uint8_t k_nalu_size = 1;
	const uint8_t k_h265_nalu_num_size = 2;
	const uint8_t k_h265_nalu_len_size = 2;

	ret = getFlag(1, info->video_codec, &flag);
	if (ret != 0) {
		return ret;
	}

	if (info->video_codec == MPI_VENC_TYPE_H264) {
		ret = write_tag_ts(
		        info, sps_len + pps_len + 16 /*5 (frame type-composition time) + 5 seq head + 3 *2 (num +len)*/,
		        timestamp, FLV_TAG_TYPE_VIDEO);
		if (ret < 0) {
			return ret;
		}
	} else if (info->video_codec == MPI_VENC_TYPE_H265) {
		ret = write_tag_ts(info,
		                   k_tag_header_size + k_hvcc_size + k_nalu_size + k_h265_nalu_num_size +
		                           k_h265_nalu_len_size + vps_len + k_nalu_size + k_h265_nalu_num_size +
		                           k_h265_nalu_len_size + sps_len + k_nalu_size + k_h265_nalu_num_size +
		                           k_h265_nalu_len_size +
		                           pps_len /*5 (frame type-composition time) + 4*3 nalu[x] len*/,
		                   timestamp, FLV_TAG_TYPE_VIDEO);
		if (ret < 0) {
			return ret;
		}
	}

	ret = write_tag_src(info, &flag, 1);
	if (ret < 0) {
		return ret;
	}
	buf = 0;
	ret = write_tag_src(info, &buf, 1); // AVCPacketType: 0x00 - AVC sequence header; 0x01 - AVC NALU
	if (ret < 0) {
		return ret;
	}
	ret = write_tag_src(info, &buf24[0], 3); // composition time
	if (ret < 0) {
		return ret;
	}

	if (info->video_codec == MPI_VENC_TYPE_H264) {
		// generate AVCC with sps and pps, AVCDecoderConfigurationRecord
		buf = 1;
		ret = write_tag_src(info, &buf, 1); // configuration_version
		if (ret < 0) {
			return ret;
		}

		/* H264 SPS is AVCC format*/
		ui08_to_bytes(&buf, sps[1]); // AVCProfileIndication
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, sps[2]); // profile_compatibility
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, sps[3]); // AVCLevelIndication
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		info->video_profile = sps[1];
		info->video_level = sps[3];

		/* 6 bits reserved (111111) + 2 bits nal size length - 1
		 (Reserved << 2) | Nal_Size_length = (0x3F << 2) | 0x03 = 0xFF*/
		ui08_to_bytes(&buf, 0xff);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		/* 3 bits reserved (111) + 5 bits number of sps (00001)
		 (Reserved << 5) | Number_of_SPS = (0x07 << 5) | 0x01 = 0xe1 */
		ui08_to_bytes(&buf, 0xe1);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

	} else if (info->video_codec == MPI_VENC_TYPE_H265) {
		/* HEVC need is HVCC format*/

		/* HVCC init */
		/* ref:https://ffmpeg.org/doxygen/2.4/libavformat_2hevc_8c_source.html#l00787 */
		HEVCDecoderConfigurationRecord hvcc;
		memset(&hvcc, 0, sizeof(HEVCDecoderConfigurationRecord));
		hvcc.configuration_version = 1;

		/* The following fields have all their valid bits set by default,
		* the ProfileTierLevel parsing code will unset them when needed.*/
		hvcc.general_profile_compatibility_flags = 0x60000003;
		hvcc.general_constraint_indicator_flags = 0x000000000300;

		hvcc.min_spatial_segmentation_idc = 0;
		/*slice-based parallel decoding from PPS*/
		hvcc.parallelism_type = 1;
		hvcc.num_temporal_layers = 1;
		hvcc.temporal_id_nested = 1;
		hvcc.avg_framerate = 0;
		hvcc.const_framerate = 0;

		/* HVCC write*/
		hvcc.num_of_arrays = 3; /*Has VPS, SPS, PPS*/

		hvcc.chroma_format = 1; /* yuv 4:2:0*/
		hvcc.length_size_minus_one = 3; /*Has VPS, SPS, PPS*/

		u_int8_t profileTierLevelHeaderBytes[12]; // set/used for H.265 only
		memcpy(&profileTierLevelHeaderBytes[0], vps + 6, 12);

		/* from SDP generate method */
		hvcc.general_profile_space = profileTierLevelHeaderBytes[0] >> 6;
		hvcc.general_tier_flag = (profileTierLevelHeaderBytes[0] >> 5) & 0x1;

		hvcc.general_profile_idc = profileTierLevelHeaderBytes[0] & 0x1F;
		info->video_profile = hvcc.general_profile_idc;

		/* https://patents.google.com/patent/EP3200465A1 */
		ui32_to_bytes((uint8_t *)&hvcc.general_profile_compatibility_flags, profileTierLevelHeaderBytes[1]);
		ui48_to_bytes((uint8_t *)&hvcc.general_constraint_indicator_flags, profileTierLevelHeaderBytes[4]);
		

		hvcc.general_level_idc = profileTierLevelHeaderBytes[11];
		info->video_level = hvcc.general_level_idc;

		/* HVCC totally 23 bytes*/
		ui08_to_bytes(&buf, hvcc.configuration_version);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf,
		              hvcc.general_profile_space << 6 | hvcc.general_tier_flag << 5 | hvcc.general_profile_idc);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui32_to_bytes(&buf32[0], hvcc.general_profile_compatibility_flags);
		ret = write_tag_src(info, &buf32[0], 4);
		if (ret < 0) {
			return ret;
		}

		ui32_to_bytes(&buf32[0], hvcc.general_constraint_indicator_flags >> 16);
		ret = write_tag_src(info, &buf32[0], 4);
		if (ret < 0) {
			return ret;
		}

		ui16_to_bytes(&buf16[0], hvcc.general_constraint_indicator_flags >> 16);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.general_level_idc); // number of vps
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui16_to_bytes(&buf16[0], hvcc.min_spatial_segmentation_idc | 0xf000);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.parallelism_type | 0xfc);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.chroma_format | 0xfc);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.bitDepthLumaMinus8 | 0xf8);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.bitDepthChromaMinus8 | 0xf8);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui16_to_bytes(&buf16[0], hvcc.avg_framerate);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.const_framerate << 6 | hvcc.num_temporal_layers << 3 |
		                            hvcc.temporal_id_nested << 2 | hvcc.length_size_minus_one);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		ui08_to_bytes(&buf, hvcc.num_of_arrays);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		/* REPEATED of Arrays (VPS/SPS/PPS)*/

		/*NAL vps*/
		ui08_to_bytes(&buf, 0x20);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}
		/*number of vps*/
		ui16_to_bytes(&buf16[0], (uint16_t)0x01);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}
		/* len of vps */
		ui16_to_bytes(&buf16[0], (uint16_t)vps_len);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}

		ret = write_tag_src(info, (uint8_t *)vps, vps_len);
		if (ret < 0) {
			return ret;
		}

		/*NAL sps*/
		ui08_to_bytes(&buf, 0x21);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}

		/*number of sps*/
		ui16_to_bytes(&buf16[0], (uint16_t)0x01);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}
	}

	/* sps len */
	ui16_to_bytes(&buf16[0], (uint16_t)sps_len);
	ret = write_tag_src(info, &buf16[0], 2);
	if (ret < 0) {
		return ret;
	}
	ret = write_tag_src(info, (uint8_t *)sps, sps_len);
	if (ret < 0) {
		return ret;
	}

	// pps
	if (info->video_codec == MPI_VENC_TYPE_H264) {
		ui08_to_bytes(&buf, 1); // number of pps
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}
	} else if (info->video_codec == MPI_VENC_TYPE_H265) {
		/*NAL pps*/
		ui08_to_bytes(&buf, 0x22);
		ret = write_tag_src(info, &buf, 1);
		if (ret < 0) {
			return ret;
		}
		/*number of pps*/
		ui16_to_bytes(&buf16[0], (uint16_t)0x01);
		ret = write_tag_src(info, &buf16[0], 2);
		if (ret < 0) {
			return ret;
		}
	}

	/* len of pps */
	ui16_to_bytes(&buf16[0], (uint16_t)pps_len);
	ret = write_tag_src(info, &buf16[0], 2);
	if (ret < 0) {
		return ret;
	}

	ret = write_tag_src(info, (uint8_t *)pps, pps_len);
	if (ret < 0) {
		return ret;
	}

	if (info->video_codec == MPI_VENC_TYPE_H264) {
		ret = write_tag_prevsize(info, sps_len + pps_len + 16);
	} else if (info->video_codec == MPI_VENC_TYPE_H265) {
		ret = write_tag_prevsize(info, k_tag_header_size /*tag header*/ + k_hvcc_size /*HVCC*/ + k_nalu_size +
		                                       k_h265_nalu_num_size + k_h265_nalu_len_size + vps_len +
		                                       k_nalu_size + k_h265_nalu_num_size + k_h265_nalu_len_size +
		                                       sps_len + k_nalu_size + k_h265_nalu_num_size +
		                                       k_h265_nalu_len_size + pps_len);
	}


	return ret;
}

int FLV_writeAACSeqHeaderTag(MediaSrcInfo *info, int sample_rate, int channel, uint32_t timestamp)
{
	AGTX_UNUSED(sample_rate);
	AGTX_UNUSED(channel);

	/* AAC-LC u(4:7-4) */
	uint8_t flag = 0xa0 + 0x0f;
	uint8_t buf = 0;
	uint8_t aacSeqHeader[5] = { 0x15, 0x88, 0x56, 0xe5, 0x00 };
	int ret = 0;

	ret = write_tag_ts(info, 7, timestamp, FLV_TAG_TYPE_AUDIO);
	if (ret < 0) {
		return ret;
	}

	/* codec type AAC */
	ret = write_tag_src(info, &flag, 1);
	if (ret < 0) {
		return ret;
	}

	/* AACPacketType: 0x00 - AAC specific config; 0x01 - Frame data */
	ret = write_tag_src(info, &buf, 1); //
	if (ret < 0) {
		return ret;
	}

	ret = write_tag_src(info, (uint8_t *)&aacSeqHeader[0], 5);
	if (ret < 0) {
		return ret;
	}

	const uint8_t k_aac_es_offset = 7;

	ret = write_tag_prevsize(info, k_aac_es_offset);
	if (ret < 0) {
		return ret;
	}

	return ret;
}

int FLV_writeAACDataTag(MediaSrcInfo *info, const uint8_t *data, uint32_t data_len, uint32_t timestamp)
{
	/*AAC-LC u(4:7-4)*/
	uint8_t flag = 0xa0 + 0x0f;
	uint8_t buf = 1;
	int ret = 0;

	ret = write_tag_ts(info, data_len + 2, timestamp, FLV_TAG_TYPE_AUDIO);
	if (ret < 0) {
		return ret;
	}

	/* codec type AAC */
	ret = write_tag_src(info, &flag, 1);
	if (ret < 0) {
		return ret;
	}

	/* AACPacketType: 0x00 - AAC specific config; 0x01 - Frame data */
	ret = write_tag_src(info, &buf, 1);
	if (ret < 0) {
		return ret;
	}

	ret = write_tag_src(info, (uint8_t *)data, data_len);
	if (ret < 0) {
		return ret;
	}

	const uint8_t k_aac_timestamp_offset = 2;

	ret = write_tag_prevsize(info, data_len + k_aac_timestamp_offset);
	return ret;
}
