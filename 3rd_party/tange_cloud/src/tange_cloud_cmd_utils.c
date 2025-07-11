/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <pthread.h>

#include "tange_cloud_cmd_utils.h"
#include "json.h"
#include "agtx_cmd.h"

/**
 * Definition
*/
#define JSON_STR_LEN 8192
#define auxCmdTransSocketFile "/tmp/ccUnxSkt"

/**
 * Variables
*/
int g_cmd_trans_fd = 0;
TGC_AGTX_CONF_S g_TGC_agtx_conf;
#define TGC_GET_AGTX_CONF() (&g_TGC_agtx_conf)

/**
 * Static Variables
*/
static pthread_mutex_t g_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_master_id = 0;

/**
 * Static Function Prototype
*/
static int TGC_parseCcConfig(int cmdId, void *data, struct json_object *json_obj);
static int TGC_compCcConfig(int cmdId, void *data, struct json_object *json_obj);
static int TGC_jsonValidation(char *buffer, int strLen);
static int TGC_jsonGetInt(char *buffer, char *dKey, int strLen);
static void TGC_getCcReply(char *text);
static int TGC_getCcReturn(char *str);
static int TGC_collectAgtxConf(void);

/**
 * Functions
*/
int TGC_getConf(TGC_AGTX_CONF_S **conf)
{
	*conf = TGC_GET_AGTX_CONF();
	return 0;
}

int TGC_initCcClient(void)
{
	int servlen = 0;
	struct sockaddr_un serv_addr = { 0 };
	int idx = 1, errNo = 0;
	struct timeval tv = { .tv_sec = 10, .tv_usec = 0 };
	char buffer[JSON_STR_LEN] = { 0 };
	g_master_id = 0;

	// bzero : ereas n bytes by zeros in the memory.
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, auxCmdTransSocketFile);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	if ((g_cmd_trans_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		PR_ERR("Failed to creating socket with CmdTranslator.\n");
	}

	//Set sockopts for time out
	setsockopt(g_cmd_trans_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));

	errNo = connect(g_cmd_trans_fd, (struct sockaddr *)&serv_addr, servlen);
	if (errNo < 0) {
		PR_ERR("Error: Connecting to ccserver...\n");
		close(g_cmd_trans_fd);
		goto fail_conn;
	} else { //send Greetings Message to server in jSON
		int retry_cnt = 3; /* try three times to initialize */
		PR_NOTICE("Connected to cc skt successfully.\n");

		while (idx) {
			// step 1
			if (retry_cnt <= 0) {
				goto fail_conn;
			}

			char *cmd_start1 = "{'master_id':0, 'cmd_id':1048577, 'cmd_type':'ctrl', 'name':'APP_HOST'}";
			bzero(buffer, JSON_STR_LEN);

			if (write(g_cmd_trans_fd, cmd_start1, strlen(cmd_start1)) < 0) {
				PR_ERR("Failed to send message to command Translator: %s\n", cmd_start1);
			} else {
				PR_INFO("Sent %s to CC.\n", cmd_start1);
				TGC_getCcReply(buffer);
				PR_INFO("Reply %s from CC.\n", buffer);
				errNo = TGC_jsonValidation(buffer, strlen(buffer));

				if (errNo == 0) {
					//is return is 0(success)
					if (TGC_jsonGetInt(buffer, "rval", strlen(buffer)) < 0) {
						--retry_cnt;
						continue;
					}
				}
			}

			sleep(1);
			// step 2
			char *cmd_start2 = "{'master_id':0, 'cmd_id':1048578, 'cmd_type':'ctrl'}";
			bzero(buffer, JSON_STR_LEN);
			if (write(g_cmd_trans_fd, cmd_start2, strlen(cmd_start2)) < 0) {
				PR_ERR("Failed to send message to command Translator: %s \n", cmd_start2);
			} else {
				PR_INFO("Sent %s to CC.\n", cmd_start2);
				TGC_getCcReply(buffer);
				PR_INFO("Reply %s from CC.\n", buffer);
				errNo = TGC_jsonValidation(buffer, strlen(buffer));

				if (errNo == 0) {
					//check if rval = 0
					if (TGC_jsonGetInt(buffer, "rval", strlen(buffer)) < 0) {
						--retry_cnt;
						continue;
					} else {
						if (strstr(buffer, "master_id")) {
							g_master_id =
							        TGC_jsonGetInt(buffer, "master_id", strlen(buffer));
							idx = 0;
						}
					}
				}
			}
		}
	}

	if (TGC_collectAgtxConf() < 0) {
		PR_INFO("Collect config failed\n");
		goto fail_conn;
	}
	return 0;
fail_conn:
	return -1;
}

void TGC_exitCcClient(void)
{
	close(g_cmd_trans_fd);
}

