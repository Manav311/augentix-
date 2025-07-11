#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "eaif_trc.h"
#include "eaif_c_test.h"
#include "lite_classifier.h"

#define TEST_IMAGE_OBAMA "../../data/face/obama.bin"
#define TEST_IMAGE_CHOI "../../data/choi1.bin"
#define TEST_CONFIG_ShuffleNet "configShuffleNet.ini"
#define TEST_IMAGE_CHOIS "../../data/choi2.bin"

static int read_imgbin_file(const char *filename, LiteImage *img)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return -1;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, 0);
	img->data = (unsigned char *)malloc(sizeof(char) * (size - 4 * 4));
	int ret = 0;
	ret = fread(&img->w, 4, 1, fp);
	ret = fread(&img->h, 4, 1, fp);
	ret = fread(&img->c, 4, 1, fp);
	ret = fread(&img->dtype, 4, 1, fp);
	ret = fread(img->data, 1, size - 4 * 4, fp);
	fclose(fp);
	img->buf_owner = 1;
	return ret;
}

int test_minimum_inference(TestSuit *suit)
{
	struct timespec start;
	int ret = 0;
	LiteImage img = {};
	LiteResultList result = {};
	LiteResultList result2 = {};
	LiteClassifierCtx ctx = {};
	MPI_IVA_OBJ_LIST_S obj_list = {};

	read_imgbin_file(TEST_IMAGE_CHOI, &img);

	ret = Lite_InitModel(&ctx, TEST_CONFIG_ShuffleNet);
	testAssert(ret == 0);

	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	obj_list.obj[0].life = 160;
	for (int i = 0; i < 10; i++) {
		TIC(start);
		ret = Lite_Invoke(&ctx, &img, &obj_list, &result2);
		TOC("inference", start);
		Lite_ReleaseResult(&result2);
	}
	TIC(start);
	ret = Lite_Invoke(&ctx, &img, &obj_list, &result);
	TOC("inference", start);
	testAssert(ret == 0);
	testAssert(result.size == 1);

	printf("Result: %s : #%d [id:%d cls_no:%d, cls:%d(%s) prob:%.3f]\n", TEST_IMAGE_CHOI, result.size,
	       result.data[0].id, result.data[0].cls_num, result.data[0].cls[0],
	       ctx.info->labels.data[result.data[0].cls[0]], result.data[0].prob[result.data[0].cls[0]]);

	ret = Lite_ReleaseResult(&result);
	testAssert(ret == 0);

	ret = Lite_ReleaseModel(&ctx);
	testAssert(ret == 0);

	free(img.data);
	testAssert(result.size == 0 && result.data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);

	//testAssert(obj_list.size == 0 && obj_list.obj == 0);
	//testAssert(cls_list.size == 0 && cls_list.obj == 0);
	//testAssert(info == 0);
	//testAssert(img == 0);
	//testAssert(engine == 0);
	return 0;
}

int test_minimum_inferenceS(TestSuit *suit)
{
	struct timespec start;
	int ret = 0;
	LiteImage img = {};
	LiteResultList result = {};
	LiteResultList result2 = {};
	LiteClassifierCtx ctx = {};
	MPI_IVA_OBJ_LIST_S obj_list = {};

	read_imgbin_file(TEST_IMAGE_CHOIS, &img);

	ret = Lite_InitModel(&ctx, TEST_CONFIG_ShuffleNet);
	testAssert(ret == 0);

	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	obj_list.obj[0].life = 160;
	for (int i = 0; i < 10; i++) {
		TIC(start);
		ret = Lite_Invoke(&ctx, &img, &obj_list, &result2);
		TOC("inference", start);
		Lite_ReleaseResult(&result2);
	}
	TIC(start);
	ret = Lite_Invoke(&ctx, &img, &obj_list, &result);
	TOC("inference", start);
	testAssert(ret == 0);
	testAssert(result.size == 1);

	printf("Result: %s : #%d [id:%d cls_no:%d, cls:%d(%s) prob:%.3f]\n", TEST_IMAGE_CHOI, result.size,
	       result.data[0].id, result.data[0].cls_num, result.data[0].cls[0],
	       ctx.info->labels.data[result.data[0].cls[0]], result.data[0].prob[result.data[0].cls[0]]);

	ret = Lite_ReleaseResult(&result);
	testAssert(ret == 0);

	ret = Lite_ReleaseModel(&ctx);
	testAssert(ret == 0);

	free(img.data);
	testAssert(result.size == 0 && result.data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);

	//testAssert(obj_list.size == 0 && obj_list.obj == 0);
	//testAssert(cls_list.size == 0 && cls_list.obj == 0);
	//testAssert(info == 0);
	//testAssert(img == 0);
	//testAssert(engine == 0);
	return 0;
}

int test_minimum_inferenceS1(TestSuit *suit)
{
	struct timespec start;
	int ret = 0;
	LiteImage img = {};
	LiteResultList result = {};
	LiteResultList result2 = {};
	LiteClassifierCtx ctx = {};
	MPI_IVA_OBJ_LIST_S obj_list = {};

	read_imgbin_file(TEST_IMAGE_CHOIS, &img);

	ret = Lite_InitModel(&ctx, TEST_CONFIG_ShuffleNet);
	Lite_Setup(&ctx, 1, 1, 1);
	testAssert(ret == 0);

	obj_list.timestamp = 0;
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 3;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	obj_list.obj[0].life = 160;
	for (int i = 0; i < 10; i++) {
		TIC(start);
		ret = Lite_Invoke(&ctx, &img, &obj_list, &result2);
		TOC("inference", start);
		Lite_ReleaseResult(&result2);
	}
	TIC(start);
	ret = Lite_Invoke(&ctx, &img, &obj_list, &result);
	TOC("inference", start);
	testAssert(ret == 0);
	testAssert(result.size == 1);

	printf("Result: %s : #%d [id:%d cls_no:%d, cls:%d(%s) prob:%.3f]\n", TEST_IMAGE_CHOI, result.size,
	       result.data[0].id, result.data[0].cls_num, (result.data[0].cls_num)? result.data[0].cls[0]: -1,
	       (result.data[0].cls_num)? ctx.info->labels.data[result.data[0].cls[0]]:"", (result.data[0].cls_num)? result.data[0].prob[result.data[0].cls[0]]:-1);

	ret = Lite_ReleaseResult(&result);
	testAssert(ret == 0);

	ret = Lite_ReleaseModel(&ctx);
	testAssert(ret == 0);

	free(img.data);
	testAssert(result.size == 0 && result.data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);
	return 0;
	//testAssert(obj_list.size == 0 && obj_list.obj == 0);
	//testAssert(cls_list.size == 0 && cls_list.obj == 0);
	//testAssert(info == 0);
	//testAssert(img == 0);
	//testAssert(engine == 0);
}

int register_test(void)
{
	TestSuit *test = TestSuit_create();
	//TestSuit_add(test, test_minimum_inference);
	//TestSuit_add(test, test_minimum_inferenceS);
	TestSuit_add(test, test_minimum_inferenceS1);
	TestSuit_run(test);
	TestSuit_report(test);
	TestSuit_free(&test);
	return 0;
}

int main()
{
	//return 0;
	return register_test();
}
