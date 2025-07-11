//#ifndef USE_NCNN
#include "inf_face.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <libgen.h>

#include <algorithm>

#include "inf_image.h"
#include "inf_face_internal.h"
#include "inf_utils.h"
#include "lite_mtcnn.h"

#define COSINE_SIM

// struct FaceData {
// 	int encode_dim;
// 	int num_faces;
// 	std::vector<std::string> faces;
// 	std::vector<std::vector<float> > encodes;

// 	void Reset(void);
// 	int Add(std::string face, std::std::vector<float> encode);
// 	int Del(std::string face);
// 	int Write(const std::string &fname);
// 	int Read(const std::string &fname);
// };

/**@brief add encode face into database
  *@retval 0                   success.
  *@retval -EEXIST             face already exists in the database
**/
int FaceData::Add(const std::string& face, const std::vector<float>& encode)
{
	for (auto &dface : faces) {
		if (dface == face) {
			return -EEXIST;
		}
	}
	faces.push_back(face);
	encodes.push_back(encode);
	num_faces++;
	return 0;
}

/**@brief check if face exist in the database
  *@retval 0                   face does not exist in the database
  *@retval 1                   face already exist in the database
**/
int FaceData::CheckExist(const std::string& face)
{
	for (auto &dface : faces) {
		if (dface == face) {
			return 1;
		}
	}
	return 0;
}

/**@brief delete encode face into database
  *@retval 0                   success.
  *@retval -ENODATA            face is not exist in the database
**/
int FaceData::Delete(const std::string& face)
{
	size_t i;

	if (face == "unknown") {
		return 0;
	}

	for (i = 0; i < faces.size(); ++i) {
		if (faces[i] == face) {
			break;
		}
	}
	if (i != faces.size()) {
		faces.erase(faces.begin() + i);
		encodes.erase(encodes.begin() + i);
		num_faces--;
		return 0;
	}
	return -ENODATA;
}


void FaceData::Reset()
{
	faces.clear();
	num_faces = 1;
	faces.emplace_back("unknown");
	encodes.clear();
	encodes.resize(1);
	encodes[0].resize(encode_dim, 0.0f);
}

/**@brief write encode face into database
  *@retval 0                   success.
  *@retval -ENOENT             cannot open file
**/
int FaceData::Write(const std::string &fname)
{
	FILE *fp = fopen(fname.c_str(), "wb");

	if (!fp) {
		inf_log_warn("Cannot open %s", fname.c_str());
		return -ENOENT;
	}

	int fnum_faces = num_faces - 1;
	fwrite(&encode_dim, sizeof(int), 1, fp);
	fwrite(&fnum_faces, sizeof(int), 1, fp);

	for (size_t i = 1; i < faces.size(); i++)
		fprintf(fp, "%s\n", faces[i].c_str());

	for (size_t i = 1; i < encodes.size(); i++)
		fwrite(encodes[i].data(), encodes[i].size() * sizeof(float), 1, fp);

	fclose(fp);
	return 0;
}

/**@brief read encode face into struct memory
  *@retval 0                   success.
  *@retval -ENOENT             cannot open file.
  *@retval -EINVAL             format is incorrect.
**/
int FaceData::Read(const std::string &fname)
{
	FILE *fp = fopen(fname.c_str(), "rb");

	if (!fp) {
		inf_log_warn("Cannot open %s", fname.c_str());
		return -ENOENT;
	}

	int header[2] = {};
	int ret = fread(header, sizeof(int), 2, fp);
	if (ret != 2) {
		inf_log_warn("Data file %s is empty", fname.c_str());
		fclose(fp);
		return 0;
	}

	int rnum_faces;
	encode_dim = header[0];
	rnum_faces = header[1];

	num_faces += rnum_faces;

	for (int i = 0; i < rnum_faces; ++i) {
		char face_name[256] = {};
		if (fgets(face_name, 255, fp) == nullptr) {
			inf_log_warn("Cannot read face name!");
			fclose(fp);
			return -EINVAL;
		}
		face_name[strlen(face_name) - 1] = '\0';
//		faces.push_back(std::string(face_name));
		faces.emplace_back(face_name);
	}

	int read_size = encode_dim * sizeof(float);
	for (int i = 0; i < rnum_faces; ++i) {
		std::vector<float> encode(encode_dim);
		ret = fread(encode.data(), read_size, 1, fp);
		if (ret != 1) {
			inf_log_warn("Cannot read face encode data!");
			fclose(fp);
			return -EINVAL;
		}
//		encodes.emplace_back(std::move(encode));
		encodes.emplace_back(encode);
	}

	fclose(fp);
	return 0;
}

