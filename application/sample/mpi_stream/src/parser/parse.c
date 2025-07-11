#include "parse.h"

#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "linux/limits.h"

#include "sample_dip.h"
#include "sample_light_src.h"

#include "parse_dev.h"
#include "parse_enc.h"
#include "parse_utils.h"
#include "parse_sys.h"

#include "utlist.h"

typedef enum { PERI_IR_CUT = 0, PERI_IR_LED, PERI_W_LED, PERI_TYPE_NUM } PeriType;

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
#ifdef MULTIPLE_IQ_SUPPORT
/**
 * @brief parse the configuartion of light source.
 * @param[in] str Configuration strings.
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed
 * @retval positive parsed
 * @retval negative error occurs
 */
static int parse_light_src(const char *str, APP_CONF_S *conf)
{
	char *del = (char *)",";
	char *p = NULL;
	int loop = 0;
	int path_idx = 0;
	unsigned int type_idx = SRC_TYPE_DAY;

	p = strtok((char *)str, del); /** input path */
	path_idx = atoi(p);

	LightSrc *item, *tmp, *target_src = NULL;

	while (p != NULL) {
		p = strtok(NULL, del);

		switch (loop) {
		case 0: /** mode */
			LL_FOREACH_SAFE(conf->head[path_idx], item, tmp)
			{
				if (strcmp(item->name, p) == 0) {
					target_src = item;
					type_idx = target_src->type;
					break;
				}
			}
			break;
		case 1: /** ini path */
			if (strcmp(p, "-1") == 0) {
				fprintf(stderr, "Use default ini path\n");
				break;
			}
			strncpy(((char *)target_src->sensor_path), p, sizeof(target_src->sensor_path));
			break;
		/**  hw info */
		case 2:
			if (type_idx == SRC_TYPE_LIGHT) {
				((LightHwConfig *)target_src->private)->w_led.id = atoi(p);
			} else if (type_idx == SRC_TYPE_IR) {
				((IrHwConfig *)target_src->private)->ir_cut[0].id = atoi(p);
			} else {
				fprintf(stderr, "day mode has no hw info: %s\n", p);
			}
			break;
		case 3: /** Activate value */
			if (type_idx == SRC_TYPE_LIGHT) {
				if (strcmp("high", p) == 0) {
					((LightHwConfig *)target_src->private)->w_led.activate_value = GPIO_VAL_HIGH;
				} else {
					((LightHwConfig *)target_src->private)->w_led.activate_value = GPIO_VAL_LOW;
				}
			} else if (type_idx == SRC_TYPE_IR) {
				if (strcmp("high", p) == 0) {
					((IrHwConfig *)target_src->private)->ir_cut[0].activate_value = GPIO_VAL_HIGH;
				} else {
					((IrHwConfig *)target_src->private)->ir_cut[0].activate_value = GPIO_VAL_LOW;
				}
			} else {
				fprintf(stderr, "day mode has no hw info: %s\n", p);
			}
			break;
		case 4:
			if (type_idx == SRC_TYPE_IR) {
				((IrHwConfig *)target_src->private)->ir_cut[1].id = atoi(p);
			} else {
			}
			break;
		case 5: /** Activate value */
			if (strcmp("high", p) == 0) {
				((IrHwConfig *)target_src->private)->ir_cut[1].activate_value = GPIO_VAL_HIGH;
			} else {
				((IrHwConfig *)target_src->private)->ir_cut[1].activate_value = GPIO_VAL_LOW;
			}
			break;
		case 6:
			if (type_idx == SRC_TYPE_IR) {
				((IrHwConfig *)target_src->private)->ir_led.id = atoi(p);
			} else {
			}
			break;
		case 7: /** Activate value */
			if (strcmp("high", p) == 0) {
				((IrHwConfig *)target_src->private)->ir_led.activate_value = GPIO_VAL_HIGH;
			} else {
				((IrHwConfig *)target_src->private)->ir_led.activate_value = GPIO_VAL_LOW;
			}
			break;
		default:
			break;
		}
		loop++;
	}

	return loop;
}
#ifdef SOFTWARE_LIGHT_SENSOR
/**
 * @brief parse the configuartion of swlight sensinge.
 * @param[in] str Configuration strings.
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed
 * @retval positive parsed
 * @retval negative error occurs
 */
