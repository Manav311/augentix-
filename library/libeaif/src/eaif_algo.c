#include "eaif_algo.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_enc.h"

#include "cm_iva_eaif_resp.h"
#include "eaif.h"
#include "eaif_log.h"

//#define EAIF_STATIC_IMAGE

static const int g_fixed_point_sf_one = 1 << EAIF_FIXED_POINT_BS;

/**
 * @brief		Update status using input object list.
 * @param[in]  inObj_list  	Candidate list
 * @param[in]  obj_life_th	Object life threshold
 * @param[in]  algo			EAIF internal parameters
 * @param[out] status		Container stores counter for each object
 */
void eaif_updateObjAttr(const MPI_IVA_OBJ_LIST_S *inObj_list, const UINT16 obj_life_th, const EaifAlgo *algo,
                        EaifStatusInternal *status)
{
	status->timestamp = inObj_list->timestamp;

	if (inObj_list->obj_num == 0) {
		status->obj_cnt = 0;
		status->obj_exist_any = 0;
		status->obj_exist_any_counter = 0;
		return;
	}

	EaifObjAttrEx new_obj_attr[MPI_IVA_MAX_OBJ_NUM] = {};
	for (int i = 0; i < inObj_list->obj_num; i++) {
		EaifObjAttrEx *const new_object = &new_obj_attr[i];

		const MPI_IVA_OBJ_ATTR_S *const object = &inObj_list->obj[i];
		const EaifObjAttrEx *const object_ex = getObjAttrEx(status->obj_attr_ex, status->obj_cnt, object->id);

		if (object_ex && object->life >= obj_life_th) {
			*new_object = *object_ex;
			new_object->frame_counter++;
		} else {
			new_object->basic.id = object->id;
			new_object->frame_counter = algo->p->neg_classify_period;
			new_object->infer_counter = new_object->confid_counter = 0;
			// new_object->stage = 0;
		}
		new_object->basic.rect = object->rect;
	}

	status->obj_cnt = inObj_list->obj_num;
	memcpy(status->obj_attr_ex, new_obj_attr, sizeof(EaifObjAttrEx) * status->obj_cnt);
#if EAIF_DEBUG_INFO
	EaifObjAttrEx *obj_attr;
	static int cnt = 0;
	printf("[%s %d]: frame:%d obj cnt: %d (id,confid,frame_count)", __func__, __LINE__, cnt++, status->obj_cnt);
	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		obj_attr = &status->obj_attr_ex[i];
		printf("(%d,%d,%d),", obj_attr->basic.id, obj_attr->confid_counter, obj_attr->frame_counter);
	}
	printf("\n");
#endif
	//eaif_log_info("Status cnt:%d obj[0] Id:%d life:%d fr:%d", status->obj_cnt, status->obj_attr[0].id, inObj_list->obj[0].life, status->obj_attr[0].frame_counter);
	return;
}

/**
 * @brief		Choose a candidate with the highest priority.
 * @details		FCFS strategy is implemented.
 * @param[in]     ori_obj_list    Candidate list
 * @param[in]     algo            EAIF internal parameters
 * @param[in,out] status          Container stores counter for each object
 * @param[out]    fin_obj_list    Object chose for model inference
 */
int eaif_checkAppendable(const MPI_IVA_OBJ_LIST_S *ori_obj_list, const EaifAlgo *algo,
                         struct eaif_status_internal_s *status, MPI_IVA_OBJ_LIST_S *fin_obj_list)
{
	fin_obj_list->timestamp = ori_obj_list->timestamp;

	if (!status->obj_cnt) {
		status->obj_exist_any = false;
		status->obj_exist_any_counter = 0;
		fin_obj_list->obj_num = 0;
		return false;
	}

	status->obj_exist_any_counter += status->obj_exist_any;
	if (algo->p->obj_exist_classify_period && status->obj_exist_any_counter <= algo->p->obj_exist_classify_period) {
		return false;
	}

	int appendable = false;
	int max_idx = 0, max_priority = INT32_MIN;
	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		if (ori_obj_list->obj[i].life < algo->p->obj_life_th) {
			continue;
		}

		const EaifObjAttrEx *const obj_attr = &status->obj_attr_ex[i];

		if ((UINT32)obj_attr->confid_counter >= algo->p->pos_stop_count_th) {
			continue;
		}

