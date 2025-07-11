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
	MPI_RECT_POINT_S rect = (MPI_RECT_POINT_S){ 0, 0, img.w, img.h };
	// obj_list.obj[0].life = 160;

	ret = Inf_InitModel(&ctx, config);
	int encode_dim = 0;
	float *data = NULL;
	testAssertMsg(ret == 0, "Cannot init face detection model \"%s\"!", config);

	// InfDetList result = {}, result2 = {};
	for (int i = 0; i < 1; i++) {
		TIC(start);
		ret = Inf_InvokeFaceEncode(&ctx, &img, &rect, &encode_dim, &data);
		TOC("inference", start);
		if (ret == 0 && data) {
			free(data);
			data = 0;
		}
	}

	TIC(start);
	ret = Inf_InvokeFaceEncode(&ctx, &img, &rect, &encode_dim, &data);
	TOC("inference", start);

	// printf("Result: %s : #%d detections\n", img_name, result.Size);

	if (encode_dim) {
		printf("\tencode_dim: %d [%.4f, %.4f, ... %.4f, %.4f]\n", encode_dim, data[0], data[1],
		       data[encode_dim - 2], data[encode_dim - 1]);
	}

	free(data);
	data = 0;

	ret = Inf_ReleaseModel(&ctx);
	testAssert(ret == 0);

	ret = Inf_Imrelease(&img);
	testAssert(ret == 0);

	testAssert(data == 0);
	testAssert(ctx.model == 0 && ctx.info == 0);
	return 0;
}

int main(int argc, char **argv)
{
	g_args.argc = argc;
	g_args.argv = argv;
	REGISTER_TEST(test_case_0);
	TEST_RUN();
}
#endif
