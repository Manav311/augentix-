#include <algorithm>
#include <string>
#include <vector>

#include <math.h>

#include "eaif_image.h"
#include "eaif_model.h"

#include "armnn_facenet_model.h"
#include "lite_facenet_model.h"
#include "facenet_model.h"

#include "armnn_mtcnn.h"
#include "lite_mtcnn.h"
#include "mtcnn.h"

#include "facereco_common.h"
#include "facereco_model.h"

using namespace std;
using namespace eaif::image;

int FacerecoModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{

	if (config.face_infer_type == eaif::engine::FaceInferNone)
		return EAIF_FAILURE;

	if (model_path.size() != 2) { // mtcnn and facenet
		eaif_err("Number of Model_paths should equal to 2\n");
		return EAIF_FAILURE;
	}

	Mtcnn *mtcnn_model = nullptr;
	FacenetModel *facenet_model = nullptr;

	if (config.face_infer_type == eaif::engine::Mtcnn ||
		config.face_infer_type == eaif::engine::FaceInferBoth) {

		m_mtcnn_config.Parse(model_dir.c_str(), model_path[0].c_str());

		if (m_mtcnn_config.engine_type == eaif::engine::TfLite) {
#ifdef USE_TFLITE
			LiteMtcnn *lmtcnn = new LiteMtcnn();
			lmtcnn->SetNumThreads(num_thread_);
			lmtcnn->SetVerbose(m_verbose);
			lmtcnn->SetDebug(m_debug);
			lmtcnn->LoadModels(model_dir, m_mtcnn_config.model_path);
			lmtcnn->SetupConfig(m_mtcnn_config);
			mtcnn_model = dynamic_cast<Mtcnn*>(lmtcnn);
			if (!mtcnn_model) {
				delete lmtcnn;
				eaif_warn("Cannot upcast mtcnn!\n");
				return EAIF_FAILURE;
			}
#else /* !USE_TFLITE */
			eaif_warn("TFLITE is not enabled");
#endif /* !USE_TFLITE */
		} else if (m_mtcnn_config.engine_type == eaif::engine::Armnn) {
#ifdef USE_ARMNN
			ArmnnMtcnn *amtcnn = new ArmnnMtcnn();
			amtcnn->SetNumThreads(num_thread_);
			amtcnn->SetVerbose(m_verbose);
			amtcnn->SetDebug(m_debug);
			amtcnn->LoadModels(model_dir, m_mtcnn_config.model_path);
			amtcnn->SetupConfig(m_mtcnn_config);
			amtcnn->SetCpuType(m_mtcnn_config.cpu_infer_type);
			if (amtcnn->GetModelType() == Eaif8U) {
				eaif_warn("MTCNN Eaif datatype for 8U is not implemented yet!\n");
				delete amtcnn;
				return EAIF_FAILURE;
			}
			if (amtcnn) {
				mtcnn_model = dynamic_cast<Mtcnn*>(amtcnn);
				if (!mtcnn_model) {
					delete amtcnn;
					eaif_warn("Cannot upcast mtcnn!\n");
					return EAIF_FAILURE;
				}
			}
#else /* !USE_ARMNN */
			eaif_warn("ARMNN is not enabled");
#endif /* USE_ARMNN */
		}

		if (mtcnn_model)
			m_mtcnn_model = unique_ptr<Mtcnn>(mtcnn_model);
	}

	if (config.face_infer_type == eaif::engine::Facenet ||
		config.face_infer_type == eaif::engine::FaceInferBoth) {

		m_facenet_config.Parse(model_dir.c_str(), model_path[1].c_str());

		if (m_facenet_config.engine_type == eaif::engine::TfLite) {
#ifdef USE_TFLITE
			LiteFacenetModel *lfacenet = new LiteFacenetModel();
			lfacenet->SetNumThreads(num_thread_);
			lfacenet->SetVerbose(m_verbose);
			lfacenet->SetDebug(m_debug);
			lfacenet->LoadModels(model_dir, m_facenet_config.model_path);
			facenet_model = dynamic_cast<FacenetModel*>(lfacenet);
			if (!facenet_model) {
				delete lfacenet;
				eaif_warn("Cannot upcast facenet model!\n");
				return EAIF_FAILURE;
			}
#else /* !USE_TFLITE */
			eaif_warn("TFLITE is not enabled!\n");
#endif /* !USE_TFLITE */
		} else if (m_facenet_config.engine_type == eaif::engine::Armnn) {
#ifdef USE_ARMNN
			ArmnnFacenetModel *afacenet = new ArmnnFacenetModel();
			afacenet->SetNumThreads(num_thread_);
			afacenet->SetVerbose(m_verbose);
			afacenet->SetDebug(m_debug);
			afacenet->LoadModels(model_dir, m_facenet_config.model_path);
			if (afacenet->GetModelType() == Eaif8S) {
				eaif_warn("Facenet 8S inference is not implemented yet!\n");
				delete afacenet;
				return EAIF_FAILURE;
			}
			if (afacenet) {
				facenet_model = dynamic_cast<FacenetModel*>(afacenet);
				if (!facenet_model) {
					delete afacenet;
					eaif_warn("Cannot upcast facenet model!\n");
					return EAIF_FAILURE;
				}
			}
#else /* !USE_ARMNN */
			eaif_warn("ARMNN is not enabled!\n");
#endif /* !USE_ARMNN */
		}

		if (facenet_model) {
			m_facenet_model = unique_ptr<FacenetModel>(facenet_model);
			auto& face_data = m_face_data;
			face_data.num_faces = 1;
			face_data.encode_dim = m_facenet_model->encode_dim_;
			face_data.faces.push_back("unknown");
			face_data.encodes.resize(1);
			face_data.encodes[0].resize(face_data.encode_dim, 0.0f);
		}
	}

	return EAIF_SUCCESS;
}

