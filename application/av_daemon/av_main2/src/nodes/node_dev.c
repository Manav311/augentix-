#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>
#include <fcntl.h>

#include "mpi_sys.h"
#include "ini.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_dip_alg.h"
#include "sensor.h"
#include "mpi_dev.h"

#include "agtx_iva.h"
#include "agtx_video.h"
#include "agtx_video_layout_conf.h"

#include "agtx_audio.h"
#include "agtx_cmd.h"
#include "agtx_osd.h"
#include "agtx_color_conf.h"

#include "sample_dip.h"
#include "sample_light_src.h"
#include "utlist.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

LightSrc *g_light_src_head[MPI_MAX_INPUT_PATH_NUM];

extern CUSTOM_SNS_CTRL_S custom_sns(SNS0_ID);
#ifdef SNS1
extern CUSTOM_SNS_CTRL_S custom_sns(SNS1_ID);
#endif

static CUSTOM_SNS_CTRL_S *p_custom_sns[] = {
	&custom_sns(SNS0_ID),
#ifdef SNS1
	&custom_sns(SNS1_ID),
#endif
};

static inline int name2pinAndActivateValue(AGTX_GPIO_ALIAS_S gpio_alias[], const int num_of_elm, const char *str,
                                           int *pin_num, GpioValue *activate_value)
{
	int i;
	int ret = -1;
	*pin_num = -1;
	*activate_value = GPIO_VAL_LOW;

	if (num_of_elm <= 0) {
		avmain2_log_err("Invaild element size(%d)!\n", num_of_elm);
		return ret;
	}

	for (i = 0; i < num_of_elm; i++) {
		if (strcmp((const char *)str, (const char *)gpio_alias[i].name) == 0) {
			*pin_num = gpio_alias[i].pin_num;
			*activate_value = gpio_alias[i].value == 1 ? 0 : 1;
			avmain2_log_info("%s %d %d", str, *pin_num, *activate_value);
			return 0;
		}
	}

	if (i == num_of_elm) {
		avmain2_log_warn("Name(%s) no match\n", str);
	}

	return ret;
}

