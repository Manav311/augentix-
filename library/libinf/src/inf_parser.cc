#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "inf_utils.h"
#include "inf_log.h"

static inline int parseInt()
{
	char *tok = strtok(NULL, "=\n");
	return atoi(tok);
}

static inline float parseFloat()
{
	char *tok = strtok(NULL, "=\n");
	return atof(tok);
}

static inline int parseIntList(InfIntList *arr, int size)
{
	arr->size = size;
	arr->data = (int*)calloc(1, sizeof(int) * size);

	char *tok = strtok(NULL, "=\n");
	if (tok != NULL)
		tok = strtok(tok, ",");
	else
		return 0;
	int i = 0;
	while (tok != NULL && i < size) {
		arr->data[i] = atoi(tok);
		tok = strtok(NULL, ",");
		i++;
	};
	if (i != size) {
		memmove(arr->data, arr->data, sizeof(int) * i);
	}
	arr->size = i;
	return 1;
}

static inline int parseFloatList(InfFloatList *arr, int size)
{
	arr->size = size;
	arr->data = (float*)calloc(1, sizeof(float) * size);

	char *tok = strtok(NULL, "=\n");
	if (tok != NULL)
		tok = strtok(tok, ",");
	else
		return 0;
	int i = 0;
	while (tok != NULL && i < size) {
		arr->data[i] = atof(tok);
		tok = strtok(NULL, ",");
		i++;
	};
	if (i != size) {
		memmove(arr->data, arr->data, sizeof(float) * i);
	}
	arr->size = i;
	return 1;
}

static inline int parseIntArr(int *arr, int size)
{
	char *tok = strtok(NULL, "=\n");
	if (tok != NULL)
		tok = strtok(tok, ",");
	else
		return 0;
	int i = 0;
	while (tok != NULL && i < size) {
		arr[i] = atoi(tok);
		tok = strtok(NULL, ",");
		i++;
	};
	return 1;
}

static inline int parseFloatArr(float *arr, int size)
{
	char *tok = strtok(NULL, "=\n");
	if (tok != NULL)
		tok = strtok(tok, ",");
	else
		return 0;
	int i = 0;
	while (tok != NULL && i < size) {
		arr[i] = atof(tok);
		tok = strtok(NULL, ",");
		i++;
	};
	return 1;
}

