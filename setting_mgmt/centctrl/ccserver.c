/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * FILENAME - FILE_DESCRIPTION
 * Copyright (C) 2018 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 * * Brief: Dummy Unix Socket synchronous IO server Supports Multi client connections
 *
 * * Author: Ram <ram.kumar@@augentix.com>
 */

/*
 *
 *  -Multi Client Synchronous io (select) server
 *  -Server Listens for clients on UnxSocket ( support multi client )
 *  -Reads data from clients and validates the payload is JSON format
 *  -Reply the same recv json cmd back to client for testing only.
 *
 *  -TODO:
 *      - JSON decode:exception handling (test for exception cases)
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/file.h>

#include "json.h"

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_cmd.h"

#include "cm_sys_info.h"
#include "cm_sys_feature_option.h"
#include "cm_sys_db_info.h"
#include "cm_adv_img_pref.h"
#include "cm_awb_pref.h"
#include "cm_color_conf.h"
#include "cm_dip_ae_conf.h"
#include "cm_dip_awb_conf.h"
#include "cm_dip_cal_conf.h"
#include "cm_dip_csm_conf.h"
#include "cm_dip_ctrl_conf.h"
#include "cm_dip_dbc_conf.h"
#include "cm_dip_dcc_conf.h"
#include "cm_dip_gamma_conf.h"
#include "cm_dip_iso_conf.h"
#include "cm_dip_lsc_conf.h"
#include "cm_dip_nr_conf.h"
#include "cm_dip_pta_conf.h"
#include "cm_dip_roi_conf.h"
#include "cm_dip_shp_conf.h"
#include "cm_dip_te_conf.h"
#include "cm_dip_enh_conf.h"
#include "cm_dip_coring_conf.h"
#include "cm_dip_fcs_conf.h"
#include "cm_dip_dhz_conf.h"
#include "cm_dip_hdr_synth_conf.h"
#include "cm_dip_stat_conf.h"
#include "cm_event_conf.h"
#include "cm_event_param.h"
#include "cm_gpio_conf.h"
#include "cm_local_record_conf.h"
#include "cm_img_pref.h"
#include "cm_iva_od_conf.h"
#include "cm_iva_md_conf.h"
#include "cm_iva_td_conf.h"
#include "cm_iva_aroi_conf.h"
#include "cm_iva_pd_conf.h"
#include "cm_iva_ld_conf.h"
#include "cm_iva_rms_conf.h"
#include "cm_iva_ef_conf.h"
#include "cm_vdbg_conf.h"
#include "cm_video_ptz_conf.h"
#include "cm_iva_shd_conf.h"
#include "cm_iva_eaif_conf.h"
#include "cm_iva_pfm_conf.h"
#include "cm_iva_bm_conf.h"
#include "cm_iva_dk_conf.h"
#include "cm_iva_fld_conf.h"
#include "cm_iaa_lsd_conf.h"
#include "cm_osd_conf.h"
#include "cm_osd_pm_conf.h"
#include "cm_audio_conf.h"
#include "cm_voice_conf.h"
#include "cm_siren_conf.h"
#include "cm_product_option.h"
#include "cm_product_option_list.h"
#include "cm_res_option.h"
#include "cm_venc_option.h"
#include "cm_stitch_conf.h"
#include "cm_video_dev_conf.h"
#include "cm_video_strm_conf.h"
#include "cm_video_layout_conf.h"
#include "cm_video_ldc_conf.h"
#include "cm_panorama_conf.h"
#include "cm_panning_conf.h"
#include "cm_surround_conf.h"
#include "cm_pwm_conf.h"
#include "cm_pir_conf.h"
#include "cm_floodlight_conf.h"
#include "cm_dip_shp_win_conf.h"
#include "cm_dip_nr_win_conf.h"

#include "cc_common.h"
#include "cc_utils.h"
#include "cc_merge.h"

#include "sqllib.h"
#define CC_DEBUG 0


int g_unx_skt_fd; //Unix Socket FD
int g_lock_fd = -1;
FILE *g_fp = NULL;

/*
 * Command line arguments.
 */
static struct option longopts[] = {
	{ "fg",  no_argument,    NULL, 'F', },
	{ "bg",  no_argument,    NULL, 'B', },
	{ NULL,            0,    NULL,   0, },
};


static int checkSingleInstance(void)
{
	int ret = 0;
	int lock_fd;
	const char *lock_file_path = LOCK_FILE_PATH;

	lock_fd = open(lock_file_path, O_CREAT | O_RDWR, 0600);
	if (lock_fd < 0) {
		fprintf(stderr, "Open lock file %s failed\n", lock_file_path);
		return -1;
	}

	ret = flock(lock_fd, LOCK_EX | LOCK_NB);
	if (ret) {
		 if (EWOULDBLOCK == errno) {
			 fprintf(stderr, "ccserver is already running ...!\n");
			 fprintf(stderr, "To restart it, you have to kill the program and delete the lock file %s\n", lock_file_path);
			 close(lock_fd);
			 return -1;
		 }
	} else {
		fprintf(stderr, "Starting ccserver ...\n");
	}

	g_lock_fd = lock_fd;

	return 0;
}

static void removeSingleInstance(void)
{
	int lock_fd = g_lock_fd;

	if (lock_fd >= 0) {
		flock(lock_fd, LOCK_UN);
		close(lock_fd);
	}

	g_lock_fd = -1;
}

static void createReadyFile(void)
{
	const char *file = SOCKET_READY_FILE_PATH;

	g_fp = fopen(file, "wb");
	if (g_fp == NULL) {
		fprintf(stderr, "Can not open ccserver file\n");
		exit(-1);
	}
}

static void removeReadyFile(void)
{
	const char *file = SOCKET_READY_FILE_PATH;

	fclose(g_fp);
	g_fp = NULL;

	if (!remove(file)) {
		char buf[64];

		memset(buf, 0, 64);
		sprintf(buf, "rm -f %s", file);
		system(buf); //force delete
	}
}

static void usage(int argc, char **argv)
{
	AGTX_UNUSED(argc);

	fprintf(stderr, "Usage: %s [--en_fg] [--en_bg]\n", argv[0]);
	exit(EXIT_FAILURE);
}

static void cleanOldFd (const char *fileName)
{
	if (!access(fileName, F_OK)) {
		fprintf(stderr, "Deleting old unix File descriptor \n");

		if (!remove(fileName)) {
			char buf[64];

			memset(buf, 0, 64);

			sprintf(buf, "rm -f %s", fileName);

			system(buf); //force delete
		}
	}
}

static void freeUnxSkt()
{
	if (g_unx_skt_fd) {
		close(g_unx_skt_fd);
	}
}

static void sigHandler (int sigNum)
{
	fprintf(stderr, "UnxSktServer: recv Signal Intr: %d \n",sigNum );
	removeReadyFile();
	freeUnxSkt();
	removeSingleInstance();
	exit(1);
}

static void segmentFaltHandler (int seg)
{
	AGTX_UNUSED(seg);

	fprintf(stderr, "==> Segmentation fault!! <==\n");
	exit(1);
}

