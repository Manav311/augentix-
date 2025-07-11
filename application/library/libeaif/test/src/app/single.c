#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef USE_MICROLITE
#pragma message("build with microlite!")
#else
#pragma message("build with tflite!")
#endif

// for fake interface
//#include "avftr.h"
//AVFTR_VIDEO_CTX_S vftr_res_shm;

// for src
#include "inf_classifier.h"
#include "inf_detect.h"

// for utils
#include "utils.h"
int g_capture = 0;

typedef struct {
	CvVideo vid;
	CvImage image;
	int is_running;
	FILE *fw;
} EaifCvCtx;

EaifCvCtx g_cv_config = {};
int g_output_console_flag = 0;
char *g_gray_scale = 0;
const char *g_image_path = NULL;
const char *g_model_config = NULL;
const char *g_output_name = NULL;
FILE *g_fp = NULL;
InfModelCtx g_model_ctx;

#define printResult(fmt, args...)                   \
	{                                           \
		if (g_output_console_flag) {        \
			printf(fmt, ##args);        \
		} else {                            \
			fprintf(g_fp, fmt, ##args); \
		}                                   \
	}

void loggerClassify(const char *image, InfResultList *result_list)
{
	const InfStrList *labels = &g_model_ctx.info->labels;

	InfResult *result = NULL;
	for (int i = 0; i < result_list->size; i++) {
		result = &result_list->data[i];
		if (result->cls_num) {
			printResult("%s %s %.4f\n", image, labels->data[result->cls[0]], result->prob[result->cls[0]]);
		} else {
			printResult("%s negative 0.0\n", image);
		}
	}
}

void loggerDetect(const char *image, InfDetList *result_list)
{
	const InfStrList *labels = &g_model_ctx.info->labels;

	InfDetResult *result = NULL;
	for (int i = 0; i < result_list->size; i++) {
		result = &result_list->data[i];
		if (result->cls_num) {
			printResult("%s %s %.4f [%d,%d,%d,%d]\n", image, labels->data[result->cls[0]], result->prob[result->cls[0]],
				result->rect.sx, result->rect.sy, result->rect.ex, result->rect.sy);
		}
	}
}

int runSingleImage(const char *image)
{
	CvImage img = {};
	MPI_IVA_OBJ_LIST_S ol;

	if (g_gray_scale) {
		CV_utils_ReadImageGray(&img, image);
	} else {
		CV_utils_ReadImage(&img, image);
	}

	ol.obj_num = 1;
	ol.obj[0] = (MPI_IVA_OBJ_ATTR_S){
		.id = 0,
		.life = 160,
		.mv.x = 0,
		.mv.y = 0,
		.rect.sx = 0,
		.rect.sy = 0,
		.rect.ex = img.w - 1,
		.rect.ey = img.h - 1,
	};
	InfImage inf_image = { img.w, img.h, img.c, img.data, 0, Inf8UC3 };
	if (g_model_ctx.info->inference_type == InfRunClassify) {
		InfResultList result = {};
		Inf_InvokeClassify(&g_model_ctx, &inf_image, &ol, &result);
		loggerClassify(image, &result);
		Inf_ReleaseResult(&result);
	} else if (g_model_ctx.info->inference_type == InfRunDetect) {
		InfDetList result = {};
		Inf_InvokeDetect(&g_model_ctx, &inf_image, &result);
		loggerDetect(image, &result);
		Inf_ReleaseDetResult(&result);
	}

	CV_utils_destroyFrame(&img);
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
	while ((read = getline(&line, &len, fr)) != -1) {
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
	return Inf_InitModel(&g_model_ctx, config);
}

void printHelp(char *msg)
{
	printf("\n\t%s\n", msg);
	printf("\n\tUsage ================================\n"
	       "\n\tsingle <model> <image-path/imagelist.txt> <output file> <optional - gray scale: 1>\n");
}

int main(int argc, char **argv)
{
	if (!(argc == 4 || argc == 5)) {
		printHelp("input args not equal to 3 / 4");
		return 0;
	}
	g_gray_scale = 0;
	g_model_config = argv[1];
	g_image_path = argv[2];
	g_output_name = argv[3];
	if (argc == 5)
		g_gray_scale = argv[4];

	if (access(g_model_config, F_OK) || access(g_image_path, F_OK))
		printHelp("Cannot find input file!");

	if (g_output_name[0] == '-')
		g_output_console_flag = 1;
	else {
		g_fp = fopen(g_output_name, "w");
		if (!g_fp)
			printf("[ERROR] Cannot open %s\n", g_output_name);
	}

	if (loadModel(g_model_config)) {
		printf("[ERROR] Cannot load model config %s!\n", g_model_config);
		return 0;
	}

	int path_len = strlen(g_image_path);
	char ext[4] = {};
	strcpy(ext, &g_image_path[path_len - 3]);

	if (!strcmp(ext, "pgm")) // cannot read pgm file
		assert(0);

	if (!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "bmp"))
		runSingleImage(g_image_path);
	else {
		runImageList(g_image_path);
	}

	if (!g_output_console_flag)
		fclose(g_fp);

	return 0;
}
