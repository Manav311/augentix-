#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for write
#include "getopt.h"
#include "errno.h"
#include "signal.h"

#include "mpi_sys.h"

#include "mpi_sys.h"

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

#include "cm_iva_aroi_conf.h"
#include "cm_iva_bm_conf.h"
#include "cm_iva_dk_conf.h"
#include "cm_iva_eaif_conf.h"
#include "cm_iva_ef_conf.h"
#include "cm_iva_fld_conf.h"
#include "cm_iva_ld_conf.h"
#include "cm_iva_md_conf.h"
#include "cm_iva_od_conf.h"
#include "cm_iva_pd_conf.h"
#include "cm_iva_pfm_conf.h"
#include "cm_iva_rms_conf.h"
#include "cm_iva_shd_conf.h"
#include "cm_iva_td_conf.h"
#include "cm_iaa_lsd_conf.h"
#include "cm_video_ptz_conf.h"

#include "log_define.h"
#include "nodes.h"
#include "handlers.h"

#define FILE_NAME_LEN (64)

Node g_monk_node[NODE_NUM];
extern Node g_nodes[NODE_NUM];
int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

int NODE_setChn(int cmd_id, void *data);
int NODE_setEnc(int cmd_id, void *data);

#define AVMAIN2_DEBUG

int initNodeTest(void)
{
	return 0;
}

int exitNodeTest(void)
{
	return 0;
}

int startNodeTest(void)
{
	return 0;
}

int stopNodeTest(void)
{
	return 0;
}

static int __attribute__((unused)) setNodeTest(void)
{
	return 0;
}

static void __attribute__((unused)) initNodes(void)
{
	/*assign node id and func ptr*/
	int idx = 0;
	g_monk_node[idx].id = VB;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = NULL;
	g_monk_node[idx].child[0] = &g_monk_node[1]; /*DEV*/
	g_monk_node[idx].child[1] = NULL; /*IMAGE_PREFERENCE*/
	idx++;

	g_monk_node[idx].id = DEV;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[0]; /*VB*/
	g_monk_node[idx].child[0] = &g_monk_node[2]; /*IMAGE PREF*/
	g_monk_node[idx].child[1] = &g_monk_node[3]; /*CHN*/
	idx++;

	g_monk_node[idx].id = IMAGE_PREFERENCE;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[0]; /*DEV*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = CHN;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].set = NODE_setChn;
	g_monk_node[idx].parent = &g_monk_node[1]; /*DEV*/
	g_monk_node[idx].child[0] = &g_monk_node[4]; /*IVA*/
	g_monk_node[idx].child[1] = &g_monk_node[5]; /*ENC*/
	idx++;

	g_monk_node[idx].id = IVA;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[3]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = ENC;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].set = NODE_setEnc;
	g_monk_node[idx].parent = &g_monk_node[3]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
}

