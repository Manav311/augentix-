#ifndef MP4_RECORDER_VIDEO_UTILS_H_
#define MP4_RECORDER_VIDEO_UTILS_H_

#include "mpi_index.h"
#include "mpi_enc.h"
#include "Ap4Types.h"
#include "Ap4AvcParser.h"
#include "Ap4HevcParser.h"

class RequestFrame {
    public:
	RequestFrame(MPI_BCHN channel);
	~RequestFrame();

	MPI_STREAM_PARAMS_V2_S &useFrame()
	{
		return _params;
	}

	bool isReady() const
	{
		return _result == MPI_SUCCESS;
	}

    private:
	int _result;
	MPI_BCHN _channel;
	MPI_STREAM_PARAMS_V2_S _params;
};

int queryEncoderType(int encoder, MPI_VENC_TYPE_E &encoder_type);
int queryEncoderFrameRate(int encoder, int &frame_rate);

AP4_Result decodeH264XPS(MPI_STREAM_PARAMS_V2_S &frame, AP4_AvcSequenceParameterSet &sps,
                         AP4_AvcPictureParameterSet &pps);

AP4_Result decodeH265XPS(MPI_STREAM_PARAMS_V2_S &frame, AP4_HevcVideoParameterSet &vps,
                         AP4_HevcSequenceParameterSet &sps, AP4_HevcPictureParameterSet &pps);

#endif //MP4_RECORDER_VIDEO_UTILS_H_