		const int frame_counter_th = obj_attr->basic.label_num ? algo->p->pos_classify_period :
		                                                         algo->p->neg_classify_period;
		if (obj_attr->frame_counter < frame_counter_th) {
			continue;
		}

		appendable = true;

		// use geq to implicitly choose one that is less frequently used
		const int priority = obj_attr->frame_counter - frame_counter_th; // waiting time
		if (priority >= max_priority) {
			max_priority = priority;
			max_idx = i;
		}
	}

	// reset counter if and only if we can infer an object
	if (appendable) {
		status->obj_exist_any_counter = 0;
	}

	fin_obj_list->obj[0] = ori_obj_list->obj[max_idx];
	fin_obj_list->obj_num = appendable;

#if EAIF_DEBUG_INFO
	printf("[%s %d]: append:%d obj cnt: %d (id,confid:%d,frame,infer)", __func__, __LINE__, appendable,
	       status->obj_cnt, algo->p->pos_stop_count_th);
	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		const EaifObjAttrEx *const obj_attr = &status->obj_attr_ex[i];
		printf("(%d,%d,%d,%d),", obj_attr->basic.id, obj_attr->confid_counter, obj_attr->frame_counter,
		       obj_attr->infer_counter);
	}
	printf("\n");
#endif
	return appendable;
}

int eaif_checkDetAppendable(const MPI_IVA_OBJ_LIST_S *ori_obj_list, const EaifAlgo *algo,
                            MPI_IVA_OBJ_LIST_S *fin_obj_list)
{
	int obj_num = 0;
	for (int i = 0; i < ori_obj_list->obj_num; i++) {
		if (ori_obj_list->obj[i].life < algo->p->obj_life_th)
			continue;
		fin_obj_list->obj[obj_num] = ori_obj_list->obj[i];
		obj_num++;
	}
	fin_obj_list->obj_num = obj_num;
	return obj_num > 0;
}

void eaif_updateFrObjAttr(const MPI_IVA_OBJ_LIST_S *ol, const EaifAlgo *algo, EaifInfo *info,
                          EaifStatusInternal *status)
{
	// register box
	// del untracked box
	// update frame counter

	const int obj_life_th = algo->p->obj_life_th;

	EaifObjAttrEx new_obj_attr[MPI_IVA_MAX_OBJ_NUM] = {};
	int marker[MPI_IVA_MAX_OBJ_NUM] = {};
	int inf_stage_set = 0;

	for (int i = 0; i < ol->obj_num; i++) {
		const MPI_IVA_OBJ_ATTR_S *iva_obj = &ol->obj[i];
		EaifObjAttrEx *new_attr = &new_obj_attr[i];
		EaifObjAttrEx *old_attr;

		bool b_new = 1;
		for (int j = 0; (UINT32)j < status->obj_cnt; j++) {
			if (marker[j])
				continue;
			old_attr = &status->obj_attr_ex[j];
			if (iva_obj->id == old_attr->basic.id) {
				marker[j] = 1;
				b_new = 0;
				if (!inf_stage_set && old_attr->stage >= 1)
					inf_stage_set = 1;
				if (ol->obj[i].life >= obj_life_th) {
					old_attr->frame_counter++;
				} else {
					b_new = 1; /* treat as new object */
					break;
				}
				*new_attr = *old_attr;
				break;
			}
		}

		if (b_new) {
			new_attr->basic.id = iva_obj->id;
			new_attr->frame_counter = 0;
			new_attr->confid_counter = 0;
			new_attr->infer_counter = 0;
			new_attr->stage = 0;
		}
	}
	if (!inf_stage_set)
		info->inf_fr_stage = 0;

	status->obj_cnt = ol->obj_num;
	status->timestamp = ol->timestamp;
	memcpy(status->obj_attr_ex, new_obj_attr, sizeof(EaifObjAttrEx) * status->obj_cnt);

#if EAIF_DEBUG_INFO
	EaifObjAttrEx *obj_attr;
	static int cnt = 0;
	printf("[%s %d]: frame:%d obj cnt: %d (id,confid,frame_count)", __func__, __LINE__, cnt++, status->obj_cnt);
	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		obj_attr = &status->obj_attr_ex[i];
		printf("(%d,%d,%d),", obj_attr->basic.id, obj_attr->confid_counter, obj_attr->frame_counter);
	}
	printf("\n");
