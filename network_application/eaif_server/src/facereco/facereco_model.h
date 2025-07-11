#ifndef MTCNN_MODEL_H_
#define MTCNN_MODEL_H_

#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_engine.h"
#include "eaif_model.h"

#include "facenet_model.h"
#include "mtcnn.h"

class FacerecoModel : public eaif::Model {
    public:
	FacerecoModel() = default;

	~FacerecoModel(void) override{};

	int LoadModels(const std::string &model_dir, const VecStr &model_path) override;
	int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) override;
	int Register(const void *img, const std::string &face_name);
	int Register(const void *img, const std::string &face_name, const Detection &detection,
	             const ModelConfig &conf);
	int FaceDetect(const void *img, std::vector<FaceBox<float> > &face_list);
	int VerifyFace(const void *img, std::vector<Detection> &detections, const FaceData<float> &face_data,
	               const ModelConfig &conf);

	void SetDebug(int debug) override
	{
		m_debug = debug;
		if (m_mtcnn_model)
			m_mtcnn_model->SetDebug(debug);
		if (m_facenet_model)
			m_facenet_model->SetDebug(debug);
	};

	void SetVerbose(int verbose) override
	{
		m_verbose = verbose;
		if (m_mtcnn_model)
			m_mtcnn_model->SetVerbose(verbose);
		if (m_facenet_model)
			m_facenet_model->SetVerbose(verbose);
	};

	int SaveFaceData(const std::string &fname)
	{
		return m_face_data.write(fname);
	}

	int LoadFaceData(const std::string &fname)
	{
		m_face_data.reset();
		return m_face_data.read(fname);
	}

	void QueryFaceInfo(VecStr &face_info)
	{
		for (auto &face_name : m_face_data.faces)
			face_info.push_back(face_name);
	}

	void SetNumThreads(int nthread)
	{
		num_thread_ = nthread;
	};

	void SetInferType(enum eaif::engine::FaceInferType infer_type_val)
	{
		infer_type = infer_type_val;
	}

	const VecStr *GetFaceNameList(void)
	{
		return &m_face_data.faces;
	};
	const ModelConfig &GetMtcnnConfig(void)
	{
		return m_mtcnn_config;
	};
	const ModelConfig &GetFacenetConfig(void)
	{
		return m_facenet_config;
	};

    private:
	int GetSortedFaceDist(const FaceData<float> &face_data, const std::vector<float> face_encode, Detection &result,
	                      DistMeasure dist_funcptr);

    protected:
	FaceData<float> m_face_data;

    private:
	std::unique_ptr<Mtcnn> m_mtcnn_model = nullptr;
	std::unique_ptr<FacenetModel> m_facenet_model = nullptr;
	ModelConfig m_mtcnn_config;
	ModelConfig m_facenet_config;

	int m_verbose;
	int m_debug;
	int num_thread_;
	enum eaif::engine::FaceInferType infer_type;
};

#endif /* !MTCNN_MODEL_H_ */
