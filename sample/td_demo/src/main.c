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
#include "mpi_sys.h"
#include "vftr_td.h"
#include "log.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_TD_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include <td_demo.h>

MPI_WIN g_win_idx;

int g_td_running;

#ifdef CONFIG_APP_TD_SUPPORT_SEI
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

	g_td_running = 0;
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		/* The default sent by kill and killall */
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		printf("Caught SIGKILL\n");
	} else if (signo == SIGQUIT) {
		printf("Caught SIGQUIT!\n");
	} else {
		perror("Unexpected signal!\n");
	}
}

int readJsonFromFile(char *file_name, VFTR_TD_PARAM_S *td_attr)
{
	int ret = 0;
	json_object *test_obj = NULL;
	json_object *tmp1_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		log_err("Cannot open %s", file_name);
		return -EBADF;
	}

	/* TD parameter */
	json_object_object_get_ex(test_obj, "en_block_det", &tmp1_obj);
	td_attr->en_block_det = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "en_block_det");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "en_redirect_det", &tmp1_obj);
	td_attr->en_redirect_det = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "en_redirect_det");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "sensitivity", &tmp1_obj);
	td_attr->sensitivity = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "sensitivity");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "endurance", &tmp1_obj);
	td_attr->endurance = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "endurance");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "redirect_sensitivity", &tmp1_obj);
	td_attr->redirect_sensitivity = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "redirect_sensitivity");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "redirect_global_change", &tmp1_obj);
	td_attr->redirect_global_change = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "redirect_global_change");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(test_obj, "redirect_trigger_delay", &tmp1_obj);
	td_attr->redirect_trigger_delay = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "redirect_trigger_delay");
		ret = -ENOCSI;
		goto error;
	}

	DBG("en_block_det: %d\n", td_attr->en_block_det);
	DBG("en_redirect_det: %d\n", td_attr->en_redirect_det);
	DBG("sensitivity: %d\n", td_attr->sensitivity);
	DBG("endurance: %d\n", td_attr->endurance);
	DBG("redirect_sensitivity: %d\n", td_attr->redirect_sensitivity);
	DBG("redirect_global_change: %d\n", td_attr->redirect_global_change);
	DBG("redirect_trigger_delay: %d\n", td_attr->redirect_trigger_delay);

	int sen_min_map = 16;
	int sen_max_map = 216;
	int temp = ((td_attr->sensitivity * (sen_max_map - sen_min_map) / 100) + sen_min_map);
	td_attr->sensitivity = (temp > VFTR_TD_SENSITIVITY_MAX) ? VFTR_TD_SENSITIVITY_MAX : temp;

error:
	json_object_put(test_obj);

	return ret;
}

void help(void)
{
	printf("USAGE:\ttd_demo -i <CONFIG>\t\n");
	printf("\t-i <file>\t\ttamper detection config in .json file\n");
	printf("\ten_block_det: Enable/disable block detection. Range: [0, 1].\n");
	printf("\ten_redirect_det: Enable/disable redirect detection. Range: [0, 1].\n");
	printf("\tsensitivity: The sensitivity of block detection. Range: [1, 255], default value: 60.\n");
	printf("\tendurance: The endurance of block detection, Range: [0, 255], default value: 60.\n");
	printf("\tredirect_sensitivity: The sensitivity of redirect detection. Range: [0, 255], default value: 128. \n");
	printf("\tredirect_global_change: The global change of redirect detection. Range: [0, 255], default value: 50. \n");
	printf("\tredirect_trigger_delay: The trigger delay of redirect detection. Range: [0, 3600], default value: 5, unit: second. \n");
	printf("OPTIONS:\n");
	printf("\t-c <channel>\t\tSpecify which video channel to use. (Deatult 0).\n");
	printf("\n");
	printf("For example:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1001_FHD -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n");
	printf("\ttd_demo -i /system/mpp/td_config/td_conf_1080p.json &\n");
#ifdef CONFIG_APP_TD_SUPPORT_SEI
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
	char cfg_file_name[256] = { 0 };

	VFTR_TD_PARAM_S td_attr;

	while ((c = getopt(argc, argv, "i:c:w:h")) != -1) {
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
		log_err("Config file path is not specified!");
		exit(1);
	}

	ret = readJsonFromFile(cfg_file_name, &td_attr);
	if (ret != MPI_SUCCESS) {
		DBG("Read Json form file:%s\n", cfg_file_name);
		return -EINVAL;
	}

	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(0, chn_idx);

	/* Initialize MPI system */
	MPI_SYS_init();

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		return -EINVAL;
	}

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video layout attribute. err: %d", ret);
		return -EINVAL;
	}

	/* NOTICE: we only support the case that only one window in a channel */
	assert(layout_attr.window_num == 1);
	assert(layout_attr.window[0].x == 0);
	assert(layout_attr.window[0].y == 0);
	assert(layout_attr.window[0].width == chn_attr.res.width);
	assert(layout_attr.window[0].height == chn_attr.res.height);

	/* Check parameters */
	ret = VFTR_TD_checkParam(&td_attr);
	if (ret) {
		log_err("Invalid TD parameters");
		return -EINVAL;
	}

#ifdef CONFIG_APP_TD_SUPPORT_SEI
	/* init AV server to wait RTSP client connect
	 * allow transfer IVA result to RTSP streaming server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */
	ret = AVFTR_initServer();
	if (ret) {
		log_err("Failed to initialize AV server %d", ret);
		return -ENOPROTOOPT;
	}
#endif

	/* Set window index */
	g_win_idx = MPI_VIDEO_WIN(0, chn_idx, win_idx);

	/* DetectTdObject() */
	ret = detectTdObject(g_win_idx, &layout_attr, &td_attr);

	/* init server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */

#ifdef CONFIG_APP_TD_SUPPORT_SEI
	AVFTR_exitServer();
#endif

	MPI_SYS_exit();

	return 0;
}
