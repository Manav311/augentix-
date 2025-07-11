#include <cerrno>
#include <libgen.h>

#include "inf_model_internal.h"
#include "inf_classifier.h"
#include "inf_face.h"

#include "inf_log.h"
#include "inf_utils.h"

#ifndef USE_NCNN
#include "lite_classifier.h"
#include "lite_faceencode.h"
#include "lite_mtcnn.h"
#include "lite_scrfd.h"
#else
#include "ncnn_classifier.h"
#include "ncnn_scrfd.h"
#include "ncnn_faceencode.h"
#endif

#ifdef INF_ENABLE_REMOTE_CORE
#include "inf_remote_internal.h"
#endif

InfModel *InfModelFactory::Create(const char* model_config)
{
	InfModelInfo* config = new InfModelInfo{};
	ParseModelConfig(model_config, config);

	InfModel *model = nullptr;

	if (!config->model_paths.size) {
		inf_log_err("No model path provided in the config \"%s\"", config->config_path);
		ReleaseConfig(config);
		delete config;
		return nullptr;
	}

	switch (config->inference_type) {
		case InfRunClassify: {
			model = InfModelFactory::CreateClassify(config);
			break;
		} case InfRunDetect: {
#ifndef USE_NCNN
			model = InfModelFactory::CreateFaceDet(config);
			break;
#endif // USE_NCNN
		} case InfRunFaceEncode: {
#ifndef USE_NCNN
			model = InfModelFactory::CreateFaceEncode(config);
			break;
#endif // USE_NCNN
		} case InfRunFaceReco: {
#ifdef USE_NCNN
			inf_log_err("NCNN does not support the this model, please use TFLite.");
			return nullptr;
#else
			model = InfModelFactory::CreateFaceReco(config);
			break;
#endif // USE_NCNN
		} default: {
			inf_log_err("Cannot create \"%s\" model from config: \"%s\"!",
						GetInfTypeStr(config->inference_type), model_config);
				ReleaseConfig(config);
				delete config;
			break;
		}
	}
	if (!model) {
		return nullptr;
	}

	if (model->m_config->dtype != Inf8U && model->m_config->dtype != Inf8S) {
		inf_log_err("%s only supports %s, %s datatype!", __func__,
			GetDTypeString(Inf8U), GetDTypeString(Inf8S));
		delete model;
		model = nullptr;
	}

	return model;
}

InfModel *InfModelFactory::CreateClassify(InfModelInfo* info)
{
#ifdef INF_ENABLE_REMOTE_CORE
	LiteClassifierBase* classify = new RemoteLiteClassifier(info);
#else
#ifdef USE_NCNN
	NcnnClassifier* classify = new NcnnClassifier(info);
#else
	LiteClassifierBase* classify = new LiteClassifier(info);
#endif

#endif

	int ret = classify->LoadModels(info->model_paths.data[0]);
	if (ret) {
		delete classify;
		return nullptr;
	} else {
		return classify;
	}
}



/***@brief create face detection model
  **@param[in/out] ctx     face detection context info.
  **@param[in] config      face detection config path.
  **@retval 0              initialize model failure.
 **/
