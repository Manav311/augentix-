#include <asm-generic/errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

#include "sample_ctrl.h"
#include "sample_utils.h"
#include "sample_stream.h"
#include "sample_light_src.h"
#include "sample_dip.h"
#include "utlist.h"

#include "parse_dev.h"
#include "parse_enc.h"
#include "parse_utils.h"
#include "parse_sys.h"
#include "parse_agtx.h"

#include "agtx_res_option.h"
#include "agtx_video_layout_conf.h"
#include "agtx_video.h"
#include "app_view_api.h"
#include "agtx_cmd.h"
#include "agtx_types.h"

#define PARAM_MAX (256)

typedef enum test_case {
	RESOLUTION = 0,
	LAYOUT,
	FPS,
	WIN_ROI,
	MIRROR,
	FLIP,
	VIEW_TYPE,
	CODEC,
	RATE_CONTROL,
	GOP,
	BITRATE,
	TEST_CASE_NUM,
} TestCase;

typedef struct test_loop {
	int interval;
	int times;
} TestLoop;

/**
 * @struct APP_CONF_S
 * @brief contain all configuration fron case_config and arguments
 */
typedef struct {
	SAMPLE_CONF_S sample_conf; /**< all config from case_config */
	INT32 reservation_level; /**< for sample_publisher */
	INT32 recycle_level; /**< for sample_publisher*/
	LightSrc *head[MPI_MAX_INPUT_PATH_NUM]; /**< whether support multiple iq or not, day lightSrc always created. */
	LightSrcDetection
	        *detection[MPI_MAX_INPUT_PATH_NUM]; /**< lightSrc detection method and params of each input path*/
} APP_CONF_S;

APP_CONF_S g_conf;
int g_run_flag = 1;

/**
 * @brief Set global flag g_bsb_run[] as 0 when SIGINT or SIGTERM is received.
 */
static void sigintHandler(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}

	g_run_flag = 0;
}

static inline void getBitRate(MPI_MCVC_RC_ATTR_S *rc, UINT32 *rate)
{
	switch (rc->mode) {
	case MPI_RC_MODE_VBR:
		*rate = ((MPI_MCVC_VBR_PARAM_S)rc->vbr).max_bit_rate;
		break;

	case MPI_RC_MODE_CBR:
		*rate = ((MPI_MCVC_CBR_PARAM_S)rc->cbr).bit_rate;
		break;

	case MPI_RC_MODE_SBR:
		*rate = ((MPI_MCVC_SBR_PARAM_S)rc->sbr).bit_rate;
		break;

	default:
		break;
	}

	return;
}

void init_win_conf(MPI_WIN_ATTR_S *p)
{
	p->path.bmp = 0x0;
	p->fps = 0;
	p->rotate = 0;
	p->mirr_en = 0;
	p->flip_en = 0;
	p->eis_en = 0;
	p->view_type = 0;
	p->roi.x = 0;
	p->roi.y = 0;
	p->roi.width = 0;
	p->roi.height = 0;
	p->prio = 0;
	p->src_id = MPI_INVALID_VIDEO_WIN;
	p->const_qual = 0;
	p->dyn_adj = 0;
}

void init_chn_conf(CONF_CHN_PARAM_S *p)
{
	//CONF_CASE_VFTR_PARAM *vftr = &p->vftr;
	MPI_CHN_ATTR_S *attr = &p->attr;
	MPI_CHN_LAYOUT_S *layout = &p->layout;
	MPI_WIN_ATTR_S *window = &p->win[0];

	/* Init channel attr */
	attr->res.width = 0;
	attr->res.height = 0;
	attr->fps = 0;
	attr->binding_capability = 1; // #45629#note-10 forward compatibility

	/* Init channel latout */
	memset(layout, 0, sizeof(MPI_CHN_LAYOUT_S));

	/* Init window attr */
	memset(window, 0, sizeof(MPI_WIN_ATTR_S) * MPI_MAX_VIDEO_WIN_NUM);
}

/**
 * @brief Initialize APP_CONF_S, 
 * @details
 * Rules to initialize configurations:
 *  - Enable bit always set to 0.
 *  - Use common / normal setting for enum. ex: op_mode = MPI_OP_MODE_NORMAL.
 *  - create LightSrc and LightSrcDetection object by input path.
 *  - not support multiple IQ 
 * @param[out] conf    pointer to configuration struct
 */
