#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <json.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_sys.h"
#include "vftr_fld.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_FLD_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include <fld_demo.h>

MPI_WIN g_win_idx;

int g_fld_running; /* stop and start fld parameter*/

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
#endif

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static void handleSigInt(int signo)
{
	DBG("[%s]leave %s\n", __func__, __FILE__);

	g_fld_running = 0;
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		printf("Caught SIGKILL\n");
	} else if (signo == SIGQUIT) {
		printf("Caught SIGQUIT!\n");
	} else {
		perror("Unexpected signal!\n");
	}
}

int readJsonFromFile(char *file_name, VFTR_FLD_PARAM_S *fld_attr)
{
	int ret = 0;
	json_object *test_obj = NULL;
	json_object *tmp1_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		fprintf(stderr, "Cannot open %s\n", file_name);
		return -EBADF;
	}

	json_object_object_get_ex(test_obj, "obj_life_th", &tmp1_obj);
	fld_attr->obj_life_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "obj_life_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "obj_falling_mv_th", &tmp1_obj);
	fld_attr->obj_falling_mv_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "obj_falling_mv_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "obj_stop_mv_th", &tmp1_obj);
	fld_attr->obj_stop_mv_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "obj_stop_mv_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "obj_high_ratio_th", &tmp1_obj);
	fld_attr->obj_high_ratio_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "obj_high_ratio_th");
		ret = -ENOCSI;
		goto error;
	}
	/* falling_period_th mean falling duration. If the object is identified as falling. */
	json_object_object_get_ex(test_obj, "falling_period_th", &tmp1_obj);
	fld_attr->falling_period_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "falling_period_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "down_period_th", &tmp1_obj);
	fld_attr->down_period_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "down_period_th");
		ret = -ENOCSI;
		goto error;
	}
	/* fallen_period_th mean fallen time. */
	json_object_object_get_ex(test_obj, "fallen_period_th", &tmp1_obj);
	fld_attr->fallen_period_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "fallen_period_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "demo_level", &tmp1_obj);
	fld_attr->demo_level = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		fprintf(stderr, "Cannot get %s object\n", "demo_level");
		ret = -ENOCSI;
		goto error;
	}

	fld_attr->obj_high_ratio_th = (fld_attr->obj_high_ratio_th << VFTR_FLD_FRACTION) / 100;

error:
	json_object_put(test_obj);

	return ret;
}

int enableOd(MPI_WIN win, const MPI_IVA_OD_PARAM_S *od)
{
	int ret = 0;

	ret = MPI_IVA_setObjParam(win, od);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to set OD param.\n");
		goto error;
	}

	ret = MPI_IVA_enableObjDet(win);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to enable OD.\n");
		goto error;
	}

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}
/* OD parameters share to all region */

void help(void)
{
	printf("USAGE:\tfld_demo -i <CONFIG>\t\n");
	printf("\t-i <file>\t\tfall detection config in .json file\n");
	printf("\tobj_life_th: obj_life_th stands for object confidence threshold of OD. It uses to filter low confident objects.\n");
	printf("The range is [0-160] and the default value 0.\n");
	printf("\tobj_falling_mv_th: obj_falling_mv_th stands for falling mv threshold of object. The range is [0-120] and the default value 28.\n");
	printf("\tobj_stop_mv_th: obj_high_ratio_th stands for high ratio threshold with height history of object.\n");
	printf("The range is [0-256] and the default value 192.\n");
	printf("\tobj_high_ratio_th: It uses to filter object with low speed.\n");
	printf("\falling_period_th: falling_period_th stands for period threshold for falling status.\n");
	printf("The range is [0, 60*100] and the default value 200.\n");
	printf("\tdown_period_th: down_period_th stands for period threshold for down status.\n");
	printf("The range is [0, 60*100] and the default value 200.\n");
	printf("\tfallen_period_th: fallen_period_th stands for period threshold for fallen status.\n");
	printf("The range is [0, 60*100] and the default value 500.\n");
	printf("\tdemo_level: demo_level stands for demo level of detail [0: simple, 1:detailed].\n");
	printf("The range is [0, 1] and the default value 0.\n");
	printf("OPTIONS:\n");
	printf("\t-c <channel>\t\tSpecify which video channel to use. (Deatult 0).\n");
	printf("\t-d <detect interval>\tSpecify which video detect interval. (Deatult 30).\n");
	/* UINT8 od_qual; < Quality index of OD performance. */
	printf("\t-q <value>\t\tSpecify OD quality index.[0-63] (Deatult 46).\n");
	/* UINT8 od_sen; < sensitivity index of OD performance. */
	printf("\t-s <channel>\t\tSpecify OD sensitivity.[0-255] (Deatult 254).\n");
	printf("\n");
	printf("For example:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1001_FHD -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n");
	printf("\tfld_demo -i /system/mpp/fld_config/fld_conf_1080p_1.json &\n");
#ifdef CONFIG_APP_FLD_SUPPORT_SEI
	printf("\ttestOnDemandRTSPServer 0 -S\n");
#else
	printf("\ttestOnDemandRTSPServer 0 -n\n");
#endif
	printf("\n");
}