#if 0 // Was used in main()
static void delay(float num_sec)
{
	//convert time to millisec
	int millSec = 1000 * num_sec;
	//start time
	clock_t startTime = clock();
	//loop till required time

	while (clock() < (startTime + millSec))
		;
}
#endif

typedef struct {
	int sd;
	int state;
	int cid;
	char *request_buf;
	size_t request_size;
	struct json_tokener *tokener;
} CC_CLIENT_INFO_S;

typedef struct {
	AGTX_INT32       master_id;
	AGTX_UINT32      cmd_id;
	AGTX_CMD_TYPE_E  cmd_type;
} CC_MSG_GEN_INFO_S;


static CC_CLIENT_INFO_S client_socket[CC_MAX_CLIENT_NUM];

#if 0
static int json_validation(char *buffer, int strlen)
{
	struct json_object *json_obj, *tmp_obj, *tmp1_obj;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	char json_buf[JSON_BUF_STR_SIZE]; // or bigger array to append data to handle pkt defrag
	char *str;

//	const char *json_string;
//	char error_string[JSON_ERR_STR_SIZE];
//	int val_type;
//	char *val_type_str;

	bzero(json_buf, JSON_BUF_STR_SIZE);

	/* Parse the buffer */
	if (strlen > 0) {
		strcat(json_buf, (char *)buffer);
		/* g_jbuf_size += strlen;  // size needed for pkt re-assembly */
		g_jbuf_size = strlen;
		json_obj = json_tokener_parse_ex(tok, json_buf, g_jbuf_size);
	} else {
		json_obj = json_tokener_parse_ex(tok, (char *)buffer, g_jbuf_size);
	}

	jerr = json_tokener_get_error(tok);

	if (jerr != json_tokener_success) {
		fprintf(stderr, "JSON Tokener errNo: %d \n", jerr);
		fprintf(stderr, "JSON Parsing errorer %s \n", json_tokener_error_desc(jerr));
		goto error;
	}

	//get master_id
	if (!json_object_object_get_ex(json_obj, CC_JSON_KEY_MASTER_ID, &tmp_obj)) {
		fprintf(stderr, "Cannot get %s object\n", CC_JSON_KEY_MASTER_ID);
		goto error;
	}
	msg_data.master_id = json_object_get_int(tmp_obj);
	fprintf(stderr, "%s = %d\n", CC_JSON_KEY_MASTER_ID, msg_data.master_id);

	//get cmd_id
	if (!json_object_object_get_ex(json_obj, CC_JSON_KEY_CMD_ID, &tmp_obj)) {
		fprintf(stderr, "Cannot get %s object\n", CC_JSON_KEY_CMD_ID);
		goto error;
	}
	msg_data.cmd_id = json_object_get_int(tmp_obj);
	fprintf(stderr, "%s = %d\n", CC_JSON_KEY_CMD_ID, msg_data.cmd_id);

	//get cmd_type
	if (!json_object_object_get_ex(json_obj, CC_JSON_KEY_CMD_TYPE, &tmp_obj)) {
		fprintf(stderr, "Cannot get %s object\n", CC_JSON_KEY_CMD_TYPE);
		goto error;
	}
	str = (char *)json_object_get_string(tmp_obj);

	if (str) {
		if (!strcmp(str, "notify")) {
			msg_data.cmd_type = AGTX_CMD_TYPE_NOTIFY;
		} else if (!strcmp(str, "ctrl")) {
			msg_data.cmd_type = AGTX_CMD_TYPE_CTRL;
		} else if (!strcmp(str, "set")) {
			msg_data.cmd_type = AGTX_CMD_TYPE_SET;
		} else if (!strcmp(str, "get")) {
			msg_data.cmd_type = AGTX_CMD_TYPE_GET;
		} else if (!strcmp(str, "reply")) {
			msg_data.cmd_type = AGTX_CMD_TYPE_REPLY;
		} else {
			fprintf(stderr, "ERROR: Invalid cmd_type (%s)\n", str);
			goto error;
		}

		fprintf(stderr, "%s = %d\n", CC_JSON_KEY_CMD_TYPE, msg_data.cmd_type);
	}
	str = NULL;

	switch(msg_data.cmd_id) {
	case AGTX_CMD_SESS_START:
		strcpy(buffer, "{\"master_id\":11, \"cmd_id\":1048577, \"cmd_type\":\"reply\", \"rval\":0, \"name\":\"CGI\"}");
		break;
	case AGTX_CMD_IMG_PREF:
	{
		if (!json_object_object_get_ex(json_obj, "brightness", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "brightness");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "brightness", json_object_get_int(tmp_obj));

		if (!json_object_object_get_ex(json_obj, "saturation", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "saturation");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "saturation", json_object_get_int(tmp_obj));

		if (!json_object_object_get_ex(json_obj, "contrast", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "contrast");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "contrast", json_object_get_int(tmp_obj));

		if (!json_object_object_get_ex(json_obj, "sharpness", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "sharpness");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "sharpness", json_object_get_int(tmp_obj));

		if (!json_object_object_get_ex(json_obj, "anti_flicker", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "anti_flicker");
			goto error;
		}
		fprintf(stderr, "%s = %s\n", "anti_flicker", json_object_get_string(tmp_obj));

		if (!json_object_object_get_ex(json_obj, "white_balance", &tmp_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "white_balance");
			goto error;
		}

		if (!json_object_object_get_ex(tmp_obj, "mode", &tmp1_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "mode");
			goto error;
		}
		fprintf(stderr, "%s = %s\n", "mode", json_object_get_string(tmp1_obj));

		if (!json_object_object_get_ex(tmp_obj, "color_temp", &tmp1_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "color_temp");
			goto error;
		}
		fprintf(stderr, "%s = %s\n", "color_temp", json_object_get_string(tmp1_obj));

		if (!json_object_object_get_ex(tmp_obj, "r_gain", &tmp1_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "r_gain");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "r_gain", json_object_get_int(tmp1_obj));

		if (!json_object_object_get_ex(tmp_obj, "b_gain", &tmp1_obj)) {
			fprintf(stderr, "Cannot get %s object\n", "b_gain");
			goto error;
		}
		fprintf(stderr, "%s = %d\n", "b_gain", json_object_get_int(tmp1_obj));

		strcpy(buffer, "{\"master_id\":11, \"cmd_id\":3145733, \"cmd_type\":\"reply\", \"rval\":0}");
		break;
	}
	default:
		break;
	}

/*	else if (jerr == json_tokener_success) {
		//TODO: pass to sub-func's which parse a given key data type
		str = NULL;
	//	struct json_object *object = json_object_new_object();

		json_object_object_foreach(json_obj, key, val) {
			fprintf(stderr, "Key: \"%s\" , type of val: ", key);
			val_type = json_object_get_type(val);

			switch (val_type) {
				case json_type_null:
					val_type_str = "val is NULL";
					break;
				case json_type_boolean:
					val_type_str = "val is a boolean";
					break;
				case json_type_double:
					val_type_str = "val is a double";
					break;
				case json_type_int:
					val_type_str = "val is an integer";
					break;
				case json_type_string:
					val_type_str = "val is a string";
					str = (char *) json_object_get_string(val);
					break;
				case json_type_object:
					val_type_str = "val is an object";
					break;
				case json_type_array:
					val_type_str = "val is an array";
					break;
			}

			fprintf(stderr,"%s", val_type_str);

			if (str) { fprintf(stderr, "\\t->\\t\"%s \n", str); }

			str = NULL;

			//pkt re-assembly Reset json_buf , g_jbuf_size
			g_jbuf_size = 0; //json_buf = NULL;
		}
	}
*/
	g_jbuf_size = 0;
	json_tokener_free(tok);

	return 0;

error:
	g_jbuf_size = 0;
	json_tokener_free(tok);
	return -1;

}
#endif