static void initConf(APP_CONF_S *conf)
{
	CONF_CASE_GEN_PARAM *casegen;
	CONF_DEV_PARAM_S *dev;
	CONF_CHN_PARAM_S *chn;
	MPI_WIN_ATTR_S *win;
	int i, j, k;
	/** libsample test app default not support OSD */
	conf->sample_conf.osd_visible = false;

	casegen = &conf->sample_conf.casegen;

	casegen->show_params = 0;

	init_sys_conf(&conf->sample_conf.sys);

	for (i = 0; i < MPI_MAX_VIDEO_DEV_NUM; ++i) {
		dev = &conf->sample_conf.dev[i];

		dev->attr.hdr_mode = MPI_HDR_MODE_NONE;
		dev->attr.stitch_en = 0;
		dev->attr.eis_en = 0;
		dev->attr.fps = 0.0;
		dev->attr.bayer = MPI_BAYER_PHASE_NUM;
		dev->attr.path.bmp = 0x0;

		memset(&dev->stitch, 0, sizeof(MPI_STITCH_ATTR_S));
		memset(&dev->ldc, 0, sizeof(MPI_LDC_ATTR_S));
		memset(&dev->panorama, 0, sizeof(MPI_PANORAMA_ATTR_S));
		memset(&dev->panning, 0, sizeof(MPI_PANNING_ATTR_S));
		memset(&dev->surround, 0, sizeof(MPI_SURROUND_ATTR_S));

		for (j = 0; j < MPI_MAX_VIDEO_CHN_NUM; ++j) {
			chn = &dev->chn[j];
			init_chn_conf(chn);

			for (k = 0; k < MPI_MAX_VIDEO_WIN_NUM; ++k) {
				win = &dev->chn[j].win[k];
				init_win_conf(win);
			}
		}
	}

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		conf->sample_conf.enc_chn[i] = (CONF_ECHN_PARAM_S){ .venc_ex = {
			                                                    .obs = MPI_VENC_OBS_DISABLE,
			                                                    .obs_off_period = 2u,
			                                            } };
	}

	for (i = 0; i < MPI_MAX_INPUT_PATH_NUM; i++) {
		char ini_name[PATH_MAX];

		sprintf(ini_name, "%s/sensor_%d.ini", DIP_FILE_PATH, i);
		if (access(ini_name, R_OK) != 0) {
			sprintf(ini_name, "%s/sensor_0.ini", DIP_FILE_PATH);
		}
		conf->head[i] = SAMPLE_newDayLightSrc("day", ini_name);
	}
}

/**
 * @brief deInitialize APP_CONF_S
 * @details
 *  - free LightSrc and LightSrcDetection object of each input path.
 * @param[out] conf    pointer to configuration struct
 */
static void deinitConf(APP_CONF_S *conf)
{
	LightSrc *item, *tmp;
	LightSrc *light_src_head;
	int idx = 0;
	for (idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		light_src_head = conf->head[idx];

		/** day lightSrc still be created in not support multiple iq case */
		LL_FOREACH_SAFE(light_src_head, item, tmp)
		{
			LL_DELETE(light_src_head, item);
			SAMPLE_deleteLightSrc(item);
		}
	}

	return;
}

static int parse_case_param(char *tok, SAMPLE_CONF_S *conf)
{
	int hit = 1;
	CONF_CASE_GEN_PARAM *p = &conf->casegen;

	if (!strcmp(tok, "sid")) {
		// Deprecated option.
		strtok(NULL, " =");
	} else if (!strcmp(tok, "show_params")) {
		get_value((void *)&p->show_params, TYPE_UINT8);
	} else {
		hit = 0;
	}

	return hit;
}

