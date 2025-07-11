#ifndef EAIF_C_ENGINE_H_
#define EAIF_C_ENGINE_H_

#ifdef __cplusplus
#include "eaif_engine.h"
typedef eaif::Engine EAIF_Engine;
extern "C" {
#else
typedef struct Engine EAIF_Engine;
#endif

#include "eaif_c_common.h"
#include "eaif_c_image.h"

typedef enum { EAIF_Classify, EAIF_Detect, EAIF_FaceReco, EAIF_Infer } EAIF_InferenceType;

typedef enum { EAIF_Armnn, EAIF_TfLite, EAIF_EngineOther } EAIF_EngineType;

typedef enum { EAIF_Sigmoid, EAIF_Linear } EAIF_ActivationType;

EAIF_EngineType EAIF_getEngineType(const char *engine_str);
EAIF_InferenceType EAIF_getInferenceType(const char *inference_str);
EAIF_ActivationType EAIF_getActivationType(const char *activation_str);
//EAIF_FaceInferType EAIF_getFaceInferType(const char *face_infer_str);

typedef struct ModelConfig EAIF_ModelConfig;

typedef struct ModelInfo EAIF_ModelInfo;

// Utils for model info
EAIF_ModelInfo *eaif_createModelInfo(void);
void eaif_destroyModelInfo(EAIF_ModelInfo **info);
int eaif_getModelInfoIdx(const EAIF_ModelInfo *info);
const char *eaif_getModelInfoName(const EAIF_ModelInfo *info);

EAIF_Engine *eaif_createEngine(void);
int eaif_destroyEngine(EAIF_Engine **engine);
int eaif_clearEngine(EAIF_Engine *engine);
int eaif_setupEngine(EAIF_Engine *engine, const char *engine_config);

// Query Function
int eaif_queryEngineModelDetail(const EAIF_Engine *engine, const char *model_str, EAIF_ModelConfig *conf);
int eaif_queryEngineModelInfo(const EAIF_Engine *engine, const char *model_str, EAIF_ModelInfo *info);
int eaif_queryEngineFaceInfo(const EAIF_Engine *engine, int model_index, EAIF_StrList *face_list);
void eaif_queryEngineModelNames(const EAIF_Engine *engine, EAIF_InferenceType type, EAIF_StrList *name_list);

// Inference Function
int eaif_engineClassifyObjList(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg,
                               const EAIF_ObjList *obj_list, EAIF_ClassifyList *result);
int eaif_engineClassifyObj(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, EAIF_ClassifyList *result);
int eaif_engineDetect(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, EAIF_DetectList *result);

// Face Utils
int eaif_engineRegisterFace(EAIF_Engine *engine, int model_index, const EAIF_Image *Wimg, const char *face_name,
                            int is_full_image);
int eaif_engineLoadFaceData(EAIF_Engine *engine, int model_index);
int eaif_engineSaveFaceData(const EAIF_Engine *engine, int model_index);

// General Engine Utils
int eaif_engineUpdateModel(EAIF_Engine *engine);
int eaif_engineClearModelStates(EAIF_Engine *engine);
int eaif_engineGetModelStates(const EAIF_Engine *engine, EAIF_ModelStateList *states);
int eaif_engineGetVerbose(const EAIF_Engine *engine);

char *eaif_engineGetTemplatePath(const EAIF_Engine *engine);

#ifdef __cplusplus
}
#endif

#endif /* EAIF_C_ENGINE_H_ */
