#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <cmocka_private.h>

#include "errno.h"
#include "getopt.h"

#include "json.h"
#include "agtx_video.h"
#include "cm_video_dev_conf.h"
#include "cm_video_layout_conf.h"
#include "cm_video_strm_conf.h"
#include "cm_osd_conf.h"
#include "cm_osd_pm_conf.h"

#include "cm_stitch_conf.h"
#include "cm_panorama_conf.h"
#include "cm_panning_conf.h"
#include "cm_surround_conf.h"
#include "cm_video_ldc_conf.h"

#include "cm_awb_pref.h"
#include "cm_anti_flicker_conf.h"
#include "cm_img_pref.h"
#include "cm_adv_img_pref.h"
#include "cm_color_conf.h"

#include "log_define.h"
#include "nodes.h"
#include "core.h"
#include "handlers.h"

int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

#define FILE_NAME_LEN (64)
Node g_monk_node[NODE_NUM];
extern GlobalConf g_conf;

static int VB_initNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_initNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_initNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_initNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_initNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_initNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_exitNodeTest(void)
{
	function_called();
	return 0;
}
static int ENC_exitNodeTest(void)
{
	avmain2_log_info("call\n");
	function_called();
	return 0;
}

static int VB_startNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_startNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_startNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_startNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_startNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_startNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int DEV_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int setAntiFlicker(MPI_DEV dev_idx, SAMPLE_DIP_CONF_S *dip_tmp, AGTX_ANTI_FLICKER_CONF_S *anti_flicker,
                          AGTX_ANTI_FLICKER_E *anti_flicker_switch)
{
	AGTX_UNUSED(dev_idx);
	AGTX_UNUSED(dip_tmp);
	AGTX_UNUSED(anti_flicker);
	AGTX_UNUSED(anti_flicker_switch);

	function_called();
	return 0;
}

static int setImg(MPI_DEV dev_idx, SAMPLE_DIP_CONF_S *dip_tmp, AGTX_IMG_PREF_S *img)
{
	AGTX_UNUSED(dev_idx);
	AGTX_UNUSED(dip_tmp);
	AGTX_UNUSED(img);

	function_called();
	return 0;
}

static int setAwbPref(MPI_DEV dev_idx, SAMPLE_DIP_CONF_S *dip_tmp, AGTX_AWB_PREF_S *awb)
{
	AGTX_UNUSED(dev_idx);
	AGTX_UNUSED(dip_tmp);
	AGTX_UNUSED(awb);

	function_called();
	return 0;
}

static int setAdvImgPref(MPI_DEV dev_idx, SAMPLE_DIP_CONF_S *dip_tmp, AGTX_ADV_IMG_PREF_S *adv_img)
{
	AGTX_UNUSED(dev_idx);
	AGTX_UNUSED(dip_tmp);
	AGTX_UNUSED(adv_img);

	function_called();
	return 0;
}

static int setColorMode(MPI_DEV dev_idx, SAMPLE_DIP_CONF_S *dip_tmp, AGTX_COLOR_CONF_S *color_cfg)
{
	AGTX_UNUSED(dev_idx);
	AGTX_UNUSED(dip_tmp);
	AGTX_UNUSED(color_cfg);

	function_called();
	return 0;
}

static int IMAGE_PREF_setNodeTest(int cmd_id, void *data)
{
	function_called();

	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	SAMPLE_DIP_CONF_S dip_tmp;

	switch (cmd_id) {
	case ANTI_FLIKER:
		if (0 !=
		    setAntiFlicker(dev_idx, &dip_tmp, (AGTX_ANTI_FLICKER_CONF_S *)data, &g_conf.img.anti_flicker)) {
			avmain2_log_err("Set device %d Anti flicker failed.", dev_idx.dev);
			return -EINVAL;
		}
		break;
	case IMG_PREF:
		if (0 != setImg(dev_idx, &dip_tmp, (AGTX_IMG_PREF_S *)data)) {
			avmain2_log_err("Set device %d Image control failed.", dev_idx.dev);
			return -EINVAL;
		}
		break;
	case AWB_PREF:
		if (0 != setAwbPref(dev_idx, &dip_tmp, (AGTX_AWB_PREF_S *)data)) {
			avmain2_log_err("Set device %d auto black and white mode failed.", dev_idx.dev);
			return -EINVAL;
		}
		break;
	case ADV_IMG_PREF:
		/*how to parse AGTX_CMD_ADV_IMG_S data to dip format*/
		if (0 != setAdvImgPref(dev_idx, &dip_tmp, (AGTX_ADV_IMG_PREF_S *)data)) {
			avmain2_log_err("Set device %d Advance image preference failed.", dev_idx.dev);
			return -EINVAL;
		}
		break;
	case COLOR_CONF:
		if (0 != setColorMode(dev_idx, &dip_tmp, (AGTX_COLOR_CONF_S *)data)) {
			avmain2_log_err("Set device %d color mode failed.", dev_idx.dev);
			return -EINVAL;
		}
		break;
	default:
		avmain2_log_err("invalid cmd_id: %d", cmd_id);
		return -EINVAL;
	}

	return 0;
}

