#ifndef USE_NCNN
#include "libgen.h"

#include "inf_types.h"
#include "inf_image.h"

#include "inf_ut.h"

char *g_img_name;
char g_basename[256] = {};

static void get_base_name(char *img_name, char *dst_base)
{
	char *img_base = basename(img_name);
	strcpy(dst_base, img_base);
	char *ext = strstr(dst_base, ".");
	if (ext)
		*ext = 0;
}

static int write_pgm(char *img_basename, char *func_name, const InfImage *img)
{
	char img_name[256] = {};
	sprintf(img_name, "image/%s_%s.pgm", func_name, img_basename);
	int ret = Inf_Imwrite(img_name, img);
	return ret;
}

static int __attribute__((unused)) write_ppm(char *img_basename, char *func_name, const InfImage *img)
{
	char img_name[256] = {};
	sprintf(img_name, "image/%s_%s.ppm", func_name, img_basename);
	int ret = Inf_Imwrite(img_name, img);
	return ret;
}

static int write_jpg(char *img_basename, char *func_name, const InfImage *img)
{
	char img_name[256] = {};
	sprintf(img_name, "image/%s_%s.jpg", func_name, img_basename);
	int ret = Inf_Imwrite(img_name, img);
	return ret;
}

int test_Inf_Imresize_case0(InfTest *test)
{
	InfImage img = {}, dst = {};
	int ret = Inf_Imread(g_img_name, &img, 0);
	testAssertMsg(ret == 0 && img.data, "Cannot read %s", g_img_name);

	int dst_w = img.w / 2;
	int dst_h = img.h / 2;
	Inf_Imresize(&img, dst_w, dst_h, &dst);
	testAssertMsg(ret == 0, "Cannot resize %s", g_img_name);

	ret = write_pgm(g_basename, "imresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.pgm", g_basename);

	ret = write_jpg(g_basename, "imresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.jpg", g_basename);

	ret = Inf_Imrelease(&dst);
	testAssertMsg(ret == 0 || !dst.data, "Cannot release dst image\n");

	ret = Inf_Imrelease(&img);
	testAssertMsg(ret == 0 || !img.data, "Cannot release img image\n");

	return 0;
}

int test_Inf_ImcropPadResize_case0(InfTest *test)
{
	InfImage img = {}, dst = {};
	int ret = Inf_Imread(g_img_name, &img, 0);
	testAssertMsg(ret == 0, "Cannot read %s", g_img_name);

	int sx = img.w / 4;
	int sy = 0;
	int ex = sx + img.w / 2;
	int ey = img.h - 1;
	int dst_w = (ex - sx + 1) / 2;
	int dst_h = (ey - sy + 1) / 2;
	int ptop = img.h / 8;
	int pbot = img.h / 8;
	int pleft = img.w / 4;
	int pright = img.w / 4;

	Inf_ImcropPadResize(&img, sx, sy, ex, ey, ptop, pbot, pleft, pright, &dst, dst_w, dst_h);

	testAssertMsg(ret == 0, "Cannot resize %s", g_img_name);

	ret = write_pgm(g_basename, "imcroppadresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.pgm", g_basename);

	ret = write_jpg(g_basename, "imcroppadresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.jpg", g_basename);

	ret = Inf_Imrelease(&dst);
	testAssertMsg(ret == 0 || !dst.data, "Cannot release dst image\n");

	ret = Inf_Imrelease(&img);
	testAssertMsg(ret == 0 || !img.data, "Cannot release img image\n");
	return 0;
}

int test_Inf_ImcropResize_case0(InfTest *test)
{
	InfImage img = {}, dst = {};
	int ret = Inf_Imread(g_img_name, &img, 0);
	testAssertMsg(ret == 0, "Cannot read %s", g_img_name);

	int sx = img.w / 4;
	int sy = 0;
	int ex = sx + img.w / 2;
	int ey = img.h - 1;
	int dst_w = (ex - sx + 1) / 2;
	int dst_h = (ey - sy + 1) / 2;
	Inf_ImcropResize(&img, sx, sy, ex, ey, &dst, dst_w, dst_h);
	testAssertMsg(ret == 0, "Cannot resize %s", g_img_name);

	ret = write_pgm(g_basename, "imcropresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.pgm", g_basename);

	ret = write_jpg(g_basename, "imcropresize", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.jpg", g_basename);

	ret = Inf_Imrelease(&dst);
	testAssertMsg(ret == 0 || !dst.data, "Cannot release dst image\n");

	ret = Inf_Imrelease(&img);
	testAssertMsg(ret == 0 || !img.data, "Cannot release img image\n");

	return 0;
}

int test_Inf_ImcropResizeAspectRatio_case0(InfTest *test)
{
	InfImage img = {}, dst = {};
	int ret = Inf_Imread(g_img_name, &img, 0);
	testAssertMsg(ret == 0, "Cannot read %s", g_img_name);

	int sx = img.w / 4;
	int sy = 0;
	int ex = sx + img.w / 2;
	int ey = img.h - 1;
	int h = (ey - sy + 1) / 2;
	int w = (ex - sx + 1) / 2;
	int dst_w = (h > w) ? h : w;
	int dst_h = dst_w;
	Inf_ImcropResizeAspectRatio(&img, sx, sy, ex, ey, &dst, dst_w, dst_h);
	testAssertMsg(ret == 0, "Cannot resize %s", g_img_name);

	ret = write_pgm(g_basename, "imcropresizeaspectratio", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.pgm", g_basename);

	ret = write_jpg(g_basename, "imcropresizeaspectratio", &dst);
	testAssertMsg(ret == 0, "Cannot save %s.jpg", g_basename);

	ret = Inf_Imrelease(&dst);
	testAssertMsg(ret == 0 || !dst.data, "Cannot release dst image\n");

	ret = Inf_Imrelease(&img);
	testAssertMsg(ret == 0 || !img.data, "Cannot release img image\n");

	return 0;
}

int main(int argc, char **argv)
{
	assert(argc == 2);
	g_img_name = argv[1];
	get_base_name(g_img_name, g_basename);

	REGISTER_TEST(test_Inf_Imresize_case0);
	REGISTER_TEST(test_Inf_ImcropPadResize_case0);
	REGISTER_TEST(test_Inf_ImcropResize_case0);
	REGISTER_TEST(test_Inf_ImcropResizeAspectRatio_case0);

	TEST_RUN();
	return 0;
}
#endif