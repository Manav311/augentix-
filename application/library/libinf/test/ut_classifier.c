#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "inf_image.h"
#include "inf_log.h"

#include "inf_ut.h"
#include "inf_classifier.h"

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

	MPI_IVA_OBJ_LIST_S obj_list;
	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	obj_list.obj[0].life = 160;

	Inf_InitModel(&ctx, config);

	InfResultList result = {}, result2 = {};
	for (int i = 0; i < 10; i++) {
		TIC(start);
		ret = Inf_InvokeClassify(&ctx, &img, &obj_list, &result2);
		TOC("inference", start);
		Inf_ReleaseResult(&result2);
	}

	TIC(start);
	ret = Inf_InvokeClassify(&ctx, &img, &obj_list, &result);
	TOC("inference", start);

	if (result.size && result.data[0].cls_num)
		printf("Result: %s : #%d [id:%d cls_no:%d, cls:%d(%s) prob:%.3f]\n", img_name, result.size, result.data[0].id,
	       result.data[0].cls_num, result.data[0].cls[0], ctx.info->labels.data[result.data[0].cls[0]],
	       result.data[0].prob[result.data[0].cls[0]]);
	else
		printf("Result: %s : does not pass threshold %.4f\n", img_name, ctx.info->conf_thresh.data[0]);

	ret = Inf_ReleaseResult(&result);
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
	TEST_RUN();

	// assert(argc == 3);
	// const char *config = argv[1];
	// const char* img_name = argv[2];
	// InfImage img = {.w=640, .h=360, .c=1, .data=0, .buf_owner=1, .dtype=Inf8UC1};
	// img.data = malloc(640*360);

	// InfClassifierCtx ctx;

	// MPI_IVA_OBJ_LIST_S obj_list;
	// obj_list.timestamp = 0;
	// obj_list.obj_num = 1;
	// obj_list.obj[0].id = 3;
	// obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	// obj_list.obj[0].life = 160;

	// Inf_InitModel(&ctx, config);

	// InfResultList result = {}, result2 = {};
	// for (int i = 0; i < 10; i++) {
	// 	TIC(start);
	// 	ret = Inf_Invoke(&ctx, &img, &obj_list, &result2);
	// 	TOC("inference", start);
	// 	Inf_ReleaseResult(&result2);
	// }

	// TIC(start);
	// ret = Inf_Invoke(&ctx, &img, &obj_list, &result);
	// TOC("inference", start);

	// ret = Inf_ReleaseResult(&result);
	// //testAssert(ret == 0);

	// ret = Inf_ReleaseModel(&ctx);
	// //testAssert(ret == 0);

	// ret = Inf_Imrelease(&img);
	// //testAssert(ret == 0);

	// //testAssert(result.Size == 0 && result.data == 0);
	// //testAssert(ctx.model == 0 && ctx.info == 0);
	return 0;
}