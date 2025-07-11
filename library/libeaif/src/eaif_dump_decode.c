#include "eaif_dump_decode.h"
#include "eaif_dump_define.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>

#include "eaif.h"
#include "inf_types.h"
#include "mpi_iva.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) < (y)) ? (x) : (y))

static void __decode_MPI_IVA_OBJ_ATTR_S(const MPI_IVA_OBJ_ATTR_S *tmp, const char *prefix)
{
	printf("%sid = %d\n", prefix, (int)tmp->id);
	printf("%slife = %d\n", prefix, (int)tmp->life);
	printf("%srect.sx = %d\n", prefix, (int)tmp->rect.sx);
	printf("%srect.sy = %d\n", prefix, (int)tmp->rect.sy);
	printf("%srect.ex = %d\n", prefix, (int)tmp->rect.ex);
	printf("%srect.ey = %d\n", prefix, (int)tmp->rect.ey);
	printf("%smv.x = %d\n", prefix, (int)tmp->mv.x);
	printf("%smv.y = %d\n", prefix, (int)tmp->mv.y);
	printf("%scat = %d\n", prefix, (int)tmp->cat);
	printf("%sconf = %d\n", prefix, (int)tmp->conf);
}

static void __decode_MPI_IVA_OBJ_LIST_S(const MPI_IVA_OBJ_LIST_S *tmp, const char *prefix)
{
	printf("%stimestamp = %u\n", prefix, (unsigned int)tmp->timestamp);
	printf("%sobj_num = %d\n", prefix, (int)tmp->obj_num);
	int num = MIN(tmp->obj_num, MPI_IVA_MAX_OBJ_NUM);
	int i = 0;

	for (i = 0; i < num; ++i) {
		char buf[32] = { 0 };
		snprintf(buf, sizeof(buf), "%sobj[%d].", prefix, i);
		__decode_MPI_IVA_OBJ_ATTR_S(&tmp->obj[i], buf);
	}
}

static int decode_MPI_IVA_OBJ_LIST_S(const void *args, int count)
{
	const MPI_IVA_OBJ_LIST_S *tmp = args;
	assert(sizeof(MPI_IVA_OBJ_LIST_S) == count);
	printf("MPI_IVA_OBJ_LIST_S:\n");
	__decode_MPI_IVA_OBJ_LIST_S(tmp, "");
	return 0;
}

static int decode_EAIF_INSTANCE_S(const void *args, int count)
{
	const EAIF_INSTANCE_S *tmp = args;
	(void)tmp;
	(void)count;
	return 0;
}

static int decode_EAIF_PARAM_S(const void *args, int count)
{
	const EAIF_PARAM_S *tmp = args;
	assert(sizeof(EAIF_PARAM_S) == count);
	(void)tmp;
	printf("EAIF_PARAM_S:\n");
	return 0;
}

static int decode_InfObjList(const void *args, int count)
{
	const InfObjList *tmp = args;
	assert(sizeof(InfObjList) == count);
	printf("InfObjList:\n");
	__decode_MPI_IVA_OBJ_LIST_S(tmp, "");
	return 0;
}

static void __decode_EAIF_OBJ_ATTR_S(const void *args, const char *prefix)
{
	const EAIF_OBJ_ATTR_S *tmp = args;
	printf("%sid = %d\n", prefix, tmp->id);
	printf("%slabel_num = %d\n", prefix, tmp->label_num);
	printf("%srect.sx = %d\n", prefix, (int)tmp->rect.sx);
	printf("%srect.sy = %d\n", prefix, (int)tmp->rect.sy);
	printf("%srect.ex = %d\n", prefix, (int)tmp->rect.ex);
	printf("%srect.ey = %d\n", prefix, (int)tmp->rect.ey);

	for (int i = 0; i < tmp->label_num; ++i) {
		printf("%scategory.%s = %s\n", prefix, tmp->category[i], tmp->prob[i]);
	}
}

static int decode_EAIF_STATUS_S(const void *args, int count)
{
	const EAIF_STATUS_S *tmp = args;
	assert(sizeof(EAIF_STATUS_S) == count);
	(void)tmp;
	printf("EAIF_STATUS_S:\n");
	printf("timestamp = %u\n", tmp->timestamp);
	printf("obj_cnt = %u\n", tmp->obj_cnt);

	for (int i = 0; i < (int)tmp->obj_cnt; ++i) {
		char buf[32] = { 0 };
		snprintf(buf, sizeof(buf), "obj[%d].", i);
		__decode_EAIF_OBJ_ATTR_S(&tmp->obj_attr[i], buf);
	}

	return 0;
}