#endif

	return;
}

int eaif_checkFrAppendable(const MPI_IVA_OBJ_LIST_S *src, const EaifAlgo *algo, struct eaif_status_internal_s *status,
                           EaifInfo *info)
{
	/* FR Appendable is to check if input object can be append to request list

		The checking follow below criteria
		1. object life threshold
		2. box aspect ratio checker
		3. confid_counter

		And then sort by following priority factor
		1. object life
		2. new/ pos/ neg object
		3. area size
		4. object stage (inf_fr_counter/ inf_fr_stage)

	*/

	const UINT32 append_priority_max = 0xffffff;

	int appendable = 0;
	int obj_num = 0;
	UINT32 priority_offset = 0;
	UINT32 max_priority = 0;
	int max_pri_obj_idx = 0;
	int width = 0, height = 0;
	int area = 0;
	int frame_counter_th = algo->p->pos_stop_count_th;
	UINT32 priority_list[MPI_IVA_MAX_OBJ_NUM] = {};
	MPI_IVA_OBJ_LIST_S *dst = &info->obj_list;

	int pos_stop_count_th = algo->p->pos_stop_count_th;
	int neg_classify_period = algo->p->neg_classify_period;
	int pos_classify_period = algo->p->pos_classify_period;
	const MPI_IVA_OBJ_ATTR_S *src_obj = NULL;
	;
	const Fraction *min_rate = &algo->min_fr_ratio;
	const Fraction *max_rate = &algo->max_fr_ratio;
	const Fraction *face_rate = &algo->face_prio_rate;

	dst->timestamp = src->timestamp;

	EaifObjAttrEx *obj_attr = NULL;

	if (!status->obj_cnt) {
		status->obj_exist_any = 0;
		status->obj_exist_any_counter = 0;
		return appendable;
	}

	status->obj_exist_any_counter += status->obj_exist_any;
	if (algo->p->obj_exist_classify_period && status->obj_exist_any_counter <= algo->p->obj_exist_classify_period) {
		return appendable;
	}

	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		src_obj = &src->obj[i];
		if (src_obj->life >= algo->p->obj_life_th) {
			obj_attr = &status->obj_attr_ex[i];

			priority_list[obj_num] = 0;

			if (obj_attr->confid_counter >= pos_stop_count_th) {
				obj_num++;
				continue;
			}

			width = src_obj->rect.ex - src_obj->rect.sx + 1;
			height = src_obj->rect.ex - src_obj->rect.sx + 1;
			area = width * height;

			// allow for face detection but do not do face recognition
			// check min box ratio
			if (width * min_rate->denominator < height * min_rate->numerator) {
				obj_num++;
				continue;
			}

			// check max box ratio
			if (width * max_rate->denominator > height * max_rate->numerator) {
				obj_num++;
				continue;
			}

			if (obj_attr->basic.label_num) {
				if (!strcmp(obj_attr->basic.category[0], "unknown") ||
				    !strcmp(obj_attr->basic.category[0], "small_face")) { // neg result
					frame_counter_th = neg_classify_period;
					priority_offset = algo->neg_obj_prio + algo->life_prio * src_obj->life +
					                  area * face_rate->numerator / face_rate->denominator;
				} else { // pos result
					frame_counter_th = pos_classify_period;
					priority_offset = algo->pos_face_prio;
				}
			} else { // no face
				frame_counter_th = algo->p->neg_classify_period;
				priority_offset = algo->neg_obj_prio + algo->life_prio * src_obj->life +
				                  area * face_rate->numerator / face_rate->denominator;
			}

			if (obj_attr->stage == 1) { // to be ready to run recognition
				priority_list[obj_num++] = append_priority_max;
				max_pri_obj_idx = i;
				appendable = 1;
				break;
			}

			// prio on new obj
			if (!obj_attr->infer_counter) {
				obj_attr->stage = 0;
				priority_list[obj_num] = algo->new_obj_prio + algo->life_prio * src_obj->life +
				                         area * face_rate->numerator / face_rate->denominator;
				obj_num++;
				appendable = 1;
				// prio on frame count th
			} else if (obj_attr->frame_counter > frame_counter_th) {
				eaif_log_debug("APPEND CNT: %d, id:%d, frame:confid:infer(%d,%d,%d)", i,
				               obj_attr->basic.id, obj_attr->frame_counter, obj_attr->confid_counter,
				               obj_attr->infer_counter);
				priority_list[obj_num] =
				        abs(obj_attr->frame_counter - frame_counter_th) + priority_offset;
				obj_num++;
				appendable = 1;
			}

			if (max_priority < priority_list[obj_num - 1]) {
				max_priority = priority_list[obj_num - 1];
				max_pri_obj_idx = i;
			}
		}
	}

	if (info->inf_fr_stage == 1) {
		appendable &= (info->inf_fr_counter > algo->fr_stage1_wait);
	} else if (info->inf_fr_stage == 2 || info->inf_fr_stage == 0) {
		appendable &= (info->inf_fr_counter > algo->fr_stage2_wait);
	} else {
		eaif_log_warn("Enter invalid stage (%d)!", info->inf_fr_stage);
	}

	info->inf_fr_counter++;

	dst->obj[0] = src->obj[max_pri_obj_idx];
	dst->obj_num = 1;

	// reset counter if and only if we can infer an object
	if (appendable) {
		info->inf_fr_counter = 0;
		status->obj_exist_any_counter = 0;
	}