void FaceData::Copy(InfStrList *strs)
{
	ReleaseStrList(strs);
	strs->size = num_faces;
	strs->data = (char**)malloc(sizeof(char*)*num_faces);
	for (int i = 0; i < num_faces; ++i) {
		strs->data[i] = (char*)malloc(sizeof(char*)*INF_STR_LEN);
		strcpy(strs->data[i], faces[i].c_str());
	}
}

InfFaceReco::InfFaceReco(InfModelInfo* info)
{
	m_config = info;
	m_face_data.Reset();
	m_face_data.Copy(&m_config->labels);
}

InfFaceReco::~InfFaceReco()
{
	ReleaseConfig(m_config);
	delete m_config;
	m_config = nullptr;
	// delete m_face_detect;
	// delete m_face_encode;
	// m_face_detect = nullptr;
	// m_face_encode = nullptr;
}

/**
 * @return The execution result
 * @retval 0      success
 * @retval -1     failure
 */
int InfFaceReco::LoadModels(InfFaceDetect *face_detect, InfFaceEncode *face_encode)
{
	m_face_detect = nullptr;
	m_face_encode = nullptr;
	int ret = 0;

	m_face_data.encode_dim = 128;

	if (face_detect) {
		m_face_detect = std::unique_ptr<InfFaceDetect>(face_detect);
	}

	if (face_encode) {
		m_face_encode = std::unique_ptr<InfFaceEncode>(face_encode);
		m_face_data.encode_dim = face_encode->GetEncodeDim();
	}

	if (!m_face_detect && !m_face_encode) {
		ret = -1;
		return ret;
	}

	m_face_data.Reset();
	m_face_data.Copy(&m_config->labels);

	return ret;
}

void InfFaceReco::SetFacePath(const char *face_data_path, const char *face_img_path)
{
	m_face_data_path = std::string(face_data_path);
	m_face_img_path = std::string(face_img_path);
}

int InfFaceReco::LoadFaceData()
{
	m_face_data.Reset();
	int ret = m_face_data.Read(m_face_data_path);
	if (ret)
		return ret;
	m_face_data.Copy(&m_config->labels);
	return 0;
}

int InfFaceReco::SaveFaceData()
{
	if (m_face_encode)
		m_face_data.encode_dim = m_face_encode->GetEncodeDim();
	return m_face_data.Write(m_face_data_path);
}

int InfFaceReco::LoadModels(const char *face_detect, const char *face_encode)
{
	inf_log_err("Not implemented yet!");
//	assert(0);
	// not implemented yet
	return -1;
}

void InfFaceReco::SetDebug(int debug)
{
	m_debug = debug;
	if (m_face_encode) {
		m_face_encode->m_debug = debug;
		SetupDebugTool(m_face_encode->m_snapshot_prefix);
	}
	if (m_face_detect) {
		m_face_detect->m_debug = debug;
		SetupDebugTool(m_face_detect->m_snapshot_prefix);
	}
}

