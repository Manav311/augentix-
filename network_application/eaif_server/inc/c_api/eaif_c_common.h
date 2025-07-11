#ifndef EAIF_C_COMMON_H_
#define EAIF_C_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MODEL_NAME_LENGTH 63
#define FACE_NAME_LENGTH 63

typedef struct {
	union {
		struct {
			int x;
			int y;
			int w;
			int h;
		};
		struct {
			int sx;
			int sy;
			int ex;
			int ey;
		};
		int c[4];
	};
} EAIF_Rect;

typedef struct {
	int idx;
	EAIF_Rect box;
} EAIF_ObjectAttr;

typedef struct {
	int size;
	EAIF_ObjectAttr *obj;
} EAIF_ObjList;

typedef struct {
	int size;
	char **str;
} EAIF_StrList;

typedef struct {
	int idx;
	int cls_num;
	int prob_num;
	int *cls;
	float *prob;
} EAIF_Classification;

typedef struct {
	int size;
	EAIF_Classification *obj;
} EAIF_ClassifyList;

typedef struct {
	float confidence;
	int prob_num;
	int cls_num;
	float *prob;
	int *cls;
	EAIF_Rect box;
} EAIF_Detection;

typedef struct {
	int size;
	EAIF_Detection *obj;
} EAIF_DetectList;

typedef struct {
	int infer_count;
	char name[MODEL_NAME_LENGTH];
} EAIF_ModelState;

typedef struct {
	int size;
	EAIF_ModelState *states;
} EAIF_ModelStateList;

void eaif_clearClassifyList(EAIF_ClassifyList *cls_list);
void eaif_clearStrList(EAIF_StrList *str_list);
void eaif_clearObjList(EAIF_ObjList *obj_list);
void eaif_clearDetList(EAIF_DetectList *det_list);
void eaif_clearStateList(EAIF_ModelStateList *sta_list);

void eaif_dumpIdx(const uint8_t *data, size_t size, int idx);
void eaif_dumpName(const uint8_t *data, size_t size, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* !EAIF_C_COMMON_H_ */