int TGC_getCcConfig(int cmdId, void *data)
{
	int len = 0;
	int rval = 0;
	int jsonRetLen = 0;
	char json_buf[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	enum json_tokener_error jerr;
	struct json_object *json_obj = NULL;
	struct json_tokener *tok = NULL;

	sprintf(json_buf, "{'master_id': %d, 'cmd_id': %d ,'cmd_type': 'get'}\n", g_master_id, cmdId);

	//CC Send
	pthread_mutex_lock(&g_cmd_mutex);
	if (write(g_cmd_trans_fd, json_buf, strlen(json_buf)) < 0) {
		pthread_mutex_unlock(&g_cmd_mutex);
		PR_ERR("failed to send message to command Translator \n");
		return -1;
	} else {
		bzero(ccRetStr, JSON_STR_LEN);
	}

	//CC recv
	if (TGC_getCcReturn(ccRetStr) != 0) {
		PR_ERR("Get cc config failed no data\n");
		pthread_mutex_unlock(&g_cmd_mutex);
		return -1;
	}
	pthread_mutex_unlock(&g_cmd_mutex);

	len = strlen(ccRetStr);
	if (len >= JSON_STR_LEN) {
		PR_ERR("command Id %d data size %d is larger than buffer size %d. \n", cmdId, len, JSON_STR_LEN);
		return -1;
	}

	tok = json_tokener_new();
	if (len > 0) {
		bzero(json_buf, JSON_STR_LEN);
		strcpy(json_buf, (char *)ccRetStr);
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_success) {
		if (TGC_parseCcConfig(cmdId, data, json_obj) < 0) {
			// Need to do something?
		}

		if (json_obj != NULL) {
			json_object_put(json_obj); //Decrement the ref count and free if it reaches zero
		}

		//cc check return val to process furthe
		rval = TGC_jsonGetInt(ccRetStr, "rval", strlen(ccRetStr));
		if (rval < 0) {
			PR_ERR(" CC response: FAIL  Return = %d \n", rval);
			jsonRetLen = -1;
		} else { // cc return success
			jsonRetLen = strlen(ccRetStr);
		}
	} else {
		PR_ERR(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		PR_ERR(" CC response: Invalid Json Str : rval = %d \n", rval);
		jsonRetLen = -1;
	}

	json_tokener_free(tok);

	return jsonRetLen;
}

int TGC_setCcConfig(int cmdId, void *data, int *cc_ret)
{
	char jsonCmd[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	struct json_object *json_obj = NULL;
	struct json_object *tmp_obj = NULL;
	int errNo;

	json_obj = json_object_new_object();
	if (!json_obj) {
		PR_ERR("Cannot create object\n");
		return -1;
	}

	if (TGC_compCcConfig(cmdId, data, json_obj) < 0) {
		// Need to do something?
	}

	tmp_obj = json_object_new_int(cmdId);
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_id", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "cmd_id");
	}

	tmp_obj = json_object_new_int(g_master_id);
	if (tmp_obj) {
		json_object_object_add(json_obj, "master_id", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "master_id");
	}

	tmp_obj = json_object_new_string("set");
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_type", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "cmd_type");
	}

	sprintf(jsonCmd, "%s", json_object_to_json_string(json_obj));
	if (json_obj) {
		json_object_put(json_obj);
	}

	pthread_mutex_lock(&g_cmd_mutex);
	if (write(g_cmd_trans_fd, jsonCmd, strlen(jsonCmd)) < 0) {
		PR_ERR("Failed to send message to command Translator \n");
	}

	if (TGC_getCcReturn(ccRetStr) != 0) {
		pthread_mutex_unlock(&g_cmd_mutex);
		PR_ERR("ccRetStr %s\n", ccRetStr);
		return -1;
	}
	pthread_mutex_unlock(&g_cmd_mutex);

	if (cc_ret != NULL) {
		json_obj = json_object_new_object();
		errNo = TGC_jsonValidation(ccRetStr, strlen(ccRetStr));
		if (errNo == 0) {
			//is return is 0(success)
			*cc_ret = TGC_jsonGetInt(ccRetStr, "rval", strlen(ccRetStr));
		} else {
			PR_ERR("ccRetStr %s\n", ccRetStr);
		}
	}

	return 0;
}

/**
 * Static Function
*/
static int TGC_parseCcConfig(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		parse_video_strm_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VIDEO_PTZ_CONF) {
		parse_video_ptz_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_MD_CONF) {
		parse_iva_md_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_PD_CONF) {
		parse_iva_pd_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		parse_img_pref(data, json_obj);
	} else if (cmdId == AGTX_CMD_ADV_IMG_PREF) {
		parse_adv_img_pref(data, json_obj);
	} else if (cmdId == AGTX_CMD_OSD_CONF) {
		parse_osd_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_OD_CONF) {
		parse_iva_od_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_LOCAL_RECORD_CONF) {
		parse_local_record_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VOICE_CONF) {
		parse_voice_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_SIREN_CONF) {
		parse_siren_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VIDEO_LAYOUT_CONF) {
		parse_layout_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_EVT_PARAM) {
		parse_event_param(data, json_obj);
	} else if (cmdId == AGTX_CMD_ANTI_FLICKER_CONF) {
		parse_anti_flicker_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_SYS_INFO) {
		parse_sys_info(data, json_obj);
	} else if (cmdId == AGTX_CMD_COLOR_CONF) {
		parse_color_conf(data, json_obj);
	} else {
		PR_ERR("Unknown cmdId %d\n", cmdId);
		return -1;
	}

	return 0;
}

