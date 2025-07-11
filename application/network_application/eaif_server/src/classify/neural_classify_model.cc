#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_data.h"
#include "eaif_model.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "classify_model.h"
#include "neural_classify_model.h"

#ifdef USE_TFLITE
#include "lite_classify_model.h"
using namespace tflite;
#endif /* USE_TFLITE */

#ifdef USE_ARMNN
#include "armnn_classify_model.h"
using namespace armnn;
#endif /* USE_ARMNN */

using namespace eaif::image;
using namespace std;

int NeuralClassifyModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	int LoadModels = 0;
	if (config.engine_type == eaif::engine::TfLite) {
#ifdef USE_TFLITE
		LiteClassifyModel *lclassifer = new LiteClassifyModel();
		ClassifyModel* model_ = dynamic_cast<ClassifyModel*>(lclassifer);
		assert(model_);
		model = unique_ptr<ClassifyModel>(model_);
		LoadModels = 1;
#else
		eaif_warn("TFLITE is not enabled.\n");
#endif
	} else if (config.engine_type == eaif::engine::Armnn) {
#ifdef USE_ARMNN
		ArmnnClassifyModel *aclassifer = new ArmnnClassifyModel();
		aclassifer->SetCpuType(config.cpu_infer_type);
		ClassifyModel* model_ = dynamic_cast<ClassifyModel*>(aclassifer);
		assert(model_);
		model = unique_ptr<ClassifyModel>(model_);
		LoadModels = 1;
#else
		eaif_warn("ARMNN is not enabled.\n");
#endif
	}
	if (LoadModels) {
		model->SetNumThreads(num_thread_);
		model->SetVerbose(m_verbose);
		model->SetDebug(m_debug);
		model->LoadModels(model_dir, model_path);
		if (model->m_type == Eaif8S && config.engine_type == eaif::engine::Armnn) {
			eaif_warn("8S model type is not implemented yet!\n");
			return EAIF_FAILURE;
		}
		return EAIF_SUCCESS;
	}
	return EAIF_FAILURE;
}

int NeuralClassifyModel::Classify(const void* Wimg, Classification &results, const ModelConfig& conf)
{

	TIMER(model->Classify(Wimg, results, conf));
	UpdateState();
	return EAIF_SUCCESS;
}

int NeuralClassifyModel::ClassifyObjList(const void *Wimg, const vector<ObjectAttr> &obj_list,
                                           vector<Classification> &results, const ModelConfig& conf)
{
	UpdateState(obj_list.size());
	TIMER(model->ClassifyObjList(Wimg, obj_list, results, conf));
	
	return EAIF_SUCCESS;
}