static int count_active_client()
{
	int numsActive = 0;
	for (int i = 0; i < CC_MAX_CLIENT_NUM; ++i) {
		if (client_socket[i].sd) {
			++numsActive;
		}
	}

	return numsActive;
}

static int get_client_sd(int cid)
{
	int sd  = 0;
	int idx = 0;
	CC_CLIENT_INFO_S *client = NULL;

	for (idx = 0; idx < CC_MAX_CLIENT_NUM; ++idx) {
		client = &client_socket[idx];

		if (client->cid == cid) {
			sd = client->sd;
			break;
		}
	}

	return sd;
}

static int validate_cmd_id(AGTX_UINT32 cmd_id)
{
	int ret = 0;

	AGTX_UINT32 cat  = AGTX_CMD_CAT(cmd_id);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (cat) {
	case AGTX_CAT_SYS:
		ret = (item > AGTX_ITEM_SYS_NONE && item < AGTX_ITEM_SYS_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_NET:
		ret = (item > AGTX_ITEM_NET_NONE && item < AGTX_ITEM_NET_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_VIDEO:
		ret = (item > AGTX_ITEM_VIDEO_NONE && item < AGTX_ITEM_VIDEO_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_AUDIO:
		ret = (item > AGTX_ITEM_AUDIO_NONE && item < AGTX_ITEM_AUDIO_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_EVT:
		ret = (item > AGTX_ITEM_EVT_NONE && item < AGTX_ITEM_EVT_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_OSD:
		ret = (item > AGTX_ITEM_OSD_NONE && item < AGTX_ITEM_OSD_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_IVA:
		ret = (item > AGTX_ITEM_IVA_NONE && item < AGTX_ITEM_IVA_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_IAA:
		ret = (item > AGTX_ITEM_IAA_NONE && item < AGTX_ITEM_IAA_NUM) ? 0 : -1;
		break;

	case AGTX_CAT_NONE:
	default:
		ret = -1;
		break;
	}

	return ret;
}

/* Function to get general information of the message:
   input takes a string (JSON)
   -  get master ID, command ID and command type from a valid JSON string

*/
static void get_msg_general_info(struct json_object *json_obj, CC_MSG_GEN_INFO_S *info)
{
	struct json_object *tmp_obj = NULL;
	const char  *str = NULL;

	if (json_object_object_get_ex(json_obj, CC_JSON_KEY_MASTER_ID, &tmp_obj)) {
		info->master_id = json_object_get_int(tmp_obj);

		if (CC_DEBUG) {
			fprintf(stderr, "%s: send data to host\n", __func__);
		}
	}

	if (json_object_object_get_ex(json_obj, CC_JSON_KEY_CMD_ID, &tmp_obj)) {
		info->cmd_id = json_object_get_int(tmp_obj);

		if (CC_DEBUG) {
			fprintf(stderr, "%s: send data to host\n", __func__);
		}
	}

	if (json_object_object_get_ex(json_obj, CC_JSON_KEY_CMD_TYPE, &tmp_obj)) {
		str = json_object_get_string(tmp_obj);

		if (str) {
			if (!strcmp(str, "notify")) {
				info->cmd_type = AGTX_CMD_TYPE_NOTIFY;
			} else if (!strcmp(str, "ctrl")) {
				info->cmd_type = AGTX_CMD_TYPE_CTRL;
			} else if (!strcmp(str, "set")) {
				info->cmd_type = AGTX_CMD_TYPE_SET;
			} else if (!strcmp(str, "get")) {
				info->cmd_type = AGTX_CMD_TYPE_GET;
			} else if (!strcmp(str, "reply")) {
				info->cmd_type = AGTX_CMD_TYPE_REPLY;
			} else {
				fprintf(stderr, "Invalid cmd_type (%s)\n", str);
			}

			if (CC_DEBUG) {
				fprintf(stderr, "%s: send data to host\n", __func__);
			}
		}
	}

	return;
}

static int get_client_identity(struct json_object *cmd_obj, int *cid)
{
	int ret = 0;
	struct json_object *tmp_obj = NULL;

	if (json_object_object_get_ex(cmd_obj, CC_JSON_KEY_CLIENT_NAME, &tmp_obj)) {
		const char *str = json_object_get_string(tmp_obj);

		if (str) {
			if (!strcmp(str, "ONVIF")) {
				*cid = CC_CLIENT_ID_ONVIF;
			} else if (!strcmp(str, "CGI")) {
				*cid = CC_CLIENT_ID_CGI;
			} else if (!strcmp(str, "UNICORN")) {
				*cid = CC_CLIENT_ID_UNICORN;
			} else if (!strcmp(str, "APP_HOST")) {
				*cid = CC_CLIENT_ID_APP_HOST;
			} else if (!strcmp(str, "AV_MAIN")) {
				*cid = CC_CLIENT_ID_AV_MAIN;
			} else if (!strcmp(str, "EVT_MAIN")) {
				*cid = CC_CLIENT_ID_EVT_MAIN;
			} else if (!strcmp(str, "APP")) {
				*cid = CC_CLIENT_ID_APP;
			} else if (!strcmp(str, "COLOR_CTRL")) {
				*cid = CC_CLIENT_ID_COLOR_CTRL;
			} else {
				ret = -1;
				fprintf(stderr, "Invalid client name (%s)\n", str);
			}

			if (!ret) {
				fprintf(stderr, "%s = %d\n", CC_JSON_KEY_CLIENT_NAME, *cid);
			}
		} else {
			ret = -1;
			fprintf(stderr, "Cannot get %s string\n", CC_JSON_KEY_CLIENT_NAME);
		}
	} else {
		ret = -1;
		fprintf(stderr, "Cannot get %s object\n", CC_JSON_KEY_CLIENT_NAME);
	}

	return ret;
}

static int reply_exception_case(char *buf, CC_MSG_GEN_INFO_S *info, int rval)
{
	int ret = 0;
	const char *json_str = NULL;
	struct json_object *json_obj = NULL;
	struct json_object *tmp_obj  = NULL;

	json_obj = json_object_new_object();
	if (!json_obj) {
		printf("Cannot create object\n");
		ret = -1;
		goto err;
	}

	tmp_obj = json_object_new_int(info->master_id);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_MASTER_ID);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_MASTER_ID, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_int(info->cmd_id);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_CMD_ID);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_CMD_ID, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_string("reply");
	if (!tmp_obj) {
		printf("Cannot create string object for %s\n", CC_JSON_KEY_CMD_TYPE);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_CMD_TYPE, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_int(-rval);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_RET_VAL);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_RET_VAL, tmp_obj);
	tmp_obj = NULL;

	json_str = json_object_to_json_string(json_obj);

	strcpy(buf, json_str);

err:
	if (json_obj) {
		json_object_put(json_obj);
	}
	return ret;
}

static int reply_cmd_reg_client(char *buf, CC_MSG_GEN_INFO_S *info)
{
	int ret = 0;
	const char *json_str = NULL;
	struct json_object *json_obj = NULL;
	struct json_object *tmp_obj  = NULL;

	json_obj = json_object_new_object();
	if (!json_obj) {
		printf("Cannot create object\n");
		ret = -1;
		goto err;
	}

	tmp_obj = json_object_new_int(info->master_id);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_MASTER_ID);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_MASTER_ID, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_int(info->cmd_id);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_CMD_ID);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_CMD_ID, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_string("reply");
	if (!tmp_obj) {
		printf("Cannot create string object for %s\n", CC_JSON_KEY_CMD_TYPE);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_CMD_TYPE, tmp_obj);
	tmp_obj = NULL;

	tmp_obj = json_object_new_int(CC_SUCCESS);
	if (!tmp_obj) {
		printf("Cannot create integer object for %s\n", CC_JSON_KEY_RET_VAL);
		ret = -1;
		goto err;
	}
	json_object_object_add(json_obj, CC_JSON_KEY_RET_VAL, tmp_obj);
	tmp_obj = NULL;

	json_str = json_object_to_json_string(json_obj);

	strcpy(buf, json_str);

err:
	if (json_obj) {
		json_object_put(json_obj);
	}
	return ret;
}

static int ctrl_sess_start(struct json_object *ret_obj, struct json_object *cmd_obj,
                           AGTX_UINT32 cmd_id)
{
	AGTX_UNUSED(cmd_obj);
	AGTX_UNUSED(cmd_id);

	int ret = 0;
	struct json_object *tmp_obj = NULL;

	static AGTX_UINT32 session_id = CC_SESSION_ID_MIN;

	tmp_obj = json_object_new_int(session_id);
	if (tmp_obj) {
		json_object_object_add(ret_obj, CC_JSON_KEY_MASTER_ID, tmp_obj);
	} else {
		ret = -1;
		fprintf(stderr, "Cannot create %s object\n", CC_JSON_KEY_MASTER_ID);
	}

	if (!ret) {
		session_id++;
		if (session_id >= CC_SESSION_ID_MAX) {
			session_id = CC_SESSION_ID_MIN;
		}
	}

	return ret;
}

static int handle_sys_ctrl_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                               AGTX_UINT32 cmd_id)
{
	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_SYS_SESS_START:
		ret = ctrl_sess_start(ret_obj, cmd_obj, cmd_id);
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown system command item: %u", item);
		break;

	}

	return ret;
}