static int TGC_compCcConfig(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		comp_video_strm_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VIDEO_PTZ_CONF) {
		comp_video_ptz_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_MD_CONF) {
		comp_iva_md_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_PD_CONF) {
		comp_iva_pd_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		comp_img_pref(json_obj, data);
	} else if (cmdId == AGTX_CMD_ADV_IMG_PREF) {
		comp_adv_img_pref(json_obj, data);
	} else if (cmdId == AGTX_CMD_OSD_CONF) {
		comp_osd_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_OD_CONF) {
		comp_iva_od_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_LOCAL_RECORD_CONF) {
		comp_local_record_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VOICE_CONF) {
		comp_voice_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_SIREN_CONF) {
		comp_siren_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VIDEO_LAYOUT_CONF) {
		comp_layout_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_EVT_PARAM) {
		comp_event_param(json_obj, data);
	} else if (cmdId == AGTX_CMD_ANTI_FLICKER_CONF) {
		comp_anti_flicker_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_SYS_INFO) {
		comp_sys_info(json_obj, data);
	} else if (cmdId == AGTX_CMD_COLOR_CONF) {
		comp_color_conf(json_obj, data);
	} else {
		PR_ERR("Unknown cmdId %d\n", cmdId);
		return -1;
	}

	return 0;
}

//validate the cc read string is in Json format
static int TGC_jsonValidation(char *buffer, int strLen)
{
	char json_buf[JSON_STR_LEN] = { 0 };
	json_object *json_obj;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	int rval = 0;

	if (strLen > 0) {
		strcpy(json_buf, (char *)buffer);
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_continue) {
		json_obj = json_tokener_parse_ex(tok, " ", -1);
		if (json_tokener_get_error(tok) != json_tokener_success)
			json_obj = NULL;
	} else if (jerr != json_tokener_success) {
		PR_ERR(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		rval = -1;
	} else if (jerr == json_tokener_success) {
		rval = 0;
	}
	if (json_obj != NULL)
		json_object_put(json_obj); //Decrement the ref count and free if it reaches zero

	json_tokener_free(tok);
	return rval;
}

//JSON: Get int val for given Key
static int TGC_jsonGetInt(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN] = { 0 };
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;

	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);

	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
		} else {
			rval = -99; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	} else { //the key may not exist
		rval = -99;
	}
	if (json_obj != NULL)
		json_object_put(json_obj);

	json_tokener_free(tok);
	return rval;
}

static void TGC_getCcReply(char *text)
{
	int ret = -1;

	while (1) {
		ret = read(g_cmd_trans_fd, text, JSON_STR_LEN);

		if (ret < 0) {
			PR_INFO("ret = %d, errno = %d(%s)\n", ret, errno, strerror(errno));
			break;
		} else if (ret == 0) {
			break;
		} else {
			break;
		}
	}
}

static int TGC_getCcReturn(char *str)
{
	int ret = -1;
	/* retry 2 times */
	int cnt = 2;

	while (cnt--) {
		ret = read(g_cmd_trans_fd, str, JSON_STR_LEN);

		if (ret < 0) {
			PR_INFO("ret = %d, errno = %d(%s)\n", ret, errno, strerror(errno));
			continue;
		} else if (ret == 0) {
			continue;
		} else {
			return 0;
		}
	}

	return -1;
}

static int TGC_collectAgtxConf(void)
{
	int i = 0;
	int cmd_num = 0;
	TGC_AGTX_CONF_S *conf = NULL;
	conf = TGC_GET_AGTX_CONF();

	AGTX_CMD_DATA_S cmd[] = {
		{ AGTX_CMD_VIDEO_STRM_CONF, &conf->strm.data, &conf->strm.update },
	};
	cmd_num = sizeof(cmd) / sizeof(AGTX_CMD_DATA_S);
	for (i = 0; i < cmd_num; i++) {
		if (TGC_getCcConfig(cmd[i].cmd, cmd[i].data) < 0) {
			PR_ERR("num %d cmd %d get config\n", i, cmd[i].cmd);
			return -1;
		}
		*cmd[i].update = 1;
	}

	PR_INFO("Tange Cloud collect config success\n");
	return 0;
}
