#include <stdio.h>

#include "eaif_common.h"
#include "eaif_trc.h"

#include "eaif_c_common.h"
#include "eaif_c_engine.h"
#include "eaif_c_image.h"
#include "eaif_c_test.h"

#define TEST_IMAGE "../../data/face/obama.jpg"
#define TEST_IMAGE_C4 "../../data/00006.jpg"
#define TEST_IMAGE_ShuffleNet "../../data/choi1.jpg"
#define TEST_IMAGE_ShuffleNetBin "../../data/choi1.bin"
#define TEST_CONFIG_C4 "configC4.ini"
#define TEST_CONFIG_ShuffleNet "configShuffleNet.ini"
#define TEST_API_C4 "C4"
#define TEST_API_ShuffleNet "shuffleNetV2"

static int read_img_file(const char *filename, unsigned char **data)
{
	FILE *fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, 0);
	*data = (unsigned char *)malloc(sizeof(char) * size);
	int ret = fread(*data, 1, size, fp);
	fclose(fp);
	if (!ret)
		return 0;
	return size;
}

EAIF_Image *read_imgbin_file(const char *filename)
{
	int w = 0, h = 0, c = 0, dtype = 0;
	unsigned char *data = 0;
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return 0;
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, 0);
	data = (unsigned char *)malloc(sizeof(char) * (size - 4 * 4));
	int ret __attribute__((unused)) = 0;
	ret = fread(&h, 4, 1, fp);
	ret = fread(&w, 4, 1, fp);
	ret = fread(&c, 4, 1, fp);
	ret = fread(&dtype, 4, 1, fp);
	ret = fread(data, 1, size - 4 * 4, fp);
	fclose(fp);
	return eaif_createImageBuffer(w, h, c, dtype, data);
}

// test image creat and destroy
int test_image_createDestroy(TestSuit *suit)
{
	EAIF_Image *a __attribute__((__unused__));
	EAIF_Image *b = eaif_createImage();
	eaif_destroyImage(&b);
	testAssert(b == 0);
	return 0;
}

// test image
// 1. imreadptr
// 2. create from buffer
// 3. imdecode
int test_image_imread(TestSuit *suit)
{
	EAIF_Image *a = eaif_ImreadPtr(TEST_IMAGE);
	int w, h, c;
	unsigned char *data = 0;
	eaif_imageGetInfo(a, &h, &w, &c, &data);
	testLog("Image read ptr :%s info : %dx%dx%d %p\n", TEST_IMAGE, h, w, c, data);

	EAIF_Image *b = eaif_createImageBuffer(w, h, c, Eaif8UC3, data);
	eaif_destroyImage(&b);
	eaif_destroyImage(&a);

	testAssert(b == 0);
	testAssert(a == 0);

	int size = read_img_file(TEST_IMAGE, &data);
	testAssert(size);
	EAIF_Image *d = eaif_createImage();
	eaif_Imdecode(data, size, Eaif8UC3, d, 3);

	unsigned char *pdata = 0;
	eaif_imageGetInfo(d, &h, &w, &c, &pdata);
	testLog("Image decode :%s info : %dx%dx%d %p\n", TEST_IMAGE, h, w, c, pdata);
	eaif_destroyImage(&d);
	// if do not segmentation fault
	testAssert(data[h * w * c - 1] != 9999999);
	testAssert(d == 0);
	free(data);
	data = NULL;
	return 0;
}

// test engine
// 1. creation
// 2. setup config
// 3. query model info
// 4. destroy engine
int test_createEngine(TestSuit *suit)
{
	EAIF_Engine *engine = eaif_createEngine();
	int ret = eaif_clearEngine(engine);
	testAssert(ret == 0);

	ret = eaif_setupEngine(engine, TEST_CONFIG_C4);
	testAssert(ret == 0);

	EAIF_StrList model_names = {};
	char msg[512] = {};
	int size = 0;

	eaif_queryEngineModelNames(engine, EAIF_Classify, &model_names);
	for (int i = 0; i < model_names.size; i++) {
		size += sprintf(&msg[size], "%s,", model_names.str[i]);
	}
	eaif_clearStrList(&model_names);
	testAssert(model_names.size == 0 && model_names.str == 0);

	eaif_queryEngineModelNames(engine, EAIF_Detect, &model_names);
	for (int i = 0; i < model_names.size; i++) {
		size += sprintf(&msg[size], "%s,", model_names.str[i]);
	}
	eaif_clearStrList(&model_names);
	testAssert(model_names.size == 0 && model_names.str == 0);

	eaif_queryEngineModelNames(engine, EAIF_FaceReco, &model_names);
	for (int i = 0; i < model_names.size; i++) {
		size += sprintf(&msg[size], "%s,", model_names.str[i]);
	}
	eaif_clearStrList(&model_names);
	testAssert(model_names.size == 0 && model_names.str == 0);

	testLog("Model info:%s\n", msg);
	testAssert(model_names.size == 0);
	ret = eaif_destroyEngine(&engine);
	testAssert(ret == 0 && engine == 0);
	return 0;
}

