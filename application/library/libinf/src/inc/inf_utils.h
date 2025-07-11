#ifndef INF_UTILS_H_
#define INF_UTILS_H_

#include <vector>

#include "mpi_dev.h"
#include "inf_types.h"
#include "inf_log.h"
#include "inf_image.h"

#define SNAPSHOT_FORMAT "%s/%s/%05d_%.3f.%s"
#define SNAPSHOT_FR_FORMAT "%s/%s/%05d_%.3f%s.%s"
#define SNAPSHOT_FD_FORMAT "%s/det/%05d_%02d.%s"

#define EPSILON (1.0e-7)

#define QuantConvert(qinfo, x) (static_cast<float>((static_cast<int>(x) - (qinfo).zero) * (qinfo).scale))

class VideoFrameGuard {
    public:
	VideoFrameGuard(MPI_WIN win, MPI_VIDEO_FRAME_INFO_S *frame)
	        : _win(win)
	        , _frame(frame)
	{
	}
	~VideoFrameGuard()
	{
		if (_frame != nullptr) {
			int err = MPI_DEV_releaseWinFrame(_win, _frame);
			if (err != MPI_SUCCESS) {
				inf_log_err("FAILED to release win frame, err=%d", err);
			}
		}
	}

    private:
	MPI_WIN _win;
	MPI_VIDEO_FRAME_INFO_S *_frame;
};

class ImageHolder {
    public:
	ImageHolder(InfImage *image)
	        : _img(image)
	{
	}
	~ImageHolder()
	{
		if (_img) {
			Inf_Imrelease(_img);
		}
	}

    private:
	InfImage *_img;
};

inline size_t GetImageElementSize(int dtype)
{
	int e_type = dtype & 0b111;
	if (e_type <= 1)
		return sizeof(uint8_t);
	else if (e_type <= 3)
		return sizeof(uint16_t);
	else if (e_type <= 5)
		return sizeof(uint32_t);
	else if (e_type <= 6)
		return sizeof(uint64_t);
	inf_log_warn("Unknown datatype %d!\n", e_type);
	return sizeof(uint8_t);
}

inline size_t GetImageSize(const InfImage &image)
{
	return image.w * image.h * image.c * GetImageElementSize(image.dtype);
}

inline const char *GetDTypeString(int type)
{
	switch (type) {
	case Inf8UC3:
	case Inf8U: {
		return "uint8";
	}
	case Inf8SC3:
	case Inf8S: {
		return "int8";
	}
	case Inf32F:
	case Inf32FC3: {
		return "float32";
	}
	default: {
		return "unknown type!!";
	}
	}
}

typedef float (*VecDistFuncPtr)(const std::vector<float> &, const std::vector<float> &);

struct DistMeasure {
	VecDistFuncPtr dist_funcptr;
	int ascend;
};

struct Shape {
	int w;
	int h;
};

struct Pads {
	int top;
	int right;
	int bot;
	int left;
};

struct Scaler {
	float w;
	float h;
};

#define Clamp(a, l, h) (((a) < (l)) ? (l) : (((a) > (h)) ? (h) : (a)))

const char *GetInfTypeStr(InfRunType inference_type);
int GetImageType(int dtype, uint32_t chn);
int GetDSize(int dtype);
int GetImageTypeChn(int dtype);
int GetOutputType(const char *output_type_str);
void PrintResult(int output_dim, float *output_buf);

int ParseModelConfig(const char *model_config_path, InfModelInfo *config);
// float QuantConvert(uint8_t in, int zero, float scale);
// float QuantConvertS(int8_t in, int zero, float scale);
void Sigmoid(float *output_addr, int num_classes);
void Softmax(float *output_addr, int num_classes);
void PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, const InfIntList *filter_cls,
                 const InfIntList *filter_out_cls, InfResult *result);
void ReleaseConfig(InfModelInfo *config);
void ReleaseIntList(InfIntList *list);
void ReleaseStrList(InfStrList *list);
void ReleaseFloatList(InfFloatList *list);
void ReleaseDetResult(InfDetResult *det);
void CopyDetResult(const InfDetResult *src, InfDetResult **dst);

unsigned char *LoadModelData(const char *model_path, int *model_data_len);

void ImsaveBin(const char *name, const InfImage *p_img);

void VecL2Norm(const std::vector<float>& src, std::vector<float>& dst);

float VecDistL2Norm(const std::vector<float>& v0, const std::vector<float>& v1);
float VecDistL1Norm(const std::vector<float>& v0, const std::vector<float>& v1);
float VecCosineSimilarity(const std::vector<float>& v0, const std::vector<float>& v1);
float NormVecDistL2Norm(const std::vector<float>& v0, const std::vector<float>& v1);

void Grouping(const MPI_IVA_OBJ_LIST_S* obj_list, MPI_RECT_POINT_S& roi);
void PadAndRescale(int pad, const Shape& img, const Shape& dst, const MPI_RECT_POINT_S& roi,
							MPI_RECT_POINT_S& dst_roi);
void RescaleFaceBox(int net_h, int net_w, MPI_RECT_POINT_S& roi);
void GetPadInfo(const InfImage* img, Pads& pad, MPI_RECT_POINT_S& roi);

int SetupDebugTool(char *prefix);

#endif /* INF_UTILS_H_ */