static void createAndInitLightSrc()
{
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		char ini_name[PATH_MAX];
		char dip_extend_normal[PATH_MAX] = { '0' };

		/**> Notice: libagtx and av_main2 can't change ini path */
		sprintf(ini_name, "%s/sensor_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) != 0) {
			sprintf(ini_name, "%s/sensor_0.ini", DIP_FILE_PATH);
			avmain2_log_err("Day mode use %s \n", ini_name);
		}
		g_light_src_head[idx] = SAMPLE_newDayLightSrc("day", ini_name);
		sprintf(dip_extend_normal, "%s/dip_extend_%d.ini", DIP_FILE_PATH, idx);
		if (access(dip_extend_normal, R_OK) == 0) {
			avmain2_log_err("day mode dip_extend use %s \n", dip_extend_normal);
			strncpy(g_light_src_head[idx]->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		} else {
			memset(dip_extend_normal, 0, sizeof(dip_extend_normal));
		}

		/**> create IR light mode hw */
		IrHwConfig ir_hw;
		name2pinAndActivateValue(g_conf.gpio.gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRCUT0_OUT",
		                         &ir_hw.ir_cut[0].id, &ir_hw.ir_cut[0].activate_value);
		name2pinAndActivateValue(g_conf.gpio.gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRCUT1_OUT",
		                         &ir_hw.ir_cut[1].id, &ir_hw.ir_cut[1].activate_value);
		name2pinAndActivateValue(g_conf.gpio.gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRLED_OUT",
		                         &ir_hw.ir_led.id, &ir_hw.ir_led.activate_value);

		/**> create Light mode hw */
		LightHwConfig light_hw;
		name2pinAndActivateValue(g_conf.gpio.gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "LED0_OUT",
		                         &light_hw.w_led.id, &light_hw.w_led.activate_value);

		/**> create IR lightSrc */
		LightSrc *tmp_src;

		sprintf(ini_name, "%s/sensor_ir_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) != 0) {
			sprintf(ini_name, "%s/sensor_night.ini", DIP_FILE_PATH);
			avmain2_log_warn("Ir mode use %s \n", ini_name);
		}
		tmp_src = SAMPLE_newIrLightSrc("ir", ini_name, &ir_hw);
		sprintf(ini_name, "%s/dip_extend_ir_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) == 0) {
			avmain2_log_warn("Ir mode dip_extend use %s \n", ini_name);
			strncpy(tmp_src->dip_extend_path, ini_name, sizeof(ini_name));
		} else {
			strncpy(tmp_src->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		}
		LL_APPEND(g_light_src_head[idx], tmp_src);

		/**> create Light lightSrc */
		sprintf(ini_name, "%s/sensor_light_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) != 0) {
			sprintf(ini_name, "%s/sensor_0.ini", DIP_FILE_PATH);
			avmain2_log_warn("Light mode use %s \n", ini_name);
		}
		tmp_src = SAMPLE_newWhiteLightSrc("light", ini_name, &light_hw);
		sprintf(ini_name, "%s/dip_extend_light_%d.ini", DIP_FILE_PATH, idx);
		if (access(ini_name, R_OK) == 0) {
			avmain2_log_warn("Light mode dip_extend use %s \n", ini_name);
			strncpy(tmp_src->dip_extend_path, ini_name, sizeof(ini_name));
		} else {
			strncpy(tmp_src->dip_extend_path, dip_extend_normal, sizeof(dip_extend_normal));
		}
		LL_APPEND(g_light_src_head[idx], tmp_src);
	}
}

static void destroyLightSrc()
{
	LightSrc *item, *tmp;
	LightSrc *light_src_head;
	for (int idx = 0; idx < MPI_MAX_INPUT_PATH_NUM; idx++) {
		if (g_conf.dev.input_path[idx].path_en == 0) {
			continue;
		}

		light_src_head = g_light_src_head[idx];
		/** day lightSrc still be created in not support multiple iq case */
		LL_FOREACH_SAFE(light_src_head, item, tmp)
		{
			LL_DELETE(light_src_head, item);
			SAMPLE_deleteLightSrc(item);
		}
	}
}

int NODE_initDev(void)
{
	createAndInitLightSrc();

	return 0;
}

int NODE_startDev(void) /*this return use MPI or errno?*/
{
	INT32 ret = MPI_FAILURE;
	int i = 0;

	/*f49763 dual sensor has 2 input path*/
	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	MPI_PATH path_idx = MPI_INPUT_PATH(g_conf.dev.video_dev_idx, g_conf.dev.input_path[0].path_idx);

	MPI_DEV_ATTR_S dev_attr;
	MPI_PATH_ATTR_S path_attr;

	/* Create video device */
	dev_attr.hdr_mode = g_conf.dev.hdr_mode;
	dev_attr.stitch_en = g_conf.dev.stitch_en;
	dev_attr.eis_en = g_conf.dev.eis_en;
	dev_attr.bayer = (g_conf.dev.bayer == AGTX_BAYER_G0) ?
	                         MPI_BAYER_PHASE_G0 :
	                         (g_conf.dev.bayer == AGTX_BAYER_R) ?
	                         MPI_BAYER_PHASE_R :
	                         (g_conf.dev.bayer == AGTX_BAYER_B) ?
	                         MPI_BAYER_PHASE_B :
	                         (g_conf.dev.bayer == AGTX_BAYER_G1) ? MPI_BAYER_PHASE_G1 : MPI_BAYER_PHASE_R;
	dev_attr.fps = g_conf.dev.input_fps;
	dev_attr.path.bmp = 0x0;

	for (i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en) {
			dev_attr.path.bmp |= (0x1 << i);
		}
	}

	ret = MPI_DEV_createDev(dev_idx, &dev_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Create video device %d failed. ret:%d", dev_idx.dev, ret);
		return -ENXIO;
	}

	/* Configure input path */
	for (i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en) {
			path_idx.path = g_conf.dev.input_path[i].path_idx;
			path_attr.sensor_idx = g_conf.dev.input_path[i].sensor_idx;
			path_attr.fps = g_conf.dev.input_path[i].fps;
			path_attr.res.width = g_conf.dev.input_path[i].width;
			path_attr.res.height = g_conf.dev.input_path[i].height;
			path_attr.eis_strength = g_conf.dev.input_path[i].eis_strength;

			ret = MPI_DEV_addPath(path_idx, &path_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set input path %d failed.ret:%d", path_idx.path, ret);
				return -ENXIO;
			}

			/* Register sensor callback function */
			p_custom_sns[g_conf.dev.input_path[i].sensor_idx]->reg_callback(path_idx);
			/* Register AE, AWB lib */
			MPI_regAeDftLib(path_idx);
			MPI_regAwbDftLib(path_idx);

			/* Get parameter from sensor driver */
			MPI_updateSnsParam(path_idx);
		}
	}

	for (i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en == 0) {
			continue;
		}
		path_idx.path = g_conf.dev.input_path[i].path_idx;
		char *ini_path = NULL;
		char *dip_extend_ini_path = NULL;

		/** whether support multiple iq or not, day lightSrc always created. */
		LightSrc *item, *tmp;
		LL_FOREACH_SAFE(g_light_src_head[i], item, tmp)
		{
			if (strcmp(item->name, "day") == 0) {
				ini_path = (char *)item->sensor_path;
				dip_extend_ini_path = (char *)item->dip_extend_path;
				break;
			}
		}

		if (access(ini_path, R_OK) != 0) {
			avmain2_log_err("Unfound default ini file:%s", ini_path);
			return -ENXIO;
		}

		if (!dip_extend_ini_path || access(dip_extend_ini_path, R_OK) != 0) {
			avmain2_log_err("Unfound default dip_extend ini file , not going to update dip_extend");
			dip_extend_ini_path = NULL;
		}

		avmain2_log_info("dip: %s, dip_extend: %s", ini_path, dip_extend_ini_path);

		ret = SAMPLE_updateDipAttrOnce(path_idx, (const char *)ini_path, (const char *)dip_extend_ini_path);
		if (ret != 0) {
			return ret;
		}

		SAMPLE_createDipAttrUpdateThread(path_idx, (const char *)ini_path, dip_extend_ini_path);
		if (ret != 0) {
			avmain2_log_err("Failed to create dip update thread, err: %d\n", ret);
		}

		char pca_file_name[PATH_MAX];
		snprintf(&pca_file_name[0], PATH_MAX, "/system/mpp/script/pca_cal_%d.lut", i);
		if (access(pca_file_name, R_OK) == 0) {
			ret = SAMPLE_updatePca(path_idx, pca_file_name);
			if (ret != 0) {
				avmain2_log_err("Unable to set PCA settings, err: %d\n", ret);
			}
		}
	}

	ret = MPI_DEV_startDev(dev_idx);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Start video device %d failed. err: %d", MPI_GET_VIDEO_DEV(dev_idx), ret);
		return -ENXIO;
	}

	return 0;
}

