#ifndef EAIF_ENGINE_DEMO_H_
#define EAIF_ENGINE_DEMO_H_

#include <vector>

struct ModelConfig;

namespace eaif
{
class EngineDemo {
    public:
	EngineDemo()
	        : m_verbose(1)
	        , m_range{ 0, 1 }
	        , m_iter(1)
	        , m_zero{ 127.5f, 127.5f, 127.5f }
	        , m_scale{ 0.0078125f, 0.0078125f, 0.0078125f }
	        , m_conf_thresh(0.4)
	        , m_iou_thresh(0.6)
	        , m_cpu_infer(0)
	        , m_debug(3)
	        , m_misc_flag(0)
	        , m_engine(0)
	        , m_nthread(1)
	        , m_channel(3){};
	~EngineDemo(){};
	int RunYolov5(const char *tflite_model, const char *img_file, const char *dataset);
	int RunYolov4(const char *tflite_model, const char *img_file, const char *dataset);
	int RunYolov5Eval(const char *tflite_model, const char *img_file, const char *dataset);
	int RunYolov4Eval(const char *tflite_model, const char *img_file, const char *dataset);
	int RunMinimum(const char *tflite_model, const char *img_file);
	int RunMtcnn(const char *img_file);
	int RunMtcnnFDDB(const char *data_dir_in, const char *dst_dir_in);
	int RunMtcnnWIDER(const char *data_dir_in, const char *dst_dir_in);
	int RunFacenetModel(const char *img_file, const char *dst_dir_in);
	int RunCascade(const char *img_file, const char *model_dir_);
	int RunHog(const char *img_file);
	int RunC4(const char *img_file, const char *model_dir_);
	int SetDim(int height, int width)
	{
		m_h = height, m_w = width;
		return 0;
	}
	void SetDebugMode(int val);
	void SetNumThreads();
	union {
		struct {
			int m_w;
			int m_h;
		};
		int m_dim[2];
	};
	int m_verbose;
	int m_range[2];
	int m_iter;
	float m_zero[3];
	float m_scale[3];

	// mtcnn
	std::vector<float> m_threshold;
	float m_scalefactor = 0.709;
	int m_minface = 40;

	// iou_thresh
	std::vector<float> m_nms_internal_thr = {0.5, 0.7, 0.7};

	// facenet

	// yolo
	float m_conf_thresh;
	float m_iou_thresh;
	int m_cpu_infer;
	int m_debug;
	float m_misc_flag;
	int m_engine;
	int m_nthread;
	int m_channel;
	const char *m_model_path = "./";
	std::vector<std::string> m_model_path_vec;
	void SetupConfig(ModelConfig &conf, const char *dataset, const VecStr *model_path = nullptr);

    private:
	int RunMinimumArmnn(const char *tflite_model, const char *img_file);
	int RunMinimumTflite(const char *tflite_model, const char *img_file);
};

} // namespace eaif

#endif /* EAIF_ENGINE_DEMO_H_ */