// test inference
// 1. create and setup config
// 2. imread from ptr
// 3. query model info
// 4. inference by model index
// 5. decode result
int test_inferenceC4(TestSuit *suit)
{
	EAIF_Engine *engine = eaif_createEngine();

	int ret = eaif_setupEngine(engine, TEST_CONFIG_C4);
	testAssert(ret == 0);

	EAIF_Image *img = eaif_ImreadPtr(TEST_IMAGE_C4);
	testAssert(img);

	int w, h, c;
	unsigned char *data = 0;
	eaif_imageGetInfo(img, &h, &w, &c, &data);
	testLog("Test Image:%s info : %dx%dx%d %p\n", TEST_IMAGE_C4, h, w, c, data);

	EAIF_ModelInfo *info = eaif_createModelInfo();
	eaif_queryEngineModelInfo(engine, TEST_API_C4, info);
	testAssert(ret == 0);

	EAIF_ClassifyList cls_list = {};
	EAIF_ObjList obj_list = {};
	obj_list.obj = (EAIF_ObjectAttr *)malloc(sizeof(EAIF_ObjectAttr) * 1);
	obj_list.size = 1;
	obj_list.obj[0].box = (EAIF_Rect){ { { 0, 0, w - 1, h - 1 } } };
	obj_list.obj[0].idx = 0;

	struct timespec start;
	TIC(start);
	ret = eaif_engineClassifyObjList(engine, eaif_getModelInfoIdx(info), img, &obj_list, &cls_list);
	TOC("C4 Inference for imread", start);
	testAssert(ret == 0);
	testAssert(cls_list.size);
	int cls = cls_list.obj[0].cls[0];
	testLog("Test Classification result: %d %.4f\n", cls, cls_list.obj[0].prob[cls]);
	eaif_clearClassifyList(&cls_list);
	eaif_destroyImage(&img);

	unsigned char *buf = 0;
	int size = read_img_file(TEST_IMAGE_C4, &buf);
	testAssert(size);
	EAIF_Image *de_img = eaif_createImage();
	eaif_Imdecode(buf, size, Eaif8UC1, de_img, 1);
	TIC(start);
	ret = eaif_engineClassifyObjList(engine, eaif_getModelInfoIdx(info), de_img, &obj_list, &cls_list);
	TOC("C4 Inference for imdecode", start);
	testAssert(ret == 0);
	testAssert(cls_list.size);
	cls = cls_list.obj[0].cls[0];
	testLog("Test Classification result: %d %.4f\n", cls, cls_list.obj[0].prob[cls]);
	free(buf);
	buf = 0;
	eaif_destroyModelInfo(&info);
	eaif_destroyImage(&de_img);
	eaif_clearObjList(&obj_list);
	eaif_clearClassifyList(&cls_list);
	eaif_destroyEngine(&engine);

	testAssert(obj_list.size == 0 && obj_list.obj == 0);
	testAssert(cls_list.size == 0 && cls_list.obj == 0);
	testAssert(info == 0);
	testAssert(de_img == 0) testAssert(img == 0);
	testAssert(engine == 0);
	return 0;
}

