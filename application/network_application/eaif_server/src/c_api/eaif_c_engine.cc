#include <string>

#include <string.h>

#include "eaif_engine.h"
#include "eaif_model.h"
#include "eaif_trc.h"

#include "eaif_c_common.h"
#include "eaif_c_engine.h"

using namespace std;

static inline void CopyVecStrToC(vector<string>& a, EAIF_StrList& strList, int strLen)
{
	strList.size = a.size();
	if (a.size()) {
		strList.str = (char**)malloc(sizeof(char*) *a.size());
		for (size_t i = 0; i < a.size(); i++) {
			strList.str[i] = (char*)malloc(sizeof(char*) * (strLen+1));
			strncpy(strList.str[i], a[i].data(), strLen);
			strList.str[i][strLen] = 0;
		}
	}
}

static inline void CopyCListToObjList(const EAIF_ObjList &a, vector<ObjectAttr> &b)
{
	if (!a.size)
		return;
	b.resize(a.size);
	for (int i = 0; i < a.size; i++) {
		auto& obj = a.obj[i];
		b[i].idx = obj.idx;
		for (int j = 0; j < 4; j++) {
			b[i].box.c[j] = obj.box.c[j];
		}
	}
}

static inline void CopyClsToC(const Classification &a, EAIF_Classification &b)
{
	b.idx = a.idx;
	b.cls_num = a.cls.size();
	b.prob_num = a.prob.size();
	b.cls = (int*)malloc(sizeof(int) * b.cls_num);
	b.prob = (float*)malloc(sizeof(float) * b.prob_num);
	memcpy(b.cls, a.cls.data(), sizeof(int) * b.cls_num);
	memcpy(b.prob, a.prob.data(), sizeof(float) * b.prob_num);
}

static inline void CopyVecClsToC(const vector<Classification> &a, EAIF_ClassifyList &b)
{
	b.size = a.size();
	if (b.size) {
		b.obj = (EAIF_Classification*)malloc(sizeof(EAIF_Classification) * b.size);
		for (size_t i = 0; i < a.size(); i++) {
			auto& result = a[i];
			CopyClsToC(result, b.obj[i]);
		}
	}
}

static inline void CopyVecDetToC(const vector<Detection> &a, EAIF_DetectList &b)
{
	b.size = a.size();
	if (b.size) {
		b.obj = (EAIF_Detection*)malloc(sizeof(EAIF_Detection) * b.size);
		for (size_t i = 0; i < a.size(); i++) {
			auto& result = a[i];
			for (int j = 0; j < 4; j++)
				b.obj[i].box.c[j] = result.box.c[j];
			b.obj[i].confidence = result.confidence;
			b.obj[i].cls_num = result.cls.size();
			b.obj[i].prob_num = result.prob.size();
			b.obj[i].cls = (int*)malloc(sizeof(int) * b.obj[i].cls_num);
			b.obj[i].prob = (float*)malloc(sizeof(float) * b.obj[i].prob_num);
			memcpy(b.obj[i].cls, result.cls.data(), sizeof(int) * b.obj[i].cls_num);
			memcpy(b.obj[i].prob, result.prob.data(), sizeof(float) * b.obj[i].prob_num);
		}
	}
}

static inline void CopyVecStateToC(const vector<StatePair> &a, EAIF_ModelStateList &b)
{
	b.size = a.size();
	if (b.size) {
		b.states = (EAIF_ModelState*)malloc(sizeof(EAIF_ModelState) * b.size);
		for (int i = 0; i < b.size; i++) {
			strncpy(b.states[i].name, a[i].first.data(), MODEL_NAME_LENGTH);
			b.states[i].infer_count = a[i].second.infer_count;
		}
	}
}

EAIF_EngineType EAIF_getEngineType(const char *engine_str)
{
	return (EAIF_EngineType)eaif::engine::GetEngineType(engine_str);
}

EAIF_InferenceType EAIF_getInferenceType(const char *inference_str)
{
	return (EAIF_InferenceType)eaif::engine::GetInferenceType(inference_str);
}

EAIF_ActivationType EAIF_getActivationType(const char *activation_str)
{
	return (EAIF_ActivationType)eaif::engine::GetActivationType(activation_str);
}

