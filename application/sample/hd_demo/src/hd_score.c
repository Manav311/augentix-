#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_errno.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#include "json.h"

#include "inf_image.h"
#include "inf_classifier.h"

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define ASSERT_MSG(cond, msg)                                              \
	{                                                                  \
		if (!(cond)) {                                             \
			printf("[ERROR] %s cond: (\"" #cond "\")\n", msg); \
			assert(0);                                         \
		}                                                          \
	}

void help(void);

int g_dim_set = 0;

int read_image_yuv(const char *y_img, int w, int h, InfImage *img)
{
	FILE *fp = fopen(y_img, "rb");
	if (!fp) {
		printf("Cannot open image file %s!\n", y_img);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, 0);
	ASSERT_MSG(size >= w * h, "Image size should eq or greater than width * height pixels");
	img->w = w;
	img->h = h;
	img->c = 1;
	img->dtype = Inf8UC1;
	img->data = (unsigned char *)malloc(sizeof(char) * w * h);
	fread(img->data, 1, w * h, fp);
	fclose(fp);
	return 0;
}

int read_image(const char *img_file, int w, int h, InfImage *img)
{
	int fn_len = strlen(img_file);
	char *ext = (char *)(img_file + fn_len - 3);
	int ret = 0;
	if (!strcmp(ext, "yuv")) {
		ASSERT_MSG(g_dim_set == 2, "Users have to specify width and height for yuv image!");
		ret = read_image_yuv(img_file, w, h, img);
	} else if (!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "pgm") || !strcmp(ext, "ppm") ||
	           !strcmp((char *)(ext - 1), "jpeg"))
		ret = Inf_Imread(img_file, img, 0);
	else {
		printf("[ERROR] Unknown/Unsupported image format!\n");
		help();
		exit(0);
	}
	return ret;
}

/**@brief print out all classification result
  *@param[in] InfImage input Y only image
  *@param[in] obj_list MPI_IVA_OBJ_LIST_S list
  *@param[in/out] ctx InfClassifierCtx context
  */
int scoring(const InfImage *img, const char *img_name, const MPI_IVA_OBJ_LIST_S *obj_list, InfModelCtx *ctx)
{
	int ret = 0;
	InfResultList result = { 0 };

	ret = Inf_InvokeClassify(ctx, img, obj_list, &result);
	assert(ret == 0 && result.size == obj_list->obj_num);

	char msg[2056] = {};
	int size = 0;
	size += sprintf(msg, "{\"img\":\"%s\",\"result\":[", img_name);
	for (int i = 0; i < result.size; i++) {
		const MPI_IVA_OBJ_ATTR_S *obj = &obj_list->obj[i];
		int cls = 0;
		float prob = 0.0f;
		if (result.data[i].cls_num) {
			cls = result.data[i].cls[0];
			prob = result.data[i].prob[cls];
		}
		size += sprintf(&msg[size], "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"cls\":%d,\"prob\":%.3f}},",
		                result.data[i].id, obj->rect.sx, obj->rect.sy, obj->rect.ex, obj->rect.ey, cls, prob);
	}

	if (obj_list->obj_num)
		size--;
	size += sprintf(&msg[size], "]}\n");
	fprintf(stderr, "%s", msg);

	ret = Inf_ReleaseResult(&result);
	assert(ret == 0);
	return 0;
}

int read_obj_list_file(const char *file_name, MPI_IVA_OBJ_LIST_S *obj_list)
{
	struct json_object *tmp_obj, *cmd_obj, *tmp1_obj, *tmp2_obj, *tmp3_obj, *tmp4_obj;

	cmd_obj = json_object_from_file(file_name);
	if (!cmd_obj) {
		fprintf(stderr, "Cannot open %s\n", file_name);
		goto error;
	}

	json_object_object_get_ex(cmd_obj, "ol", &tmp_obj);
	if (!tmp_obj) {
		fprintf(stderr, "Cannot get %s object\n", "ol");
		goto error;
	}

	int obj_num = json_object_array_length(tmp_obj);
	obj_list->obj_num = obj_num;
	DBG("obj_num:%d\n", obj_num);

	for (int i = 0; i < obj_num; i++) {
		MPI_IVA_OBJ_ATTR_S *obj = &obj_list->obj[i];
		tmp1_obj = json_object_array_get_idx(tmp_obj, i);
		if (!tmp1_obj) {
			fprintf(stderr, "Cannot get array:%d object\n", i);
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj", &tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj");
			goto error;
		}

		json_object_object_get_ex(tmp2_obj, "id", &tmp3_obj);
		if (tmp3_obj) {
			obj->id = json_object_get_int(tmp3_obj);
		} else {
			obj->id = i;
		}

		json_object_object_get_ex(tmp2_obj, "rect", &tmp3_obj);
		if (!tmp3_obj) {
			fprintf(stderr, "Cannot get %s object\n", "rect");
			goto error;
		}

		int rect_num = json_object_array_length(tmp3_obj);
		if (rect_num != 4) {
			fprintf(stderr, "Rectangle should have 4 integers!\n");
			goto error;
		}

		tmp4_obj = json_object_array_get_idx(tmp3_obj, 0);
		obj->rect.sx = json_object_get_int(tmp4_obj);

		tmp4_obj = json_object_array_get_idx(tmp3_obj, 1);
		obj->rect.sy = json_object_get_int(tmp4_obj);

		tmp4_obj = json_object_array_get_idx(tmp3_obj, 2);
		obj->rect.ex = json_object_get_int(tmp4_obj);

		tmp4_obj = json_object_array_get_idx(tmp3_obj, 3);
		obj->rect.ey = json_object_get_int(tmp4_obj);
	}
	return 0;