static int CHN_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int IVA_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int ENC_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}

static void initMonkNodes(void)
{
	/*assign node id and func ptr*/
	int idx = 0;
	g_monk_node[idx].id = VB;
	g_monk_node[idx].init = VB_initNodeTest;
	g_monk_node[idx].exit = VB_exitNodeTest;
	g_monk_node[idx].start = VB_startNodeTest;
	g_monk_node[idx].stop = VB_stopNodeTest;
	g_monk_node[idx].set = VB_setNodeTest;
	g_monk_node[idx].parent = NULL;
	g_monk_node[idx].child[0] = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[1] = NULL; /*IMAGE_PREFERENCE*/
	idx++;

	g_monk_node[idx].id = DEV;
	g_monk_node[idx].init = DEV_initNodeTest;
	g_monk_node[idx].exit = DEV_exitNodeTest;
	g_monk_node[idx].start = DEV_startNodeTest;
	g_monk_node[idx].stop = DEV_stopNodeTest;
	g_monk_node[idx].set = DEV_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[VB]; /*VB*/
	g_monk_node[idx].child[0] = &g_monk_node[IMAGE_PREFERENCE]; /*IMAGE PREF*/
	g_monk_node[idx].child[1] = &g_monk_node[CHN]; /*CHN*/
	idx++;

	g_monk_node[idx].id = IMAGE_PREFERENCE;
	g_monk_node[idx].init = IMAGE_PREF_initNodeTest;
	g_monk_node[idx].exit = IMAGE_PREF_exitNodeTest;
	g_monk_node[idx].start = IMAGE_PREF_startNodeTest;
	g_monk_node[idx].stop = IMAGE_PREF_stopNodeTest;
	g_monk_node[idx].set = IMAGE_PREF_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = CHN;
	g_monk_node[idx].init = CHN_initNodeTest;
	g_monk_node[idx].exit = CHN_exitNodeTest;
	g_monk_node[idx].start = CHN_startNodeTest;
	g_monk_node[idx].stop = CHN_stopNodeTest;
	g_monk_node[idx].set = CHN_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[0] = &g_monk_node[IVA]; /*IVA*/
	g_monk_node[idx].child[1] = &g_monk_node[ENC]; /*ENC*/
	idx++;

	g_monk_node[idx].id = IVA;
	g_monk_node[idx].init = IVA_initNodeTest;
	g_monk_node[idx].exit = IVA_exitNodeTest;
	g_monk_node[idx].start = IVA_startNodeTest;
	g_monk_node[idx].stop = IVA_stopNodeTest;
	g_monk_node[idx].set = IVA_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[CHN]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = ENC;
	g_monk_node[idx].init = ENC_initNodeTest;
	g_monk_node[idx].exit = ENC_exitNodeTest;
	g_monk_node[idx].start = ENC_startNodeTest;
	g_monk_node[idx].stop = ENC_stopNodeTest;
	g_monk_node[idx].set = ENC_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[CHN]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;
}

static int parseCmdId(char *data)
{
	if (NULL != strstr(&data[0], "AGTX_CMD_VIDEO_DEV_CONF")) {
		return AGTX_CMD_VIDEO_DEV_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_VIDEO_LAYOUT_CONF")) {
		return AGTX_CMD_VIDEO_LAYOUT_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_VIDEO_STRM_CONF")) {
		return AGTX_CMD_VIDEO_STRM_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_AWB_PREF")) {
		return AGTX_CMD_AWB_PREF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_IMG_PREF")) {
		return AGTX_CMD_IMG_PREF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_ADV_IMG_PREF")) {
		return AGTX_CMD_ADV_IMG_PREF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_COLOR_CONF")) {
		return AGTX_CMD_COLOR_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_OSD_PM_CONF")) {
		return AGTX_CMD_OSD_PM_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_OSD_CONF")) {
		return AGTX_CMD_OSD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_STITCH_CONF")) {
		return AGTX_CMD_STITCH_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_ANTI_FLICKER_CONF")) {
		return AGTX_CMD_ANTI_FLICKER_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_LDC_CONF")) {
		return AGTX_CMD_LDC_CONF;
	}

	return -EINVAL;
}