static int handle_sys_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
//	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_SYS_INFO:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_SYS_INFO_S));
		break;
	case AGTX_ITEM_SYS_FEATURE_OPTION:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_SYS_FEATURE_OPTION_S));
		break;
	case AGTX_ITEM_SYS_PRODUCT_OPTION_LIST:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_PRODUCT_OPTION_LIST_S));
		break;
	case AGTX_ITEM_SYS_DB_INFO:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_SYS_DB_INFO_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown system command item: %u", item);
		break;
	}

	return ret;
}

static int handle_video_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                                AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_VIDEO_DEV_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_DEV_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_STRM_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_STRM_CONF_S));
		if (ret != 0) {
			break;
		}
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_STRM_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_LAYOUT_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_LAYOUT_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_STITCH_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_STITCH_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_AWB_PREF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_AWB_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_IMG_PREF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IMG_PREF_S));
		if (ret != 0) {
			break;
		}
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IMG_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_ADV_IMG_PREF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_ADV_IMG_PREF_S));
		if (ret != 0) {
			break;
		}
		sd = get_client_sd(CC_CLIENT_ID_EVT_MAIN);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_ADV_IMG_PREF_S));
		if (ret != 0) {
			break;
		}
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_ADV_IMG_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_CAL:
	case AGTX_ITEM_VIDEO_DIP_DBC:
	case AGTX_ITEM_VIDEO_DIP_DCC:
	case AGTX_ITEM_VIDEO_DIP_LSC:
	case AGTX_ITEM_VIDEO_DIP_CTRL:
	case AGTX_ITEM_VIDEO_DIP_AE:
	case AGTX_ITEM_VIDEO_DIP_AWB:
	case AGTX_ITEM_VIDEO_DIP_PTA:
	case AGTX_ITEM_VIDEO_DIP_CSM:
	case AGTX_ITEM_VIDEO_DIP_SHP:
	case AGTX_ITEM_VIDEO_DIP_NR:
	case AGTX_ITEM_VIDEO_DIP_ROI:
	case AGTX_ITEM_VIDEO_DIP_TE:
	case AGTX_ITEM_VIDEO_DIP_GAMMA:
	case AGTX_ITEM_VIDEO_DIP_ENH:
	case AGTX_ITEM_VIDEO_DIP_CORING:
	case AGTX_ITEM_VIDEO_DIP_FCS:
	case AGTX_ITEM_VIDEO_DIP_DHZ:
	case AGTX_ITEM_VIDEO_DIP_HDR_SYNTH:
	case AGTX_ITEM_VIDEO_DIP_STAT:
	case AGTX_ITEM_VIDEO_DIP_ISO:
	case AGTX_ITEM_VIDEO_DIP_EXP_INFO:
	case AGTX_ITEM_VIDEO_DIP_WB_INFO:
	case AGTX_ITEM_VIDEO_VIEW_TYPE:
		ret = CC_DEPRECATED;
		fprintf(stderr, "%u is not supported!", item);
		break;
	case AGTX_ITEM_VIDEO_COLOR_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_COLOR_CONF_S));
		if (ret != 0) {
			break;
		}
		sd = get_client_sd(CC_CLIENT_ID_EVT_MAIN);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_COLOR_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PRODUCT_OPTION:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(_AGTX_PRODUCT_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_RES_OPTION:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_RES_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_VENC_OPTION:
		ret = set_data_to_active_db(ret_obj, cmd_obj, 0, cmd_id, sizeof(AGTX_VENC_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_LDC:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_LDC_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PANORAMA:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_PANORAMA_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PANNING:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_PANNING_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_SURROUND:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_SURROUND_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_ANTI_FLICKER_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_ANTI_FLICKER_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_SHP_WIN:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_DIP_SHP_WIN_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_NR_WIN:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_DIP_NR_WIN_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PRIVATE_MODE:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_PRIVATE_MODE_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown video command item: %u", item);
		break;
	}

	return ret;
}

static int handle_audio_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_AUDIO_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_AUDIO_CONF_S));
		break;
	case AGTX_ITEM_VOICE_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_VOICE_CONF_S));
		break;
	case AGTX_ITEM_SIREN_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_SIREN_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown audio command item: %u", item);
		break;
	}

	return ret;
}

