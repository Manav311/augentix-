#include "nodes.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sample_dip.h"
#include "sample_light_src.h"
#include "utlist.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern Node g_nodes[NODE_NUM];

extern LightSrc *g_light_src_head[MPI_MAX_INPUT_PATH_NUM];
static LightSrcDetection *g_light_src_detection[MPI_MAX_INPUT_PATH_NUM] = { NULL };

/**
 * @brief Set the image Brightness object.
 * Get sensor PTA data and calculate image control brightness.
 * Set MPI Pta attribute from a collection of ratio.
 */
static int setBrightness(MPI_DEV dev_idx, AGTX_INT16 *brightness)
{
	INT32 ret = 0;
	INT32 ratio = ((AGTX_INT16)*brightness << 8) / PREF_TH;


	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setBrightnessRatio(MPI_INPUT_PATH(dev_idx.dev, i), ratio);
		if (ret != 0) {
			avmain2_log_err("Failed to get ImgPref(Brightness). ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set the image Contrast object.
 * Get sensor PTA data and calculate image control Contrast.
 * Set MPI Pta attribute from a collection of ratio.
 */
static int setContrast(MPI_DEV dev_idx, AGTX_INT16 *contrast)
{
	int ret = 0;
	INT32 ratio = ((INT32)*contrast << 8) / PREF_TH;

	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setContrastRatio(MPI_INPUT_PATH(dev_idx.dev, i), ratio);
		if (ret != 0) {
			avmain2_log_err("Failed to set ImgPref(Contrast). ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set the image saturation object.
 * Get sensor Csm data and calculate image control saturation.
 * Set MPI Csm attribute from a collection of ratio.
 */
static int setSaturation(MPI_DEV dev_idx, AGTX_INT16 *saturation)
{
	INT32 ret = 0;
	INT32 ratio = ((INT32)*saturation << 8) / PREF_TH;

	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setSaturationRatio(MPI_INPUT_PATH(dev_idx.dev, i), ratio);
		if (ret != 0) {
			avmain2_log_err("Failed to set ImgPref(Saturation). ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set the image hue object.
 * Get sensor Csm data and calculate image control hue.
 * Set MPI Csm attribute from a collection of ratio.
 */
static int setHue(MPI_DEV dev_idx, INT16 *hue)
{
	INT32 ret = 0;

	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setHueRatio(MPI_INPUT_PATH(dev_idx.dev, i), *hue);
		if (ret != 0) {
			avmain2_log_err("Failed to set ImgPref(Hue). ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

static int setSharpness(MPI_DEV dev_idx, AGTX_INT16 *sharpness)
{
	/*f49763 has 2 sensor input path*/
	int ret = MPI_SUCCESS;

	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setSharpnessRatio(MPI_INPUT_PATH(dev_idx.dev, i), *sharpness);
		if (ret != 0) {
			avmain2_log_err("Failed to get shp v2, ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set anti-flicker switch mode.
 * Get sensor Ae data and calculate image control anti-flicker.
 * Set MPI Ae attribute from a collection of switch mode.
 */
static int setAntiFlicker(MPI_DEV dev_idx, AGTX_ANTI_FLICKER_CONF_S *anti_flicker,
                          AGTX_ANTI_FLICKER_E *anti_flicker_switch)
{
	/*f49763 has 2 sensor input path*/
	INT32 ret = 0;
	INT32 enable = 0;
	INT32 frequency = 0;
	INT32 frame_rate = 0;

	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		switch (*anti_flicker_switch) {
		case AGTX_ANTI_FLICKER_OFF:
			enable = 0;
			break;
		case AGTX_ANTI_FLICKER_AUTO:
			enable = 1;
			frequency = anti_flicker->frequency_list[anti_flicker->frequency_idx].frequency;
			frame_rate = anti_flicker->frequency_list[anti_flicker->frequency_idx].fps;
			break;
		case AGTX_ANTI_FLICKER_50HZ:
			enable = 1;
			frequency = anti_flicker->frequency_list[0].frequency;
			frame_rate = anti_flicker->frequency_list[0].fps;
			break;
		case AGTX_ANTI_FLICKER_60HZ:
			enable = 1;
			frequency = anti_flicker->frequency_list[1].frequency;
			frame_rate = anti_flicker->frequency_list[1].fps;
			break;
		default:
			avmain2_log_err("AGTX_IMG_PREF_S anti_flicker is out of range.");
			return -EINVAL;
		}

		ret = SAMPLE_setAntiflicker(MPI_INPUT_PATH(dev_idx.dev, i), enable, frequency, frame_rate);
		if (ret != 0) {
			avmain2_log_err("Failed to set ImgPref(Anti_flicker). ret:%d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set anti-flicker switch frequency list.
 * Get sensor Ae data and calculate image control anti-flicker.
 * Set MPI Ae attribute from a collection of switch mode.
 */
static int setAntiFlickerList(MPI_DEV dev_idx, AGTX_ANTI_FLICKER_CONF_S *anti_flicker)
{
	avmain2_log_debug("has copy new freq list and fps to g_conf, need to run on each mode, enable: %d, idx: %d",
	                  anti_flicker->enable, anti_flicker->frequency_idx);
	if (0 != setAntiFlicker(dev_idx, anti_flicker, &g_conf.img.anti_flicker)) {
		return -EINVAL;
	}

	return 0;
}

static int setImgPref(MPI_DEV dev_idx, AGTX_IMG_PREF_S *img)
{
	if (0 != setAntiFlicker(dev_idx, &g_conf.anti_flicker, &img->anti_flicker)) {
		return -EINVAL;
	}

	/* set Image can only set anti-flicker, brightness, contrast, hue, saturation, shp in once */
	if (0 != setBrightness(dev_idx, &img->brightness)) {
		return -EINVAL;
	}

	if (0 != setContrast(dev_idx, &img->contrast)) {
		return -EINVAL;
	}

	if (0 != setSaturation(dev_idx, &img->saturation)) {
		return -EINVAL;
	}

	if (0 != setHue(dev_idx, &img->hue)) {
		return -EINVAL;
	}

	if (0 != setSharpness(dev_idx, &img->sharpness)) {
		return -EINVAL;
	}

	return 0;
}

static int handleWdr(MPI_DEV dev_idx, AGTX_ADV_IMG_PREF_S *adv_img)
{
	INT32 ret = 0;

	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setWdr(MPI_INPUT_PATH(dev_idx.dev, i), adv_img->wdr_en, adv_img->wdr_strength);
		if (ret != 0) {
			avmain2_log_err("Failed to set AdvImgPref(WDR).. ret:%d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

static int handleBlc(MPI_DEV dev_idx, AGTX_ADV_IMG_PREF_S *adv_img)
{
	INT32 ret = 0;
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		ret = SAMPLE_setBlc(MPI_INPUT_PATH(dev_idx.dev, i), adv_img->backlight_compensation);
		if (ret != 0) {
			avmain2_log_err("Failed to set AdvImgPref(Backlight Compensation). ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}

static void createAndInitAllLightSrcDetection(AGTX_SW_LIGHT_SENSING_PARAM *params)
{
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		SwLightSensorParam param;
		char detection_file[PATH_MAX];

		switch (g_conf.adv_img.night_mode) {
		case AGTX_NIGHT_MODE_AUTO:
			/** Init or update sw light sensing param */
			param.day2ir_th = params[idx].day2ir_th;
			param.ir2day_th = params[idx].ir2day_th;
			param.rg_ratio_min = params[idx].rg_ratio_min;
			param.rg_ratio_max = params[idx].rg_ratio_max;
			param.bg_ratio_min = params[idx].bg_ratio_min;
			param.bg_ratio_max = params[idx].bg_ratio_max;
			memcpy(&param.detect_name, params[idx].detect_name, sizeof(param.detect_name));
			g_light_src_detection[idx] = SAMPLE_newSwLightSensor(&param);
			break;
		case AGTX_NIGHT_MODE_AUTOSWITCH:
		case AGTX_NIGHT_MODE_ON:
		case AGTX_NIGHT_MODE_OFF:
			sprintf(detection_file, "/tmp/augentix/iq/iq_mode_%d", idx);
			g_light_src_detection[idx] = SAMPLE_newExternalFileControl(detection_file);
			break;
		default:
			break;
		}
	}
}

static void destroyAllLightSrcDetection()
{
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		if (g_light_src_detection[idx] == NULL) {
			continue;
		}

		SAMPLE_deleteLightSrcDetection(g_light_src_detection[idx]);
		g_light_src_detection[idx] = NULL;
		avmain2_log_info("del path[%d]light src detection", idx);
	}
}

static int writeFile(const char *filename, const char *str, int len)
{
	FILE *fp;
	size_t write_num = 0;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		avmain2_log_err("failed to open the file.");
		return -EINVAL;
	}

	write_num = fwrite(str, sizeof(char), len, fp);
	if ((int)write_num != len) {
		avmain2_log_err("failed to write %s file: %s", filename, str);
	}
	fclose(fp);

	return 0;
}

static int SwitchDay(AGTX_ADV_IMG_PREF_S *adv_img)
{
	int ret = 0;
	if (adv_img->night_mode != AGTX_NIGHT_MODE_AUTO) {
		avmain2_log_info("adv img night mode: %d", adv_img->night_mode);

		for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
			if (g_conf.dev.input_path[idx].path_en == 0) {
				continue;
			}

			if (adv_img->night_mode == AGTX_NIGHT_MODE_ON) {
				ret = writeFile(g_light_src_detection[idx]->method_param.file_config.filename, "ir", 2);
			}

			if (adv_img->night_mode == AGTX_NIGHT_MODE_OFF) {
				ret = writeFile(g_light_src_detection[idx]->method_param.file_config.filename, "day",
				                3);
			}
		}
	}

	return ret;
}

static int SwitchNight(AGTX_ADV_IMG_PREF_S *adv_img)
{
	return SwitchDay(adv_img);
}

static int resetLightSrcToCurrentMode(MPI_DEV dev_idx)
{
	int ret = 0;
	/**> reset to current mode */
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		LightSrc *item, *tmp;
		/** off all light src */
		LL_FOREACH_SAFE(g_light_src_head[idx], item, tmp)
		{
			if ((item->off != NULL) && (item->private != NULL)) {
				item->off(item->private);
			}
		}
		char req_name[NAME_MAX] = { '0' };
		SAMPLE_detectLightSrcOnce(MPI_INPUT_PATH(dev_idx.dev, idx), g_light_src_detection[idx], req_name,
		                          NAME_MAX);

		if (strcmp(req_name, "0") == 0) {
			sprintf(req_name, "%s", "day");
		}

		avmain2_log_info("[streaming] SAMPLE_detectLightSrcOnce() success, mode name is %s", req_name);
		sprintf(g_light_src_detection[idx]->curr_name, "%s", req_name);

		LL_FOREACH_SAFE(g_light_src_head[idx], item, tmp)
		{
			if (strcmp(item->name, req_name) == 0) {
			}
		}
	}

	return ret;
}

static int restartLightSrcDetection(MPI_DEV dev_idx, AGTX_COLOR_CONF_S *color_conf)
{
	int ret = 0;

	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		MPI_PATH path_idx = MPI_INPUT_PATH(dev_idx.dev, idx);
		/* if recent have detection thread, destroy it */
		ret = SAMPLE_destroyLightSrcDetectionThread(path_idx);
		if (ret != 0) {
			return ret;
		}
	}

	destroyAllLightSrcDetection();
	createAndInitAllLightSrcDetection(&color_conf->params[0]);
	resetLightSrcToCurrentMode(dev_idx);

	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}
		MPI_PATH path_idx = MPI_INPUT_PATH(dev_idx.dev, idx);
		ret = SAMPLE_createLightSrcDetectionThread(path_idx, g_light_src_head[idx], g_light_src_detection[idx]);
		if (ret != 0) {
			return ret;
		}
	}

	return ret;
}

static bool isChangeDetectionMethod(AGTX_ADV_IMG_PREF_S *adv_img)
{
	DetectionType new_detection_type = DETECT_FILE;
	if (adv_img->night_mode == AGTX_NIGHT_MODE_AUTO) {
		new_detection_type = DETECT_SW_LIGHT_SENSOR;
	}

	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		if (g_light_src_detection[idx]->method != new_detection_type) {
			return true;
		}
	}

	return false;
}

static int setAdvImgPref(MPI_DEV dev_idx, AGTX_ADV_IMG_PREF_S *adv_img, AGTX_COLOR_CONF_S *color_conf)
{
	int ret = 0;

	if (g_light_src_detection[0] == NULL || isChangeDetectionMethod(adv_img) == true) {
		ret = restartLightSrcDetection(dev_idx, color_conf);
	}

	if (0 != ret) {
		avmain2_log_err("Failed to create lightsrc detection.");
		return -EINVAL;
	}

	if (adv_img->night_mode == AGTX_NIGHT_MODE_OFF) {
		ret = SwitchDay(adv_img);
	}

	if (adv_img->night_mode == AGTX_NIGHT_MODE_ON) {
		ret = SwitchNight(adv_img);
	}

	if (0 != ret) {
		avmain2_log_err("Failed to switch day/night mode.");
		return -EINVAL;
	}

	if (MPI_SUCCESS != handleBlc(dev_idx, adv_img)) {
		return -EINVAL;
	}

	if (MPI_SUCCESS != handleWdr(dev_idx, adv_img)) {
		return -EINVAL;
	}

	if ((adv_img->ir_light_suppression > 100) || (adv_img->ir_light_suppression < 0)) {
		avmain2_log_err("adv_img ir_light_suppression out of range.");
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Set auto black and white object.
 * Get sensor Awb data and calculate image temp.
 * Set MPI Awb attribute from a collection of ratio.
 */
static int setAwbPref(MPI_DEV dev_idx, AGTX_AWB_PREF_S *awb)
{
	INT32 ret = 0;

	/*f49763 has 2 sensor input path*/
	for (int i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}

		int r_gain_ratio = ((INT32)awb->r_gain << 8) / PREF_TH;
		int b_gain_ratio = ((INT32)awb->b_gain << 8) / PREF_TH;
		ret = SAMPLE_setAwbPref(MPI_INPUT_PATH(dev_idx.dev, i), awb->mode, awb->color_temp, r_gain_ratio,
		                        b_gain_ratio);
		if (ret != 0) {
			avmain2_log_err("Failed to set AWBPref. ret: %d", ret);
			return -EINVAL;
		}
	}

	return 0;
}


int NODE_initImagePreference(void)
{
	return 0;
}

int NODE_startImagePreference(void)
{
	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);

	if (0 != setImgPref(dev_idx, &g_conf.img)) {
		return -EINVAL;
	}

	if (0 != setAdvImgPref(dev_idx, &g_conf.adv_img, &g_conf.color)) {
		return -EINVAL;
	}

	/*sensor.ini may change max FPS input, set product anti-flicker here to cover COLOR_CONF & ADV_IMG_PREF enfluence*/
	if (0 != setAntiFlickerList(dev_idx, &g_conf.anti_flicker)) {
		return -EINVAL;
	}

	if (0 != setAwbPref(dev_idx, &g_conf.awb)) {
		return -EINVAL;
	}

	avmain2_log_info("Start Image Preference.");

	return 0;
}

int NODE_stopImagePreference(void)
{
	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	int ret = 0;
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}
		MPI_PATH path_idx = MPI_INPUT_PATH(dev_idx.dev, idx);
		ret = SAMPLE_destroyLightSrcDetectionThread(path_idx);
		if (ret != 0) {
			return ret;
		}
	}
	destroyAllLightSrcDetection();

	return 0;
}

int NODE_exitImagePreference(void)
{
	return 0;
}

int NODE_setImagePreference(int cmd_id, void *data)
{
	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);

	switch (cmd_id) {
	case ANTI_FLIKER:
		if (0 != setAntiFlickerList(dev_idx, (AGTX_ANTI_FLICKER_CONF_S *)data)) {
			return -EINVAL;
		}
		break;
	case IMG_PREF:
		if (0 != setImgPref(dev_idx, (AGTX_IMG_PREF_S *)data)) {
			return -EINVAL;
		}
		break;
	case AWB_PREF:
		if (0 != setAwbPref(dev_idx, (AGTX_AWB_PREF_S *)data)) {
			return -EINVAL;
		}
		break;
	case ADV_IMG_PREF:
		if (0 != setAdvImgPref(dev_idx, (AGTX_ADV_IMG_PREF_S *)data, &g_conf.color)) {
			return -EINVAL;
		}

		/*sensor.ini may change max FPS input, rewrite anti flicker conf & image pref conf antiflicker switch mode*/
		if (0 != setAntiFlicker(dev_idx, &g_conf.anti_flicker, &g_conf.img.anti_flicker)) {
			return -EINVAL;
		}
		break;
	case COLOR_CONF:

		if (0 != setImgPref(dev_idx, &g_conf.img)) {
			return -EINVAL;
		}

		if (0 != setAwbPref(dev_idx, &g_conf.awb)) {
			return -EINVAL;
		}

		if (0 != setAdvImgPref(dev_idx, &g_conf.adv_img, (AGTX_COLOR_CONF_S *)data)) {
			return -EINVAL;
		}

		/*sensor.ini may change max FPS input, rewrite anti flicker conf & image pref conf antiflicker switch mode*/
		if (0 != setAntiFlicker(dev_idx, &g_conf.anti_flicker, &g_conf.img.anti_flicker)) {
			return -EINVAL;
		}

		/*call set image pref by win here*/
		if (NODES_execSet((Node *)&g_nodes[WIN_IMAGE_PREFERENCE], NR_WIN, &g_conf.nr_win) != 0) {
		}

		if (NODES_execSet((Node *)&g_nodes[WIN_IMAGE_PREFERENCE], SHP_WIN, &g_conf.shp_win) != 0) {
		}

		break;

	default:
		avmain2_log_err("invalid cmd_id: %d", cmd_id);
		return -EINVAL;
	}

	return 0;
}