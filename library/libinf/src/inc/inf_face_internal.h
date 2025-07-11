#ifndef INF_FACE_INTERNAL_H_
#define INF_FACE_INTERNAL_H_

#include <cstdio>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "inf_types.h"

#include "inf_utils.h"
#include "inf_model_internal.h"

#define NMS_UNION 1
#define NMS_MIN 2

struct ScaleWindow {
	int h;
	int w;
	float scale;
};

struct FaceLandmark {
	float x[5];
	float y[5];
};

struct FaceBox {
	float x0;
	float y0;
	float x1;
	float y1;

	/* confidence score */
	float score;

	/*regression scale */
	float regress[4];

	/* padding stuff*/
	float px0;
	float py0;
	float px1;
	float py1;

	FaceLandmark landmark;
};

struct Detection {
	int id;
	float confidence;
	std::vector<int> cls;
	std::vector<float> prob;
	MPI_RECT_POINT_S rect;
};

// TODO. use sqlite database
struct FaceData {
	int encode_dim = 128;
	int num_faces = 0;
	std::vector<std::string> faces;
	std::vector<std::vector<float> > encodes;

	void Reset(void);
	int CheckExist(const std::string &face);
	int Add(const std::string &face, const std::vector<float> &encode);
	int Delete(const std::string &face);
	int Write(const std::string &fname);
	int Read(const std::string &fname);
	void Copy(InfStrList *strs);
};

struct ScrfdPostProcessConfig {
	int num_anchors; // num_anchors
	int feature_stride; // feat_stride_fpn
	int number_output;
	float conf_thresh;
	float iou_thresh;
	Shape input;
};

class InfFaceDetect : public InfModel {
    public:
	InfFaceDetect() = default;
	virtual int LoadModels(const char* model_dir, const InfStrList* model_paths) = 0;
	virtual int FaceDetect(const InfImage* img, std::vector<FaceBox>& result) = 0;
	virtual int FaceDetect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list,
	                       std::vector<FaceBox>& result) = 0;
};

class InfFaceEncode : public InfModel {
    public:
	InfFaceEncode() = default;
	virtual int LoadModels(const char* model_path) = 0;
	virtual int EncodeFace(const InfImage* img, const MPI_RECT_POINT_S* roi, std::vector<float>& face) = 0;
	virtual int EncodeFace(std::vector<float>& face) = 0;
	virtual int SetFaceEncodeImage(const InfImage* img, const MPI_RECT_POINT_S* roi) = 0;
	const int *GetInputDim()
	{
		return m_input_dim;
	}
	int GetEncodeDim() const
	{
		return m_encode_dim;
	}

    protected:
	int m_input_dim[3]; /* h x w x c */
	int m_encode_dim;
	bool m_input_set = false;
};

class InfFaceReco : public InfModel {
	public:
	    explicit InfFaceReco(InfModelInfo *info);
	    ~InfFaceReco() override;
	    int LoadModels(const char *face_detect, const char *face_encode);
	    int LoadModels(InfFaceDetect *face_detect, InfFaceEncode *face_encode);

	    int FaceIdentify(const InfImage *img, InfDetList *det_list);

	    int Detect(const InfImage *img, InfDetList *result) override;
	    int Detect(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *result) override;
	    //int FaceReco(const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result);
	    int Register(const InfImage *img, const std::string &face_name);
	    int Register(const InfImage *img, const std::string &face_name, const MPI_RECT_POINT_S *roi,
	                 bool det = FALSE);
	    void SetDebug(int debug) override;
	    void SetFacePath(const char *face_data_path, const char *face_img_path);
	    int LoadFaceData();
	    int SaveFaceData();

	    // face detection based on det_list[0] and set encode image
	    int FaceRecoStageOne(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *det_list);
	    // invoke face encode and matching only
	    int FaceRecoStageTwo(InfDetList *det_list);

	    int m_stage_id;

	    FaceData m_face_data;
	    std::string m_face_data_path;
	    std::string m_face_img_path;

	    std::unique_ptr<InfFaceEncode> m_face_encode = nullptr;
	    std::unique_ptr<InfFaceDetect> m_face_detect = nullptr;

	    // InfFaceEncode* m_face_encode = nullptr;
	    // InfFaceDetect* m_face_detect = nullptr;

	private:
	    int GetSortedFaceDist(const FaceData &face_data, const std::vector<float> &face_encode,
	                          InfDetResult &result, DistMeasure dist_method);
	    /* if roi is not fix the input ratio, it rescales the roi
		 * example: input = 96x96, ar = 1:1
		 *          roi w = 90, h=112 => rescale => w = 112, h = 112
		 */
	    const int m_scale_input_box = 1;
};

void NmsBoxes(std::vector<FaceBox> &input, float threshold, int type, std::vector<FaceBox> &output);
void RegressBoxes(std::vector<FaceBox> &rects);
void ProcessBoxes(std::vector<FaceBox> &input, int img_h, int img_w, std::vector<FaceBox> &rects, float nms_th);
void SquareBoxes(std::vector<FaceBox> &rects);
void Padding(int img_h, int img_w, std::vector<Detection> &rects);

void GenerateBoundingBox(const QuantInfo &info_conf, const int8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const int8_t *reg_data, float scale, float threshold, int feature_h,
                         int feature_w, std::vector<FaceBox> &output, bool transposed);
void GenerateBoundingBox(const QuantInfo &info_conf, const uint8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const uint8_t *reg_data, float scale, float threshold,
                         int feature_h, int feature_w, std::vector<FaceBox> &output, bool transposed);

void CalPyramidList(int height, int width, int min_size, float factor, std::vector<ScaleWindow> &list);
void TransformResult(const std::vector<FaceBox> &face_list, InfDetList *detections, float resize_ratio = 1.0f, int cls_num = 2);

template<typename Tbuffer>
int ScrfdPostProcess(const Tbuffer* pconf, const Tbuffer* preg,
	const QuantInfo& qconf, const QuantInfo& qreg, const ScrfdPostProcessConfig& conf,
	std::vector<FaceBox>& face_list);

#ifdef USE_NCNN
int ScrfdPostProcess(const float* pconf, const float* preg,
	const ScrfdPostProcessConfig& conf,
	std::vector<FaceBox>& face_list);

#endif /* USE_NCNN */

#endif /* INF_FACE_INTERNAL_H_ */