static int parse_sw_light_sensor_param(const char *str, APP_CONF_S *conf)
{
	char *del = (char *)",";
	char *p = NULL;
	int loop = 0;
	int path_idx = 0;

	p = strtok((char *)str, del); /** input path */
	path_idx = atoi(p);
	SwLightSensorParam *param = &(conf->detection[path_idx]->method_param.sw_light_sensor_param);
	float tmp = 0;

	while (p != NULL) {
		p = strtok(NULL, del);
		switch (loop) {
		case 0:
			if (strcmp(p, "-1") == 0) {
				fprintf(stderr, "Detect default ir\n");
				break;
			}
			strncpy(((char *)param->detect_name), p, NAME_MAX);
			break;
		case 1:
			param->day2ir_th = atoi(p);
			break;
		case 2:
			param->ir2day_th = atoi(p);
			break;
		case 3:
			tmp = atof(p);
			if (tmp > RG_BG_RATIO_MAX || tmp < RG_BG_RATIO_MIN) {
				fprintf(stderr, "Invalid rg ratio: %f\n", tmp);
				break;
			}
			param->rg_ratio_min = (UINT16)(tmp * 256 + 0.5);
			break;
		case 4:
			tmp = atof(p);
			if (tmp > RG_BG_RATIO_MAX || tmp < RG_BG_RATIO_MIN) {
				fprintf(stderr, "Invalid rg ratio: %f\n", tmp);
				break;
			}
			param->rg_ratio_max = (UINT16)(tmp * 256 + 0.5);
			break;
		case 5:
			tmp = atof(p);
			if (tmp > RG_BG_RATIO_MAX || tmp < RG_BG_RATIO_MIN) {
				fprintf(stderr, "Invalid bg ratio: %f\n", tmp);
				break;
			}
			param->bg_ratio_min = (UINT16)(tmp * 256 + 0.5);
			break;
		case 6:
			tmp = atof(p);
			if (tmp > RG_BG_RATIO_MAX || tmp < RG_BG_RATIO_MIN) {
				fprintf(stderr, "Invalid bg ratio: %f\n", tmp);
				break;
			}
			param->bg_ratio_max = (UINT16)(tmp * 256 + 0.5);
			break;
		default:
			break;
		}

		loop++;
	}

	return loop;
}
#endif

#ifdef GPIO_LIGHT_SENSOR
/**
 * @brief parse the configuartion of gpio sensinge.
 * @param[in] str Configuration strings.
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed
 * @retval positive parsed
 * @retval negative error occurs
 */
static int parse_gpio_sensor_param(const char *str, APP_CONF_S *conf)
{
	char *del = (char *)",";
	char *p = NULL;
	int loop = 0;
	int path_idx = 0;

	p = strtok((char *)str, del); /** input path */
	path_idx = atoi(p);
	GpioLightSensorParam *param = &(conf->detection[path_idx]->method_param.gpio_light_sensor_param);

	while (p != NULL) {
		p = strtok(NULL, del);
		switch (loop) {
		case 0:
			param->sensor_gpio.id = atoi(p);
			break;
		case 1:
			if (strcmp("high", p) == 0) {
				param->night_value = GPIO_VAL_HIGH;
			} else {
				param->night_value = GPIO_VAL_LOW;
			}
			break;
		default:
			break;
		}

		loop++;
	}

	return loop;
}
#endif
#ifdef ADC_LIGHT_SENSOR
/**
 * @brief parse the configuartion of adc avlue.
 * @param[in] str Configuration strings.
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed
 * @retval positive parsed
 * @retval negative error occurs
 */