error:
	json_object_put(cmd_obj);
	return -1;
}

int hd_score(const char *img_file, int w, int h, const char *obj_list_file, const char *cfg_file_name)
{
	InfImage img = { 0 };
	InfModelCtx ctx = { 0 };
	MPI_IVA_OBJ_LIST_S obj_list = { 0 };

	int ret = read_image(img_file, w, h, &img);
	if (ret) {
		printf("[ERROR] Cannot read image file %s\n", img_file);
		return ret;
	}

	ret = read_obj_list_file(obj_list_file, &obj_list);
	if (ret) {
		return ret;
	}

	ret = Inf_InitModel(&ctx, cfg_file_name);
	ASSERT_MSG(ret == 0 && ctx.model, "Cannot init model!");

	if (ctx.info->conf_thresh.data[0] != 0.0) {
		printf("[WARN] Model conf_thresh(%.3f) should be 0.0!\n", ctx.info->conf_thresh.data[0]);
	}

	scoring(&img, img_file, &obj_list, &ctx);

	ret = Inf_ReleaseModel(&ctx);
	ASSERT_MSG(ret == 0, "Cannot release model!");

	free(img.data);
	return 0;
}

void help(void)
{
	printf("USAGE:\thd_score -i <yuv/pgm/ppm/jpg/jpeg)> -c <model-config>\t\n");
	printf("\t-l <obj list json file>\t\tobject list in .json format\n");
	printf("\t-w <yuv image width>\t\tdefault 640\n");
	printf("\t-h <yuv image height>\t\tdefault 360\n");
	printf("\t-H Help message and exit\n\n");
	printf("\tNOTE.1 User should first modify conf_thresh to 0 in model config.\n");
	printf("\tNOTE.2 User should provide dimension for yuv image.\n");
	printf("For example:\n");
	printf("\thd_score -i snapshot_y0.yuv -c /system/eaif/models/classifiers/shuffleNetV2/inapp.ini -l obj_list_0.json -w 640 -h 360\n");
	printf("\thd_score -i snapshot_y1.pgm -c /system/eaif/models/classifiers/shuffleNetV2/inapp.ini -l obj_list_1.json\n");
	printf("\t// example object list json file //\n");
	printf("\t{\"ol\":[{\"obj\":{\"rect\":[0,0,300,320]}}]}\n");
	printf("\n");
	printf("Example result: \n");
	printf("\t{\"img\":\"snapshot_y0.yuv\",\"result\":[{\"obj\":{\"id\":1,\"rect\":[0,0,300,320],\"cls\":0,\"prob\":0.691}}]}\n");
	printf("\t{\"img\":\"snapshot_y0.pgm\",\"result\":[{\"obj\":{\"id\":1,\"rect\":[0,0,300,320],\"cls\":0,\"prob\":0.720}}]}\n");
}

int main(int argc, char **argv)
{
	char cfg_file_name[256] = {};
	char img_file[256] = {};
	char obj_list_file[256] = {};
	int w = 640;
	int h = 360;
	int c = 1;
	while ((c = getopt(argc, argv, "i:c:l:w:h:H")) != -1) {
		switch (c) {
		case 'i':
			sprintf(img_file, optarg);
			DBG("input image file:%s\n", img_file);
			break;
		case 'c':
			sprintf(cfg_file_name, optarg);
			DBG("cfg_file_name:%s\n", cfg_file_name);
			break;
		case 'w':
			w = atoi(optarg);
			DBG("image width:%d\n", w);
			g_dim_set++;
			break;
		case 'h':
			h = atoi(optarg);
			DBG("image height:%d\n", h);
			g_dim_set++;
			break;
		case 'l':
			sprintf(obj_list_file, optarg);
			DBG("obj_list_file:%s\n", obj_list_file);
			break;
		case 'H':
			help();
			exit(0);
		default:
			abort();
		}
	}

	if (strlen(cfg_file_name) == 0) {
		fprintf(stderr, "Config file path is not specified !\n");
		help();
		exit(1);
	}

	if (strlen(img_file) == 0) {
		fprintf(stderr, "Image file path is not specified !\n");
		help();
		exit(1);
	}

	if (strlen(obj_list_file) == 0) {
		fprintf(stderr, "Object list file path is not specified !\n");
		help();
		exit(1);
	}

	hd_score(img_file, w, h, obj_list_file, cfg_file_name);

	return 0;
}