static int freeJsonCmd(json_object *obj)
{
	json_object_put(obj);
	return 0;
}

static int getJsonCmd(char *path_name, void *data, int *len, int *cmd_id)
{
	struct json_object *obj = NULL;
	obj = json_object_from_file(path_name);
	if (obj == NULL) {
		avmain2_log_err("failed to open %s", path_name);
		return -ENODATA;
	}

	char cmd_type[FILE_NAME_LEN];
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(obj, "cmd_id", &tmp_obj)) {
		sprintf(&cmd_type[0], "%s", json_object_get_string(tmp_obj));
		avmain2_log_info("%s = %s\n", "cmd_id", cmd_type);
	} else {
		avmain2_log_err("failed to get cmd_type");
	}
	*cmd_id = parseCmdId(cmd_type);

	switch (*cmd_id) {
	case AGTX_CMD_VIDEO_DEV_CONF:
		parse_video_dev_conf((AGTX_DEV_CONF_S *)data, obj);
		*len = sizeof(AGTX_DEV_CONF_S);
		break;
	case AGTX_CMD_VIDEO_LAYOUT_CONF:
		parse_layout_conf((AGTX_LAYOUT_CONF_S *)data, obj);
		*len = sizeof(AGTX_LAYOUT_CONF_S);
		break;
	case AGTX_CMD_VIDEO_STRM_CONF:
		parse_video_strm_conf((AGTX_STRM_CONF_S *)data, obj);
		*len = sizeof(AGTX_STRM_CONF_S);
		break;
	case AGTX_CMD_STITCH_CONF:
		parse_stitch_conf((AGTX_STITCH_CONF_S *)data, obj);
		*len = sizeof(AGTX_STITCH_CONF_S);
		break;
	case AGTX_CMD_AWB_PREF:
		parse_awb_pref((AGTX_AWB_PREF_S *)data, obj);
		*len = sizeof(AGTX_AWB_PREF_S);
		break;
	case AGTX_CMD_IMG_PREF:
		parse_img_pref((AGTX_IMG_PREF_S *)data, obj);
		*len = sizeof(AGTX_IMG_PREF_S);
		break;
	case AGTX_CMD_ADV_IMG_PREF:
		parse_adv_img_pref((AGTX_ADV_IMG_PREF_S *)data, obj);
		*len = sizeof(AGTX_ADV_IMG_PREF_S);
		break;
	case AGTX_CMD_COLOR_CONF:
		parse_color_conf((AGTX_COLOR_CONF_S *)data, obj);
		*len = sizeof(AGTX_COLOR_CONF_S);
		break;
	case AGTX_CMD_LDC_CONF:
		parse_ldc_conf((AGTX_LDC_CONF_S *)data, obj);
		*len = sizeof(AGTX_LDC_CONF_S);
		break;
	case AGTX_CMD_PANORAMA_CONF:
		parse_panorama_conf((AGTX_PANORAMA_CONF_S *)data, obj);
		*len = sizeof(AGTX_PANORAMA_CONF_S);
		break;
	case AGTX_CMD_PANNING_CONF:
		parse_panning_conf((AGTX_PANNING_CONF_S *)data, obj);
		*len = sizeof(AGTX_PANNING_CONF_S);
		break;
	case AGTX_CMD_SURROUND_CONF:
		parse_surround_conf((AGTX_SURROUND_CONF_S *)data, obj);
		*len = sizeof(AGTX_SURROUND_CONF_S);
		break;
	case AGTX_CMD_ANTI_FLICKER_CONF:
		parse_anti_flicker_conf((AGTX_ANTI_FLICKER_CONF_S *)data, obj);
		*len = sizeof(AGTX_ANTI_FLICKER_CONF_S);
		break;
	case AGTX_CMD_OSD_CONF:
		parse_osd_conf((AGTX_OSD_CONF_S *)data, obj);
		*len = sizeof(AGTX_OSD_CONF_S);
		break;
	case AGTX_CMD_OSD_PM_CONF:
		parse_osd_pm_conf((AGTX_OSD_PM_CONF_S *)data, obj);
		*len = sizeof(AGTX_OSD_PM_CONF_S);
		break;
	default:
		freeJsonCmd(obj);
		avmain2_log_err("failed to parse cmd_id:%d", *cmd_id);
		return -EINVAL;
	}

	if (!obj) {
		freeJsonCmd(obj);
	}

	return 0;
}