int InfFaceReco::FaceIdentify(const InfImage *img, InfDetList *det_list)
{
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	//InfFaceEncode* encoder = m_face_encode;
	const int *encoder_input_dim = encoder->GetInputDim();

	for (int i = 0; i < det_list->size ; ++i) {
		std::vector<float> face_encode;
		auto& roi = det_list->data[i].rect;
		auto& det = det_list->data[i];

		if (m_config->verbose)
			TIC(start);

		if (m_scale_input_box) {
			int encoder_input_h = encoder_input_dim[0];
			int encoder_input_w = encoder_input_dim[1];
			RescaleFaceBox(encoder_input_h, encoder_input_w, roi);
		}

		int ret = encoder->EncodeFace(img, &roi, face_encode);
		if (ret) {
			return ret;
		}

		if (m_config->verbose)
			TOC("Encode Face call", start);

		ReleaseDetResult(&det);

		DistMeasure dist_funcptr{
#ifdef COSINE_SIM
			&VecCosineSimilarity, // dist func,
			0 // descending
#else
			&NormVecDistL2Norm, // dist func,
			1 // ascending
#endif
		};

		GetSortedFaceDist(m_face_data, face_encode, det,
			dist_funcptr);

		if (m_config->verbose) {
			printf("-----> identify result (%s) <----\n",
				m_face_data.faces[det.cls[0]].c_str());
			for (int j = 0; j < det.cls_num; j++) {
				printf("%16s - %.4f\n",
				m_face_data.faces[det.cls[j]].c_str(),
				det.prob[det.cls[j]]);
			}
		}


		if (m_debug) {
			InfImage wimg = encoder->GetInputImage();
			char snapshot_img_name[256] = {};
			bool unknown = det.prob[det.cls[0]] < m_config->conf_thresh.data[0];
			sprintf(snapshot_img_name, SNAPSHOT_FR_FORMAT, m_snapshot_prefix,
				unknown ? "neg" : "pos",
				m_snapshot_cnt,
				det.confidence,
				unknown ? "unknown" : m_config->labels.data[det.cls[0]],
				wimg.c == 1 ? "pgm" : "ppm");
			Inf_Imwrite(snapshot_img_name, &wimg);
			m_snapshot_cnt++;
		}

		if ((!dist_funcptr.ascend && (det.prob[det.cls[0]] < m_config->conf_thresh.data[0])) ||
		    (dist_funcptr.ascend && (det.prob[det.cls[0]] > m_config->conf_thresh.data[0]))) {
			det.cls[0] = 0; /* cls-0 is unknown class */
			det.confidence = 0.0;
		}

		if (m_config->topk > 0 && m_config->topk < m_config->labels.size) {
			det.cls_num = m_config->topk;
			memmove(det.cls, det.cls, sizeof(int)*m_config->topk);
		}
	}
	return 0;
}

int InfFaceReco::Detect(const InfImage *img, InfDetList *result)
{
	std::unique_ptr<InfFaceDetect>& detector = m_face_detect;
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	// InfFaceDetect* detector = m_face_detect;
	// InfFaceEncode* encoder = m_face_encode;
	int ret = 0;
	if (detector) {
		ret = detector->Detect(img, result);
		if (ret) {
			return ret;
		}
	}
	if (encoder) {
		ret = FaceIdentify(img, result);
		if (ret) {
			return ret;
		}
	}
	return ret;
}

int InfFaceReco::Detect(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *result)
{
	std::unique_ptr<InfFaceDetect>& detector = m_face_detect;
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	// InfFaceDetect* detector = m_face_detect;
	// InfFaceEncode* encoder = m_face_encode;
	int ret;
	if (detector) {
		ret = detector->Detect(img, obj_list, result);
		if (ret) {
			return -1;
		}
	}

	if (encoder) {
		ret = FaceIdentify(img, result);
		if (ret) {
			return -1;
		}
	}

	return 0;
}

void PickList(const MPI_IVA_OBJ_LIST_S *obj_list, const InfDetList& src, InfDetList *dst, float rate)
{
	const auto& rect = obj_list->obj[0].rect;

	int sx, sy, ex, ey;
	int overlap_area;
	int denom;

	for (int i = 0; i < src.size; i++) {
		const auto& det = src.data[i].rect;

		sx = std::max(rect.sx, det.sx);
		sy = std::max(rect.sy, det.sy);
		ex = std::min(rect.ex, det.ex);
		ey = std::min(rect.ey, det.ey);

		overlap_area = std::max(ex-sx+1, 0) * std::max(ey-sy+1, 0); // overlap area
		denom = (det.ex-det.sx+1) * (det.ey-det.sy+1); // face area

		if (((float) overlap_area / denom) > rate) {
			dst->size = 1;
			CopyDetResult(&src.data[i], &dst->data);
			dst->data[0].id = obj_list->obj[0].id;
			break;
		}

	}
}