int main(int argc, char **argv)
{
	int ret;
	int c;
	int chn_idx = 0;
	int win_idx = 0;
	int win_dummy=0; 
	char cfg_file_name[256] = { 0 };

	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 62, .od_track_refine = 30, .od_size_th = 12, .od_sen = 254, .en_stop_det = 0, .en_gmv_det = 0
	};

	VFTR_FLD_PARAM_S fld_attr;

	while ((c = getopt(argc, argv, "i:c:w:r:q:s:t:T:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(cfg_file_name, optarg);
			DBG("input .json file:%s\n", cfg_file_name);
			break;
		case 'c':
			chn_idx = atoi(optarg);
			DBG("set device channel:%d\n", chn_idx);
			break;
		case 'w':
			win_idx = atoi(optarg);
			DBG("set device window:%d\n", win_idx);
			break;
		case 'r':
			od_param.od_track_refine = atoi(optarg);
			DBG("set detect refine:%d\n", optarg);
			break;
		case 'q':
			od_param.od_qual = atoi(optarg);
			DBG("set detect quality index:%s\n", optarg);
			break;
		case 't':
			od_param.od_size_th = atoi(optarg);
			DBG("set detect size threshold:%s\n", optarg);
			break;
		case 'T':
			win_dummy = atoi(optarg);
			break;
		case 's':
			od_param.od_sen = atoi(optarg);
			DBG("set detect sensitivity:%s\n", optarg);
			break;
		case 'h':
			help();
			exit(0);
		default:
			abort();
		}
	}

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (strlen(cfg_file_name) == 0) {
		fprintf(stderr, "Config file path is not specified !\n");
		exit(1);
	}

	ret = readJsonFromFile(cfg_file_name, &fld_attr);
	if (ret != MPI_SUCCESS) {
		DBG("Read Json form file:%s\n", cfg_file_name);
		return ret;
	}

	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(0, chn_idx);

	/* Initialize MPI system */
	MPI_SYS_init();

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_DEV_getChnAttr.\n");
		return -EINVAL;
	}

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_DEV_getlayoutAttr.\n");
		return -EINVAL;
	}

	/* NOTICE: we only support the case that only one window in a channel */
	assert(layout_attr.window_num == 1);
	assert(layout_attr.window[0].x == 0);
	assert(layout_attr.window[0].y == 0);
	assert(layout_attr.window[0].width == chn_attr.res.width);
	assert(layout_attr.window[0].height == chn_attr.res.height);

	/* Check parameters */
	ret = VFTR_FLD_checkParam(&fld_attr);
	if (ret) {
		fprintf(stderr, "Invalid FLD parameters\n");
		return -EINVAL;
	}

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
	/* init AV server to wait RTSP client connectd
	 * allow transfer IVA result to RTSP streaming server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */
	ret = AVFTR_initServer();
	if (ret) {
		fprintf(stderr, "Failed to initalize AV server %d\n", ret);
		return -ENOPROTOOPT;
	}
#endif

	/* Set window index */
	g_win_idx = MPI_VIDEO_WIN(0, chn_idx, win_idx);
	g_win_idx.dummy0 = win_dummy;

	/* Implment the following function */
	/* EnableOd() */
	ret = enableOd(g_win_idx, &od_param);
	if (ret < 0) {
		fprintf(stderr, "Failed to enable OD, please check if OD is enabled\n");
		return -EINVAL;
	}

	/* DetectFldObject() */

	ret = detectFldObject(g_win_idx, &chn_attr.res, &fld_attr);
	ret = MPI_IVA_disableObjDet(g_win_idx);

	/* init server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
	AVFTR_exitServer();
#endif
	MPI_SYS_exit();
	/* disableod */
	return 0;
}