static int handle_evt_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_EVT_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_EVT_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_EVENT_CONF_S));
		break;
	case AGTX_ITEM_EVT_GPIO_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_GPIO_CONF_S));
		break;
	case AGTX_ITEM_EVT_PARAM:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_EVENT_PARAM_S));
		break;
	case AGTX_ITEM_LOCAL_RECORD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_LOCAL_RECORD_CONF_S));
		break;
	case AGTX_ITEM_PWM_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_PWM_CONF_S));
		break;
	case AGTX_ITEM_PIR_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_PIR_CONF_S));
		break;
	case AGTX_ITEM_FLOODLIGHT_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_FLOODLIGHT_CONF_S));
		break;
	case AGTX_ITEM_LIGHT_SENSOR_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_LIGHT_SENSOR_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown evt command item: %u", item);
		break;
	}

	return ret;
}

static int handle_osd_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_OSD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_OSD_CONF_S));
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_OSD_CONF_S));
		break;
	case AGTX_ITEM_OSD_PM_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_OSD_PM_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown osd command item: %u, id:%u\n", item, cmd_id);
		break;
	}

	return ret;
}

static int handle_iva_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_IVA_OD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_OD_CONF_S));
		break;
	case AGTX_ITEM_IVA_TD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_TD_CONF_S));
		break;
	case AGTX_ITEM_IVA_MD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_MD_CONF_S));
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret |= set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_MD_CONF_S));
		break;
	case AGTX_ITEM_IVA_AROI_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_AROI_CONF_S));
		break;
	case AGTX_ITEM_IVA_PD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_PD_CONF_S));
		break;
	case AGTX_ITEM_IVA_LD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_LD_CONF_S));
		break;
	case AGTX_ITEM_IVA_RMS_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_RMS_CONF_S));
		break;
	case AGTX_ITEM_IVA_EF_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_EF_CONF_S));
		break;
	case AGTX_ITEM_VDBG_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_VDBG_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PTZ_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_VIDEO_PTZ_CONF_S));
		sd = get_client_sd(CC_CLIENT_ID_APP);
		ret |= set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_VIDEO_PTZ_CONF_S));
		break;
	case AGTX_ITEM_IVA_SHD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_SHD_CONF_S));
		break;
	case AGTX_ITEM_IVA_EAIF_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_EAIF_CONF_S));
		break;
	case AGTX_ITEM_IVA_PFM_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_PFM_CONF_S));
		break;
	case AGTX_ITEM_IVA_BM_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_BM_CONF_S));
		break;
	case AGTX_ITEM_IVA_DK_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_DK_CONF_S));
		break;
	case AGTX_ITEM_IVA_FLD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IVA_FLD_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown iva command item: %u", item);
		break;
	}

	return ret;
}

static int handle_iaa_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id)
{
	int ret = 0;
	int sd = get_client_sd(CC_CLIENT_ID_AV_MAIN);
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_IAA_LSD_CONF:
		ret = set_data_to_active_db(ret_obj, cmd_obj, sd, cmd_id, sizeof(AGTX_IAA_LSD_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown iaa command item: %u", item);
		break;
	}

	return ret;
}

static int handle_sys_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_SYS_INFO:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_SYS_INFO_S));
		break;
	case AGTX_ITEM_SYS_FEATURE_OPTION:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_SYS_FEATURE_OPTION_S));
		break;
	case AGTX_ITEM_SYS_PRODUCT_OPTION_LIST:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PRODUCT_OPTION_LIST_S));
		break;
	case AGTX_ITEM_SYS_DB_INFO:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_SYS_DB_INFO_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown system command item: %u", item);
		break;
	}

	return ret;
}

static int handle_video_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                                AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_VIDEO_DEV_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_DEV_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_STRM_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_STRM_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_LAYOUT_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_LAYOUT_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_STITCH_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_STITCH_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_AWB_PREF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_AWB_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_IMG_PREF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IMG_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_ADV_IMG_PREF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_ADV_IMG_PREF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_CAL:
	case AGTX_ITEM_VIDEO_DIP_DBC:
	case AGTX_ITEM_VIDEO_DIP_DCC:
	case AGTX_ITEM_VIDEO_DIP_LSC:
	case AGTX_ITEM_VIDEO_DIP_CTRL:
	case AGTX_ITEM_VIDEO_DIP_AE:
	case AGTX_ITEM_VIDEO_DIP_AWB:
	case AGTX_ITEM_VIDEO_DIP_PTA:
	case AGTX_ITEM_VIDEO_DIP_CSM:
	case AGTX_ITEM_VIDEO_DIP_SHP:
	case AGTX_ITEM_VIDEO_DIP_NR:
	case AGTX_ITEM_VIDEO_DIP_ROI:
	case AGTX_ITEM_VIDEO_DIP_TE:
	case AGTX_ITEM_VIDEO_DIP_GAMMA:
	case AGTX_ITEM_VIDEO_DIP_ENH:
	case AGTX_ITEM_VIDEO_DIP_CORING:
	case AGTX_ITEM_VIDEO_DIP_FCS:
	case AGTX_ITEM_VIDEO_DIP_DHZ:
	case AGTX_ITEM_VIDEO_DIP_HDR_SYNTH:
	case AGTX_ITEM_VIDEO_DIP_STAT:
	case AGTX_ITEM_VIDEO_DIP_ISO:
	case AGTX_ITEM_VIDEO_DIP_EXP_INFO:
	case AGTX_ITEM_VIDEO_DIP_WB_INFO:
	case AGTX_ITEM_VIDEO_VIEW_TYPE:
		ret = CC_DEPRECATED;
		fprintf(stderr, "%u is not supported!", item);
		break;
	case AGTX_ITEM_VIDEO_COLOR_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_COLOR_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PRODUCT_OPTION:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(_AGTX_PRODUCT_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_RES_OPTION:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_RES_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_VENC_OPTION:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_VENC_OPTION_S));
		break;
	case AGTX_ITEM_VIDEO_LDC:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_LDC_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PANORAMA:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PANORAMA_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PANNING:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PANNING_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_SURROUND:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_SURROUND_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_ANTI_FLICKER_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_ANTI_FLICKER_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_SHP_WIN:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_DIP_SHP_WIN_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_DIP_NR_WIN:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_DIP_NR_WIN_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PRIVATE_MODE:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PRIVATE_MODE_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown video command item: %u", item);
		break;
	}

	return ret;
}

static int handle_audio_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_AUDIO_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_AUDIO_CONF_S));
		break;
	case AGTX_ITEM_VOICE_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_VOICE_CONF_S));
		break;
	case AGTX_ITEM_SIREN_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_SIREN_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown audio command item: %u", item);
		break;
	}

	return ret;
}

static int handle_evt_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_EVT_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_EVENT_CONF_S));
		break;
	case AGTX_ITEM_EVT_GPIO_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_GPIO_CONF_S));
		break;
	case AGTX_ITEM_EVT_PARAM:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_EVENT_PARAM_S));
		break;
	case AGTX_ITEM_LOCAL_RECORD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_LOCAL_RECORD_CONF_S));
		break;
	case AGTX_ITEM_PWM_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PWM_CONF_S));
		break;
	case AGTX_ITEM_PIR_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_PIR_CONF_S));
		break;
	case AGTX_ITEM_FLOODLIGHT_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_FLOODLIGHT_CONF_S));
		break;
	case AGTX_ITEM_LIGHT_SENSOR_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_LIGHT_SENSOR_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown evt command item: %u", item);
		break;
	}

	return ret;
}