/* Test case that fails as leak_memory() leaks a dynamically allocated block. */
static void readDBMemLeakTest(void **state)
{
	(void)state; /* unused */

	HANDLERS_allReadDb(TMP_ACTIVE_DB);
}

static void IMAGE_PREF_antiFlikerApplyTest(void **state)
{
	(void)state; /* unused */

	expect_function_call(IMAGE_PREF_setNodeTest);
	expect_function_call(setAntiFlicker);

	AgtxConf data;
	int cmd_id = 0;
	int len = 0;

	/*transfer json to agtx format*/
	if (0 != getJsonCmd("/mnt/nfs/ethnfs/test-case/ANTI_FLICKER/ANTI.json", (void *)&data, &len, &cmd_id)) {
		avmain2_log_err("failed to open %s", "/mnt/nfs/ethnfs/test-case/ANTI_FLICKER/ANTI.json");
		return;
	}

	HANDLERS_apply(cmd_id, len, (void *)&data, &g_monk_node[VB]);
}

static void IMAGE_PREF_awbApplyTest(void **state)
{
	(void)state; /* unused */

	expect_function_call(IMAGE_PREF_setNodeTest);
	expect_function_call(setAwbPref);

	AgtxConf data;
	int cmd_id = 0;
	int len = 0;

	/*transfer json to agtx format*/
	if (0 != getJsonCmd("/mnt/nfs/ethnfs/test-case/AWB/AWB.json", (void *)&data, &len, &cmd_id)) {
		avmain2_log_err("failed to open %s", "/mnt/nfs/ethnfs/test-case/AWB/AWB.json");
		return;
	}

	HANDLERS_apply(cmd_id, len, (void *)&data, &g_monk_node[VB]);
}

static void IMAGE_PREF_advImgApplyTest(void **state)
{
	(void)state; /* unused */

	expect_function_call(IMAGE_PREF_setNodeTest);
	expect_function_call(setAdvImgPref);

	AgtxConf data;
	int cmd_id = 0;
	int len = 0;

	/*transfer json to agtx format*/
	if (0 != getJsonCmd("/mnt/nfs/ethnfs/test-case/ADVIMG/ADVIMG.json", (void *)&data, &len, &cmd_id)) {
		avmain2_log_err("failed to open %s", "/mnt/nfs/ethnfs/test-case/ADVIMG/ADVIMG.json");
		return;
	}

	HANDLERS_apply(cmd_id, len, (void *)&data, &g_monk_node[VB]);
}

static void IMAGE_PREF_imgPrefApplyTest(void **state)
{
	(void)state; /* unused */

	expect_function_call(IMAGE_PREF_setNodeTest);
	expect_function_call(setImg);

	AgtxConf data;
	int cmd_id = 0;
	int len = 0;

	/*transfer json to agtx format*/
	if (0 != getJsonCmd("/mnt/nfs/ethnfs/test-case/IMG_PREF/IMGPREF.json", (void *)&data, &len, &cmd_id)) {
		avmain2_log_err("failed to open %s", "/mnt/nfs/ethnfs/test-case/IMG_PREF/IMGPREF.json");
		return;
	}

	HANDLERS_apply(cmd_id, len, (void *)&data, &g_monk_node[VB]);
}

int main(void)
{
	avmain2_log_info("call\n");

	initMonkNodes();

	const struct CMUnitTest test_1[] = {
		cmocka_unit_test(readDBMemLeakTest),           cmocka_unit_test(IMAGE_PREF_antiFlikerApplyTest),
		cmocka_unit_test(IMAGE_PREF_awbApplyTest),     cmocka_unit_test(IMAGE_PREF_advImgApplyTest),
		cmocka_unit_test(IMAGE_PREF_imgPrefApplyTest),
	};

	int result = 0;
	result = cmocka_run_group_tests(test_1, NULL, NULL);

	return result;
}