int InfFaceReco::FaceRecoStageOne(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *det_list)
{
	std::unique_ptr<InfFaceDetect>& detector = m_face_detect;
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	const int *encoder_input_dim = encoder->GetInputDim();

	InfDetList local_list{};
	int ret;
	if (detector) {
		ret = detector->Detect(img, obj_list, &local_list);
		if (ret) {
			return -1;
		}
	}

	if (det_list->size) {
		inf_log_warn("size of det_list should be zero!");
		Inf_ReleaseDetResult(det_list);
	}


	float overlap_rate = 0.2;
	PickList(obj_list, local_list, det_list, overlap_rate); // ouptut here

	if (!det_list->size) {
		m_stage_id = -1;
		return 0;
	}

	auto& roi = det_list->data[0].rect;

	if (m_scale_input_box) {
		int encoder_input_h = encoder_input_dim[0];
		int encoder_input_w = encoder_input_dim[1];
		RescaleFaceBox(encoder_input_h, encoder_input_w, roi);
	}

	if (encoder) {
		ret = encoder->SetFaceEncodeImage(img, &roi); // prepare for stage-2
		if (ret) {
			return -1;
		}
		m_stage_id = det_list->data[0].id;
	}

	return 0;
}

int InfFaceReco::FaceRecoStageTwo(InfDetList *det_list)
{
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	int ret;

	if (!encoder)
		return -1;

	if (det_list->size)
		Inf_ReleaseDetResult(det_list);

	std::vector<float> face_encode;

	if (m_config->verbose)
		TIC(start);

	ret = encoder->EncodeFace(face_encode);
	if (ret) {
		return ret;
	}

	if (m_config->verbose)
		TOC("Encode Face call", start);

	det_list->size = 1;
	det_list->data = (InfDetResult*)malloc(sizeof(InfDetResult)*1);
	det_list->data[0].id = m_stage_id;
	auto& det = det_list->data[0];

	DistMeasure dist_funcptr{
#ifdef COSINE_SIM
		&VecCosineSimilarity, // dist func,
		0 // descending
#else
		&NormVecDistL2Norm, // dist func,
		1 // ascending
#endif
	};

	GetSortedFaceDist(m_face_data, face_encode, det,
		dist_funcptr);

	if (m_config->verbose) {
		printf("-----> identify result (%s) <----\n",
			m_face_data.faces[det.cls[0]].c_str());
		for (int i = 0; i < det.cls_num; i++) {
			printf("%16s - %.4f\n",
			m_face_data.faces[det.cls[i]].c_str(),
			det.prob[det.cls[i]]);
		}
	}

	if (m_debug) {
		InfImage wimg = encoder->GetInputImage();
		char snapshot_img_name[256] = {};
		bool unknown = det.prob[det.cls[0]] < m_config->conf_thresh.data[0];
		sprintf(snapshot_img_name, SNAPSHOT_FR_FORMAT, m_snapshot_prefix,
			unknown ? "neg" : "pos",
			m_snapshot_cnt,
			det.confidence,
			unknown ? "unknown" : m_config->labels.data[det.cls[0]],
			wimg.c == 1 ? "pgm" : "ppm");
		Inf_Imwrite(snapshot_img_name, &wimg);
		m_snapshot_cnt++;
	}

	if ((!dist_funcptr.ascend && (det.prob[det.cls[0]] < m_config->conf_thresh.data[0])) ||
	    (dist_funcptr.ascend && (det.prob[det.cls[0]] > m_config->conf_thresh.data[0]))) {
		det.cls[0] = 0; /* cls-0 is unknown class */
		det.confidence = 0.0;
	}
	if (m_config->topk > 0 && m_config->topk < m_config->labels.size) {
		det.cls_num = m_config->topk;
		memmove(det.cls, det.cls, sizeof(int)*m_config->topk);
	}
	return 0;
}