static int handle_osd_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_OSD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_OSD_CONF_S));
		break;
	case AGTX_ITEM_OSD_PM_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_OSD_PM_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown osd command item: %u", item);
		break;
	}

	return ret;
}

static int handle_iva_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_IVA_OD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_OD_CONF_S));
		break;
	case AGTX_ITEM_IVA_TD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_TD_CONF_S));
		break;
	case AGTX_ITEM_IVA_MD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_MD_CONF_S));
		break;
	case AGTX_ITEM_IVA_AROI_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_AROI_CONF_S));
		break;
	case AGTX_ITEM_IVA_PD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_PD_CONF_S));
		break;
	case AGTX_ITEM_IVA_LD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_LD_CONF_S));
		break;
	case AGTX_ITEM_IVA_RMS_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_RMS_CONF_S));
		break;
	case AGTX_ITEM_IVA_EF_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_EF_CONF_S));
		break;
	case AGTX_ITEM_VDBG_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_VDBG_CONF_S));
		break;
	case AGTX_ITEM_VIDEO_PTZ_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_VIDEO_PTZ_CONF_S));
		break;
	case AGTX_ITEM_IVA_SHD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_SHD_CONF_S));
		break;
	case AGTX_ITEM_IVA_EAIF_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_EAIF_CONF_S));
		break;
	case AGTX_ITEM_IVA_PFM_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_PFM_CONF_S));
		break;
	case AGTX_ITEM_IVA_BM_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_BM_CONF_S));
		break;
	case AGTX_ITEM_IVA_DK_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_DK_CONF_S));
		break;
	case AGTX_ITEM_IVA_FLD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IVA_FLD_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown iva command item: %u", item);
		break;
	}

	return ret;
}

static int handle_iaa_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                              AGTX_UINT32 cmd_id, int sd)
{
	AGTX_UNUSED(cmd_obj);

	int ret = 0;
	AGTX_UINT32 item = AGTX_CMD_ITEM(cmd_id);

	switch (item) {
	case AGTX_ITEM_IAA_LSD_CONF:
		ret = get_data_from_active_db(ret_obj, sd, cmd_id, sizeof(AGTX_IAA_LSD_CONF_S));
		break;
	default:
		ret = -1;
		fprintf(stderr, "Unknown iaa command item: %u", item);
		break;
	}

	return ret;
}

