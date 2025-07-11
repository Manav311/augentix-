#ifndef EAIF_MODEL_H_
#define EAIF_MODEL_H_

#include <memory>
#include <vector>
#include "eaif_common.h"

#define CONFIG_NAME_LEN_MAX (32)
#define CONFIG_PATH_LEN_MAX (512)

namespace eaif
{
namespace engine
{
enum InferenceType { Classify, Detect, FaceReco, Infer };
enum EngineType { Armnn, TfLite, Other };
enum ActivationType { Sigmoid, Linear };
enum FaceInferType { FaceInferNone, Mtcnn, Facenet, FaceInferBoth };
enum ModelType {
	ModelTypeUnknown,
	ModelTypeNeuralClassify,
	ModelTypeC4,
	ModelTypeYolov4,
	ModelTypeYolov5,
	ModelTypeMtcnn,
	ModelTypeFaceNet,
	ModelTypeFaceReco
};

enum EngineType GetEngineType(const char *engine_str);
enum InferenceType GetInferenceType(const char *inference_str);
enum ActivationType GetActivationType(const char *activation_str);
enum FaceInferType GetFaceInferType(const char *face_infer_str);
enum ModelType GetModelType(const char *model_type_str);

} // namespace engine

} // namespace eaif

struct ModelConfig {
	int channels = 3;
	enum eaif::engine::InferenceType inference_type;
	enum eaif::engine::EngineType engine_type;
	enum eaif::engine::ActivationType activation_type;
	enum eaif::engine::ModelType model_type;

	std::string model_dir;
	std::string model_name;
	std::vector<std::string> model_path;
	std::string model_conf;
	Shape input_shape;
	int num_classes;
	int dataset = -1;
	std::vector<std::string> labels;
	std::vector<int> filter_cls;
	std::vector<int> filter_out_cls;
	std::vector<float> zeros;
	std::vector<float> stds;
	std::vector<float> conf_thresh;
	int cpu_infer_type;
	int topk = 0;
	int image_pre_resize = -1;
	int resize_keep_ratio = 0;

	float input_int8_scale = 60.501507;

	// detect
	float iou_thresh;

	// human_classify
	float conf_thresh_low;

	// c4
	Shape extend_size;

	// mtcnn
	int window_min_size = 40;
	float window_scale_factor = 0.709;
	std::vector<float> nms_internal_thresh;

	// facenet
	int align_margin = 32;

	// facereco
	int face_infer_type = 1;

	int Parse(const char *input_model_path, const char *config_name = EAIF_MODEL_CONFIG_NAME);
};

struct ModelInfo {
	const char *name;
	int index = -1;
	int dataset;
	int channels;
	int topk;
	const std::vector<std::string> *labels;
	const std::vector<int> *filter_cls;
	const std::vector<int> *filter_out_cls;
	enum eaif::engine::InferenceType inference_type;
};

struct ModelState {
	int infer_count;
	std::vector<int> module_count;
};

namespace eaif
{
class Engine;

class Model {
    public:
	Model()
	        : m_state{} {};
	friend class Engine;

	// member functions
	virtual ~Model(){};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) = 0;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &result, const ModelConfig &conf)
	{
		TRC_NOT_IMPL();
		return 0;
	};
	virtual int Classify(const void *Wimg, Classification &results, const ModelConfig &conf)
	{
		TRC_NOT_IMPL();
		return 0;
	};
	virtual int Detect(const void *Wimg, std::vector<Detection> &detections, const ModelConfig &conf)
	{
		TRC_NOT_IMPL();
		return 0;
	};
	virtual void SetDebug(int debug){};
	virtual void SetVerbose(int verbose){};
	int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                    std::vector<Classification> &results);
	int Classify(const void *Wimg, Classification &result);
	int Detect(const void *Wimg, std::vector<Detection> &detections);
	const ModelState &GetModelState(void) const;
	float GetResizeRatio(void) const
	{
		return m_pre_resize_ratio;
	};

	// member variable
	ModelConfig config;

    protected:
	void UpdateState(int num = 1);
	void CalcImgPreResizeRatio(const void *Wimg);

    private:
	void ClearModelState(void);
	float m_pre_resize_ratio = 1.0f;
	ModelState m_state;
};

class ModelFactory {
    public:
	static std::shared_ptr<Model> CreateModelInstance(const Engine &engine, const std::string &model_path);
};

} // namespace eaif

#endif /* !EAIF_MODEL_H_ */