int NODE_stopDev(void)
{
	INT32 ret = MPI_FAILURE;
	int i = 0;

	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	ret = MPI_DEV_stopDev(dev_idx);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Stop video device %d failed. err: %d", MPI_GET_VIDEO_DEV(dev_idx), ret);
		return -ENXIO;
	}

	/*create, destroy chn in NODE dev or chn ?*/
	for (i = 0; i < MPI_MAX_INPUT_PATH_NUM; ++i) {
		if (g_conf.dev.input_path[i].path_en) {
			MPI_PATH path_idx = MPI_INPUT_PATH(g_conf.dev.video_dev_idx, g_conf.dev.input_path[i].path_idx);
			SAMPLE_destroyDipAttrUpdateThread(path_idx);

			/* Deregister AE, AWB lib */
			MPI_deregAeDftLib(path_idx);
			MPI_deregAwbDftLib(path_idx);

			/* Deregister sensor callback function */
			p_custom_sns[g_conf.dev.input_path[i].sensor_idx]->dereg_callback(path_idx);

			MPI_DEV_deletePath(path_idx);
		}
	}

	ret = MPI_DEV_destroyDev(dev_idx);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Destroy video device %d failed. err: %d", MPI_GET_VIDEO_DEV(dev_idx), ret);
		return -ENXIO;
	}

	return 0;
}

int NODE_exitDev(void)
{
	destroyLightSrc();
	return 0;
}

int NODE_setDev(int cmd_id, void *data)
{
	INT32 ret = 0;
	MPI_PATH path = MPI_INPUT_PATH(g_conf.dev.video_dev_idx, 0);
	MPI_PATH_ATTR_S path_attr;

	if (cmd_id == EIS_STRENGTH) {
		u_int8_t new_eis_strength = 0;
		avmain2_log_notice("AGTX_CMD_VIDEO_DEV_CONF change eis_strength");
		for (AGTX_UINT8 i = 0; i < MPI_MAX_INPUT_PATH_NUM; i++) {
			if (!g_conf.dev.input_path[i].path_en) {
				continue;
			}

			path.path = i;

			ret = MPI_DEV_getPathAttr(path, &path_attr);
			if (MPI_SUCCESS != ret) {
				avmain2_log_err("Get dev path attr for path %d failed. ret:%d", path.path, ret);
				return -EINVAL;
			}

			new_eis_strength = ((AGTX_DEV_CONF_S *)data)->input_path[i].eis_strength;
			avmain2_log_info("Set path[%d]eis_strength %d --> %d", i, path_attr.eis_strength,
			                 new_eis_strength);
			path_attr.eis_strength = new_eis_strength;
			ret = MPI_DEV_setPathAttr(path, &path_attr);

			if (MPI_SUCCESS != ret) {
				avmain2_log_err("Set dev path attr for dev path %d failed. ret:%d", path.path, ret);
				return -EINVAL;
			}
		}
	} else {
	}

	return 0;
}