static int handle_ctrl_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                           AGTX_UINT32 master_id, AGTX_UINT32 cmd_id)
{
	AGTX_UNUSED(master_id);

	int ret = 0;
	AGTX_UINT32 cat = AGTX_CMD_CAT(cmd_id);

	/* TODO: check if this client has control grant */

	switch (cat) {
	case AGTX_CAT_SYS:
		ret = handle_sys_ctrl_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int handle_set_cmd(struct json_object *ret_obj, struct json_object *cmd_obj,
                          AGTX_UINT32 master_id, AGTX_UINT32 cmd_id)
{
	AGTX_UNUSED(master_id);

	int ret = 0;
	AGTX_UINT32 cat = AGTX_CMD_CAT(cmd_id);

	/* TODO: check if this client has control grant */

	switch (cat) {
	case AGTX_CAT_SYS:
		ret = handle_sys_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_NET:
	//	ret = handle_net_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_VIDEO:
		ret = handle_video_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_AUDIO:
		ret = handle_audio_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_EVT:
		ret = handle_evt_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_OSD:
		ret = handle_osd_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_IVA:
		ret = handle_iva_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	case AGTX_CAT_IAA:
		ret = handle_iaa_set_cmd(ret_obj, cmd_obj, cmd_id);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int handle_get_cmd(struct json_object *ret_obj, struct json_object *cmd_obj, AGTX_UINT32 cmd_id)
{
	int ret = 0;
	AGTX_UINT32 cat = AGTX_CMD_CAT(cmd_id);

	switch (cat) {
	case AGTX_CAT_SYS:
		ret = handle_sys_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_NET:
	//	ret = handle_net_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_VIDEO:
		ret = handle_video_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_AUDIO:
		ret = handle_audio_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_EVT:
		ret = handle_evt_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_OSD:
		ret = handle_osd_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_IVA:
		ret = handle_iva_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	case AGTX_CAT_IAA:
		ret = handle_iaa_get_cmd(ret_obj, cmd_obj, cmd_id, 0 /* dummy sd */);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static void comp_reply_msg(struct json_object *ret_obj, CC_MSG_GEN_INFO_S *info, int rval)
{
	struct json_object *tmp_obj = NULL;

	if (info->cmd_id != AGTX_CMD_SESS_START) {
		tmp_obj = json_object_new_int(info->master_id);
		if (tmp_obj) {
			json_object_object_add(ret_obj, CC_JSON_KEY_MASTER_ID, tmp_obj);
		} else {
			printf("Cannot create %s object\n", CC_JSON_KEY_MASTER_ID);
		}
	}

	tmp_obj = json_object_new_int(info->cmd_id);
	if (tmp_obj) {
		json_object_object_add(ret_obj, CC_JSON_KEY_CMD_ID, tmp_obj);
	} else {
		printf("Cannot create %s object\n", CC_JSON_KEY_CMD_ID);
	}

	tmp_obj = json_object_new_string("reply");
	if (tmp_obj) {
		json_object_object_add(ret_obj, CC_JSON_KEY_CMD_TYPE, tmp_obj);
	} else {
		printf("Cannot create %s object\n", CC_JSON_KEY_CMD_TYPE);
	}

	tmp_obj = json_object_new_int(rval);
	if (tmp_obj) {
		json_object_object_add(ret_obj, CC_JSON_KEY_RET_VAL, tmp_obj);
	} else {
		printf("Cannot create %s object\n", CC_JSON_KEY_RET_VAL);
	}

	return;
}

static int handle_host_cmd(char *buf, struct json_object *cmd_obj,
                           CC_MSG_GEN_INFO_S *info)
{
	int ret = 0;
	const char *json_str = NULL;
	struct json_object *ret_obj = NULL;

	ret_obj = json_object_new_object();
	if (!ret_obj) {
		fprintf(stderr, "Cannot create object\n");
		goto no_mem;
	}

	switch (info->cmd_type) {
	case AGTX_CMD_TYPE_NOTIFY:
//		ret = handle_notify_cmd(ret_obj, cmd_obj, info->master_id, info->cmd_id);
		break;
	case AGTX_CMD_TYPE_CTRL:
		ret = handle_ctrl_cmd(ret_obj, cmd_obj, info->master_id, info->cmd_id);
		break;
	case AGTX_CMD_TYPE_SET:
		ret = handle_set_cmd(ret_obj, cmd_obj, info->master_id, info->cmd_id);
		break;
	case AGTX_CMD_TYPE_GET:
		ret = handle_get_cmd(ret_obj, cmd_obj, info->cmd_id);
		break;
	default:
		break;
	}
	comp_reply_msg(ret_obj, info, ret);

	json_str = json_object_to_json_string_ext(ret_obj, JSON_C_TO_STRING_PLAIN);

	if (strlen(json_str) < CC_JSON_STR_BUF_SIZE) {
		strcpy(buf, json_str);
	} else {
		reply_exception_case(buf, info, AGTX_ERR_LARGE_JSON_STR);
		printf("Warning: reply string length(%d) is larger than buffer size(%d)\n", strlen(json_str), CC_JSON_STR_BUF_SIZE);
	}

	json_object_put(ret_obj);

	return 0;

no_mem:
	/* TODO: handle the case of no memory */

	return -1;
}

static int handle_module_cmd(AGTX_MSG_HEADER_S *msg, int sd)
{
	int ret = 0;
	AGTX_UINT32 cmd_id = msg->cid;
	AGTX_UINT32 cat = AGTX_CMD_CAT(cmd_id);

	switch (cat) {
	case AGTX_CAT_SYS:
	//	ret = handle_sys_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_NET:
	//	ret = handle_net_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_VIDEO:
		ret = handle_video_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_AUDIO:
		ret = handle_audio_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_EVT:
		ret = handle_evt_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_OSD:
		ret = handle_osd_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_IVA:
		ret = handle_iva_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	case AGTX_CAT_IAA:
		ret = handle_iaa_get_cmd(NULL, NULL, cmd_id, sd);
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static void overwrite_db_with_cal_data(unsigned int force_update)
{
	DECLARE_TIMEVAL(btime);
	DECLARE_TIMEVAL(etime);

	get_time(&btime);

	/* Update calibration data */
	update_cal_data(STITCH_CONF_JSON_FILE, STITCH_CONF_UPD_FILE, force_update);
	update_cal_data(COLOR_DIFF_JSON_FILE, COLOR_DIFF_UPD_FILE, force_update);
	update_cal_data(PANORAMA_JSON_FILE, PANORAMA_UPD_FILE, force_update);
	update_cal_data(ANTI_FLICKER_JSON_FILE, ANTI_FLICKER_UPD_FILE, force_update);
	update_cal_data(LIGHT_SENSOR_JSON_FILE, LIGHT_SENSOR_UPD_FILE, force_update);

	get_time(&etime);
	print_timediff(btime, etime);
}


int main (int argc, char **argv)
{
	int ret = 0;
	int activity;
	int idx = 0, addrlen, valread;
	int max_sd, sd, new_socket;
	int cid = CC_CLIENT_ID_INVALID;
	int opt = 1;
	struct timeval tv = { .tv_sec = CC_SKT_TIMEOUT_SEC, .tv_usec = CC_SKT_TIMEOUT_USEC };
//	unsigned char srv_fg = 0; // var to make server run in background(0) of foreground(1)

	struct sockaddr_un addr;
	fd_set readfds; //set of socket descriptors
	const char *skt_file_path = SOCKET_FILE_PATH;

	static char ret_buf[CC_JSON_STR_BUF_SIZE];
	static char request_pool[CC_MAX_CLIENT_NUM][CC_JSON_STR_BUF_SIZE];
	CC_CLIENT_INFO_S   *client   = NULL;
	struct json_object *json_obj = NULL;
	CC_MSG_GEN_INFO_S info = { .master_id = -1, .cmd_id = 0, .cmd_type = -1 };

	DECLARE_TIMEVAL(btime);
	DECLARE_TIMEVAL(etime);

	get_time(&btime);

	/* SIG handler  */
	signal ( SIGTERM, sigHandler );
	signal ( SIGQUIT, sigHandler );
	signal ( SIGKILL, sigHandler );
	signal ( SIGHUP, sigHandler );
	signal ( SIGINT, sigHandler );
	signal ( SIGUSR1, sigHandler );
	signal ( SIGSEGV, segmentFaltHandler );

	// When the client FD is broken while in Use linux rises SIGPIPE for this app we ignore
	signal(SIGPIPE, SIG_IGN);

	while ((idx = getopt_long(argc, argv, "FB", longopts, NULL)) != -1) {
		switch (idx) {
#if 0
		case 'F':
			fprintf(stderr, "Central Control Server: run in foreground \n");
			srv_fg = 1;
			break;
		case 'B':
			fprintf(stderr, "Central Control Server: run in background \n");
			srv_fg = 0;
			break;
#endif
		default:
		//	srv_fg = 1;
			usage(argc, argv);
			break;
		}
	}

	ret = checkSingleInstance();
	if (ret) {
		exit(-1);
	}

	//Initialise all client_socket[] to invalid value
	for (idx = 0; idx < CC_MAX_CLIENT_NUM; idx++) {
		client = &client_socket[idx];

		client->sd    = 0;
		client->state = CC_CLIENT_STATE_NONE;
		client->cid   = CC_CLIENT_ID_INVALID;
		client->request_buf = request_pool[idx];
		client->request_size = 0;
		client->tokener = json_tokener_new();
	}

	/* Update active database form factory default for firmware upgrade */
	if (access(SYSUPD_FILE_PATH, R_OK) != -1) {
		FILE *db_merge_done_file = NULL;

		fprintf(stderr, "Merging database ...\n");

		merge_database();

		/* Force to overwrite database */
		overwrite_db_with_cal_data(1);

		/* Set DB merge done flag for dbmonitor */
		db_merge_done_file = fopen(DB_MERGE_DONE_FILE, "w");
		if (db_merge_done_file) {
			fclose(db_merge_done_file);
		} else {
			fprintf(stderr, "Failed to set DB merge done flag\n");
		}

		fprintf(stderr, "Merging database done\n");
	} else {
		FILE *cal_done_file = fopen(CC_UPDATE_CAL_DONE_FILE, "r");

		if (cal_done_file) {
			/* Overwite cal data if that is updated */
			overwrite_db_with_cal_data(0);
			fclose(cal_done_file);
		} else {
			/* Overwrite cal data for the first time */
			overwrite_db_with_cal_data(1);

			/* Create update cal file */
			cal_done_file = fopen(CC_UPDATE_CAL_DONE_FILE, "w");
			if (cal_done_file) {
				fclose(cal_done_file);
			} else {
				fprintf(stderr, "Failed to set cal update flag\n");
			}
		}
	}

//	if (!srv_fg) { daemon(0, 1); }

	/* clean up any Orphen socket before start server */
	cleanOldFd(skt_file_path);

	//create socket
	if ((g_unx_skt_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf (stderr, "socket error \n");
		exit(-1);
	}

	//set master socket to allow multiple connections
	if (setsockopt(g_unx_skt_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0) {
		fprintf (stderr, "setsockopt error \n");
		exit(-1);
	}

	//Type of Socket (Unix Socket)
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, skt_file_path);

	//bind the socket to socketfd file
	if (bind(g_unx_skt_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		fprintf(stderr, "bind error \n");
		exit(-1);
	}

	//Listen on the socketfile
	if (listen(g_unx_skt_fd, CC_MAX_CLIENT_NUM) == -1) {
		fprintf(stderr,"listen error \n");
		exit(-1);
	}

	createReadyFile();

	//accept incomming connections
	addrlen = sizeof(addr);
	fprintf(stderr, "Waiting for connections .... \n");

	get_time(&etime);
	print_timediff(btime, etime);

	while (1) {
		//Clear Socket set
		FD_ZERO(&readfds);

		//Add Master server socket to the SET
		FD_SET(g_unx_skt_fd, &readfds);

		max_sd = g_unx_skt_fd;

		//Add child sockets to SET
		for (idx = 0; idx < CC_MAX_CLIENT_NUM; ++idx) {
			//socket descriptor
			sd = client_socket[idx].sd;

			//if valid socket descriptor then add to read list
			if (sd > 0) {
				FD_SET(sd, &readfds);
			}

			//highest file descriptor number, need it for the select function
			if (sd > max_sd) {
				max_sd = sd;
			}
		}

		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
		activity = select(max_sd + 1 , &readfds , NULL , NULL , NULL);

		if ((activity < 0) && (errno != EINTR)) {
			fprintf(stderr, "select() error\n");
		}

		//If something happens on Master Socket then its incomming connection
		/* If we are serving CC_MAX_CLIENT_NUM of client, DO NOT accept client connection. */
		if (FD_ISSET(g_unx_skt_fd, &readfds) && count_active_client() < CC_MAX_CLIENT_NUM) {
			if ((new_socket = accept(g_unx_skt_fd, (struct sockaddr *)&addr, (socklen_t*)&addrlen)) < 0) {
				fprintf(stderr, "Accept Error ..\n");
				exit(-1);
			}

			//set sockopts for time out
			if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0) {
				fprintf (stderr, "setsockopt error \n");
				exit(-1);
			}

			//debuf print socket number - used in send and receive commands
			fprintf(stderr, "New connection , socket fd is %d\n" , new_socket);

			//Send a greeting message to the client and ask for client ID
		//	ret = send(new_socket, CC_REPLY_CONN, strlen(CC_REPLY_CONN), 0)

			//add new socket to array of sockets
			for (idx = 0; idx < CC_MAX_CLIENT_NUM; ++idx) {
				client = &client_socket[idx];

				//if position is empty
				if(client->sd == 0) {
					client->sd    = new_socket;
					client->state = CC_CLIENT_STATE_CONNECTING;

					fprintf(stderr, "Adding to list of sockets as %d\n" , idx);
					break;
				}
			}
		}

		// Else its some IO operation on some other socket
		for (idx = 0; idx < CC_MAX_CLIENT_NUM; idx++) {
			client = &client_socket[idx];

			sd = client->sd;

			if (FD_ISSET(sd, &readfds)) {
				// accumulate request data to client specific buffer
				valread = read(sd, client->request_buf + client->request_size,
				               CC_JSON_STR_BUF_SIZE - client->request_size);
				if (valread == 0) {
					//Somebody disconnected , get his details and print
					getpeername(sd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);

					fprintf(stderr, "Host disconnected\n" );

					//Close the socket and mark as 0 in list for reuse
					close(sd);

					client->sd    = 0;
					client->state = CC_CLIENT_STATE_NONE;
					client->cid   = CC_CLIENT_ID_INVALID;
					client->request_size = 0;
					json_tokener_reset(client->tokener);

					continue;  /* handle next client */
				} else if (valread > 0) {
					client->request_size += valread;
				}

				/* some request is NOT JSON document but certain command structure?! */
				bool is_msg_command = false;
				switch (client->cid) {
				case CC_CLIENT_ID_AV_MAIN:
				case CC_CLIENT_ID_EVT_MAIN:
				case CC_CLIENT_ID_APP:
					if (client->state == CC_CLIENT_STATE_CONNECTED) {
						is_msg_command = true;
					}
					break;
				}

				if (is_msg_command) {
					char *cursor = client->request_buf;
					while (client->request_size >= sizeof(AGTX_MSG_HEADER_S)) {
						handle_module_cmd((AGTX_MSG_HEADER_S *)cursor, sd);
						cursor += sizeof(AGTX_MSG_HEADER_S);
						client->request_size -= sizeof(AGTX_MSG_HEADER_S);
					}
					if (client->request_size) {
						memmove(client->request_buf, cursor, client->request_size);
					}
					continue;  /* handle next client */
				}
				
				/* extract every request in buffer */
				char *anchor = client->request_buf;
				while (client->request_size > 0) {
					json_obj = json_tokener_parse_ex(client->tokener, anchor, client->request_size);
					enum json_tokener_error json_err = json_tokener_get_error(client->tokener);
					if (json_err == json_tokener_continue) {
						json_tokener_reset(client->tokener);
						break;  /* request is NOT complete yet */
					} else if (json_err != json_tokener_success) {
						fprintf(stderr, "Received malformed request(%s), drop it(%d bytes)!\n",
						        json_tokener_error_desc(json_err), client->request_size);
						/* we have no idea how much bytes processed by tokener, so drop all in buffer */
						client->request_size = 0;
						json_tokener_reset(client->tokener);
						break;
					}
					/* complete request extracted */
					size_t frame_size = client->tokener->char_offset;
					anchor += frame_size;
					client->request_size -= frame_size;

					info.master_id = CC_SESSION_ID_INVALID;
					info.cmd_id    = CC_CMD_ID_INVALID;
					info.cmd_type  = AGTX_CMD_TYPE_NONE;

					ret_buf[0] = 0;
					get_msg_general_info(json_obj, &info);
					if (client->state == CC_CLIENT_STATE_CONNECTING) {
						if (info.cmd_id == AGTX_CMD_REG_CLIENT) {
							get_client_identity(json_obj, &cid);

							if (cid != CC_CLIENT_ID_INVALID) {
								client->state = CC_CLIENT_STATE_CONNECTED;
								client->cid   = cid;

								fprintf(stderr, "Client %d connected\n", cid);
								reply_cmd_reg_client(ret_buf, &info);
							} else {
								fprintf(stderr, "Invalid client name\n");
								reply_exception_case(ret_buf, &info, AGTX_ERR_UNDEF_CLIENT_ID);
							}
						} else {
							fprintf(stderr, "Please register first\n");
							reply_exception_case(ret_buf, &info, AGTX_ERR_NOT_PERM_OPT);
						}
					} else if (client->state == CC_CLIENT_STATE_CONNECTED) {
						fprintf(stderr, "Receive message from client ID %d\n", client->cid);
						ret = validate_cmd_id(info.cmd_id);
						if (!ret) {
							handle_host_cmd(ret_buf, json_obj, &info);
						} else {
							reply_exception_case(ret_buf, &info, AGTX_ERR_INVALID_CMD_ID);
						}
					} else {
						/* Undefined client state */
						reply_exception_case(ret_buf, &info, AGTX_ERR_UNDEF_STATE);
					}

					size_t response_size = strlen(ret_buf);
					if (response_size > 0) {
						if (writen(sd, ret_buf, response_size) < 0) {
							fprintf(stderr, "FAIL to send response to client(id=%d), err=%d.\n",
								client->cid, errno);
						}
					}
					json_object_put(json_obj);
				}
				if (anchor > client->request_buf && client->request_size > 0) {
					memmove(client->request_buf, anchor, client->request_size);
				}
			}
		}
	}

	removeReadyFile();

	freeUnxSkt();

	unlink(skt_file_path);

	removeSingleInstance();

	return 0;
}