/**
 * @brief parse 1 parameter
 * @param[in]  str     string in format "key=value"
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 */
static int parse_param(char *str, SAMPLE_CONF_S *conf)
{
	int hit = 0;
	char *tok = strtok(str, " =");

	while (tok != NULL) {
		hit = 0;

		/* Stop parsing this line when comment sign found */
		if (!strncmp(tok, "#", strlen("#"))) {
			hit = 1;
			break;
		}

		/* Parse case general parameter */
		hit = parse_case_param(tok, conf);
		if (hit) {
			goto next;
		}

		/* Parse system parameter */
		hit = parse_sys_param(tok, conf);
		if (hit) {
			goto next;
		}

		/* Parse device parameter */
		hit = parse_dev_param(tok, conf);
		if (hit) {
			goto next;
		}

		/* Parse encoder parameter */
		hit = parse_enc_chn_param(tok, conf);
		if (hit) {
			goto next;
		}

		if (!hit) {
			/* Bypass error for newline sign */
			if (!strcmp(tok, "\n")) {
				hit = 1;
			} else {
				/* Stop parsing when unknown parameter found */
				fprintf(stderr, "Unknown parameter: %s\n", tok);
				break;
			}
		}

	next:
		/* Parse other parameters in same line */
		tok = strtok(NULL, " =");
	}

	return hit;
}

/**
 * @brief parse the configuartion file
 * @param[in] filename name of the configuration file
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed
 * @retval positive parsed
 * @retval negative error occurs
 */
static int parse_config_file(const char *filename, SAMPLE_CONF_S *conf)
{
	int ret = 0;
	char str[256];
	FILE *fp;

	/* Open config file for parsing */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error opening file. err: %d\n", -errno);
		return -EINVAL;
	}

	/* Parse config file line by line */
	while (fgets(str, sizeof(str), fp) != NULL) {
		ret = parse_param(str, conf);

		/* Stop parsing when a unknown parameter found */
		if (!ret) {
			fprintf(stderr, "Parsing parameter file failed.\n");
			break;
		}
	}

	fclose(fp);

	return ret;
}

/**
 * @brief Transform setting based on application spec.
 */
static void adjustConf(SAMPLE_CONF_S *conf)
{
	MPI_CHN_LAYOUT_S *layout;
	MPI_CHN_ATTR_S *attr;
	int i, j, k;

	for (i = 0; i < MPI_MAX_VIDEO_DEV_NUM; i++) {
		/**
		 * Update member field 'path' in struct MPI_DEV_ATTR_S
		 *
		 * path<N>_en must be set as 1 when you want to read image
		 * signal from path<N>.
		 */
		conf->dev[i].attr.path.bit.path0_en = conf->dev[i].path[0].enable;
		conf->dev[i].attr.path.bit.path1_en = conf->dev[i].path[1].enable;

		/**
		 * Generate path to PCA setting file.
		 * Similar to the default .ini file, but without flexibility to change the target path.
		 */
		for (j = 0; j < MPI_MAX_INPUT_PATH_NUM; j++) {
			snprintf(conf->dev[i].path[j].pca_file, PATH_MAX - 1, "%s/pca_cal_%d.lut", DIP_FILE_PATH, j);
		}

		/**
		 * Transfrom video channel layout from ratio (max: 1024) to absolute coordinate.
		 * The coordinate of right bottom corder should not run over 1024.
		 */
		for (j = 0; j < MPI_MAX_VIDEO_CHN_NUM; j++) {
			attr = &conf->dev[i].chn[j].attr;
			layout = &conf->dev[i].chn[j].layout;

			for (k = 0; k < layout->window_num; k++) {
				layout->win_id[k] = MPI_VIDEO_WIN(i, j, k);

				assert(layout->window[k].x <= 1024);
				assert(layout->window[k].y <= 1024);
				assert(layout->window[k].x + layout->window[k].width <= 1024);
				assert(layout->window[k].y + layout->window[k].height <= 1024);
				layout->window[k] = SAMPLE_toMpiLayoutWindow(&layout->window[k], &attr->res);
			}
		}
	}
}

/**
 * @brief Print help messages to stdout
 * @param[in] str    name of executable
 */
