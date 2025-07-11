#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_data.h"
#include "eaif_model.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "detect_model.h"
#include "yolo_model.h"

#ifdef USE_TFLITE
#include "lite_yolo_model.h"
using namespace tflite;
#endif /* USE_TFLITE */

#ifdef USE_ARMNN
#include "armnn_yolo_model.h"
using namespace armnn;
#endif /* USE_ARMNN */

using namespace std;
using namespace eaif::image;

int Yolov5Model::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	int load_models = 0;
	if (config.engine_type == eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteYolov5Model *lyolov5 = new LiteYolov5Model();
		DetectModel* model_ = dynamic_cast<DetectModel*>(lyolov5);
		assert(model_);
		model = unique_ptr<DetectModel>(model_);
		load_models = 1;
#else /* !USE_TFLITE */
		eaif_warn("TFLITE is not enabled.\n");
#endif /* !USE_TFLITE */
	} else if (config.engine_type == eaif::engine::Armnn) {
#ifdef USE_ARMNN
		ArmnnYolov5Model *ayolov5 = new ArmnnYolov5Model();
		ayolov5->SetCpuType(config.cpu_infer_type);
		DetectModel* model_ = dynamic_cast<DetectModel*>(ayolov5);
		assert(model_);
		model = unique_ptr<DetectModel>(model_);
		load_models = 1;
#else /* !USE_ARMNN */
		eaif_warn("ARMNN is not enabled.\n");
#endif /* !USE_ARMNN */
	}
	if (load_models) {
		model->SetNumThreads(num_thread_);
		model->SetVerbose(m_verbose);
		model->SetDebug(m_debug);
		model->LoadModels(model_dir, model_path);
		if (model->m_type == Eaif8U) {
			eaif_warn("Not implemented yet!\n");
			return EAIF_FAILURE;
		}
		model->SetPostProcessMethod(11);
		return EAIF_SUCCESS;
	}
	return EAIF_FAILURE;
}

int Yolov5Model::Detect(const void* Wimg, vector<Detection>& detections, const ModelConfig& conf)
{
	TIMER(model->Detect(Wimg, detections, conf));
	const WImage *img = (const WImage*) Wimg;
	printf("total detections:%d [w:%d, h:%d]\n", (int)detections.size(), img->cols, img->rows);
	//for (auto det : detections) {
	//	for (unsigned int c = 0; c < det.classes.size(); ++c) {
	//		if (det.classes[c] != 0.0f) {
	//			printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.4f cls:%.4f label:%s\n",
	//			++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence, det.classes[c],
	//			data::getLabel(config.dataset, c));
	//		}
	//	}
	//};
	UpdateState();
	return EAIF_SUCCESS;
}