#if EAIF_DEBUG_INFO
	printf("[%s %d]: append:%d obj cnt: %d (id,confid:%d,frame,infer)", __func__, __LINE__, appendable, obj_num,
	       algo->p->pos_stop_count_th);
	for (int i = 0; (UINT32)i < status->obj_cnt; i++) {
		obj_attr = &status->obj_attr_ex[i];
		printf("(%d,%d,%d,%d),", obj_attr->basic.id, obj_attr->confid_counter, obj_attr->frame_counter,
		       obj_attr->infer_counter);
	}
	printf("\n");
#endif
	appendable &= info->req_sta.avail;

	if (appendable) {
		info->inf_fr_stage = status->obj_attr_ex[max_pri_obj_idx].stage;
	}

	return appendable;
}

///////////////////////////////////////////////////
/// object list
/// status copy utility functions
///////////////////////////////////////////////////

void eaif_copyObjList(const MPI_IVA_OBJ_LIST_S *src, MPI_IVA_OBJ_LIST_S *dst)
{
	dst->timestamp = src->timestamp;
	dst->obj_num = src->obj_num;
	memcpy(dst->obj, src->obj, sizeof(MPI_IVA_OBJ_ATTR_S) * src->obj_num);
}

void eaif_copyScaledObjList(const EaifFixedPointSize *scale_factor, const MPI_IVA_OBJ_LIST_S *src,
                            MPI_IVA_OBJ_LIST_S *dst)
{
	if (scale_factor->width == g_fixed_point_sf_one && scale_factor->height == g_fixed_point_sf_one) {
		eaif_copyObjList(src, dst);
		return;
	}

	dst->timestamp = src->timestamp;
	dst->obj_num = src->obj_num;
	MPI_IVA_OBJ_ATTR_S *left = NULL;
	const MPI_IVA_OBJ_ATTR_S *right = NULL;

	for (int i = 0; i < src->obj_num; i++) {
		right = &src->obj[i];
		left = &dst->obj[i];
		left->id = right->id;
		left->life = right->life;
		left->mv = right->mv;
		left->rect.sx = ((int)right->rect.sx * scale_factor->width) >> EAIF_FIXED_POINT_BS;
		left->rect.ex = ((int)right->rect.ex * scale_factor->width) >> EAIF_FIXED_POINT_BS;
		left->rect.sy = ((int)right->rect.sy * scale_factor->height) >> EAIF_FIXED_POINT_BS;
		left->rect.ey = ((int)right->rect.ey * scale_factor->height) >> EAIF_FIXED_POINT_BS;
	}
}