static void help(char *pname)
{
	printf("Usage: %s -d case_config [OPTION]...\n", pname);
	printf("Example: %s -d /system/mpp/case_config/case_config_1001_FHD\n", pname);
	printf("--loop <interval>,<times>  test case repeat, from case_config to argv \n");
	printf("e.g. --loop 1,10 loop for 10 times, interval:1 sec\n\n");
	printf("Options: [only have 1, loop opt (if exist) at first]\n");
	printf("\t--res <chn idx>,<width>,<height> e.g. -res 0,1920,1080\n");
	printf("\t--layout <chn idx>,<layout.json>\n");
	printf("\t--fps <chn idx>,<fps num> \n");
	printf("\t--win_roi <dev idx>,<chn idx>,<win idx>,<win_roi x>,<win_roi y>,<win_roi w>,<win_roi h>\n");
	printf("\t--mirror <0 for off, 1 for on>\n");
	printf("\t--flip <0 for off, 1 for on>\n");
	printf("\t--view_type <view_type name>,<view_type.json>\n");
	printf("\te.g. --view_type ldc,test_ldc.json (only support WIN(0,0,0))\n");
	printf("\t\tvalid view_type: panning, panorama, stitch, ldc, surround\n");
	printf("\t--codec <enc idx>,<video_strm.json>\n");
	printf("\t--rc <enc idx>,<video_strm.json>\n");
	printf("\t--gop <enc idx>,<gop size>\n");
	printf("\t--bitrate <enc idx>,<bitrate size>\n");

	return;
}

/**
 * @brief parse the command arguments and convert as configuration struct
 * @param[in] argc     argument count
 * @param[in] argv     argument vector
 * @param[out] conf    pointer to configuration struct
 * @param[out] test_case    whitch case to test, only 1
 * @param[out] param   test case attribute str
 * @param[out] loop looping test case & case_config 
 * @return The execution result
 * @retval 0        nothing is parsed or some error occurs
 * @retval positive parsed
 */
static int parseCmdArgs(int argc, char *argv[], APP_CONF_S *conf, TestCase *test_case, char *param, TestLoop *loop)
{
	int ret = 0;
	int c;
	char *del = (char *)",";
	char *p = NULL;

	const char *optstring = "R:l:F:m:f:w:v:c:r:g:b:d:hl:L:";
	struct option opts[] = { { "res", 1, NULL, 'R' },       { "layout", 1, NULL, 'l' },
		                 { "fps", 1, NULL, 'F' },       { "win_roi", 1, NULL, 'w' },
		                 { "mirror", 1, NULL, 'm' },    { "flip", 1, NULL, 'f' },
		                 { "view_type", 1, NULL, 'v' }, { "codec", 1, NULL, 'c' },
		                 { "rc", 1, NULL, 'r' },        { "gop", 1, NULL, 'g' },
		                 { "bitrate", 1, NULL, 'b' },   { "case_config", 1, NULL, 'd' },
		                 { "help", 0, NULL, 'h' },      { "loop", 1, NULL, 'L' } };
	*test_case = TEST_CASE_NUM;

	if (argc == 1) {
		help(NULL);
		return -EALREADY;
	}

	while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
		/** only select 1 test case */
		if (*test_case != TEST_CASE_NUM) {
			return 1;
		}

		printf("opt: %s\n", optarg);

		if (optarg != NULL) {
			strcpy(param, optarg);
		}

		switch (c) {
		case 'h':
			help(argv[0]);
			return -EALREADY;
			break;
		case 'd': /** basic case_config to start video pipeline */
			ret = parse_config_file(optarg, &conf->sample_conf);
			if (!ret) {
				return ret;
			}
			break;
		case 'L':
			p = strtok(optarg, del);
			loop->interval = atoi(p);
			p = strtok(NULL, del);
			loop->times = atoi(p);
			printf("loop for %d times, interval: %d\n", loop->times, loop->interval);

			break;
		case 'R':
			*test_case = RESOLUTION;
			break;
		case 'l':
			*test_case = LAYOUT;
			break;
		case 'F':
			*test_case = FPS;
			break;
		case 'w':
			*test_case = WIN_ROI;
			break;
		case 'm':
			*test_case = MIRROR;
			break;
		case 'f':
			*test_case = FLIP;
			break;
		case 'v':
			*test_case = VIEW_TYPE;
			break;
		case 'c':
			*test_case = CODEC;
			break;
		case 'r':
			*test_case = RATE_CONTROL;
			break;
		case 'g':
			*test_case = GOP;
			break;
		case 'b':
			*test_case = BITRATE;
			break;
		default:
			printf("No test case\n");
			return -EALREADY;
		}
	}

	/** always copy test case cmd */
	if (*test_case != TEST_CASE_NUM) {
		printf("end param: %s\n", param);
	}

	return ret;
}