static int parse_adc_sensor_param(const char *str, APP_CONF_S *conf)
{
	char *del = (char *)",";
	char *p = NULL;
	int loop = 0;
	int path_idx = 0;

	p = strtok((char *)str, del); /** input path */
	path_idx = atoi(p);
	AdcLightSensorParam *param = &(conf->detection[path_idx]->method_param.adc_light_sensor_param);

	while (p != NULL) {
		p = strtok(NULL, del);
		switch (loop) {
		case 0:
			param->sensor_adc.id = atoi(p);
			break;
		case 1:
			param->sensor_adc.threshold_hl = atoi(p);
			break;
		case 2:
			if (strcmp("high", p) == 0) {
				param->night_value = ADC_VAL_HIGH;
			} else {
				param->night_value = ADC_VAL_LOW;
			}
			break;
		default:
			break;
		}
		loop++;
	}
	return loop;
}
#endif
#else
static int parse_update_ini_path(const char *str, APP_CONF_S *conf)
{
	char *del = (char *)",";
	char *p = NULL;
	int loop = 0;

	p = strtok((char *)str, del); /** path 0 sensor.ini */
	if (strcmp(p, "-1") == 0) {
		fprintf(stderr, "Use default ini path\n");
	}
	strncpy((char *)conf->head[0]->sensor_path, p, sizeof(conf->head[0]->sensor_path));

	while (p != NULL) {
		p = strtok(NULL, del);

		switch (loop) {
		case 0:
			if (strcmp(p, "-1") == 0) {
				fprintf(stderr, "Use default ini path\n");
				break;
			}
			strncpy((char *)conf->head[1]->sensor_path, p, sizeof(conf->head[1]->sensor_path));
			break;
		default:
			break;
		}
		loop++;
	}

	return loop;
}
#endif
static bool parse_threshold(const char *param, int32_t *value)
{
	char *cursor;
	errno = 0;
	long parsed_value = strtol(param, &cursor, 10);
	if (errno) {
		perror("strtol");
		return false;
	}

	if (*cursor == '%') {
		*value = -parsed_value;
	} else {
		*value = parsed_value;
	}
	return true;
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

void init_enc_chn_conf(CONF_ECHN_PARAM_S *p)
{
	MPI_ENC_CHN_ATTR_S *chn = &p->attr;
	MPI_ENC_BIND_INFO_S *info = &p->bind;
	MPI_VENC_ATTR_S *venc = &p->venc_attr;
	MPI_VENC_ATTR_EX_S *venc_ex = &p->venc_ex;

	/* Init encoder channel attr */
	chn->res.width = 0;
	chn->res.height = 0;
	chn->max_res.width = 0;
	chn->max_res.height = 0;

	/* Init encoder channel binding info */
	info->idx = MPI_VIDEO_CHN(0, 0);

	/* Init video encoder attr */
	memset(venc, 0, sizeof(MPI_VENC_ATTR_S));
	venc->type = MPI_VENC_TYPE_H264;
	venc->h264.profile = MPI_PRFL_BASELINE;
	venc->h264.rc.mode = MPI_RC_MODE_VBR;
	venc->h264.rc.gop = 0;
	venc->h264.rc.frm_rate_o = 0;
	venc->h264.rc.vbr.max_bit_rate = 0;
	venc->h264.rc.vbr.quality_level_index = 0;
	venc->h264.rc.vbr.fluc_level = 0;
	venc->h264.rc.vbr.regression_speed = 0;
	venc->h264.rc.vbr.scene_smooth = 0;
	venc->h264.rc.vbr.i_continue_weight = 0;
	venc->h264.rc.vbr.i_qp_offset = 0;

	/* Init video encoder attr extend*/
	venc_ex->obs = MPI_VENC_OBS_DISABLE;
	venc_ex->obs_off_period = 2U;
}

/**
 * @brief print the content of SAMPLE_CONF_S to stdout
 * @param[in] conf    pointer to configuration struct
 */
void printConf(SAMPLE_CONF_S *conf)
{
	int i, j, k;

	printf("VB config:\n");
	printf("max pool cnt = %d.\n", conf->sys.vb_conf.max_pool_cnt);
	for (i = 0; i < MPI_MAX_PUB_POOL; ++i) {
		printf("VB Pool %d:\n", i);
		printf("block size = %d, block cnt = %d, name = %s.\n", conf->sys.vb_conf.pub_pool[i].blk_size,
		       conf->sys.vb_conf.pub_pool[i].blk_cnt, conf->sys.vb_conf.pub_pool[i].name);
	}

	printf("\n");

	for (i = 0; i < MPI_MAX_VIDEO_DEV_NUM; ++i) {
		CONF_DEV_PARAM_S *dev = &conf->dev[i];

		if (!dev->enable) {
			continue;
		}

		printf("Video device %d:\n", i);
		printf("stitch_en = %d, eis_en = %d, hdr_mode = %d, input_frame_rate = %f, bayer = %d.\n",
		       dev->attr.stitch_en, dev->attr.eis_en, dev->attr.hdr_mode, dev->attr.fps, dev->attr.bayer);
		printf("\n");

		for (j = 0; j < MPI_MAX_INPUT_PATH_NUM; ++j) {
			CONF_PATH_PARAM_S *path = &dev->path[j];

			if (!path->enable) {
				continue;
			}

			printf("Input path %d, %d:\n", i, j);
			printf("sensor index = %d, path_fps = %f, res= %dx%d, eis_strength= %d.\n", path->attr.sensor_idx, path->attr.fps, path->attr.res.width,
			       path->attr.res.height, path->attr.eis_strength);
		}
		printf("\n");

		for (j = 0; j < MPI_MAX_VIDEO_CHN_NUM; ++j) {
			CONF_CHN_PARAM_S *chn = &conf->dev[i].chn[j];

			if (!chn->enable) {
				continue;
			}

			printf("Video channel %d, %d:\n", i, j);
			printf("res = %dx%d, fps = %f.\n", chn->attr.res.width, chn->attr.res.height, chn->attr.fps);

			printf("Channel layout:\n");
			print_chn_layout(&chn->layout);
			printf("\n");

			printf("Window Attribute:\n");
			for (k = 0; k < chn->layout.window_num; ++k) {
				print_window_attr(k, &chn->win[k]);
			}
			printf("\n");
		}

		print_stitch_attr(&dev->stitch);
		print_ldc_attr(&dev->ldc);
		print_panorama_attr(&dev->panorama);
		print_panning_attr(&dev->panning);
		print_surround_attr(&dev->surround);

		printf("\n");
	}

	for (i = 0; i < MPI_MAX_ENC_CHN_NUM; ++i) {
		CONF_ECHN_PARAM_S *enc_chn = &conf->enc_chn[i];
		CONF_BITSTREAM_PARAM_S *bitstream = &conf->bitstream[i];

		printf("Encoder channel %d:\n", i);
		printf("General param:\n");
		printf("udpstream = %d, record = %d.\n", bitstream->stream.enable, bitstream->record.enable);
		printf("client_ip = %s, client_port = %d.\n", bitstream->stream.client_ip,
		       bitstream->stream.client_port);
		printf("output_file = %s, frame_num = %d, max_dumped_files = %d\n", bitstream->record.fname,
		       bitstream->record.frame_num, bitstream->record.max_dumped_files);
		printf("Channel attr:\n");
		printf("res = %dx%d.\n", enc_chn->attr.res.width, enc_chn->attr.res.height);
		printf("Binding info:\n");
		printf("bind_dev_idx = %d, bind_chn_idx = %d.\n", enc_chn->bind.idx.dev, enc_chn->bind.idx.chn);
		printf("Encoder: type = %d\n", enc_chn->venc_attr.type);
		print_venc_attr(&enc_chn->venc_attr);
		printf("Encoder extension:\n");
		printf("obs = %u, obs_off_period = %u.\n", enc_chn->venc_ex.obs, enc_chn->venc_ex.obs_off_period);

		printf("\n");
	}
}

/**
 * @brief Initialize APP_CONF_S
 * @details
 * Rules to initialize configurations:
 *  - Enable bit always set to 0.
 *  - Use common / normal setting for enum. ex: op_mode = MPI_OP_MODE_NORMAL.
 *  - create LightSrc and LightSrcDetection object by input path.
 * @param[out] conf    pointer to configuration struct
 */
void initConf(APP_CONF_S *conf)
{
	CONF_CASE_GEN_PARAM *casegen;
	CONF_DEV_PARAM_S *dev;
	CONF_CHN_PARAM_S *chn;
	MPI_WIN_ATTR_S *win;
	int i, j, k;
	conf->sample_conf.osd_visible = true;

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

	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		char sensor_normal[PATH_MAX] = { '0' };
		char dip_extend_normal[PATH_MAX] = { '0' };

		sprintf(sensor_normal, "%s/sensor_%d.ini", DIP_FILE_PATH, idx);
		if (access(sensor_normal, R_OK) != 0) {
			sprintf(sensor_normal, "%s/sensor_0.ini", DIP_FILE_PATH);
		}
		conf->head[idx] = SAMPLE_newDayLightSrc("day", sensor_normal);

		sprintf(dip_extend_normal, "%s/dip_extend_%d.ini", DIP_FILE_PATH, idx);
		if (access(dip_extend_normal, R_OK) == 0) {
			fprintf(stdout, "day mode dip_extend use %s \n", dip_extend_normal);
			strncpy(conf->head[idx]->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		} else {
			memset(dip_extend_normal, 0, sizeof(dip_extend_normal));
		}

#ifdef MULTIPLE_IQ_SUPPORT
		IrHwConfig ir_hw;
		ir_hw.ir_cut[0].id = -1;
		ir_hw.ir_cut[0].activate_value = GPIO_VAL_HIGH;

		ir_hw.ir_cut[1].id = -1;
		ir_hw.ir_cut[1].activate_value = GPIO_VAL_HIGH;
		ir_hw.ir_led.id = -1;
		ir_hw.ir_led.activate_value = GPIO_VAL_HIGH;

		LightHwConfig light_hw;
		light_hw.w_led.id = -1;
		light_hw.w_led.activate_value = GPIO_VAL_HIGH;

		char ini_name[PATH_MAX];
		LightSrc *tmp_src;
		sprintf(ini_name, "%s/sensor_ir_%d.ini", DIP_FILE_PATH, idx);

		if (access(ini_name, R_OK) == 0) {
			fprintf(stdout, "Ir mode use %s \n", ini_name);
		} else {
			sprintf(ini_name, "%s", sensor_normal);
		}
		tmp_src = SAMPLE_newIrLightSrc("ir", ini_name, &ir_hw);

		sprintf(ini_name, "%s/dip_extend_ir_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) == 0) {
			fprintf(stdout, "Ir mode dip_extend use %s \n", ini_name);
			strncpy(tmp_src->dip_extend_path, ini_name, sizeof(ini_name));
		} else {
			strncpy(tmp_src->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		}
		LL_APPEND(conf->head[idx], tmp_src);

		sprintf(ini_name, "%s/sensor_light_%d.ini", DIP_FILE_PATH, idx);

		if (access(ini_name, R_OK) == 0) {
			fprintf(stdout, "Light mode use %s \n", ini_name);
		} else {
			sprintf(ini_name, "%s", sensor_normal);
		}
		tmp_src = SAMPLE_newWhiteLightSrc("light", ini_name, &light_hw);

		sprintf(ini_name, "%s/dip_extend_light_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) == 0) {
			fprintf(stdout, "Light mode dip_extend use %s \n", ini_name);
			strncpy(tmp_src->dip_extend_path, ini_name, sizeof(ini_name));
		} else {
			strncpy(tmp_src->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		}
		LL_APPEND(conf->head[idx], tmp_src);

#ifdef EXTERNAL_IQ_MODE_SWITCH_ENABLE
		char detection_file[PATH_MAX];
		sprintf(detection_file, "/tmp/augentix/iq/iq_mode_%d", idx);
		conf->detection[idx] = SAMPLE_newExternalFileControl(detection_file);
#endif
#ifdef SOFTWARE_LIGHT_SENSOR
		/** Init sw light sensing param */
		SwLightSensorParam param = { "ir", -1, -1, -1, -1, -1, -1 };
		conf->detection[idx] = SAMPLE_newSwLightSensor(&param);
#endif
#ifdef GPIO_LIGHT_SENSOR
		/** Init gpio light sensing param */
		GpioLightSensorParam param = { .sensor_gpio.id = 0, .night_value = 0 };

		conf->detection[idx] = SAMPLE_newGpioLightSensor(&param);
#endif
#ifdef ADC_LIGHT_SENSOR
		AdcLightSensorParam param = { .sensor_adc.id = 0, .night_value = 0 };
		conf->detection[idx] = SAMPLE_newAdcLightSensor(&param);
#endif
#endif
	}
	sprintf(conf->window_path, "%s/window.ini", DIP_FILE_PATH);

	conf->sample_conf.casegen.show_params = 1;
	if (conf->sample_conf.casegen.show_params) {
		for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
			LightSrc *item_test, *tmp_test;
			printf("Path%d : \n", idx);
			LL_FOREACH_SAFE(conf->head[idx], item_test, tmp_test)
			{
				printf("\tLightSRcType = %d, sensor ini  : %s , dip_extend ini is %s\n",
				       item_test->type, item_test->sensor_path, item_test->dip_extend_path);
			}
		}
	}
}
/**
 * @brief deInitialize APP_CONF_S
 * @details
 *  - free LightSrc and LightSrcDetection object of each input path.
 * @param[out] conf    pointer to configuration struct
 */
void deinitConf(APP_CONF_S *conf)
{
	LightSrc *item, *tmp;
	LightSrc *light_src_head;
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		light_src_head = conf->head[idx];
		/** day lightSrc still be created in not support multiple iq case */
		LL_FOREACH_SAFE(light_src_head, item, tmp)
		{
			LL_DELETE(light_src_head, item);
			SAMPLE_deleteLightSrc(item);
		}
#ifdef MULTIPLE_IQ_SUPPORT
		SAMPLE_deleteLightSrcDetection(conf->detection[idx]);
#endif
	}

	return;
}

/**
 * @brief Print help messages to stdout
 * @param[in] str    name of executable
 */
void help(char *pname)
{
	printf("Usage: %s -d case_config [OPTION]...\n", pname);
	printf("Example: %s -d /system/mpp/case_config/case_config_1001_FHD\n", pname);
	printf("         %s -d /usrdata/case_config -s /usrdata/sensor_day.ini\n", pname);
	printf("         %s -d /usrdata/case_config -p record_enable=1 -p output_file=/mnt/sdcard/record.raw\n", pname);
	printf("Options:\n");
	printf("  -h                Display this help and exit.\n");
	printf("  -d case_config    Select streaming configuration file.\n");
	printf("  -p param=value    Overwrite a parameter of the streaming configuration with a value.\n");
	printf("                    This option can be specified multiple times for different parameters.\n");
	printf("  -R 1000           Reserve 1000 MBytes.\n");
	printf("  -R 10%%            Reserve 10%% storage.\n");
	printf("  -r 500            Recycle 500 MBytes files.\n");
	printf("  -r 5%%             Recycle files to 5%% of storage.\n");
#ifdef OSD_ENABLE
	printf("  -o                Hide OSD.\n");
#endif /* OSD_ENABLE */
	printf("Options for ini files:\n");
	printf("  -w path           Window setting. Default: /system/mpp/script/window.ini\n");
#ifdef MULTIPLE_IQ_SUPPORT
	printf("  -s <path id>,<mode name>,<ini path>,<hw info...>    Specify IQ mode light src\n");
	printf("  day setting:\n");
	printf("    -s 0,day,<ini path>\n");
	printf("  night ir setting:\n");
	printf("    -s 0,ir,<ini path>,<ir cut p>,<ir cut p activate value>,\n");
	printf("                       <ir cut n>,<ir cut n activate value>,\n"
	       "                       <ir led>,<ir led activate value>\n");
	printf("    e.g., 1-pin ir-cut: -s 0,ir,/usdrata/test.ini,59,high,-1,high,50,low\n");
	printf("    e.g., 2-pin ir-cut: -s 0,ir,/usdrata/test.ini,59,high,61,high,50,low\n");
	printf("  night light setting:\n");
	printf("    -s 0,light,<ini path>,<white led 0>,<led 0 activate value>\n");
	printf("    e.g., -s 0,light,/usdrata/test.ini,59,high\n");
#ifdef SOFTWARE_LIGHT_SENSOR
	printf("Options for software light sensor:\n");
	printf("  -l <idx>,<detect_name>,<day2ir_th>,<ir2day_th>,\n");
	printf("                         <rg_ratio_min>,<rg_ratio_max>,\n"
	       "                         <bg_ratio_min>,<bg_ratio_max>\n");
	printf("  where <xx_ratio_min>,<xx_ratio_max> shouldi be in 4.8f format.\n");
	printf("  where <detect_name> can switch to ir or light.\n");
	printf("  e.g., -l 0,ir,32,23,0.5,0.8,1.0,0.2\n");
#endif
#ifdef GPIO_LIGHT_SENSOR
	printf("Options for gpio light sensor:\n");
	printf("  -g <idx>,<gpio id>,<night value>\n");
	printf("  e.g., -g 0,2,high\n");
#endif
#ifdef ADC_LIGHT_SENSOR
	printf("Options for adc light sensor:\n");
	printf("  -a <idx>,<adc channel id>,<threshold>,<active high or low>\n");
	printf("    <idx> : path id\n");
	printf("    <adc channel id> : adc channel id user would like to observe\n");
	printf("    <threshold> : The value measured by the ADC is considered 'high' if it exceeds this threshold\n");
	printf("    <high or low> : ir mode is activate-high or activate-low\n");
	printf("  e.g., -a0,2,1000,high\n");
#endif
#else
	printf("  -s <path 0 sensor.ini>[,<path 1 sensor.ini>]\n");
	printf("  e.g., -s /usrdata/sensor_0.ini\n");
#endif
	printf("Note:\n");
	printf("  sensor.ini should not be put in NFS, otherwise it does not work correctly.\n");
	exit(0);
}

/**
 * @brief parse the command arguments and convert as configuration struct
 * @param[in] argc     argument count
 * @param[in] argv     argument vector
 * @param[out] conf    pointer to configuration struct
 * @return The execution result
 * @retval 0        nothing is parsed or some error occurs
 * @retval positive parsed
 */
int parseCmdArgs(int argc, char *argv[], APP_CONF_S *conf)
{
	int ret = 0;
	int c;
	opterr = 0;

	while ((c = getopt(argc, argv, "R:r:N:d:hop:s:l:w:g:a:")) != -1) {
		switch (c) {
		case 'h':
			help(argv[0]);
			break;
		case 'R':
			if (!parse_threshold(optarg, &conf->reservation_level)) {
				fprintf(stderr, "[ERROR] don't understand the value for reservation_level!\n");
				return 0;
			}
			break;
		case 'r':
			if (!parse_threshold(optarg, &conf->recycle_level)) {
				fprintf(stderr, "[ERROR] don't understand the value for recycle_level!\n");
				return 0;
			}
			break;
		case 'd':
			/* Parse parameter from config file. */
			ret = parse_config_file(optarg, &conf->sample_conf);
			if (!ret) {
				return ret;
			}
			break;
		case 'p':
			/* Parse parameter from command line. */
			ret = parse_param(optarg, &conf->sample_conf);
			if (!ret) {
				return ret;
			}
			break;
#ifdef MULTIPLE_IQ_SUPPORT
		case 's':
			/* Parse light src paramters */
			ret = parse_light_src(optarg, conf);
			if (!ret) {
				return ret;
			}
			break;
#ifdef SOFTWARE_LIGHT_SENSOR
		case 'l':
			/* Parse software light sensor detection param */
			ret = parse_sw_light_sensor_param(optarg, conf);
			if (!ret) {
				return ret;
			}
			break;
#endif
#ifdef GPIO_LIGHT_SENSOR
		case 'g':
			printf("parse gpio light sensor\n");
			/* Parse gpio light sensor detection param */
			ret = parse_gpio_sensor_param(optarg, conf);
			if (!ret) {
				return ret;
			}
			break;
#endif
#ifdef ADC_LIGHT_SENSOR
		case 'a':
			fprintf(stdout, "parse adc light sensor\n");
			ret = parse_adc_sensor_param(optarg, conf);
			if (!ret) {
				return ret;
			}
			break;
#endif
#else
		case 's':
			/*Not support multiple iq case. Only assign a sensor_path of each input path */
			ret = parse_update_ini_path(optarg, conf);
			if (!ret) {
				return ret;
			}
			break;
#endif
		case 'o':
			conf->sample_conf.osd_visible = false;
			break;
		case 'w':
			strncpy(conf->window_path, optarg, sizeof(conf->window_path));
			break;
		case ':':
			fprintf(stderr, "oops\n");
			break;
		case '?':
			if (strchr("CLNScdlnps", optopt) != NULL) {
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			} else if (isprint(optopt)) {
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			} else {
				fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
			}
			return 0;
		default:
			return 0;
		}
	}

	return ret;
}