void help()
{
	printf("[usage]");
	printf("-c --datapath <path of changed cmd>");
	printf("-d --dbpath <db path>, dft /tmp/ini.db");
	printf("-h --help");
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
	} else if (NULL != strstr(&data[0], "AGTX_CMD_TD_CONF")) {
		return AGTX_CMD_TD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_MD_CONF")) {
		return AGTX_CMD_MD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_AROI_CONF")) {
		return AGTX_CMD_AROI_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_PD_CONF")) {
		return AGTX_CMD_PD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_OD_CONF")) {
		return AGTX_CMD_OD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_RMS_CONF")) {
		return AGTX_CMD_RMS_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_LD_CONF")) {
		return AGTX_CMD_LD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_EF_CONF")) {
		return AGTX_CMD_EF_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_VIDEO_PTZ_CONF")) {
		return AGTX_CMD_VIDEO_PTZ_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_SHD_CONF")) {
		return AGTX_CMD_SHD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_EAIF_CONF")) {
		return AGTX_CMD_EAIF_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_PFM_CONF")) {
		return AGTX_CMD_PFM_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_BM_CONF")) {
		return AGTX_CMD_BM_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_DK_CONF")) {
		return AGTX_CMD_DK_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_FLD_CONF")) {
		return AGTX_CMD_FLD_CONF;
	} else if (NULL != strstr(&data[0], "AGTX_CMD_LSD_CONF")) {
		return AGTX_CMD_LSD_CONF;
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
	case AGTX_CMD_TD_CONF:
		parse_iva_td_conf((AGTX_IVA_TD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_TD_CONF_S);
		break;
	case AGTX_CMD_MD_CONF:
		parse_iva_md_conf((AGTX_IVA_MD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_MD_CONF_S);
		break;
	case AGTX_CMD_AROI_CONF:
		parse_iva_aroi_conf((AGTX_IVA_AROI_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_AROI_CONF_S);
		break;
	case AGTX_CMD_PD_CONF:
		parse_iva_pd_conf((AGTX_IVA_PD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_PD_CONF_S);
		break;
	case AGTX_CMD_OD_CONF:
		parse_iva_od_conf((AGTX_IVA_OD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_OD_CONF_S);
		break;
	case AGTX_CMD_RMS_CONF:
		parse_iva_rms_conf((AGTX_IVA_RMS_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_RMS_CONF_S);
		break;
	case AGTX_CMD_LD_CONF:
		parse_iva_ld_conf((AGTX_IVA_LD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_LD_CONF_S);
		break;
	case AGTX_CMD_EF_CONF:
		parse_iva_ef_conf((AGTX_IVA_EF_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_EF_CONF_S);
		break;
	case AGTX_CMD_VIDEO_PTZ_CONF:
		parse_video_ptz_conf((AGTX_VIDEO_PTZ_CONF_S *)data, obj);
		*len = sizeof(AGTX_VIDEO_PTZ_CONF_S);
		break;
	case AGTX_CMD_SHD_CONF:
		parse_iva_shd_conf((AGTX_IVA_SHD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_SHD_CONF_S);
		break;
	case AGTX_CMD_EAIF_CONF:
		parse_iva_eaif_conf((AGTX_IVA_EAIF_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_EAIF_CONF_S);
		break;
	case AGTX_CMD_PFM_CONF:
		parse_iva_pfm_conf((AGTX_IVA_PFM_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_PFM_CONF_S);
		break;
	case AGTX_CMD_BM_CONF:
		parse_iva_bm_conf((AGTX_IVA_BM_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_BM_CONF_S);
		break;
	case AGTX_CMD_DK_CONF:
		parse_iva_dk_conf((AGTX_IVA_DK_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_DK_CONF_S);
		break;
	case AGTX_CMD_FLD_CONF:
		parse_iva_fld_conf((AGTX_IVA_FLD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IVA_FLD_CONF_S);
		break;
	case AGTX_CMD_LSD_CONF:
		parse_iaa_lsd_conf((AGTX_IAA_LSD_CONF_S *)data, obj);
		*len = sizeof(AGTX_IAA_LSD_CONF_S);
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

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
	g_run_flag = 0;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int ret = 0;
	AgtxConf data; /*?*/
	int cmd_id = 0;
	int len = 0;
	int c;

	const char *optstring = "hc:d:";
	char DB_path[FILE_NAME_LEN] = { 0 };
	snprintf(&DB_path[0], FILE_NAME_LEN, "/tmp/ini.db");
	char data_path[FILE_NAME_LEN] = { 0 };

	struct option opts[] = { { "help", 0, NULL, 'h' }, { "datapath", 1, NULL, 'c' }, { "dbpath", 1, NULL, 'd' } };
	while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'c':
			snprintf(&data_path[0], FILE_NAME_LEN, "%s", argv[optind - 1]);
			break;
		case 'd':
			snprintf(&DB_path[0], FILE_NAME_LEN, "%s", argv[optind - 1]);
			break;
		default:
			help();
			exit(1);
		}
	}

	if (access(data_path, F_OK) != 0) {
		avmain2_log_err("data path not exist %s", data_path);
		return -ENODATA;
	}

	if (access(DB_path, F_OK) != 0) {
		avmain2_log_err("DB_path not exist %s", DB_path);
		return -ENODATA;
	}

	avmain2_log_info("data at: %s, db %s", data_path, DB_path);

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Initialize system failed.");
		return -ENXIO;
	}

	/*AGTX path compare with vb*/
	HANDLERS_allReadDb(DB_path);
#if 0
	initNodes();
#else
	NODES_initNodes();
#endif

	NODES_enterNodespreOrderTraversal(&g_nodes[VB]);

	sleep(1);

	/*transfer json to agtx format*/
	if (0 != getJsonCmd(data_path, (void *)&data, &len, &cmd_id)) {
		avmain2_log_err("failed to open %s", data_path);
		return -ENODATA;
	}

	ret = HANDLERS_apply(cmd_id, len, (void *)&data, &g_nodes[VB]);
	if (ret != 0) {
		avmain2_log_err("failed to apply %d", cmd_id);
	}

	g_run_flag = 1;
	while (g_run_flag) {
		sleep(1);
	}

	NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);

	ret = MPI_SYS_exit();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Exit system failed.");
		return -ENXIO;
	}

	return 0;
}