/**
 * @brief diff with mpi_stream: not support OSD/frame_dump/UDP stream/update ini/multiple IQ
 * 
 * @param[in] conf 
 * @return int 0 for run success
 * @see stopStream
 */
static int startStream(const APP_CONF_S *conf)
{
	MPI_PATH path_idx;
	int idx = 0;
	int ret = 0;
	int dip_path_cnt = 1;

	/** Step 1. Initialize MPP system for the process. */

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Initialize system failed. err: %d\n", ret);
		return ret;
	}

	/** Step 2. Create and configure video pipeline. */
	ret = SAMPLE_createVideoPipeline(&conf->sample_conf);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	/** Step 3. Create thread to update DIP attributes when sensor IQ file is modified. */
	/** Step 3.1 Determine how many ini update threads are needed */
	if ((conf->sample_conf.dev[0].path[0].enable == 1 && conf->sample_conf.dev[0].path[1].enable == 1) &&
	    (conf->sample_conf.dev[0].stitch.enable == 0)) {
		/** Path 0,1 both enabled & not stitch */
		dip_path_cnt = 2;
	}

	/** Step 3.2 Create ini update threads for each input path */
	for (idx = 0; idx < dip_path_cnt; idx++) {
		path_idx = MPI_INPUT_PATH(0, idx);
		const CONF_PATH_PARAM_S *path_param = &conf->sample_conf.dev[0].path[idx];
		char *ini_path = NULL;

		/** whether support multiple iq or not, day lightSrc always created. */
		LightSrc *item, *tmp;
		LL_FOREACH_SAFE(conf->head[idx], item, tmp)
		{
			if (strcmp(item->name, "day") == 0) {
				ini_path = (char *)item->sensor_path;
				break;
			}
		}

		ret = SAMPLE_updateDipAttrOnce(path_idx, (const char *)ini_path, NULL);
		if (ret != 0) {
			return ret;
		}

		if (access(path_param->pca_file, R_OK) == 0) {
			ret = SAMPLE_updatePca(path_idx, path_param->pca_file);
			if (ret != 0) {
				fprintf(stderr, "Unable to set PCA settings, err: %d\n", ret);
			}
		}
	}

	/** Step 5. Start to run video pipeline. */
	ret = SAMPLE_startStream(&conf->sample_conf);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	/**
	 * Step 6. Create bit-stream channel to pull frames.
	 *
	 * After video pipeline is started, you can get frames from desired
	 * encoder (ECHN) channel via bit-stream channel (BCHN).
	 * To do so, User should initialize bit stream system first.
	 */

	MPI_initBitStreamSystem();

	return ret;
}

static int stopStream(const APP_CONF_S *conf)
{
	int ret = 0;

	/**
	 * When streaming is no need anymore, User should release the MPP resources
	 * properly. Generally you need to stop streaming, suspend video pipeline and
	 * destroy them, just do the actions with the reverse order of construction
	 * phase.
	 */

	MPI_exitBitStreamSystem();
	SAMPLE_stopStream(&conf->sample_conf);

	SAMPLE_destroyVideoPipeline(&conf->sample_conf);

	ret = MPI_SYS_exit();
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Exit system failed. err: %d\n", ret);
		return ret;
	}

	return ret;
}