// Encode face using whole roi
int InfFaceReco::Register(const InfImage *img, const std::string &face_name, const MPI_RECT_POINT_S* roi, bool det)
{
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	std::unique_ptr<InfFaceDetect>& detector = m_face_detect;
	// InfFaceEncode* encoder = m_face_encode;
	const int *encoder_input_dim = encoder->GetInputDim();
	std::vector<float> face_encode;

	int ret = m_face_data.CheckExist(face_name);
	if (ret == 1) {
		inf_log_warn("Cannot register as face existed in the database!");
		return -EEXIST;
	}

	MPI_RECT_POINT_S roi_local = *roi;

	if (det && detector) {
		InfDetList det_list = {};
		MPI_IVA_OBJ_LIST_S obj_list = {};
		obj_list.obj_num = 1;
		obj_list.obj[0].rect= *roi;
		ret = detector->Detect(img, &obj_list, &det_list);
		if (ret) {
			Inf_ReleaseDetResult(&det_list);
			return ret;
		}
		if (det_list.size != 1) {
			inf_log_warn("Invalid face number detected, should be 1 (#%d found) for face registration! Please try again!", det_list.size);
			Inf_ReleaseDetResult(&det_list);
			return -EINVAL;
		}
		roi_local = det_list.data[0].rect;
		Inf_ReleaseDetResult(&det_list);
	}

	if (m_scale_input_box) {
		int encoder_input_h = encoder_input_dim[0];
		int encoder_input_w = encoder_input_dim[1];
		RescaleFaceBox(encoder_input_h, encoder_input_w, roi_local);
	}

	ret = encoder->EncodeFace(img, &roi_local, face_encode);
	if (ret) {
		return ret;
	}

	if (m_debug) {
		InfImage wimg = encoder->GetInputImage();
		char snapshot_img_name[256] = {};
		sprintf(snapshot_img_name, SNAPSHOT_FR_FORMAT, m_snapshot_prefix,
			"pos",
			m_snapshot_cnt + 10000,
			1.0,
			face_name.c_str(),
			wimg.c == 1 ? "pgm" : "ppm");
		Inf_Imwrite(snapshot_img_name, &wimg);
		m_snapshot_cnt++;
	}

	m_face_data.encode_dim = encoder->GetEncodeDim();
	m_face_data.num_faces++;
	m_face_data.faces.push_back(face_name);
	m_face_data.encodes.push_back(face_encode);
	m_face_data.Copy(&m_config->labels);
	return 0;
}

// Encode using DetectObjList.
int InfFaceReco::Register(const InfImage *img, const std::string &face_name)
{
	std::unique_ptr<InfFaceDetect>& detector = m_face_detect;
	std::unique_ptr<InfFaceEncode>& encoder = m_face_encode;
	// InfFaceDetect* detector = m_face_detect;
	// InfFaceEncode* encoder = m_face_encode;
	InfDetList det_list;
	MPI_RECT_POINT_S *roi;
	int ret = m_face_data.CheckExist(face_name);
	if (ret == 1) {
		inf_log_warn("Cannot register as face existed in the database!");
		return -EEXIST;
	}

	MPI_IVA_OBJ_LIST_S obj_list = {};
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 0;
	obj_list.obj[0].rect.sx = 0;
	obj_list.obj[0].rect.sy = 0;
	obj_list.obj[0].rect.ex = img->w-1;
	obj_list.obj[0].rect.ey = img->h-1;
	obj_list.obj[0].life = 160;
	obj_list.obj[0].mv = (MPI_MOTION_VEC_S){0, 0};
	roi = &obj_list.obj[0].rect;

	if (detector) {
		ret = detector->Detect(img, &obj_list, &det_list);
		if (det_list.size != 1) {
			inf_log_err("Human face detected in the image != 1 (%d), failed to register",
			            det_list.size);
			Inf_ReleaseDetResult(&det_list);
			return -EIO;
		}
		roi = &det_list.data[0].rect;
	}

	if (encoder) {
		ret = Register(img, face_name, roi);
	}

	Inf_ReleaseDetResult(&det_list);

	return 0;

}