InfModel *InfModelFactory::CreateFaceDet(InfModelInfo* info)
{

	char model_dir_str[INF_MODEL_PATH_LEN]{};
	strncpy(model_dir_str, info->config_path, INF_MODEL_PATH_LEN-1);

	InfModel *model;

	char *model_dir = dirname(model_dir_str);

	if (!strcmp(info->model_name, INF_FACEDET_MTCNN_NAME)) {
#ifdef USE_NCNN
		inf_log_err("MTCNN does not suuport on ncnn! Please use TFLite");
		return nullptr;
#else
		LiteMtcnn *lmtcnn = new LiteMtcnn();
		lmtcnn->m_config = info;
		lmtcnn->SetNumThreads(info->num_threads);
		lmtcnn->SetVerbose(info->verbose);
		lmtcnn->SetDebug(info->debug);
		lmtcnn->SetupConfig(info);
		int ret = lmtcnn->LoadModels(model_dir, &info->model_paths);
		if (ret) {
			delete lmtcnn;
			inf_log_err("Cannot initalize MTCNN!");
			return nullptr;
		}
		model = lmtcnn;
#endif // USE_NCNN
	} else if (!strcmp(info->model_name, INF_FACEDET_SCRFD_NAME)) {
#ifdef INF_ENABLE_REMOTE_CORE
		LiteScrfdBase *lscrfd = new RemoteLiteScrfd(info);
#else
#ifdef USE_NCNN
		NcnnScrfd *lscrfd = new NcnnScrfd(info);
#else
		LiteScrfdBase *lscrfd = new LiteScrfd(info);
#endif // USE_NCNN
#endif
		int ret = lscrfd->LoadModels(model_dir, &info->model_paths);
		if (ret) {
			delete lscrfd;
			inf_log_warn("Cannot initalize SCRFD!");
			return nullptr;
		}
		model = lscrfd;
	} else {
		inf_log_err("Unknown Face detection model \"%s\"!", info->model_name);
		ReleaseConfig(info);
		delete info;
		return nullptr;
	}

	return model;
}


/***@brief init face_encode model
  **@param[in] config      face_encode config path.
  **@retval nullptr              Cannot create model!
 **/
InfModel *InfModelFactory::CreateFaceEncode(InfModelInfo* info)
{
	InfModel *model;

	char *model_dir = dirname(info->config_path);
	char model_path[256] = {};
	snprintf(model_path, 255, "%s/%s", model_dir, info->model_paths.data[0]);

#ifdef INF_ENABLE_REMOTE_CORE
	LiteFaceEncodeBase* face_encode = new RemoteLiteFaceEncode(info);
#else
#ifdef USE_NCNN
	NcnnFaceEncode* face_encode = new NcnnFaceEncode(info);
#else
	LiteFaceEncodeBase* face_encode = new LiteFaceEncode(info);
#endif // USE_NCNN
#endif
	int ret = face_encode->LoadModels(model_path);
	if (ret) {
		delete face_encode;
		return nullptr;
	} else {
		model = face_encode;
	}
	return model;
}

/***@brief init face_encode model
  **@param[in] config      face_encode config path.
  **@retval nullptr              Cannot create model!
 **/
InfModel *InfModelFactory::CreateFaceReco(InfModelInfo* info)
{
	InfFaceReco *face_reco = nullptr;
	InfFaceDetect *face_det = nullptr;
	InfFaceEncode *face_enc = nullptr;

	char model_config[256] = {};
	char *model_dir;

	if (info->model_paths.size != 2) {
		return nullptr;
	}

	model_dir = dirname(info->config_path);

    // scrfd
	if (strcmp(info->model_paths.data[0], "null")) {
		size_t size = sprintf(model_config, "%s/%s", model_dir, info->model_paths.data[0]);
		model_config[size] = 0;
		InfModel *model = InfModelFactory::Create(model_config);
		if (model) {
			face_det = dynamic_cast<InfFaceDetect*>(model);
		}
	}

    // faceencode
	if (strcmp(info->model_paths.data[1], "null")) {
		size_t size = sprintf(model_config, "%s/%s", model_dir, info->model_paths.data[1]);
		model_config[size] = 0;
		InfModel *model = InfModelFactory::Create(model_config);
		if (model) {
			face_enc = dynamic_cast<InfFaceEncode*>(model);
		}
	}

	if (face_det || face_enc) {
		face_reco = new InfFaceReco(info);
		face_reco->LoadModels(face_det, face_enc);
	}

	return face_reco;
}

/**
 * @brief Initialize model resource
 * @details
 * @param[in,out] ctx          empty model context to be initialized
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @see Inf_InitModel()
 */
