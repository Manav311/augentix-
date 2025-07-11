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

int register_face(int argo, char **args)
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

	for (int i = 3; i < argo; i++) {
		const char *img_name = args[i];
		char *face = get_basename(img_name);

		InfImage img = {};
		Inf_Imread(img_name, &img, 0);
		MPI_RECT_POINT_S roi = { 0, 0, img.w - 1, img.h - 1 };
		ret = Inf_RegisterFaceRoiDet(&ctx, &img, &roi, face);
		inf_assert(ret == 0, "Cannot regis face \"%s\" from \"%s\"", face, img_name);

		Inf_Imrelease(&img);
		inf_assert(img.data == 0, "cannot release img!");
	}

	//inf_assert(ctx.info->labels.Size == (argo-2+1), "Number of face(%d) in the database are wrong input (%d)!",
	//	ctx.info->labels.Size, argo-2+1);
	printf("Success to register faces!\n");
	for (int i = 0; i < ctx.info->labels.size; i++) {
		printf("%s, ", ctx.info->labels.data[i]);
	}
	printf("\n");

	ret = Inf_SaveFaceData(&ctx, face_file);

	Inf_ReleaseModel(&ctx);
	inf_assert(ret == 0 && ctx.model == 0 && ctx.info == 0, "Cannot save face data please check dst path:%s!",
	           face_file);
	return 0;
}

int main(int argc, char **argv)
{
	return register_face(argc, argv);
}
