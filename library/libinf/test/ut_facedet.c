#ifndef USE_NCNN
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "inf_image.h"
#include "inf_log.h"

#include "inf_ut.h"
#include "inf_face.h"

struct timespec start;

typedef struct {
	int argc;
	char **argv;
} Args;

Args g_args;

int test_case_0(InfTest *test)
{
	int argc = g_args.argc;
	char **argv = g_args.argv;
	int ret = 0;

	assert(argc == 3);
	const char *config = argv[1];
	const char *img_name = argv[2];
	// InfImage img = {.w=640, .h=360, .c=1, .data=0, .buf_owner=1, .dtype=Inf8UC1};
	// img.data = malloc(640*360);

	InfImage img = {};
	ret = Inf_Imread(img_name, &img, 0);
	testAssertMsg(ret == 0, "Cannot read image %s!", img_name);

	InfModelCtx ctx;

	// MPI_IVA_OBJ_LIST_S obj_list;
	// obj_list.timestamp = 0;
	// obj_list.obj_num = 1;
	// obj_list.obj[0].id = 3;
	// obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	// obj_list.obj[0].life = 160;

	ret = Inf_InitModel(&ctx, config);
	testAssertMsg(ret == 0, "Cannot init face detection model \"%s\"!", config);

	InfDetList result = {}, result2 = {};
	for (int i = 0; i < 1; i++) {
		TIC(start);
		ret = Inf_InvokeFaceDet(&ctx, &img, &result2);
		TOC("inference", start);
		Inf_ReleaseDetResult(&result2);
	}

	TIC(start);
	ret = Inf_InvokeFaceDet(&ctx, &img, &result);
	TOC("inference", start);

	char msg[2046] = {};
	int ind = 0;
	ind = sprintf(msg, "detFrame = {\"result\": \"%s\", \"detections\":[\n", img_name);

	for (int i = 0; i < result.size; i++) {
		ind += sprintf(
		        &msg[ind],
		        "{\"num\":%d,\"id\":%d,\"cls_no\":%d,\"cls\":%d,\"cls_name\":\"%s\",\"coord\":[%d,%d,%d,%d],"
		        "\"prob\":%.3f},\n",
		        i, result.data[i].id, result.data[i].cls_num, result.data[i].cls[0],
		        ctx.info->labels.data[result.data[i].cls[0]], result.data[i].rect.sx, result.data[i].rect.sy,
		        result.data[i].rect.ex, result.data[i].rect.ey, result.data[i].prob[result.data[i].cls[0]]);
	}

	if (result.size) {
		ind -= 2;
	}

	ind += sprintf(&msg[ind], "]}\n");
	fprintf(stderr, "%s", msg);
	ret = Inf_ReleaseDetResult(&result);
	testAssert(ret == 0);

	ret = Inf_ReleaseModel(&ctx);
	testAssert(ret == 0);

	ret = Inf_Imrelease(&img);
	testAssert(ret == 0);

	testAssert(result.size == 0 && result.data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);
	return 0;
}

int test_case_1(InfTest *test)
{
	int argc = g_args.argc;
	char **argv = g_args.argv;
	int ret = 0;

	assert(argc == 3);
	const char *config = argv[1];
	const char *img_name = argv[2];
	// InfImage img = {.w=640, .h=360, .c=1, .data=0, .buf_owner=1, .dtype=Inf8UC1};
	// img.data = malloc(640*360);

	InfImage img = {};
	ret = Inf_Imread(img_name, &img, 0);
	testAssertMsg(ret == 0, "Cannot read image %s!", img_name);

	InfModelCtx ctx;

	MPI_IVA_OBJ_LIST_S obj_list;
	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ img.w / 4, 0, (img.w * 3) / 4, img.h };
	obj_list.obj[0].life = 160;

	ret = Inf_InitModel(&ctx, config);
	testAssertMsg(ret == 0, "Cannot init face detection model \"%s\"!", config);

	InfDetList result = {}, result2 = {};
	for (int i = 0; i < 1; i++) {
		TIC(start);
		ret = Inf_InvokeFaceDetObjList(&ctx, &img, &obj_list, &result2);
		TOC("inference", start);
		Inf_ReleaseDetResult(&result2);
	}

	TIC(start);
	ret = Inf_InvokeFaceDetObjList(&ctx, &img, &obj_list, &result);
	TOC("inference", start);

	char msg[2046] = {};
	int ind = 0;
	ind = sprintf(msg, "detObjList = {\"result\": \"%s\", \"detections\":[\n", img_name);

	for (int i = 0; i < result.size; i++) {
		ind += sprintf(
		        &msg[ind],
		        "{\"num\":%d,\"id\":%d,\"cls_no\":%d,\"cls\":%d,\"cls_name\":\"%s\",\"coord\":[%d,%d,%d,%d],"
		        "\"prob\":%.3f},\n",
		        i, result.data[i].id, result.data[i].cls_num, result.data[i].cls[0],
		        ctx.info->labels.data[result.data[i].cls[0]], result.data[i].rect.sx, result.data[i].rect.sy,
		        result.data[i].rect.ex, result.data[i].rect.ey, result.data[i].prob[result.data[i].cls[0]]);
	}

	if (result.size) {
		ind -= 2;
	}

	ind += sprintf(&msg[ind], "]}\n");
	fprintf(stderr, "%s", msg);

	ret = Inf_ReleaseDetResult(&result);
	testAssert(ret == 0);

	ret = Inf_ReleaseModel(&ctx);
	testAssert(ret == 0);

	ret = Inf_Imrelease(&img);
	testAssert(ret == 0);

	testAssert(result.size == 0 && result.data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);
	return 0;
}

int main(int argc, char **argv)
{
	g_args.argc = argc;
	g_args.argv = argv;
	REGISTER_TEST(test_case_0);
	REGISTER_TEST(test_case_1);
	TEST_RUN();
}
#endif