static int runTestCase(TestCase *test, char *param)
{
	int ret = 0;
	char *del = (char *)",";
	char *p = NULL;
	int dev_idx = 0;
	int chn_idx = 0;
	int enc_idx = 0;
	int win_idx = 0;
	int enable = 0;
	int fps = 0;
	char str[PARAM_MAX];
	memcpy(str, param, sizeof(str));

	MPI_WIN_VIEW_TYPE_E view_type = MPI_WIN_VIEW_TYPE_NORMAL;
	char path[PATH_MAX];

	int gop = 0;
	int bitrate = 0;
	MPI_SIZE_S size = { 0, 0 };

	printf("start case, param: %s\n", str);
	switch (*test) {
	case RESOLUTION:
		p = strtok((char *)str, del); /** enc */
		chn_idx = atoi(p);
		p = strtok(NULL, del); /** resolution width */
		size.width = atoi(p);
		p = strtok(NULL, del); /** resolution height */
		size.height = atoi(p);
		SAMPLE_reconfigResolution(&g_conf.sample_conf, MPI_VIDEO_CHN(0, chn_idx), &size);
		break;
	case LAYOUT:
		p = strtok((char *)str, del); /** chn  idx */
		chn_idx = atoi(p);
		p = strtok(NULL, del); /** path */
		strncpy(path, p, sizeof(path));
		ret = parseLayout(&g_conf.sample_conf, chn_idx, path);
		break;
	case FPS:
		p = strtok((char *)str, del); /** chn idx */
		chn_idx = atoi(p);
		p = strtok(NULL, del); /** fps */
		fps = atoi(p);
		ret = SAMPLE_updateFps(MPI_VIDEO_CHN(0, chn_idx), fps);
		break;
	case WIN_ROI:
		p = strtok((char *)str, del); /** dev idx */
		dev_idx = atoi(p);
		p = strtok(NULL, del); /** chn idx */
		chn_idx = atoi(p);
		p = strtok(NULL, del); /** win idx */
		win_idx = atoi(p);

		MPI_WIN_ATTR_S window_attr;
		p = strtok(NULL, del); /** roi.x */
		window_attr.roi.x = atoi(p);
		p = strtok(NULL, del); /** roi.y */
		window_attr.roi.y = atoi(p);
		p = strtok(NULL, del); /** roi.w */
		window_attr.roi.width = atoi(p);
		p = strtok(NULL, del); /** roi.h */
		window_attr.roi.height = atoi(p);
		ret = SAMPLE_updateWindowRoi(MPI_VIDEO_WIN(dev_idx, chn_idx, win_idx), &(window_attr.roi));
		break;
	case MIRROR:
		p = strtok((char *)str, del); /** enable */
		enable = atoi(p);
		ret = SAMPLE_updateMirrorAttr(enable);
		break;
	case FLIP:
		p = strtok((char *)str, del); /** enable */
		enable = atoi(p);
		ret = SAMPLE_updateFlipAttr(enable);
		break;
	case VIEW_TYPE:
		p = strtok((char *)str, del); /** view_type */
		if (!strcmp(p, "ldc")) {
			view_type = MPI_WIN_VIEW_TYPE_LDC;
		} else if (!strcmp(p, "panorama")) {
			view_type = MPI_WIN_VIEW_TYPE_PANORAMA;
		} else if (!strcmp(p, "panning")) {
			view_type = MPI_WIN_VIEW_TYPE_PANNING;
		} else if (!strcmp(p, "surround")) {
			view_type = MPI_WIN_VIEW_TYPE_SURROUND;
		} else if (!strcmp(p, "stitch")) {
			view_type = MPI_WIN_VIEW_TYPE_STITCH;
		} else if (!strcmp(p, "normal")) {
			/** do nothing */
		}

		p = strtok(NULL, del); /** path */
		strncpy(path, p, sizeof(path));

		ret = parseViewTypeAttr(&g_conf.sample_conf, view_type, path);
		break;
	case CODEC:
		p = strtok((char *)str, del); /** enc_idx */
		enc_idx = atoi(p);
		p = strtok(NULL, del); /** path */
		strncpy(path, p, sizeof(path));

		ret = parseCodecAttr(&g_conf.sample_conf, enc_idx, path);
		break;
	case RATE_CONTROL:
		p = strtok((char *)str, del); /** enc_idx */
		enc_idx = atoi(p);
		p = strtok(NULL, del); /** path */
		strncpy(path, p, sizeof(path));
		ret = parseRateControlAttr(enc_idx, path);
		break;
	case GOP:
		p = strtok((char *)str, del); /** enc */
		enc_idx = atoi(p);
		p = strtok(NULL, del); /** gop */
		gop = atoi(p);
		ret = SAMPLE_updateGopAttr(MPI_ENC_CHN(enc_idx), gop);
		break;
	case BITRATE:
		p = strtok((char *)str, del); /** enc */
		enc_idx = atoi(p);
		p = strtok(NULL, del); /** bitrate */
		bitrate = atoi(p);
		ret = SAMPLE_updateBitRate(MPI_ENC_CHN(enc_idx), bitrate);
		break;
	default:
		break;
	}

	return ret;
}