int eaif_copyScaledListWithBoundary(const MPI_SIZE_S *resoln, const EaifFixedPointSize *scale_factor,
                                    const MPI_IVA_OBJ_LIST_S *src, MPI_IVA_OBJ_LIST_S *dst)
{
#define MIN_OBJ_SIZE (10)

	if (!resoln || !scale_factor || !src || !dst)
		return -EFAULT;

	dst->timestamp = src->timestamp;
	dst->obj_num = 0;
	MPI_IVA_OBJ_ATTR_S *left = NULL;
	const MPI_IVA_OBJ_ATTR_S *right = NULL;

	for (int i = 0; i < src->obj_num; i++) {
		right = &src->obj[i];

		MPI_RECT_POINT_S rect = right->rect;
		rect.sx = clamp(rect.sx, 0, rect.ex);
		rect.sy = clamp(rect.sy, 0, rect.ey);
		rect.ex = clamp(rect.ex, rect.sx, resoln->width);
		rect.ey = clamp(rect.ey, rect.sy, resoln->height);
		int obj_w = rect.ex - rect.sx + 1;
		int obj_h = rect.ey - rect.sy + 1;

		if (obj_w < MIN_OBJ_SIZE || obj_h < MIN_OBJ_SIZE)
			continue;

		left = &dst->obj[dst->obj_num];
		left->id = right->id;
		left->life = right->life;
		left->mv = right->mv;
		left->rect.sx = ((int)rect.sx * scale_factor->width) >> EAIF_FIXED_POINT_BS;
		left->rect.ex = ((int)rect.ex * scale_factor->width) >> EAIF_FIXED_POINT_BS;
		left->rect.sy = ((int)rect.sy * scale_factor->height) >> EAIF_FIXED_POINT_BS;
		left->rect.ey = ((int)rect.ey * scale_factor->height) >> EAIF_FIXED_POINT_BS;
		dst->obj_num++;
	}
	return 0;
}

int eaif_cpyInternalStatus(const EaifStatusInternal *src, EaifStatusInternal *dst)
{
	dst->timestamp = src->timestamp;
	dst->server_reachable = src->server_reachable;
	dst->obj_cnt = src->obj_cnt;
	memcpy(dst->obj_attr_ex, src->obj_attr_ex, sizeof(EaifObjAttrEx) * src->obj_cnt);

	return 0;
}

/**
 * @brief Merge destination status with souce status
 * @details Keep all elements in destination status and merge result in source to it.
			1. If something only exist in destination, we keep it.
			2. If something exist in both source and destination, we merge source to destination.
			3. If something only exist in source, we ignore it.
			Note that we would preserve the destination frame counter and rectangle.
 * @param[in]  src        	source status
 * @param[out] dst			destination status
 * @return The execution result.
 * @retval 0                           success.
 */
int eaif_mergeInternalStatus(const EaifStatusInternal *src, EaifStatusInternal *dst)
{
	const EaifObjAttrEx *src_obj = NULL;
	EaifObjAttrEx *dst_obj = NULL;
	int marker[MPI_IVA_MAX_OBJ_NUM] = {};

	for (int i = 0; (UINT32)i < dst->obj_cnt; i++) {
		dst_obj = &dst->obj_attr_ex[i];
		for (int j = 0; (UINT32)j < src->obj_cnt; j++) {
			if (marker[j]) {
				continue;
			}

			src_obj = &src->obj_attr_ex[j];
			if (dst_obj->basic.id == src_obj->basic.id) {
				marker[j] = 1;

				// preserve the latest rectangle and frame counter
				const MPI_RECT_POINT_S curr_rect = dst_obj->basic.rect;
				dst_obj->basic = src_obj->basic;
				dst_obj->basic.rect = curr_rect;

				dst_obj->confid_counter = src_obj->confid_counter;
				dst_obj->infer_counter = src_obj->infer_counter;
				dst_obj->stage = src_obj->stage;
			}
		}
	}
	return 0;
}

int eaif_cpyRespStatus(const EaifStatusInternal *src, EAIF_STATUS_S *dst)
{
	dst->timestamp = src->timestamp;
	dst->server_reachable = src->server_reachable;
	dst->obj_cnt = src->obj_cnt;
	for (int i = 0; (UINT32)i < src->obj_cnt; i++)
		dst->obj_attr[i] = src->obj_attr_ex[i].basic;
	return 0;
}