int InfFaceReco::GetSortedFaceDist(const FaceData &face_data, const std::vector<float>& face_encode, InfDetResult &result,
	                      DistMeasure dist_method)
{
	const int num_faces = m_face_data.num_faces;
	result.cls_num = num_faces;
	result.prob_num = num_faces;
	result.prob = (float*)malloc(sizeof(float) * num_faces);
	result.cls = (int*)malloc(sizeof(int) * num_faces);

	for (int i = 0; i < num_faces; ++i) {
		result.cls[i] = i;
		result.prob[i] = dist_method.dist_funcptr(m_face_data.encodes[i], face_encode);
		if (m_verbose)
			inf_log_notice("face-data:name:%12s : %.4f", face_data.faces[i].c_str(), result.prob[i]);
	}

	// sort in ascending order
	if (dist_method.ascend) {
		std::sort(result.cls, result.cls + num_faces, [&](const int& cls0, const int& cls1) {
			return result.prob[cls0] < result.prob[cls1];
		});
	} else {
		std::sort(result.cls, result.cls + num_faces, [&](const int& cls0, const int& cls1) {
			return result.prob[cls0] > result.prob[cls1];
		});
	}

	result.confidence = result.prob[result.cls[0]];
	return 0;
}

/**
 * @brief Frame based face detection
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] result           detection results.
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @see Inf_DeleteFaceData()
 */
extern "C" int Inf_InvokeFaceDet(InfModelCtx *ctx, const InfImage *img, InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && result);

	static_cast<InfModel*>(ctx->model)->Detect(img, result);

	return 0;
}

/**
 * @brief Object list referenced face detection
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] ol               object list w.r.t. to the image
 * @param[in] result           detection results.
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @see Inf_InvokeFaceDet()
 */
extern "C" int Inf_InvokeFaceDetObjList(InfModelCtx *ctx, const InfImage *img,
                                        const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && ol && result);

	static_cast<InfModel*>(ctx->model)->Detect(img, ol, result);

	return 0;
}

static void showCurrentNameList(const InfStrList *strs)
{
	printf("\n\t Current Name List ** #%d persons\n", strs->size-1);
	for (int i = 0; i < strs->size-1; i++) {
		printf("\t%s\n", strs->data[i+1]);
	}
	if (strs->size-1) {
		printf("\n");
	}
}

/**
 * @brief Register face by region of interest (No face alignment during registration)
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] roi              region of interest
 * @param[in] face_name        face name to be registered
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @retval -EIO                input image detected face number is not equal to one
 * @see Inf_DeleteFaceData()
 */
extern "C" int Inf_RegisterFaceRoi(InfModelCtx *ctx, const InfImage *img,
	                               const MPI_RECT_POINT_S *roi, const char *face_name)
{
	retIfNull(ctx && ctx->model && img && roi && face_name);

	if (ctx->info->inference_type != InfRunFaceReco) {
		return -EINVAL;
	}

	InfModel* model = static_cast<InfModel*>(ctx->model);
	InfFaceReco* face_reco = dynamic_cast<InfFaceReco*>(model);
	if (!face_reco) {
		inf_log_err("Cannot cast model to FR model!");
		return -EINVAL;
	}

	int ret = face_reco->Register(img, std::string(face_name), roi);
	if (ret == 0) {
		face_reco->m_face_data.Copy(&face_reco->m_config->labels);
		showCurrentNameList(&face_reco->m_config->labels);
	}

	return ret;
}

/**
 * @brief Register face by region of interest
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] roi              region of interest
 * @param[in] face_name        face name to be registered
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_DeleteFaceData()
 */
