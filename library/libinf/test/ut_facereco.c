#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>

#include "inf_image.h"
#include "inf_log.h"

#include "inf_ut.h"
#include "inf_face.h"
#include "inf_detect.h"

#define FACEDATA "face.bin"

struct timespec start;

typedef struct {
	int argc;
	char **argv;
} Args;

Args g_args;

int test_register_face_0(InfTest *test)
{
	int argc = g_args.argc;
	char **argv = g_args.argv;
	int ret = 0;

	testAssertMsg(argc > 2, "<inapp.ini> <img 0> <img 1> ...");
	const char *config = argv[1];

	InfModelCtx ctx;
	ret = Inf_InitModel(&ctx, config);
	testAssertMsg(ret == 0 && ctx.model, "Cannot init face reco model \"%s\"!", config);

	for (int i = 2; i < argc; i++) {
		const char *img_name = argv[i];
		int str_len = strlen(img_name);
		char fface[64] = {};
		char *face = NULL;
		strcpy(fface, img_name);
		fface[str_len - 4] = 0;
		face = basename(fface);

		InfImage img = {};
		Inf_Imread(img_name, &img, 0);
		MPI_RECT_POINT_S roi = { 0, 0, img.w - 1, img.h - 1 };
		ret = Inf_RegisterFaceRoi(&ctx, &img, &roi, face);
		testAssertMsg(ret == 0, "Cannot regis face \"%s\" from \"%s\"", face, img_name);

		Inf_Imrelease(&img);
		testAssert(img.data == 0);
	}

	testAssertMsg(ctx.info->labels.size == (argc - 2 + 1),
	              "Number of face(%d) in the database are wrong input (%d)!", ctx.info->labels.size, argc - 2 + 1);
	for (int i = 0; i < ctx.info->labels.size; i++) {
		printf("%s, ", ctx.info->labels.data[i]);
	}
	printf("\n");

	Inf_SaveFaceData(&ctx, FACEDATA);

	Inf_ReleaseModel(&ctx);
	testAssert(ret == 0);
	testAssert(ctx.model == 0);
	testAssert(ctx.info == 0);

	return 0;
}

static char *get_basename(const char *name)
{
	int str_len = strlen(name);
	static char fface[256] = {};
	char *face = NULL;
	strcpy(fface, name);
	fface[str_len - 4] = 0;
	face = basename(fface);
	return face;
}

int test_load_and_identify_0(InfTest *test)
{
	int argc = g_args.argc;
	char **argv = g_args.argv;
	int ret = 0;

	testAssertMsg(argc > 2, "<inapp.ini> <img 0> <img 1> ...");
	const char *config = argv[1];

	InfModelCtx ctx;
	ret = Inf_InitModel(&ctx, config);
	testAssertMsg(ret == 0 && ctx.model, "Cannot init face reco model \"%s\"!", config);

	ret = Inf_LoadFaceData(&ctx, FACEDATA);
	testAssertMsg(ret == 0, "Cannot Load face data \"%s\"!", config);

	for (int i = 0; i < ctx.info->labels.size; i++) {
		printf("%s, ", ctx.info->labels.data[i]);
	}
	printf("\n");

	for (int i = 2; i < argc; i++) {
		const char *img_name = argv[i];
		char *face = get_basename(img_name);

		InfDetList det = {};
		InfImage img = {};
		Inf_Imread(img_name, &img, 0);
		MPI_IVA_OBJ_LIST_S obj_list = {};
		obj_list.obj_num = 1;
		obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w - 1, img.h - 1 };
		//ret = Inf_InvokeDetectObjList(&ctx, &img, &obj_list, &det);
		ret = Inf_InvokeFaceRecoRoi(&ctx, &img, &obj_list.obj[0].rect, &det);

		testAssertMsg(ret == 0 && det.size > 0, "Cannot identify gt:\"%s\" result: no detected face!\n", face);
		testAssertMsg(ret == 0, "Cannot identify gt:\"%s\" result:\"%s\":%.4f", face,
		              ctx.info->labels.data[det.data[0].cls[0]], det.data[0].confidence);
		for (int i = 0; i < det.data[0].cls_num; i++) {
			printf("face: (%.4f) \"%s\" gt:(%s)\n", det.data[0].prob[i], ctx.info->labels.data[i], face);
		}

		Inf_Imrelease(&img);
		testAssert(img.data == 0);

		Inf_ReleaseDetResult(&det);
		testAssert(det.size == 0 && det.data == 0);
	}

	Inf_ReleaseModel(&ctx);

	return 0;
}

int main(int argc, char **argv)
{
	g_args.argc = argc;
	g_args.argv = argv;
	REGISTER_TEST(test_register_face_0);
	REGISTER_TEST(test_load_and_identify_0);
	TEST_RUN();
}