extern "C" int Inf_InitModel(InfModelCtx *ctx, const char* model_config)
{
	retIfNull(ctx && model_config);

	InfModel *model = InfModelFactory::Create(model_config);
	if (!model) {
		return -1;
	}

	ctx->info = model->m_config;
	ctx->model = model;

	return 0;
}

/**
 * @brief release model resource
 * @details
 * @param[in] ctx              model context
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @see Inf_InitModel()
 */
extern "C" int Inf_ReleaseModel(InfModelCtx *ctx)
{
	retIfNull(ctx && ctx->model);

	delete reinterpret_cast<InfModel*>(ctx->model);
	ctx->model = nullptr;
	ctx->info = nullptr;
	return 0;
}

/**
 * @brief Setup model verbse, debug level, num thread
 * @details
 * @param[in] ctx                  model context
 * @param[in] verbose              verbose level
 * @param[in] debug                debug level
 * @param[in] num_thread           number of thread(experimental)
 * @retval 0                       success
 * @retval -EFAULT                 input pointers are null
 * @see Inf_ReleaseModel()
 */
extern "C" int Inf_Setup(InfModelCtx *ctx, int verbose, int debug, int num_thread)
{
	retIfNull(ctx && ctx->model);

	const InfModelInfo* config = ctx->info;
	InfModel* model = static_cast<InfModel*>(ctx->model);
	model->m_verbose = verbose;
	model->m_debug = debug;
	model->m_num_thread = num_thread;

	switch (config->inference_type) {
		case InfRunDetect: {
			if (!strcmp(config->model_name, INF_FACEDET_MTCNN_NAME)) {
#ifndef USE_NCNN
				LiteMtcnn *lmtcnn = dynamic_cast<LiteMtcnn*>(model);
				if (!lmtcnn) {
					inf_log_err("Cannot cast MTCNN from FD context. Please check input args!");
					return 0;
				}
				lmtcnn->SetVerbose(verbose);
				lmtcnn->SetDebug(debug);
				lmtcnn->SetModelThreads(num_thread);
#else
				inf_log_err("MTCNN does not suuport on ncnn! Please use TFLite");
				return 0;
#endif
			} else if (!strcmp(config->model_name, INF_FACEDET_SCRFD_NAME)) {
				SetupDebugTool(model->m_snapshot_prefix);
				if (debug)
					model->SetDebug(debug);
			}
			break;
		}
		case InfRunFaceEncode: {
			SetupDebugTool(model->m_snapshot_prefix);
			break;
		}
		case InfRunFaceReco: {
			SetupDebugTool(model->m_snapshot_prefix);
			if (debug)
				model->SetDebug(debug);
			break;
		}
		case InfRunClassify:{
			break;
		}
		default:{
			inf_log_err("Cannot setup \"%s\" model!", GetInfTypeStr(config->inference_type));
			break;
		}
	}
	return 0;
}

/**
 * @brief Release detections result.
 * @details
 * @param[in,out] result       Detections result
 * @retval 0                   success
 * @see Inf_ReleaseResult()
 */
extern "C" int Inf_ReleaseDetResult(InfDetList *result)
{
	retIfNull(result);

	if (!result->size) {
		return 0;
	}

	for (int i = 0; i < result->size; i++) {
		auto& obj = result->data[i];
		free(obj.cls);
		free(obj.prob);
	}

	free(result->data);
	result->data = nullptr;
	result->size = 0;

	return 0;
}

/**
 * @brief Release classification result.
 * @details
 * @param[in,out] result       classification result
 * @retval 0                   success
 * @see Inf_ReleaseDetResult()
 */
extern "C" int Inf_ReleaseResult(InfResultList *result)
{
	if (!result->size)
		return 0;

	for (int i = 0; i < result->size; i++) {
		auto& obj = result->data[i];
		free(obj.cls);
		free(obj.prob);
	}

	free(result->data);
	result->data = nullptr;
	result->size = 0;

	return 0;
}
