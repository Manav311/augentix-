#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>

#include "inf_image.h"
#include "inf_face.h"

#define AssertCond(cond, retval, fmt, args...) \
	{                                      \
		if (!(cond)) {                 \
			printf(fmt, ##args);   \
			return (retval);       \
		}                              \
	}

#define TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define TOC(str, start)                                                                                             \
	{                                                                                                           \
		struct timespec end;                                                                                \
		float delta_s;                                                                                      \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                           \
		delta_s = (float)((end.tv_sec - start.tv_sec) + (float)(end.tv_nsec - start.tv_nsec) / 1000000000); \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                               \
	}

#define printResult(fmt, args...)                   \
	{                                           \
		if (g_output_console_flag) {        \
			printf(fmt, ##args);        \
		} else {                            \
			fprintf(g_fp, fmt, ##args); \
		}                                   \
	}

struct timespec start;

int g_output_console_flag = 0;
int g_verbose = 0;
const char *g_image_path = NULL;
const char *g_model_config = NULL;
const char *g_output_name = NULL;
FILE *g_fp = NULL;
InfModelCtx g_model_ctx;

int runSingleImage(const char *img_name)
{
	InfImage img = { 0 };
	InfDetList result = { 0 };
	char img_name_no_ext[256] = { 0 };
	struct timespec sstart;

	InfModelCtx *ctx = &g_model_ctx;
	int size = sprintf(img_name_no_ext, "%s", img_name);
	img_name_no_ext[size - 4] = 0;
	if (img_name_no_ext[size - 5] == '.')
		img_name_no_ext[size - 5] = 0;

	int ret = Inf_Imread(img_name, &img, 0);
	AssertCond(ret == 0 || img.data, -1, "Cannot read image %s!\n", img_name);

	printf("[INFO] Detecting on: %s HxWxC: %dx%dx%d\n", img_name, img.h, img.w, img.c);

	if (g_verbose)
		TIC(sstart);

	MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	obj_list.obj_num = 1;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w - 1, img.h - 1 };
	ret = Inf_InvokeFaceDetObjList(ctx, &img, &obj_list, &result);

	AssertCond(ret == 0, Inf_Imrelease(&img), "Cannot invoke face detection!\n");
	if (g_verbose)
		TOC("Inference", sstart);

	printf("[INFO] Total %d detections\n", result.size);
	printResult("%s\n", img_name_no_ext);
	printResult("%d\n", result.size);

	if (!result.size) {
		printResult("0.0 0.0 0.0 0.0 0.99\n");
	} else {
		for (int i = 0; i < result.size; i++) {
			int w = result.data[i].rect.ex - result.data[i].rect.sx + 1;
			int h = result.data[i].rect.ey - result.data[i].rect.sy + 1;
			printResult("%d %d %d %d %.3f\n", result.data[i].rect.sx, result.data[i].rect.sy, w, h,
			            result.data[i].prob[result.data[i].cls[0]]);
		}
	}

	ret = Inf_ReleaseDetResult(&result);
	AssertCond(ret == 0 && result.size == 0 && result.data == 0, -1, "Cannot Relase detection result!\n");

	ret = Inf_Imrelease(&img);
	AssertCond(ret == 0, -1, "Cannot Release image!\n");

	return 0;
}

int runImageList(const char *img_list)
{
	FILE *fr = fopen(img_list, "r");
	if (!fr) {
		printf("[ERROR] Cannot open file %s!\n", img_list);
		return -1;
	}

	char *line = NULL;
	size_t len = 0;
	size_t read;
	int number = 0;
	while ((read = getline(&line, &len, fr)) != (size_t)EOF) {
		line[read - 1] = 0;
		runSingleImage(line);
		number += 1;
		if ((number % 20) == 0) {
			printf("[INFO] Process %d\n", number);
		}
	}
	if (line)
		free(line);
	printf("[INFO] Finish %d!\n", number);
	return 0;
}

int loadModel(const char *config)
{
	int ret = Inf_InitModel(&g_model_ctx, config);
	char *cap = getenv("INF_CAP_PREFIX");
	if (cap) {
		Inf_Setup(&g_model_ctx, 0, 1, 1);
	}
	return ret;
}

void printHelp(char *msg)
{
	printf("\n\t%s\n", msg);
	printf("\n\tUsage ================================\n"
	       "\n\tfacedet_image <-h / --help> - display help message and exit\n"
	       "\n\tfacedet_image <model-config> <image-path/imagelist.txt> <output \"- to stdout\"/\"filename to textfile\"> <disp infer duration(optional):1/0[default]>\n"
	       "\n"
	       "\n\tOutput format\n"
	       "\t<image name>\n"
	       "\t<number of detections>\n"
	       "\t<sx:integer sy:integer w:integer h:integer prob:float>\n"
	       "\n");
}

void createdir(const char *filename)
{
	char directory[256] = {};
	char *dir = NULL;
	char cmd[256] = {};
	strcpy(directory, filename);
	dir = dirname(directory);
	sprintf(cmd, "mkdir -p %s", dir);
	system(cmd);
}

int main(int argc, char **argv)
{
	if (!((argc == 4) || (argc == 5)) || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))) {
		printHelp("input args not equal to 3");
		return 0;
	}

	g_model_config = argv[1];
	g_image_path = argv[2];
	g_output_name = argv[3];
	g_verbose = (argc == 5) ? atoi(argv[4]) : 0;

	if (access(g_model_config, F_OK) || access(g_image_path, F_OK)) {
		printHelp("Cannot find input file!");
		return 0;
	}

	if (g_output_name[0] == '-')
		g_output_console_flag = 1;
	else {
		createdir(g_output_name);
		g_fp = fopen(g_output_name, "w");
		if (!g_fp) {
			printf("[ERROR] Cannot open %s\n", g_output_name);
			return 0;
		}
	}

	if (loadModel(g_model_config)) {
		printf("[ERROR] Cannot load model config %s!\n", g_model_config);
		return 0;
	}

	int path_len = strlen(g_image_path);
	char ext[4] = {};
	strcpy(ext, &g_image_path[path_len - 3]);

	TIC(start);

	if (!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "bmp") || !strcmp(ext, "ppm") ||
	    !strcmp(ext, "pgm"))
		runSingleImage(g_image_path);
	else
		runImageList(g_image_path);

	TOC("Total time spent", start);

	if (!g_output_console_flag)
		fclose(g_fp);

	int ret = Inf_ReleaseModel(&g_model_ctx);
	AssertCond(ret == 0, 0, "Cannot Release Model!\n");

	return 0;
}