extern "C" int Inf_RegisterFaceRoiDet(InfModelCtx *ctx, const InfImage *img,
	                                  const MPI_RECT_POINT_S *roi, const char *face_name)
{
	retIfNull(ctx && ctx->model && img && roi && face_name);

	if (ctx->info->inference_type != InfRunFaceReco) {
		return -EINVAL;
	}

	InfModel* model = static_cast<InfModel*>(ctx->model);
	InfFaceReco* face_reco = dynamic_cast<InfFaceReco*>(model);
	if (!face_reco) {
		inf_log_err("Cannot cast model to FR model!");
		return -EINVAL;
	}

	int ret = face_reco->Register(img, std::string(face_name), roi, true);
	if (ret == 0) {
		face_reco->m_face_data.Copy(&face_reco->m_config->labels);
		showCurrentNameList(&face_reco->m_config->labels);
	}

	return ret;
}


/**
 * @brief Invoke face encode model
 * @retval 0                   success.
 * @retval -EFAULT             input pointers are null.
 * @retval -EINVAL             incompatible model inference type.
 */
extern "C" int Inf_InvokeFaceEncode(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *roi,
                                    int *encode_dim, float **face_encode)
{
	retIfNull(ctx && ctx->model && img && roi && encode_dim && face_encode);

	if (ctx->info->inference_type != InfRunFaceEncode) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceEncode *inf_model = dynamic_cast<InfFaceEncode*>(model);
	if (!inf_model)
		return -EINVAL;

	std::vector<float> face;
	int ret = inf_model->EncodeFace(img, roi, face);
	*encode_dim = inf_model->GetEncodeDim();
	*face_encode = (float*)malloc(sizeof(float) * (*encode_dim));
	memcpy(*face_encode, face.data(), sizeof(float) * (*encode_dim));

	return ret;
}

/**
 * @brief Delete face data from onload database
 * @details
 * @param[in] ctx              inference model context
 * @param[in] face_name        face name to be deleted
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_DeleteFaceData()
 */
extern "C" int Inf_DeleteFace(InfModelCtx *ctx, const char *face_name)
{
	retIfNull(ctx && ctx->model && face_name);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_warn("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model) {
		return -EINVAL;
	}

	int ret = inf_model->m_face_data.Delete(std::string(face_name));
	if (ret==0) {
		inf_model->m_face_data.Copy(&inf_model->m_config->labels);
		showCurrentNameList(&inf_model->m_config->labels);
	}

	return ret;
}

/**
 * @brief Save face database from model to disk.
 * @details
 * @param[in] ctx              inference model context
 * @param[in] face_database    face database file directory to be saved.
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @retval -ENOENT             fail to open file
 * @see Inf_LoadFaceData()
 */
extern "C" int Inf_SaveFaceData(InfModelCtx *ctx, const char *face_database)
{
	retIfNull(ctx && ctx->model && face_database);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model)
		return -EINVAL;

	inf_model->m_face_data_path = std::string(face_database);
	int ret = inf_model->SaveFaceData();
	if (ret == 0)
		showCurrentNameList(&inf_model->m_config->labels);

	return ret;
}

/**
 * @brief Load face database from file to model.
 * @details
 * @param[in] ctx              inference model context
 * @param[in] face_database    face database file directory.
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type,
 *                             encode dimension is different to current onload model output
 * @retval -ENOENT             fail to open file
 * @see Inf_SaveFaceData()
 */
extern "C" int Inf_LoadFaceData(InfModelCtx *ctx, const char *face_database)
{
	retIfNull(ctx && ctx->model && face_database);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model)
		return -EINVAL;

	inf_model->m_face_data_path = std::string(face_database);
	int ret = inf_model->LoadFaceData();
	if (inf_model->m_face_encode) {
		showCurrentNameList(&inf_model->m_config->labels);
		if (inf_model->m_face_encode->GetEncodeDim() != inf_model->m_face_data.encode_dim) {
			inf_log_err("Face data dimension is not consisting to face encode model output dimension!");
			return -EINVAL;
		}
	}

	return ret;
}

