#ifndef EAIF_ENGINE_H_
#define EAIF_ENGINE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "eaif_common.h"
#include "eaif_model.h"

using StatePair = std::pair<const std::string &, const ModelState &>;

struct EngineConfig {
	std::string config_path;
	std::string template_path;
	std::string face_bin_path;
	int debug;
	int verbose;
	int num_thread;
};

namespace eaif
{
class Engine {
    public:
	Engine(){};
	~Engine()
	{
		Clear();
	};
	void Clear(void)
	{
		models.clear();
	};

	friend class eaif::ModelFactory;

	int Setup(std::string &engine_config);

	// Query Function
	int QueryModelDetail(const std::string &model_str, ModelConfig &conf) const;
	int QueryModelInfo(const std::string &model_str, ModelInfo &info) const;
	int QueryAvailModel(std::vector<ModelInfo> &infos) const;
	void QueryModelNames(eaif::engine::InferenceType type, std::vector<std::string> &str) const;
	int QueryFaceInfo(int model_index, std::vector<std::string> &face_list) const;

	// Inference Function
	int ClassifyObjList(int model_index, const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                    std::vector<Classification> &results);

	int Classify(int model_index, const void *Wimg, Classification &results);
	int Detect(int model_index, const void *Wimg, std::vector<Detection> &results);

	// Face Utility function
	int RegisterFace(int model_index, const void *Wimg, const std::string &face_name, int is_full_image = 1);
	int LoadFaceData(int model_index);
	int SaveFaceData(int model_index) const;

	// General Engine Utility
	void Update(void);
	void ClearModelStates(void);
	void GetModelStates(std::vector<StatePair> &states) const;
	int GetVerbose(void) const;

	std::string GetTemplatePath(void) const;

    private:
	std::vector<std::shared_ptr<eaif::Model> > models;
	EngineConfig m_config;
};

} // namespace eaif

#endif /* EAIF_ENGINE_H_ */
