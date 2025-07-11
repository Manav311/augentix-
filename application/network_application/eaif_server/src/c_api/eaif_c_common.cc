#include "eaif_common.h"
#include "eaif_model.h"

#include "eaif_c_common.h"
#include "eaif_c_engine.h"

void eaif_dumpIdx(const uint8_t *data, size_t size, int idx)
{
	return Dump(data, size, idx);
}

void eaif_dumpName(const uint8_t *data, size_t size, const char *name)
{
	return Dump(data, size, name);
}

int eaif_getModelInfoIdx(const EAIF_ModelInfo *info)
{
	return info->index;
}

const char *eaif_getModelInfoName(const EAIF_ModelInfo *info)
{
	return info->name;
}

void eaif_clearClassifyList(EAIF_ClassifyList *cls_list)
{
	if (!cls_list->size)
		return;

	for (int i = 0; i < cls_list->size; i++) {
		auto& obj = cls_list->obj[i];
		free(obj.cls);
		free(obj.prob);
	}
	free(cls_list->obj);
	cls_list->obj = nullptr;
	cls_list->size = 0;
}

void eaif_clearStrList(EAIF_StrList *str_list)
{
	if (!str_list->size)
		return;

	for (int i = 0; i < str_list->size; i++) {
		free(str_list->str[i]);
	}
	free(str_list->str);
	str_list->str = nullptr;
	str_list->size = 0;
}

void eaif_clearObjList(EAIF_ObjList *obj_list)
{
	if (!obj_list->size)
		return;
	free(obj_list->obj);
	obj_list->obj = nullptr;
	obj_list->size = 0;
}

void eaif_clearDetList(EAIF_DetectList *det_list)
{
	if (!det_list->size)
		return;

	for (int i = 0; i < det_list->size; i++) {
		auto& obj = det_list->obj[i];
		free(obj.prob);
		free(obj.cls);
	}
	free(det_list->obj);
	det_list->obj = nullptr;
	det_list->size = 0;
}

void eaif_clearStateList(EAIF_ModelStateList *sta_list)
{
	if (!sta_list->size)
		return;
	free(sta_list->states);
	sta_list->states = nullptr;
	sta_list->size = 0;
	return;
}