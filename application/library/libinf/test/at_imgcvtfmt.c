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

static void help(char *msg)
{
	printf("\t%s\n", msg);
	printf("\t usage: -h display help message and exit\n");
	printf("\t display image information: <src_img>\n");
	printf("\t convert image: <src_img fmt: pgm(P5), ppm(P6), jpg, png, bmp> <dst_img fmt: pgm, ppm, jpg>\n");
	printf("\t convert and resize image: <src_img> <dst_img> <resize dim: w> <resize dim: h> <resize dim: c>\n");
	printf("\t\n\t example:\n"
	       "\tat_imgcvtfmt face/obama.pgm face/obama.jpg\n");
}

int convert_img_resize(int argo, char **args)
{
	const char *src_img_name = args[1];
	const char *dst_img_name = args[2];
	int dst_w = atoi(args[3]);
	int dst_h = atoi(args[4]);
	int dst_c = atoi(args[5]);
	inf_assert(dst_w > 0 && dst_h > 0 && dst_w < 0xffff && dst_h < 0xffff && (dst_c == 1 || dst_c == 3),
	           "Invalid image dimension %dx%d", dst_w, dst_h);

	int ret = 0;

	InfImage img = {};
	ret = Inf_Imread(src_img_name, &img, 0);
	inf_assert(ret == 0, "Cannot read image \"%s\"", src_img_name);

	InfImage resize = Inf_ImcreateEmpty(dst_w, dst_h, dst_c, Inf8U);
	inf_assert(resize.data != NULL, "Cannot create image");

	Inf_Imresize(&img, dst_w, dst_h, &resize);

	ret = Inf_Imwrite(dst_img_name, &resize);
	inf_assert(ret == 0, "Cannot write image \"%s\"", dst_img_name);

	ret = Inf_Imrelease(&resize);
	inf_assert(ret == 0 && resize.data == NULL, "Cannot release image.1.");

	ret = Inf_Imrelease(&img);
	inf_assert(ret == 0 && resize.data == NULL, "Cannot release image.2.");
	return 0;
}

int display(int argo, char **args)
{
	const char *src_img_name = args[1];
	int ret = 0;

	InfImage img = {};
	ret = Inf_Imread(src_img_name, &img, 0);
	inf_assert(ret == 0, "Cannot read image \"%s\"", src_img_name);

	printf("\t%s dimension: height:%d width:%d channel:%d\n", src_img_name, img.h, img.w, img.c);
	Inf_Imrelease(&img);

	return 0;
}

int convert_img(int argo, char **args)
{
	if ((argo == 2 || argo == 3 || argo == 6) != 1 || !strcmp(args[1], "-h")) {
		help("args should eq to 1, 3, 5!\n");
		return 0;
	}

	if (argo == 2)
		return display(argo, args);

	if (argo == 6)
		return convert_img_resize(argo, args);

	const char *src_img_name = args[1];
	const char *dst_img_name = args[2];
	int ret = 0;

	InfImage img = {};
	ret = Inf_Imread(src_img_name, &img, 0);
	inf_assert(ret == 0, "Cannot read image \"%s\"", src_img_name);

	ret = Inf_Imwrite(dst_img_name, &img);
	inf_assert(ret == 0, "Cannot write image \"%s\"", dst_img_name);

	Inf_Imrelease(&img);
	inf_assert(img.data == 0, "Cannot release image!\n");
	return 0;
}

int main(int argc, char **argv)
{
	return convert_img(argc, argv);
}