static int updateRcParamByMode(int enc_idx, MPI_MCVC_RC_ATTR_S *rc_attr)
{
	int ret = 0;
	if (rc_attr->mode == MPI_RC_MODE_VBR) {
		ret = SAMPLE_updateVbrParams(MPI_ENC_CHN(enc_idx), &(rc_attr->vbr));
	} else if (rc_attr->mode == MPI_RC_MODE_CBR) {
		ret = SAMPLE_updateCbrParams(MPI_ENC_CHN(enc_idx), &(rc_attr->cbr));
	} else if (rc_attr->mode == MPI_RC_MODE_SBR) {
		ret = SAMPLE_updateSbrParams(MPI_ENC_CHN(enc_idx), &(rc_attr->sbr));
	} else if (rc_attr->mode == MPI_RC_MODE_CQP) {
		ret = SAMPLE_updateCqpParams(MPI_ENC_CHN(enc_idx), &(rc_attr->cqp));
	} else {
		fprintf(stderr, "unknown rc mode: %d\n", rc_attr->mode);
	}

	return ret;
}

static int resetTestCase(TestCase *test, SAMPLE_CONF_S *conf)
{
	int ret = 0;
	int gop = 0;
	UINT32 bitrate = 0;

	MPI_WIN_ATTR_S win_attr;
	MPI_VENC_ATTR_S attr;
	int i = 0;
	int j = 0;

	switch (*test) {
	case RESOLUTION:
		for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
			if (conf->dev[0].chn[i].enable) {
				ret = SAMPLE_reconfigResolution(conf, MPI_VIDEO_CHN(0, i),
				                                &(conf->dev[0].chn[i].attr.res));
			}
		}
		break;
	case LAYOUT:
		for (i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
			if (!conf->dev[0].chn[i].enable) {
				continue;
			}
			ret = SAMPLE_reconfigLayout(conf, MPI_VIDEO_CHN(0, i), &(conf->dev[0].chn[i].layout));
		}
		break;
	case FPS:
		for (i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
			ret = SAMPLE_updateFps(MPI_VIDEO_CHN(0, i), conf->dev[0].chn[i].attr.fps);
		}
		break;
	case WIN_ROI:
		for (i = 0; i < MPI_MAX_VIDEO_CHN_NUM; i++) {
			if (!conf->dev[0].chn[i].enable) {
				continue;
			}
			for (j = 0; j < conf->dev[0].chn[i].layout.window_num; j++) {
				ret = SAMPLE_updateWindowRoi(MPI_VIDEO_WIN(0, i, j), &(conf->dev[0].chn[i].win[j].roi));
			}
		}
		break;
	case MIRROR:
		ret = SAMPLE_updateMirrorAttr(conf->dev[0].chn[0].win[0].mirr_en);
		break;
	case FLIP:
		ret = SAMPLE_updateFlipAttr(conf->dev[0].chn[0].win[0].flip_en);
		break;
	case VIEW_TYPE: /** Only support win(0, 0, 0) */
		MPI_DEV_getWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &win_attr);
		if (win_attr.view_type == conf->dev[0].chn[0].win[0].view_type) {
			if (win_attr.view_type == MPI_WIN_VIEW_TYPE_LDC) {
				ret = SAMPLE_updateLdcAttr(&(conf->dev[0].ldc));
			} else if (win_attr.view_type == MPI_WIN_VIEW_TYPE_PANORAMA) {
				ret = SAMPLE_updatePanoramaAttr(&(conf->dev[0].panorama));
			} else if (win_attr.view_type == MPI_WIN_VIEW_TYPE_PANNING) {
				ret = SAMPLE_updatePanningAttr(&(conf->dev[0].panning));
			} else if (win_attr.view_type == MPI_WIN_VIEW_TYPE_SURROUND) {
				ret = SAMPLE_updateSurroundAttr(&(conf->dev[0].surround));
			} else if (win_attr.view_type == MPI_WIN_VIEW_TYPE_STITCH) {
				ret = SAMPLE_updateStitchAttr(&(conf->dev[0].stitch));
			} else {
				fprintf(stderr, "unknown type :%d\n", win_attr.view_type);
			}
		} else {
			ret = SAMPLE_reconfigWindowViewType(conf, MPI_VIDEO_WIN(0, 0, 0),
			                                    conf->dev[0].chn[0].win[0].view_type);
		}

		break;
	case CODEC:
		for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
			if (!conf->enc_chn[i].enable) {
				continue;
			}
			ret = SAMPLE_reconfigCodec(conf, MPI_ENC_CHN(i), &(conf->enc_chn[i].venc_attr));
		}
		break;
	case RATE_CONTROL:
		for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
			if (!conf->enc_chn[i].enable) {
				continue;
			}

			MPI_ENC_getVencAttr(MPI_ENC_CHN(i), &attr);

			if (attr.type == MPI_VENC_TYPE_H264) {
				ret = updateRcParamByMode(i, &(attr.h264.rc));
			} else if (attr.type == MPI_VENC_TYPE_H265) {
				ret = updateRcParamByMode(i, &(attr.h265.rc));
			}
		}
		break;
	case GOP:
		for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
			if (!conf->enc_chn[i].enable) {
				continue;
			}

			if (conf->enc_chn[i].venc_attr.type == MPI_VENC_TYPE_H264) {
				gop = conf->enc_chn[i].venc_attr.h264.rc.gop;
			} else if (conf->enc_chn[i].venc_attr.type == MPI_VENC_TYPE_H265) {
				gop = conf->enc_chn[i].venc_attr.h265.rc.gop;
			} else {
				break;
			}
			ret = SAMPLE_updateGopAttr(MPI_ENC_CHN(i), gop);
		}

		break;
	case BITRATE:
		for (i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
			if (!conf->enc_chn[i].enable) {
				continue;
			}

			if (conf->enc_chn[i].venc_attr.type == MPI_VENC_TYPE_H264) {
				getBitRate(&(conf->enc_chn[i].venc_attr.h264.rc), &bitrate);
			} else if (conf->enc_chn[i].venc_attr.type == MPI_VENC_TYPE_H265) {
				getBitRate(&(conf->enc_chn[i].venc_attr.h265.rc), &bitrate);
			} else {
				break;
			}
			ret = SAMPLE_updateBitRate(MPI_ENC_CHN(i), bitrate);
		}

		break;
	default:
		break;
	}

	return ret;
}