EAIF_ModelInfo *eaif_createModelInfo(void)
{
	return new EAIF_ModelInfo;
}

void eaif_destroyModelInfo(EAIF_ModelInfo **info)
{
	if (!info)
		return;
	delete *info;
	*info = nullptr;
	return;
}

EAIF_Engine *eaif_createEngine(void)
{
	return new eaif::Engine;
}

int eaif_destroyEngine(EAIF_Engine **engine)
{
	if (!engine) {
		eaif_warn("%s input is empty!\n", __func__);
		return -1;
	}

	(*engine)->Clear();
	delete *engine;
	*engine = nullptr;
	return 0;
}

int eaif_clearEngine(EAIF_Engine *engine)
{
	if (engine)
		engine->Clear();
	return 0;
}

int eaif_setupEngine(EAIF_Engine *engine, const char *engine_config)
{
	if (engine) {
		string engine_conf_str = std::string(engine_config);
		return engine->Setup(engine_conf_str);
	} else {
		eaif_warn("%s input is empty!\n", __func__);
		return -1;
	}
}

// Query Function
int eaif_queryEngineModelDetail(const EAIF_Engine *engine, const char *model_str, EAIF_ModelConfig *conf)
{
	return engine->QueryModelDetail(model_str, *conf);
}

int eaif_queryEngineModelInfo(const EAIF_Engine *engine, const char *model_str, EAIF_ModelInfo *info)
{
	return engine->QueryModelInfo(model_str, *info);
}

int eaif_queryEngineFaceInfo(const EAIF_Engine *engine, int model_index, EAIF_StrList *face_list)
{
	vector<string> face_name;
	int ret = engine->QueryFaceInfo(model_index, face_name);
	CopyVecStrToC(face_name, *face_list, FACE_NAME_LENGTH);
	return ret;
}

void eaif_queryEngineModelNames(const EAIF_Engine *engine, EAIF_InferenceType type, EAIF_StrList *name_list)
{
	vector<string> names;
	engine->QueryModelNames((eaif::engine::InferenceType)type, names);
	CopyVecStrToC(names, *name_list, MODEL_NAME_LENGTH);
	return;
}

// Inference Function
int eaif_engineClassifyObjList(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, const EAIF_ObjList *obj_list, EAIF_ClassifyList *result)
{
	int ret = 0;
	vector<ObjectAttr> vobj_list;
	vector<Classification> vresults;
	CopyCListToObjList(*obj_list, vobj_list);
	ret = engine->ClassifyObjList(model_index, Wimg, vobj_list, vresults);
	if (vresults.size())
		CopyVecClsToC(vresults, *result);
	return ret;
}

int eaif_engineClassifyObj(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, EAIF_Classification *result)
{
	Classification cls_result;
	int ret = engine->Classify(model_index, Wimg, cls_result);
	CopyClsToC(cls_result, *result);
	return ret;
}

int eaif_engineDetect(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, EAIF_DetectList *result)
{
	vector<Detection> det_result;
	int ret = engine->Detect(model_index, Wimg, det_result);
	CopyVecDetToC(det_result, *result);
	return ret;
}

// Face Utils
int eaif_engineRegisterFace(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, const char *face_name, int is_full_image)
{
	return engine->RegisterFace(model_index, Wimg, string(face_name), 0);
}

int eaif_engineLoadFaceData(EAIF_Engine *engine, int model_index)
{
	return engine->LoadFaceData(model_index);
}

int eaif_engineSaveFaceData(const EAIF_Engine *engine, int model_index)
{
	return engine->SaveFaceData(model_index);
}


int eaif_engineUpdateModel(EAIF_Engine *engine)
{
	engine->Update();
	return 0;
}

int eaif_engineClearModelStates(EAIF_Engine *engine)
{
	engine->ClearModelStates();
	return 0;
}

int eaif_engineGetModelStates(const EAIF_Engine *engine, EAIF_ModelStateList *states)
{
	vector<StatePair> state_pairs;
	engine->GetModelStates(state_pairs);
	CopyVecStateToC(state_pairs, *states);
	return 0;
}

int eaif_engineGetVerbose(const EAIF_Engine *engine)
{
	return engine->GetVerbose();
}