/**
 * @brief Delete a registered face from onload face database.
 * @details
 * @param[in] ctx              inference model context
 * @param[in] face             registered face to be deleted
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @retval -ENODATA            no such face name in database
 * @see Inf_RegisterFaceRoi()
 */
extern "C" int Inf_DeleteFaceData(InfModelCtx *ctx, const char *face)
{
	retIfNull(ctx && ctx->model && face);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model)
		return -EINVAL;

	int ret = inf_model->m_face_data.Delete(std::string(face));
	if (ret == 0) {
		inf_model->m_face_data.Copy(&inf_model->m_config->labels);
		showCurrentNameList(&inf_model->m_config->labels);
	}

	return ret;
}

/**
 * @brief Reset onload face database.
 * @details
 * @param[in] ctx              inference model context
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_DeleteFaceData()
 */
extern "C" int Inf_ResetFaceData(InfModelCtx *ctx)
{
	retIfNull(ctx && ctx->model);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model)
		return -EINVAL;

	inf_model->m_face_data.Reset();
	inf_model->m_face_data.Copy(&inf_model->m_config->labels);
	showCurrentNameList(&inf_model->m_config->labels);

	return 0;
}

/**
 * @brief Invoke face recongition by region of interest.
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] roi              region of interest w.r.t. the image
 * @param[in] result           face recognition result
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_InvokeFaceRecoList()
 */
extern "C" int Inf_InvokeFaceRecoRoi(InfModelCtx *ctx, const InfImage *img, const MPI_RECT_POINT_S *rect,
                                     InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && rect && result);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel *model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	if (!inf_model)
		return -EINVAL;

	result->size = 1;
	result->data = (InfDetResult*)calloc(1, sizeof(InfDetResult));
	result->data[0].rect = *rect;
	inf_model->FaceIdentify(img, result);

	return 0;
}

/**
 * @brief Invoke face recongition by object list.
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] ol               object list w.r.t. the image
 * @param[in] result           face recognition result
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_InvokeFaceRecoRoi()
 */
extern "C" int Inf_InvokeFaceRecoList(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol,
                                      InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && ol && result);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	static_cast<InfModel*>(ctx->model)->Detect(img, ol, result);

	return 0;
}

/**
 * @brief Invoke face recongition by object list (stage one).
 * @details
 * @param[in] ctx              inference model context
 * @param[in] img              image
 * @param[in] ol               object list w.r.t. the image
 * @param[in] result           face recognition result
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_InvokeFaceRecoRoi()
 */
extern "C" int Inf_InvokeFaceRecoStageOne(InfModelCtx *ctx, const InfImage *img,
                                          const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && ol && result);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	if (ol->obj_num != 1) {
		inf_log_err("There should be one obj in the input obj_list but expect (%d)!", ol->obj_num);
		return -EINVAL;
	}

	InfModel* model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	inf_model->FaceRecoStageOne(img, ol, result);

	return 0;
}

/**
 * @brief Invoke face recongition (stage two).
 * @details
 * @param[in] ctx              inference model context
 * @param[in] result           face recognition result
 * @retval 0                   success
 * @retval -EFAULT             input pointers are null
 * @retval -EINVAL             input model has incompatible inference type
 * @see Inf_InvokeFaceRecoRoi()
 */
extern "C" int Inf_InvokeFaceRecoStageTwo(InfModelCtx *ctx, InfDetList *result)
{
	retIfNull(ctx && ctx->model && result);

	if (ctx->info->inference_type != InfRunFaceReco) {
		inf_log_err("Incompatible model type (%s) inference %s!", GetInfTypeStr(ctx->info->inference_type), __func__);
		return -EINVAL;
	}

	InfModel* model = static_cast<InfModel*>(ctx->model);
	InfFaceReco *inf_model = dynamic_cast<InfFaceReco*>(model);
	inf_model->FaceRecoStageTwo(result);

	return 0;
}
//#endif // USE_NCNN