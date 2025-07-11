#include <string>
#include "video_utils.h"

RequestFrame::RequestFrame(MPI_BCHN channel)
        : _result(0), _channel(channel)
{
	_result = MPI_getBitStreamV2(_channel, &_params, 1000);
}

RequestFrame::~RequestFrame()
{
	if (_result == MPI_SUCCESS) {
		MPI_releaseBitStreamV2(_channel, &_params);
	}
}

int queryEncoderType(int encoder, MPI_VENC_TYPE_E& encoder_type)
{
	int ret;
	MPI_VENC_ATTR_S attr;

	ret = MPI_ENC_getVencAttr(MPI_ENC_CHN(static_cast<UINT8>(encoder)), &attr);
	if (ret != MPI_SUCCESS) {
		return ret;
	}
	encoder_type = attr.type;

	return ret;
}

int queryEncoderFrameRate(int encoder, int& frame_rate)
{
	int ret;
	MPI_VENC_ATTR_S attr;

	ret = MPI_ENC_getVencAttr(MPI_ENC_CHN(static_cast<UINT8>(encoder)), &attr);
	if (ret != MPI_SUCCESS) {
		return ret;
	}
	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		frame_rate = attr.h264.rc.frm_rate_o;
		break;
	case MPI_VENC_TYPE_H265:
		frame_rate = attr.h265.rc.frm_rate_o;
		break;
	case MPI_VENC_TYPE_MJPEG:
		frame_rate = static_cast<int>(attr.mjpeg.rc.frm_rate_o & 0x7FFFFFFF);
		break;
	default:
		frame_rate = -1;
		break;
	}
	return ret;
}

AP4_Result decodeH264XPS(MPI_STREAM_PARAMS_V2_S& frame, AP4_AvcSequenceParameterSet& sps,
                         AP4_AvcPictureParameterSet& pps)
{
	if (frame.seg[0].type != MPI_FRAME_TYPE_SPS) {
		return AP4_FAILURE;
	}

	AP4_AvcFrameParser parser;
	MPI_BUF_SEG_S& sps_seg = frame.seg[0];
	/* 4 bytes NAL unit start code: 00 00 00 01 */
	AP4_CHECK(parser.ParseSPS(sps_seg.uaddr + 4, sps_seg.size - 4, sps));

	MPI_BUF_SEG_S& pps_seg = frame.seg[1];
	AP4_CHECK(parser.ParsePPS(pps_seg.uaddr + 4, pps_seg.size - 4, pps));
	return AP4_SUCCESS;
}

AP4_Result decodeH265XPS(MPI_STREAM_PARAMS_V2_S& frame, AP4_HevcVideoParameterSet& vps,
                         AP4_HevcSequenceParameterSet& sps, AP4_HevcPictureParameterSet& pps)
{
	if (frame.seg[0].type != MPI_FRAME_TYPE_SPS) {
		return AP4_FAILURE;
	}

	const char *start_code = "\x00\x00\x00\x01";
	MPI_BUF_SEG_S& vps_sps_pps = frame.seg[0];
	std::string nal_contents{ reinterpret_cast<const char *>(vps_sps_pps.uaddr), vps_sps_pps.size };
	size_t vps_offset = 4;
	std::string::size_type anchor = nal_contents.find(start_code, vps_offset, 4);
	if (anchor == std::string::npos) {
		return AP4_ERROR_INTERNAL;
	}
	size_t sps_offset = anchor + 4;
	anchor = nal_contents.find(start_code, sps_offset, 4);
	if (anchor == std::string::npos) {
		return AP4_ERROR_INTERNAL;
	}
	size_t pps_offset = anchor + 4;
	AP4_CHECK(vps.Parse(vps_sps_pps.uaddr + vps_offset, sps_offset - vps_offset - 4));
	AP4_CHECK(sps.Parse(vps_sps_pps.uaddr + sps_offset, pps_offset - sps_offset - 4));
	AP4_CHECK(pps.Parse(vps_sps_pps.uaddr + pps_offset, vps_sps_pps.size - pps_offset));
	return AP4_SUCCESS;
}