static int decode_InfImage(const void *args, int count)
{
	const InfImage *tmp = args;
	assert(sizeof(InfImage) == count);

	printf("InfImage: \n");
	printf("w = %d\n", tmp->w);
	printf("h = %d\n", tmp->h);
	printf("c = %d\n", tmp->c);
	printf("data = %s\n", "next item");
	printf("buf_owner = %d\n", tmp->buf_owner);
	printf("dtype = %d\n", tmp->dtype);

	return 0;
}

static int decode_InfU8Array(const void *args, int count)
{
	printf("InfU8Array Data: not available in text format\n");
	(void)args;
	(void)count;
	return 0;
}

static int decode_InfResultList(const void *args, int count)
{
	const InfResultList *tmp = args;
	assert(sizeof(InfResultList) == count);

	printf("InfResultList Data:\n");
	printf("size = %d\n", tmp->size);
	return 0;
}

static int decode_InfResult(const void *args, int count)
{
	const InfResult *tmp = args;

	assert(sizeof(InfResult) == count);
	printf("InfResult Data:\n");
	printf("id = %d\n", tmp->id);
	printf("cls_num = %d\n", tmp->cls_num);
	printf("prob_num = %d\n", tmp->prob_num);
	printf("cls = %s\n", "next cls item");
	printf("prob = %s\n", "next prob item");
	return 0;
}

static int decode_InfIntArray(const void *args, int count)
{
	const int *tmp = args;
	int nsample = count / sizeof(int);

	printf("InfIntArray Data:\n");
	printf("InfResult.cls[] = ");
	for (int i = 0; i < nsample; ++i) {
		printf("%d, ", tmp[i]);
	}

	printf("\n");
	return 0;
}

static int decode_InfFloatArray(const void *args, int count)
{
	const float *tmp = args;
	int nsample = count / sizeof(float);

	printf("InfFloatArray Data:\n");
	printf("InfResult.prob[] = ");
	for (int i = 0; i < nsample; ++i) {
		printf("%1.03f, ", tmp[i]);
	}

	printf("\n");
	return 0;
}

static int decode_InfDetList(const void *args, int count)
{
	const InfDetList *tmp = args;
	assert(sizeof(InfDetList) == count);

	printf("InfDetList Data:\n");
	printf("size = %d\n", tmp->size);
	return 0;
}

static int decode_InfDetResult(const void *args, int count)
{
	const InfDetResult *tmp = args;

	assert(sizeof(InfDetResult) == count);
	printf("InfDetResult Data:\n");
	printf("id = %d\n", tmp->id);
	printf("rect.sx = %d\n", (int)tmp->rect.sx);
	printf("rect.sy = %d\n", (int)tmp->rect.sy);
	printf("rect.ex = %d\n", (int)tmp->rect.ex);
	printf("rect.ey = %d\n", (int)tmp->rect.ey);
	printf("cls_num = %d\n", tmp->cls_num);
	printf("prob_num = %d\n", tmp->prob_num);
	printf("confidence = %f\n", tmp->confidence);
	return 0;
}

static int decode_InfFaceImage(const void *args, int count)
{
	const InfImage *tmp = args;
	assert(sizeof(InfImage) == count);

	printf("InfFaceImage: \n");
	printf("w = %d\n", tmp->w);
	printf("h = %d\n", tmp->h);
	printf("c = %d\n", tmp->c);
	printf("data = %s\n", "next item");
	printf("buf_owner = %d\n", tmp->buf_owner);
	printf("dtype = %d\n", tmp->dtype);

	return 0;
}

static int decode_InfFaceU8Array(const void *args, int count)
{
	printf("InfFaceU8Array Data: not available in text format\n");
	(void)args;
	(void)count;
	return 0;
}

int EAIF_showDump(const VOID *buf, size_t count, UINT32 flag, TIMESPEC_S timestamp, PID_T tid)
{
	struct tm *timeinfo;
	char date_str[32];
	timeinfo = gmtime(&timestamp.tv_sec);
	strftime(date_str, sizeof(date_str), "%Y-%m-%d_%H:%M:%S", timeinfo);
	printf("[TIME: %s:%03u, TID: %lu] ", date_str, (unsigned int)timestamp.tv_nsec / 1000000, (unsigned long)tid);

#define EXPORT_DUMP(name, type)            \
	case EAIF_ID_##name: {             \
		decode_##name(buf, count); \
	} break;

	switch (flag & FLAG_MASK_ID) {
		/**
	 * The following code is 
	 * case EAIF_ID_MPI_IVA_OBJ_LIST_S : printf("%s\n", "MPI_IVA_OBJ_LIST_S"); break;
	 */
		EXPORT_EAIF_DUMP_ARRAY

#undef EXPORT_DUMP

	default:
		return -1;
	}

	return 0;
}
