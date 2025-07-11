#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>

#include "inf_image.h"
#include "inf_face.h"
#include "inf_detect.h"

#define inf_assert(cond, fmt, args...)                                                                                \
	do {                                                                                                          \
		if (!(cond)) {                                                                                        \
			fprintf(stderr, "%s %d assert fail[" fmt "] cond:%s !\n", __func__, __LINE__, ##args, #cond); \
			assert(0);                                                                                    \
		}                                                                                                     \
	} while (0)

struct timespec start;

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

static void help(char *msg)
{
	printf("\t%s\n", msg);
	printf("\t usage: <inapp.ini> <dst_face file> <face_name0.pgm> <face_name1.pgm> ...\n");
	printf("\t\n\t example:\n"
	       "\tregistface face.bin face/obama.pgm face/trump.pgm\n");
}

int test_face(int argo, char **args)
{
	const char *config = args[1];
	const char *face_file = args[2];

	if (argo < 3) {
		help("args should greater than 3!\n");
		return 0;
	}

	char *capture = getenv("INF_CAP_PREFIX");

	InfModelCtx ctx;
	int ret = Inf_InitModel(&ctx, config);
	inf_assert(ret == 0 && ctx.model, "Cannot init face reco model \"%s\"!", config);

	if (capture)
		Inf_Setup(&ctx, 0, 1, 1);

	ret = Inf_LoadFaceData(&ctx, face_file);
	inf_assert(ret == 0, "Cannot Load face data \"%s\"!", config);

	printf("Database name list: ");
	for (int i = 0; i < ctx.info->labels.size; i++) {
		printf("%s, ", ctx.info->labels.data[i]);
	}
	printf("\n");

	for (int i = 3; i < argo; i++) {
		const char *img_name = args[i];
		char *face = get_basename(img_name);

		InfDetList det = {};
		InfImage img = {};
		Inf_Imread(img_name, &img, 0);
		MPI_IVA_OBJ_LIST_S obj_list = {};
		obj_list.obj_num = 1;
		obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w - 1, img.h - 1 };

		/* two stage detection */
		/* stage one */
		ret = Inf_InvokeFaceRecoStageOne(&ctx, &img, &obj_list, &det);

		/* stage two */
		if (det.size) {
			ret = Inf_InvokeFaceRecoStageTwo(&ctx, &det);
			for (int i = 0; i < det.data[0].prob_num; i++) {
				printf("face: (%.4f) \"%s\" gt:(%s)\n", det.data[0].prob[i], ctx.info->labels.data[i],
				       face);
			}
		} else {
			printf("Cannot detect face from the image :%s\n", img_name);
		}

		Inf_Imrelease(&img);
		inf_assert(img.data == 0, "cannot release img!");

		Inf_ReleaseDetResult(&det);
		inf_assert(det.size == 0 && det.data == 0, "cannot release det result!");
	}

	Inf_ReleaseModel(&ctx);
	inf_assert(ret == 0 && ctx.model == 0 && ctx.info == 0, "Cannot save face data please check dst path:%s!",
	           face_file);
	return 0;
}

int main(int argc, char **argv)
{
	return test_face(argc, argv);
}