int main(int argc, char **argv)
{
	TestCase test_case;
	TestLoop loop = { 5, 0 };
	char param_str[PARAM_MAX];
	int ret = 0;

	/** Set signal handler. */
	if (signal(SIGINT, sigintHandler) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	/**
	 * Parse cmdargs then store to variable 'conf'. Then, print config
	 * to console if needed.
	 */

	initConf(&g_conf);

	ret = parseCmdArgs(argc, argv, &g_conf, &test_case, param_str, &loop);
	if (ret == -EALREADY) {
		exit(1);
	} else if (ret < 0) {
		goto end;
	}

	adjustConf(&g_conf.sample_conf);

	/** 1.Run video pipeline as case_config */
	startStream(&g_conf);

	printf("----start test case: %d, arg: %s, \nloop: %d times, interval: %d sec\n", test_case, param_str,
	       loop.times, loop.interval);

	/** 2. Run select test case */
	if (test_case == TEST_CASE_NUM) {
		goto end;
	}

	ret = runTestCase(&test_case, param_str);
	if (ret != 0) {
		goto end;
	}

	sleep(loop.interval);

	while (g_run_flag) {
		if (!loop.times) {
			printf("Not loop...\n");
			sleep(loop.interval);
			continue;
		}

		printf("loop last: %d times, param: %s, reset case\n", loop.times, param_str);

		ret = resetTestCase(&test_case, &g_conf.sample_conf);
		if (ret != 0) {
			break;
		}

		sleep(loop.interval);

		ret = runTestCase(&test_case, param_str);
		if (ret != 0) {
			goto end;
		}

		sleep(loop.interval);

		loop.times--;
	}

end:
	/** 3. Stop video pipeline */
	stopStream(&g_conf);

	deinitConf(&g_conf);

	return 0;
}
