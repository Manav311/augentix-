#ifndef CLASSIFY_MODEL_H_
#define CLASSIFY_MODEL_H_

#include <memory>
#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_model.h"
#include "eaif_utils.h"

class ClassifyModel {
    public:
	ClassifyModel()
	{
		m_verbose = 0;
		m_num_thread = -1;
	};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) = 0;
	virtual int Classify(const void *Wimg, Classification &result, const ModelConfig &conf) = 0;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &results, const ModelConfig &conf) = 0;
	virtual void SetDebug(int debug) = 0;
	virtual ~ClassifyModel(){};

	void SetNumThreads(int nthread)
	{
		m_num_thread = nthread;
	}
	void SetVerbose(int verbose)
	{
		m_verbose = verbose;
	}
	void SetCpuType(int cpu_type){}; // for armnn

	EaifDataType m_type; // model type (quant / floating inference)

    protected:
	int m_verbose;
	int m_debug;
	int m_num_thread;
};

namespace classify
{
void Sigmoid(float *output_addr, int num_classes);
void FastSigmoid(float *output_addr, int num_classes);
void PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, Classification &result);
void PostProcessConf(const float *output_addr, int num_classes, float conf_thresh, Classification &result);
void PostProcessTopK(const float *output_addr, int num_classes, int topk, Classification &result);

} // classify

#endif /* CLASSIFY_MODEL_H_ */