int ParseConfigParam(char *str, InfModelInfo* conf)
{
	int hit = 1;
	char *tok = NULL;
	if (!strcmp(str, "output_type")) {
		tok = strtok(NULL, "=\n");
		conf->output_type = (InfOutputTypeE)GetOutputType(tok);
	} else if (!strcmp(str, "model_name")) {
		tok = strtok(NULL, "=\n");
		strncpy(conf->model_name, tok, INF_MODEL_NAME_LEN-1);
	} else if (!strcmp(str, "verbose")) {
		conf->verbose = parseInt();
	} else if (!strcmp(str, "debug")) {
		conf->debug = parseInt();
	} else if (!strcmp(str, "num_threads")) {
		conf->num_threads = parseInt();
	} else if (!strcmp(str, "width")) {
		conf->w = parseInt();
	} else if (!strcmp(str, "height")) {
		conf->h = parseInt();
	} else if (!strcmp(str, "channels")) {
		conf->c = parseInt();
	} else if (!strcmp(str, "input_int8_scale")) {
		conf->input_int8_scale = parseFloat();
	} else if (!strcmp(str, "topk")) {
		conf->topk = parseInt();
	} else if (!strcmp(str, "feature_output_pairs")) {
		conf->feature_output_pairs = parseInt();
	} else if (!strcmp(str, "num_anchors_per_feature")) {
		conf->num_anchors_per_feature = parseInt();
	} else if (!strcmp(str, "use_kps")) {
		conf->use_kps = parseInt();
	} else if (!strcmp(str, "feature_stride")) {
#define MAX_FEAT_STRIDE_NUM (10)
		if (!parseIntArr(conf->feature_stride, MAX_FEAT_STRIDE_NUM))
			return 0;
	} else if (!strcmp(str, "inference_type")) {
		tok = strtok(NULL, "=\n");
		if (!strcmp(tok, INF_CLASSIFY_STR))
			conf->inference_type = InfRunClassify;
		else if (!strcmp(tok, INF_DETECT_STR))
			conf->inference_type = InfRunDetect;
		else if (!strcmp(tok, INF_FACEENCODE_STR))
			conf->inference_type = InfRunFaceEncode;
		else if (!strcmp(tok, INF_FACERECO_STR))
			conf->inference_type = InfRunFaceReco;
		else {
			return -1;
		}
	} else if (!strcmp(str, "iou_thresh")) {
		conf->iou_thresh = parseFloat();
	} else if (!strcmp(str, "window_min_size")) {
		conf->window_min_size = parseInt();
	} else if (!strcmp(str, "window_scale_factor")) {
		conf->window_scale_factor = parseFloat();
	} else if (!strcmp(str, "align_margin")) {
		conf->align_margin = parseInt();
	} else if (!strcmp(str, "model_paths")) {
		tok = strtok(NULL, "=\n");

#define MAX_MODEL_SIZE (10)
		conf->model_paths.size = MAX_MODEL_SIZE;
		conf->model_paths.data = (char **)malloc(sizeof(char *) * conf->model_paths.size);
		int i, j;
		for (i = 0; i < conf->model_paths.size; i++) {
			conf->model_paths.data[i] = (char *)malloc(sizeof(char) * INF_MODEL_PATH_LEN);
		}
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		i = 0;
		while (tok != NULL && i < MAX_MODEL_SIZE ) {
			strcpy(conf->model_paths.data[i], tok);
			tok = strtok(NULL, ",");
			i++;
		}
		for (j = i; j < conf->model_paths.size; j++) {
			free(conf->model_paths.data[j]);
			conf->model_paths.data[j] = nullptr;
		}
		conf->model_paths.size = i;
	} else if (!strcmp(str, "resize_aspect_ratio")) {
		conf->resize_aspect_ratio = parseInt();
	} else if (!strcmp(str, "num_classes")) {
		conf->labels.size = parseInt();
		conf->labels.data = (char **)malloc(sizeof(char *) * conf->labels.size);
		for (int i = 0; i < conf->labels.size; i++) {
			conf->labels.data[i] = (char *)malloc(sizeof(char) * INF_STR_LEN);
		}
	} else if (!strcmp(str, "labels")) {
		tok = strtok(NULL, "=\n");
		if (!conf->labels.size)
			return -1;
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != NULL) {
			strcpy(conf->labels.data[i], tok);
			tok = strtok(NULL, ",");
			i++;
		}
		if (i != conf->labels.size) {
			return -1;
		}

	} else if (!strcmp(str, "zeros")) {
		if (!parseFloatArr(conf->norm_zeros, 3))
			return 0;
	} else if (!strcmp(str, "stds")) {
		if (!parseFloatArr(conf->norm_scales, 3))
			return 0;
	} else if (!strcmp(str, "conf_thresh")) {
		if (!parseFloatList(&conf->conf_thresh, 10))
			return 0;
	} else if (!strcmp(str, "nms_internal_thresh")) {
		if (!parseFloatList(&conf->nms_internal_thresh, 10))
			return 0;
	} else if (!strcmp(str, "filter_cls")) {
		if (!parseIntList(&conf->filter_cls, INF_MODEL_CLASS_MAX))
			return 0;
	} else if (!strcmp(str, "filter_out_cls")) {
		if (!parseIntList(&conf->filter_out_cls, INF_MODEL_CLASS_MAX))
			return 0;
	} else {
		hit = 0;
	}
	return hit;
}

int ParseParam(char *str, InfModelInfo* config)
{
	int hit = 0;
	char *tok = strtok(str, "=");

	while (tok != NULL) {
		hit = 0;
		if (!strncmp(tok, "#", 1)) {
			hit = 1;
			break;
		} else if (!strncmp(tok, "\n", strlen("\n"))) {
			hit = 1;
			break;
		}
		hit = ParseConfigParam(tok, config);
		if (hit == 1)
			goto next;
		else if (hit == 0) {
			printf("Unknown parameter: %s\n", tok);
			hit = 1;
			break;
		} else if (hit < 0) {
			if (!strncmp(tok, "\n", strlen("\n"))) {
				hit = 1;
				break;
			} else {
				printf("Unknown parameter: %s\n", tok);
				break;
			}
		}
	next:
		tok = strtok(NULL, "=");
	}
	return hit;
}

// call does not handle error yet.
int ParseModelConfig(const char *model_config_path, InfModelInfo* config)
{
	config->verbose = 0;
	config->num_threads = 1;
	config->debug = 0; // default As zero;

	strncpy(config->config_path, model_config_path, INF_MODEL_PATH_LEN - 1);

	FILE *fp = fopen(model_config_path, "r");
	if (!fp) {
		inf_log_err("Cannot open config file %s. %s", model_config_path, strerror(errno));
		return -errno;
	}

	char str[256];
	int hit = 1;
	while (fgets(str, 256, fp) != NULL) {
		hit = ParseParam(str, config);
		/* Stop parsing when a unknown parameter found */
		if (!hit) {
			printf("Parsing parameter file failed.\n");
			break;
		}
	}

	fclose(fp);
	return (hit == 0) ? -EINVAL : 0;
}