int FacerecoModel::Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf)
{
	if (conf.face_infer_type == eaif::engine::FaceInferNone)
		return EAIF_SUCCESS;

	const WImage *wimg = (const WImage*)img;
	WImage dst_img;

	if (config.image_pre_resize > 0) {
		CalcImgPreResizeRatio(img);
		float resize_ratio = GetResizeRatio();
		COND_TIMER_FUNC(
			m_verbose,
			"Pre Image resize",
			Imresize(*wimg, dst_img, wimg->cols * resize_ratio, wimg->rows * resize_ratio)
			);
		wimg = &dst_img;
	}

	if (conf.face_infer_type == eaif::engine::Mtcnn ||
		conf.face_infer_type == eaif::engine::FaceInferBoth) {
		EaifDataType dtype = m_mtcnn_model->GetModelType();
		if (dtype == Eaif32F || dtype == Eaif8U || dtype == Eaif8S) {
			vector<FaceBox<float> > face_list;
			m_mtcnn_model->FaceDetect((const void*)wimg, face_list, m_mtcnn_config);
			TransformResult(face_list, detections, 1.0f / GetResizeRatio());
		} else {
			return -1;
		}
	}

	if (conf.face_infer_type == eaif::engine::FaceInferBoth) {
		VerifyFace(img, detections, m_face_data, m_facenet_config);
	}
	UpdateState();
	return EAIF_SUCCESS;
}

int FacerecoModel::GetSortedFaceDist(const FaceData<float> &face_data, const std::vector<float> face_encode, Detection &result, DistMeasure dist_method)
{
	result.prob.resize(face_data.num_faces);
	result.cls.resize(face_data.num_faces);

	for (int i = 0; i < face_data.num_faces; ++i) {
		result.cls[i] = i;
		result.prob[i] = dist_method.dist_funcptr(face_data.encodes[i], face_encode);
		if (m_verbose)
			eaif_info_h("face-data:name:%12s : %.4f\n", face_data.faces[i].c_str(), result.prob[i]);
	}

	// sort in ascending order
	if (dist_method.ascend) {
		sort(result.cls.begin(), result.cls.end(), [&](const int& cls0, const int& cls1) {
			return result.prob[cls0] < result.prob[cls1];
		});
	} else {
		sort(result.cls.begin(), result.cls.end(), [&](const int& cls0, const int& cls1) {
			return result.prob[cls0] > result.prob[cls1];
		});
	}

	result.confidence = result.prob[result.cls[0]];
	return EAIF_SUCCESS;
}