// test inference
// 1. create and setup config
// 2. imread from ptr
// 3. query model info
// 4. inference by model index
// 5. decode result
int test_inferenceShuffleNet(TestSuit *suit)
{
	EAIF_Engine *engine = eaif_createEngine();

	int ret = eaif_setupEngine(engine, TEST_CONFIG_ShuffleNet);
	testAssert(ret == 0);

	EAIF_Image *img = eaif_ImreadPtr(TEST_IMAGE_ShuffleNet);
	testAssert(img);

	int w, h, c;
	unsigned char *data = 0;
	eaif_imageGetInfo(img, &h, &w, &c, &data);
	testLog("Test Image:%s info : %dx%dx%d %p\n", TEST_IMAGE_ShuffleNet, h, w, c, data);

	EAIF_ModelInfo *info = eaif_createModelInfo();
	eaif_queryEngineModelInfo(engine, TEST_API_ShuffleNet, info);
	testAssert(ret == 0);

	EAIF_ClassifyList cls_list = {};
	EAIF_ObjList obj_list = {};
	obj_list.obj = (EAIF_ObjectAttr *)malloc(sizeof(EAIF_ObjectAttr) * 1);
	obj_list.size = 1;
	obj_list.obj[0].box = (EAIF_Rect){ { { 0, 0, w - 1, h - 1 } } };
	obj_list.obj[0].idx = 0;

	struct timespec start;
	TIC(start);
	ret = eaif_engineClassifyObjList(engine, eaif_getModelInfoIdx(info), img, &obj_list, &cls_list);
	TOC("ShuffleNet Inference for imread", start);
	testAssert(ret == 0);
	testAssert(cls_list.size);
	int cls = cls_list.obj[0].cls[0];
	testLog("Test Classification result: %d %.4f\n", cls, cls_list.obj[0].prob[cls]);
	eaif_clearClassifyList(&cls_list);
	eaif_destroyImage(&img);

	eaif_destroyModelInfo(&info);
	eaif_clearObjList(&obj_list);
	eaif_clearClassifyList(&cls_list);
	eaif_destroyEngine(&engine);

	testAssert(obj_list.size == 0 && obj_list.obj == 0);
	testAssert(cls_list.size == 0 && cls_list.obj == 0);
	testAssert(info == 0);
	testAssert(img == 0);
	testAssert(engine == 0);
	return 0;
}

int test_inferenceShuffleNetBin(TestSuit *suit)
{
	EAIF_Engine *engine = eaif_createEngine();

	int ret = eaif_setupEngine(engine, TEST_CONFIG_ShuffleNet);
	testAssert(ret == 0);

	EAIF_Image *img = read_imgbin_file(TEST_IMAGE_ShuffleNetBin);
	testAssert(img);

	int w, h, c;
	unsigned char *data = 0;
	eaif_imageGetInfo(img, &h, &w, &c, &data);
	testLog("Test Image:%s info : %dx%dx%d %p\n", TEST_IMAGE_ShuffleNetBin, h, w, c, data);

	EAIF_ModelInfo *info = eaif_createModelInfo();
	eaif_queryEngineModelInfo(engine, TEST_API_ShuffleNet, info);
	testAssert(ret == 0);

	EAIF_ClassifyList cls_list = {}, cls_list2 = {};
	EAIF_ObjList obj_list = {};
	obj_list.obj = (EAIF_ObjectAttr *)malloc(sizeof(EAIF_ObjectAttr) * 1);
	obj_list.size = 1;
	obj_list.obj[0].box = (EAIF_Rect){ { { 0, 0, w - 1, h - 1 } } };
	obj_list.obj[0].idx = 0;

	struct timespec start;
	TIC(start);
	ret = eaif_engineClassifyObjList(engine, eaif_getModelInfoIdx(info), img, &obj_list, &cls_list2);
	TOC("ShuffleNet Inference for imread", start);
	testAssert(ret == 0);
	testAssert(cls_list2.size);
	int cls = cls_list2.obj[0].cls[0];
	testLog("Test Classification result: %d %.4f\n", cls, cls_list2.obj[0].prob[cls]);
	eaif_clearClassifyList(&cls_list2);

	TIC(start);
	ret = eaif_engineClassifyObjList(engine, eaif_getModelInfoIdx(info), img, &obj_list, &cls_list);
	TOC("ShuffleNet Inference for imread", start);
	testAssert(ret == 0);
	testAssert(cls_list.size);
	cls = cls_list.obj[0].cls[0];
	testLog("Test Classification result: %d %.4f\n", cls, cls_list.obj[0].prob[cls]);

	eaif_clearClassifyList(&cls_list);
	eaif_destroyImage(&img);
	free(data);
	eaif_destroyModelInfo(&info);
	eaif_clearObjList(&obj_list);
	eaif_clearClassifyList(&cls_list);
	eaif_destroyEngine(&engine);

	testAssert(obj_list.size == 0 && obj_list.obj == 0);
	testAssert(cls_list.size == 0 && cls_list.obj == 0);
	testAssert(info == 0);
	testAssert(img == 0);
	testAssert(engine == 0);
	return 0;
}

int register_test(void)
{
	TestSuit *test = TestSuit_create();
	//TestSuit_add(test, test_image_createDestroy);
	//TestSuit_add(test, test_image_imread);
	//TestSuit_add(test, test_createEngine);
	//TestSuit_add(test, test_inferenceC4);
	//TestSuit_add(test, test_inferenceShuffleNet);
	TestSuit_add(test, test_inferenceShuffleNetBin);
	TestSuit_run(test);
	TestSuit_report(test);
	TestSuit_free(&test);
	return 0;
}

int main()
{
	return register_test();
}