int eaif_cpyScaledDetectionStatus(const EaifFixedPointSize *scale_factor, const EaifStatusInternal *src,
                                  EaifStatusInternal *dst)
{
	if (scale_factor->width == g_fixed_point_sf_one && scale_factor->height == g_fixed_point_sf_one) {
		*dst = *src;
		return 0;
	}

	dst->timestamp = src->timestamp;
	dst->obj_cnt = src->obj_cnt;
	EaifObjAttrEx *left = NULL;
	const EaifObjAttrEx *right = NULL;

	for (int i = 0; (UINT32)i < src->obj_cnt; i++) {
		right = &src->obj_attr_ex[i];
		left = &dst->obj_attr_ex[i];
		left->basic.id = right->basic.id;
		left->confid_counter = right->confid_counter;
		left->infer_counter = right->infer_counter;
		left->basic.rect.sx = ((int)right->basic.rect.sx << EAIF_FIXED_POINT_BS) / scale_factor->width;
		left->basic.rect.ex = ((int)right->basic.rect.ex << EAIF_FIXED_POINT_BS) / scale_factor->width;
		left->basic.rect.sy = ((int)right->basic.rect.sy << EAIF_FIXED_POINT_BS) / scale_factor->height;
		left->basic.rect.ey = ((int)right->basic.rect.ey << EAIF_FIXED_POINT_BS) / scale_factor->height;
		left->basic.label_num = right->basic.label_num;
		/* TODO: change category/probability to non-string */
		for (int j = 0; j < left->basic.label_num; j++) {
			strncpy(left->basic.category[j], right->basic.category[j], EAIF_CHAR_LEN - 1);
			strncpy(left->basic.prob[j], right->basic.prob[j], EAIF_CHAR_LEN - 1);
		}
	}
	return 0;
}

static inline int overlap(const MPI_RECT_POINT_S *r0, const MPI_RECT_POINT_S *r1)
{
	if ((r0->ex < r1->sx) || (r0->sx > r1->ex) || (r0->ey < r1->sy) || (r0->sy > r1->ey))
		return 0;
	return 1;
}

static void assignIdByOverlapRoi(const MPI_IVA_OBJ_LIST_S *ol, EaifStatusInternal *dst)
{
	EaifObjAttrEx *left = NULL;
	const MPI_IVA_OBJ_ATTR_S *right = NULL;

	int marker[MPI_IVA_MAX_OBJ_NUM] = {};
	for (int i = 0; (UINT32)i < dst->obj_cnt; i++) {
		left = &dst->obj_attr_ex[i];
		int assign = 0;
		for (int j = 0; j < ol->obj_num; j++) {
			if (marker[j] == 1)
				continue;
			right = &ol->obj[j];
			if (overlap(&left->basic.rect, &right->rect)) {
				left->basic.id = right->id;
				marker[j] = 1;
				assign = 1;
				break;
			}
		}
		if (!assign)
			left->basic.id = 0x3fffffff;
	}
}

int eaif_cpyScaledFaceStatus(const EaifFixedPointSize *scale_factor, const EaifStatusInternal *src,
                             const MPI_IVA_OBJ_LIST_S *ol, EaifStatusInternal *dst)
{
	if (scale_factor->width == g_fixed_point_sf_one && scale_factor->height == g_fixed_point_sf_one) {
		*dst = *src;
		assignIdByOverlapRoi(ol, dst);
		return 0;
	}

	dst->timestamp = src->timestamp;
	dst->obj_cnt = src->obj_cnt;
	EaifObjAttrEx *left = NULL;
	const EaifObjAttrEx *right = NULL;

	for (int i = 0; (UINT32)i < src->obj_cnt; i++) {
		right = &src->obj_attr_ex[i];
		left = &dst->obj_attr_ex[i];
		left->basic.id = right->basic.id;
		left->confid_counter = right->confid_counter;
		left->infer_counter = right->infer_counter;
		left->basic.rect.sx = ((int)right->basic.rect.sx << EAIF_FIXED_POINT_BS) / scale_factor->width;
		left->basic.rect.ex = ((int)right->basic.rect.ex << EAIF_FIXED_POINT_BS) / scale_factor->width;
		left->basic.rect.sy = ((int)right->basic.rect.sy << EAIF_FIXED_POINT_BS) / scale_factor->height;
		left->basic.rect.ey = ((int)right->basic.rect.ey << EAIF_FIXED_POINT_BS) / scale_factor->height;
		left->basic.label_num = right->basic.label_num;
		for (int j = 0; j < left->basic.label_num; j++) {
			strncpy(left->basic.category[j], right->basic.category[j], EAIF_CHAR_LEN - 1);
			strncpy(left->basic.prob[j], right->basic.prob[j], EAIF_CHAR_LEN - 1);
		}
	}
	assignIdByOverlapRoi(ol, dst);
	return 0;
}