int FacerecoModel::VerifyFace(const void *img, std::vector<Detection> &detections, const FaceData<float> &face_data, const ModelConfig &conf)
{
	for (auto& det : detections) {
		if (m_facenet_model->GetModelType() == Eaif32F || m_facenet_model->GetModelType() == Eaif8U ||
			m_facenet_model->GetModelType() == Eaif8S) {
			vector<float> face_encode(m_facenet_model->encode_dim_);
			m_facenet_model->EncodeFace(img, det, face_encode, conf);
			GetSortedFaceDist(face_data, face_encode, det,
				(DistMeasure){
					&VecCosineSimilarity, // dist func,
					0 // descending
				});
			if (det.prob[det.cls[0]] < conf.conf_thresh[0]) {
				det.cls[0] = 0;
				det.confidence = 0.0;
			}
		} else {
			eaif_warn("%s %s inference is not implemented yet!\n", __func__, eaif_GetDTypeString(m_facenet_model->GetModelType()));
			return -1;
		}
	}
	return EAIF_SUCCESS;
}

int FacerecoModel::Register(const void *img, const string &face_name, const Detection &detection, const ModelConfig &conf)
{
	vector<float> face_data(m_facenet_model->encode_dim_);
	if ((m_facenet_model->EncodeFace(img, detection, face_data, conf)) == -1)
		return EAIF_FAILURE;
	m_face_data.encodes.push_back(face_data);
	m_face_data.faces.push_back(face_name);
	m_face_data.encode_dim = face_data.size();
	m_face_data.num_faces++;

	return EAIF_SUCCESS;
}

int FacerecoModel::Register(const void *img, const string &face_name)
{

	if (config.face_infer_type != eaif::engine::FaceInferBoth) {
		eaif_warn("Config type should be Both");
		return EAIF_SUCCESS;
	}

	const WImage *wimg = (const WImage*)img;
	WImage dst_img;

	if (config.image_pre_resize > 0) {
		CalcImgPreResizeRatio(img);
		float resize_ratio = GetResizeRatio();
		COND_TIMER_FUNC(
			m_verbose,
			"Pre Image resize",
			Imresize(*wimg, dst_img, wimg->cols * resize_ratio, wimg->rows * resize_ratio)
			);
		wimg = &dst_img;
	}

	vector<Detection> detections;

	if (config.face_infer_type == eaif::engine::Mtcnn ||
		config.face_infer_type == eaif::engine::FaceInferBoth) {
		EaifDataType dtype = m_mtcnn_model->GetModelType();
		if (dtype == Eaif32F || dtype == Eaif8U || dtype == Eaif8S) {
			vector<FaceBox<float> > face_list;
			m_mtcnn_model->FaceDetect((const void*)wimg, face_list, m_mtcnn_config);
			TransformResult(face_list, detections, 1.0f / GetResizeRatio());
		}
	}

	if (detections.size() != 1) {
		eaif_warn("Cannot register face as face detected in the image is not equal to one!\n");
		return -1;
	}

	if (config.face_infer_type == eaif::engine::FaceInferBoth) {
		Register(img, face_name, detections[0], m_facenet_config);
	}

	eaif_info_h("Register face for %s!\n", face_name.c_str());

	return EAIF_SUCCESS;
}

int FacerecoModel::FaceDetect(const void *img, std::vector<FaceBox<float> > &face_list)
{
	if (config.face_infer_type == eaif::engine::FaceInferNone)
		return EAIF_SUCCESS;
	if (config.face_infer_type == eaif::engine::Mtcnn ||
		config.face_infer_type == eaif::engine::FaceInferBoth) {
		if (m_mtcnn_model->GetModelType() == Eaif8U) {
			vector<FaceBox<int> > iface_list;
			m_mtcnn_model->FaceDetect(img, iface_list, m_mtcnn_config);
		} else {
			m_mtcnn_model->FaceDetect(img, face_list, m_mtcnn_config);
		}
	}
	return EAIF_SUCCESS;
}
