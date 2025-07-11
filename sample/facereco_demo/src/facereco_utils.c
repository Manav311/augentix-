#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <glob.h>
#include <libgen.h>
#include <sys/stat.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_errno.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#include "inf_image.h"
#include "inf_face.h"

#define FACE_IMAGE_DEFAULT_PATH "/usrdata/eaif/facereco/faces"
#define FACE_BIN_DEFAULT_PATH "/usrdata/eaif/facereco/face.bin"
#define FACERECO_INI_DEFAULT_PATH "/system/eaif/models/facereco/inapp_facereco.ini"
#define FACE_IMG_FMT "{*.jpg}"

#define assert_ret(cond, ret, fmt, args...)                                                                           \
	do {                                                                                                          \
		if (!(cond)) {                                                                                        \
			fprintf(stderr, "%s:%d Assert fail[\"" fmt "\"] cond:( %s ) !\n", __func__, __LINE__, ##args, \
			        #cond);                                                                               \
			return (ret);                                                                                 \
		}                                                                                                     \
	} while (0)

typedef union {
	int val;
	struct {
		int _query : 1;
		int _register : 1;
		int _delete : 1;
		int _reset : 1;
		int _test: 1;
	};
} FACE_CMD;

int g_inf_with_list = 0;
char g_glob_fmt[256] = FACE_IMG_FMT;

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

void help(void);

int isDirectory(const char *path)
{
	struct stat statbuf;
	if (stat(path, &statbuf) != 0)
		return 0;
	return S_ISDIR(statbuf.st_mode);
}

char *getBasename(const char *name)
{
	int str_len = strlen(name);
	static char fface[256] = {};
	char *face = NULL;
	strcpy(fface, name);
	if (fface[str_len - 4] == '.')
		fface[str_len - 4] = 0;
	else if (fface[str_len - 5] == '.')
		fface[str_len - 5] = 0;
	face = basename(fface);
	return face;
}

int mkdirp_helper(char *path, int *err);

int mkdirp_helper(char *path, int *err)
{
	if (access(path, F_OK) != 0) {
		char local_path[256] = {};
		mode_t target_mode = 0777;
		strcpy(local_path, path);
		char *dir = dirname(local_path);
		mkdirp_helper(dir, err);
		if (*err == EEXIST)
			return 0;
		*err = mkdir(path, target_mode);
		if (*err == 0)
			chmod(path, target_mode);
	}
	return 0;
}

int mkdirp(const char *path, int *err)
{
	char local_path[256] = {};
	strcpy(local_path, path);
	return mkdirp_helper(local_path, err);
}

int mkFileDirIfNotExist(const char *path)
{
	char filedir[256] = {};
	strcpy(filedir, path);
	char *dir = dirname(filedir);
	if (access(dir, F_OK) != 0) {
		int err = 0;
		return mkdirp(dir, &err);
	}
	return 0;
}

void printName(const InfStrList *labels)
{
	printf("Database (total %d person registered) \nname list: ", labels->size - 1);
	for (int i = 1; i < labels->size; i++) {
		printf("%s, ", labels->data[i]);
	}
	printf("\n");
}
int readImage(const char *img_file, InfImage *img)
{
	int fn_len = strlen(img_file);
	char *ext = (char *)(img_file + fn_len - 3);
	int ret = 0;
	if (!strcmp(ext, "jpg") || !strcmp(ext, "png") || !strcmp(ext, "pgm") || !strcmp(ext, "ppm") ||
	    !strcmp((char *)(ext - 1), "jpeg"))
		ret = Inf_Imread(img_file, img, 0);
	else {
		fprintf(stderr, "[ERROR] Unknown/Unsupported image format!\n");
		exit(0);
	}
	return ret;
}

int copyRegisterFace(const char *src_image, const char *dst_path)
{
	char buf[4096] = {};
	char dst_image_path[512] = {};
	strcpy(buf, src_image);
	sprintf(dst_image_path, "%s/%s", dst_path, basename(buf));
	int copy_size = 4096;

	FILE *fr = fopen(src_image, "r");
	FILE *fw = fopen(dst_image_path, "w");

	if (!fr) {
		fprintf(stderr, "[ERROR] Cannot Open src \"%s\"!\n", src_image);
		return -1;
	}
	if (!fw) {
		fclose(fr);
		fprintf(stderr, "[ERROR] Cannot Open dst \"%s\"!\n", dst_image_path);
		return -1;
	}

	while (!feof(fr)) {
		fread(buf, 1, copy_size, fr);
		fwrite(buf, 1, copy_size, fw);
	}

	fclose(fr);
	fclose(fw);
	return 0;
}

int rmRegisterFace(const char *face, const char *dst_path)
{
	char fmt[4096] = { 0 };
	sprintf(fmt, "%s/%s.*", dst_path, face);
	glob_t globbuf = { 0 };
	glob(fmt, 0, NULL, &globbuf);
	for (int i = 0; (UINT32)i < globbuf.gl_pathc; i++) {
		if (remove(globbuf.gl_pathv[i])) {
			fprintf(stderr, "[ERROR] Cannot Remove register face \"%s\"\n", globbuf.gl_pathv[i]);
			return -1;
		}
	}
	return 0;
}

int loadDataIfExist(InfModelCtx *ctx, const char *face_file)
{
	if (access(face_file, F_OK) != 0)
		return -1;

	printf("[INFO] Load data from %s\n", face_file);
	int ret = Inf_LoadFaceData(ctx, face_file);
	return ret;
}

int runQuery(const char *cfg_file_name, const char *face_bin_file)
{
	const char *config = cfg_file_name;
	const char *face_file = face_bin_file;
	InfModelCtx ctx = { 0 };
	int ret = Inf_InitModel(&ctx, config);
	assert_ret(ret == 0, -1, "Cannot initialize model from \"%s\"", config);

	ret = loadDataIfExist(&ctx, face_file);
	if (ret) {
		printf("[ERROR] Cannot load face data from %s!\n", face_file);
	} else {
		printName(&ctx.info->labels);
	}
	ret = Inf_ReleaseModel(&ctx);
	assert_ret(ret == 0 && !ctx.model, -1, "Cannot release model");
	return 0;
}

int runDelete(const char *cfg_file_name, const char *face_bin_file, const char *register_image_path,
              const char *face_name)
{
	const char *config = cfg_file_name;
	const char *face_file = face_bin_file;
	InfModelCtx ctx = { 0 };
	int ret = Inf_InitModel(&ctx, config);
	assert_ret(ret == 0, -1, "Cannot initialize model from \"%s\"", config);

	ret = loadDataIfExist(&ctx, face_file);
	if (ret) {
		fprintf(stderr, "[ERROR] Cannot load face data from %s!\n", face_file);
	} else {
		printf("[INFO] Delete %s from model\n", face_name);
		ret = Inf_DeleteFaceData(&ctx, face_name);
		if (ret) {
			if (ret == -ENODATA)
				fprintf(stderr, "[ERROR] Cannot delete %s as no such face in the db!\n", face_name);
			else
				fprintf(stderr, "[ERROR] Cannot delete %s!\n", face_name);
		} else {
			printName(&ctx.info->labels);
			ret = mkFileDirIfNotExist(face_file);
			if (ret == 0) {
				printf("[INFO] update database!\n");
				ret = Inf_SaveFaceData(&ctx, face_file);
				if (ret) {
					fprintf(stderr, "[ERROR] Cannot save face data!\n");
				} else {
					rmRegisterFace(face_name, register_image_path);
				}
			}
		}
	}

	ret = Inf_ReleaseModel(&ctx);
	assert_ret(ret == 0 && !ctx.model, -1, "Cannot release model");
	return 0;
}

int runReset(const char *cfg_file_name, const char *face_bin_file, const char *register_image_path)
{
	const char *config = cfg_file_name;
	const char *face_file = face_bin_file;
	InfModelCtx ctx = { 0 };
	int ret = Inf_InitModel(&ctx, config);
	assert_ret(ret == 0, -1, "Cannot initialize model from \"%s\"", config);

	ret = loadDataIfExist(&ctx, face_file);
	if (ret) {
		fprintf(stderr, "[ERROR] Cannot load face data from %s!\n", face_file);
	} else {
		printf("[INFO] Reset Data\n");
		ret = Inf_ResetFaceData(&ctx);
		if (ret) {
			fprintf(stderr, "[ERROR] Cannot reset database!\n");
		} else {
			printName(&ctx.info->labels);
			ret = mkFileDirIfNotExist(face_file);
			if (ret == 0) {
				printf("[INFO] update database!\n");
				ret = Inf_SaveFaceData(&ctx, face_file);
				if (ret) {
					fprintf(stderr, "[ERROR] Cannot save face data!\n");
				} else {
					printf("[INFO] Removing all images!\n");
					rmRegisterFace("*", register_image_path);
				}
			}
		}
	}

	ret = Inf_ReleaseModel(&ctx);
	assert_ret(ret == 0 && !ctx.model, -1, "Cannot release model");
	return 0;
}

int runRegisterSingleFace(InfModelCtx *ctx, char *img_file)
{
	const char *img_name = img_file;
	char *face = getBasename(img_name);

	InfImage img = { 0 };
	Inf_Imread(img_name, &img, 0);
	MPI_RECT_POINT_S roi = { 0, 0, img.w - 1, img.h - 1 };
	int ret = Inf_RegisterFaceRoiDet(ctx, &img, &roi, face);
	assert_ret(ret == 0, -1, "Cannot regis face \"%s\" from \"%s\"", face, img_name);
	Inf_Imrelease(&img);
	assert_ret(img.data == 0, -1, "cannot release img!");

	if (ret == 0) {
		printf("[INFO] Success to register %s\n", face);
	}

	return 0;
}

int runRegister(const char *cfg_file_name, const char *face_bin_file, const char *register_image_path, char *input)
{
	const char *config = cfg_file_name;
	const char *face_file = face_bin_file;
	const char *register_img_dir = register_image_path;
	const char *glob_fmt = g_glob_fmt;

	InfModelCtx ctx;
	int ret = Inf_InitModel(&ctx, config);
	assert_ret(ret == 0 && ctx.model, -1, "Cannot init face reco model \"%s\"!", config);

	if (access(face_bin_file, F_OK) == 0) {
		ret = Inf_LoadFaceData(&ctx, face_file);
		if (ret) {
			fprintf(stderr, "[ERROR] Cannot load existing face database!\n");
		}
	}

	char *prefix = getenv("INF_CAP_PREFIX");
	if (prefix)
		Inf_Setup(&ctx, 0, 1, 1);

	int size_update = ctx.info->labels.size;

	glob_t globbuf = { 0 };

	int *markers = 0;

	bool is_dir = isDirectory(input);
	if (is_dir) {
		char fmt[256] = {};
		sprintf(fmt, "%s/%s", input, glob_fmt);
		glob(fmt, GLOB_BRACE, NULL, &globbuf);
		markers = (int *)calloc(1, sizeof(int) * globbuf.gl_pathc);
		for (int i = 0; (UINT32)i < globbuf.gl_pathc; ++i) {
			char *path = globbuf.gl_pathv[i];
			if (path[0] != 0) {
				ret = runRegisterSingleFace(&ctx, path);
				if (ret) {
					fprintf(stderr, "Cannot register %s\n", path);
				} else {
					markers[i] = 1;
				}
			}
		}
	} else {
		ret = runRegisterSingleFace(&ctx, input);
	}

	if (size_update != ctx.info->labels.size) {
		printName(&ctx.info->labels);
		int err = 0;
		ret = mkFileDirIfNotExist(face_file);
		if (ret == 0) {
			printf("[INFO] update database!\n");
			ret = Inf_SaveFaceData(&ctx, face_file);
			if (ret) {
				fprintf(stderr, "[ERROR] Cannot save face data!\n");
			} else {
				mkdirp(register_img_dir, &err);
				printf("[INFO] copying registered face image to %s!\n", register_img_dir);
				if (is_dir) {
					for (int i = 0; (UINT32)i < globbuf.gl_pathc; ++i) {
						char *src = globbuf.gl_pathv[i];
						if (markers[i]) {
							copyRegisterFace(src, register_img_dir);
						}
					}
				} else {
					copyRegisterFace(input, register_img_dir);
				}
			}
		}
	}

	if (markers)
		free(markers);

	if (globbuf.gl_pathc)
		globfree(&globbuf);

	ret = Inf_ReleaseModel(&ctx);
	assert_ret(ret == 0 && !ctx.model, -1, "Cannot release model");
	return 0;
}

int runTestSingleFace(InfModelCtx *ctx, char *img_file)
{
	const char *img_name = img_file;
	char *face = getBasename(img_name);

	InfImage img = { 0 };
	InfDetList result = { 0 };
	const InfStrList *labels = NULL;
	Inf_Imread(img_name, &img, 0);
	MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	obj_list.obj_num = 1;
	obj_list.obj[0].id = 1;
	obj_list.obj[0].rect = (MPI_RECT_POINT_S){ 0, 0, img.w - 1, img.h - 1 };
	obj_list.obj[0].life = 160;
	int ret = 0;
	if (g_inf_with_list)
		ret = Inf_InvokeFaceDetObjList(ctx, &img, &obj_list, &result);
	else
		ret = Inf_InvokeFaceDet(ctx, &img, &result);

	assert_ret(ret == 0, -1, "Cannot test face \"%s\" from \"%s\"", face, img_name);
	Inf_Imrelease(&img);
	assert_ret(img.data == 0, -1, "cannot release img!");

	if (ret == 0) {
		if (result.size) {
			for (int i = 0; i < result.size; i++) {
				labels = &ctx->info->labels;
				if (result.data[i].cls_num == 0) {
					printf("Results for ---> %s (%d faces in db)\n", img_name, labels->size);
				} else {
					printf("Recognize as (%s) for ---> %s (%d faces in db)\n",
					labels->data[result.data[i].cls[0]], img_name, labels->size-1);
				}
				for (int j = 0; j < result.data[i].prob_num; j++) {
					printf("\t%-16s : %.3f\n", labels->data[j], result.data[i].prob[j]);
				}
			}
			Inf_ReleaseDetResult(&result);
		} else {
			printf("[INFO] No face detected.\n");
		}
	}

	return 0;
}

int runTest(const char *cfg_file_name, const char *face_bin_file, char *input)
{
	const char *config = cfg_file_name;
	const char *face_file = face_bin_file;
	const char *glob_fmt = g_glob_fmt;

	InfModelCtx ctx;
	int ret = Inf_InitModel(&ctx, config);
	assert_ret(ret == 0 && ctx.model, -1, "Cannot init face reco model \"%s\"!", config);

	if (access(face_bin_file, F_OK) == 0) {
		ret = Inf_LoadFaceData(&ctx, face_file);
		if (ret) {
			fprintf(stderr, "[ERROR] Cannot load existing face database!\n");
		} else {
			printName(&ctx.info->labels);
		}
	}

	glob_t globbuf = { 0 };

	if (isDirectory(input)) {
		char fmt[256] = { 0 };
		sprintf(fmt, "%s/%s", input, glob_fmt);
		glob(fmt, GLOB_BRACE, NULL, &globbuf);
		for (int i = 0; (UINT32)i < globbuf.gl_pathc; ++i) {
			char *path = globbuf.gl_pathv[i];
			if (path[0] != 0) {
				ret = runTestSingleFace(&ctx, path);
				if (ret) {
					fprintf(stderr, "Cannot test %s\n", path);
				}
			}
		}
		globfree(&globbuf);
	} else {
		ret = runTestSingleFace(&ctx, input);
		if (ret) {
			fprintf(stderr, "Cannot test %s\n", input);
		}
	}

	ret = Inf_ReleaseModel(&ctx);
	assert_ret(ret == 0 && !ctx.model, -1, "Cannot release model");
	return 0;
}

void help(void)
{
	printf("USAGE:\tfacereco_utils <request-command>\t\n");
	printf("\t-r <image-path/img_file> \t- Register faces, if it is a path, it searches all jpg file inside\n"
	       "\t                         \t  the directory, and take its name as face name\n");
	printf("\t-t <image-path/img_file> \t- Test faces, if it is a path, it searches all jpg file inside\n"
	       "\t                         \t  the directory\n");
	printf("\t-q                       \t- Query faces in database\n");
	printf("\t-d <face name>           \t- Delete face name registered in database\n");
	printf("\t-s                       \t- Reset database\n");
	printf("\t-H/?                     \t- Display help message and exit\n\n");
	printf("Optional:\n");
	printf("\t-l                       \t- Allow faster face position detection\n");
	printf("\t-c <model config file>   \t- Model config path (default: \"/system/eaif/models/facereco/inapp_facereco.ini\")\n");
	printf("\t-f <face database path>  \t- Face database path (default: \"/usrdata/eaif/facereco/face.bin\")\n");
	printf("\t-i <face images path>    \t- Path to store face images after registration (default: \"/usrdata/eaif/facereco/faces\")\n");
	printf("\t-m <glob image format>   \t- Glob image format (default: \"{*.jpg})\"\n\n");
	printf("For example:\n");
	printf("\t** Register face\n");
	printf("\t\tfacereco_utils -r tony.jpg\n");
	printf("\t** Test face\n");
	printf("\t\tfacereco_utils -t tony.jpg\n");
	printf("\t\tfacereco_utils -t faces_dir\n");
	printf("\t** Query database\n");
	printf("\t\tfacereco_utils -q\n");
	printf("\t** Delete registered face from database\n");
	printf("\t\tfacereco_utils -d tony\n");
	printf("\t** Reset face database\n");
	printf("\t\tfacereco_utils -s\n");
}

int check(const char *file)
{
	if (access(file, F_OK) != 0) {
		printf("[ERROR] file %s not exists!\n", file);
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	char face_bin_file[256] = FACE_BIN_DEFAULT_PATH;
	char cfg_file_name[256] = FACERECO_INI_DEFAULT_PATH;
	char register_image_path[256] = FACE_IMAGE_DEFAULT_PATH;
	char input[256] = {};
	int c;

	FACE_CMD cmd = {};

	while ((c = getopt(argc, argv, "c:f:i:m:d:r:t:qslH?")) != -1) {
		switch (c) {
		case 'c':
			sprintf(cfg_file_name, optarg);
			DBG("cfg_file_name:%s\n", cfg_file_name);
			break;
		case 'f':
			sprintf(face_bin_file, optarg);
			DBG("face_bin_file:%s\n", face_bin_file);
			break;
		case 'i':
			sprintf(register_image_path, optarg);
			DBG("register_image_path:%s\n", face_bin_file);
			break;
		case 'm':
			sprintf(g_glob_fmt, optarg);
			DBG("glob image format:%s\n", g_glob_fmt);
			break;
		case 'r':
			assert_ret(cmd.val == 0, -1, "Cannot have more than one request command!\n");
			strcpy(input, optarg);
			cmd._register = 1;
			DBG("request to register face\n");
			break;
		case 't':
			assert_ret(cmd.val == 0, -1, "Cannot have more than one request command!\n");
			strcpy(input, optarg);
			cmd._test = 1;
			DBG("request to test face\n");
			break;
		case 'd':
			assert_ret(cmd.val == 0, -1, "Cannot have more than one request command!\n");
			strcpy(input, optarg);
			cmd._delete = 1;
			DBG("request to delete face\n");
			break;
		case 'q':
			assert_ret(cmd.val == 0, -1, "Cannot have more than one request command!\n");
			cmd._query = 1;
			DBG("request to query faces in database\n");
			break;
		case 's':
			assert_ret(cmd.val == 0, -1, "Cannot have more than one request command!\n");
			cmd._reset = 1;
			DBG("request to reset face database\n");
			break;
		case 'l':
			g_inf_with_list = 1;
			break;
		case 'H':
		case '?':
			help();
			exit(0);
			break;
		default:
			abort();
		}
	}

	if (!cmd.val)
		cmd._query = 1;

	if (check(cfg_file_name)) {
		printf("Model config %s not found!\n", cfg_file_name);
		return -1;
	}

	if (cmd._register && check(input) == 0) {
		runRegister(cfg_file_name, face_bin_file, register_image_path, input);
	} else if (cmd._test && check(input) == 0) {
		runTest(cfg_file_name, face_bin_file, input);
	} else if (cmd._query && check(face_bin_file) == 0) {
		runQuery(cfg_file_name, face_bin_file);
	} else if (cmd._delete) {
		runDelete(cfg_file_name, face_bin_file, register_image_path, input);
	} else if (cmd._reset) {
		runReset(cfg_file_name, face_bin_file, register_image_path);
	}

	return 0;
}
