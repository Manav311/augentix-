#include "mpi_sys.h"
#include "mpi_dip_alg.h"
#include "mpi_dev.h"
#include "mpi_index.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <sys/un.h>

#include "action.h"
#include "agtx_cmd.h"
#include "cc_data.h"
#include "utils.h"
#include "json.h"
#include "stream.h"
#include "ccclient.h"
#include "agtx_types.h"

static APP_CFG_S g_app_cfgs;
static MPI_ISP_VAR_S g_var[MAX_FOCUS_ROI_NUM];

static void sigHandler(int sigNum)
{
	ERR("recv Signal Intr: %d \n", sigNum);
	MPI_SYS_exit();
	exit(1);
}

void totalVar(MPI_ISP_VAR_S *var, int cnt, int *array)
{
	int i;
	for (i = 0; i < cnt; i++) {
		array[1 + i] = (var[i].ver_sum + var[i].hor_sum);
		//DBG_MED( "[%d] = %8d, %8d; array[%d] %d", i, var[i].ver_sum, var[i].hor_sum, i+1,array[1 + i]);
	}
	//DBG_MED( "\n");
}

int runMpiSys(char *type, int clientfd)
{
	int ret = ACT_SUCCESS;
	if (strstr(type, "FOCUS") == type) {
		ret = runFocusMpiSys(type, clientfd);
	} else if (strstr(type, "panorama_conf") != NULL) {
		ret = runPanoramaMpiSys(type, clientfd);
	} else {
		char *jStr = type; // {"type": "FOCUS", "data": '[ctrl,0,0,0,0,0,0]'}
		char *type_mode = unicorn_json_get_string(jStr, "type", strlen(jStr));
		if (type_mode == NULL) {
			ERR("Error: Unable to get type, %s\n", jStr);
			ret = ACT_FAILURE;
		} else if (strstr(type_mode, "FOCUS") == type_mode) {
			if (g_app_cfgs.roi_cnt == 0) {
				char jstr_cmd[JSON_STR_LEN] = { '\0' };
				strcpy(jstr_cmd, "{\"module\": \"video_layout_conf\"}\0");
				g_app_cfgs.roi_cnt = ccClientGet(jstr_cmd, CC_GET_WIN_NUM);
			}
			ret = runMultiRoiFocusMpiSys(jStr, clientfd);
		} else {
			DBG_MED("[UNICORN] Error!! There are no this type %s \n", type_mode);
		}
		if (type_mode) {
			free(type_mode);
		}
	}
	return ret;
}

int runMultiRoiFocusMpiSys(char *jStr, int clientfd)
{
	signal(SIGTERM, sigHandler);
	signal(SIGQUIT, sigHandler);
	signal(SIGKILL, sigHandler);
	signal(SIGHUP, sigHandler);
	signal(SIGINT, sigHandler);

	/* parse config */
	int i;
	int ctrl;
	int d_idx = 0, c_idx = 0, w_idx = 0;
	char *tmp;
	char string[256] = { '\0' };
	tmp = unicorn_json_get_string(jStr, "data", strlen(jStr));
	if (tmp == NULL) {
		ERR("Error: Unable to get data, %s\n", jStr);
		return ACT_FAILURE;
	}
	sprintf(string, "%s", tmp);
	if (tmp) {
		free(tmp);
	}

	DBG_MED("=== string %s ===\n", string);
	int input_array[16] = { 0 };
	parseIntArray(string, input_array);
	int length = sizeof(input_array) / sizeof(input_array[0]);
	ctrl = input_array[0];

	/* select mode */
	switch (ctrl) {
	case ACTION_GET_ROI:
		if (length == 4) {
			DBG_MED("=== ACTION_GET_ROI length not equal 4 ===\n");
			return ACT_FAILURE;
		}

		d_idx = input_array[1];
		c_idx = input_array[2];
		w_idx = input_array[3];

		g_app_cfgs.win_idx = MPI_VIDEO_WIN(d_idx, c_idx, w_idx);
		g_app_cfgs.cfg_idx = 0;

		MPI_DEV_getIspVarCfg(g_app_cfgs.win_idx, g_app_cfgs.cfg_idx, &g_app_cfgs.var_cfg);

		DBG_MED("=== MPI_DEV_getIspVarCfg ===\n");
		DBG_MED("win_idx(%d, %d, %d), sx = %d, sy = %d, ex = %d, ey = %d\n", g_app_cfgs.win_idx.dev,
		        g_app_cfgs.win_idx.chn, g_app_cfgs.win_idx.win, g_app_cfgs.var_cfg.roi.sx,
		        g_app_cfgs.var_cfg.roi.sy, g_app_cfgs.var_cfg.roi.ex, g_app_cfgs.var_cfg.roi.ey);

		break;
	case ACTION_SET_ROI:
		if (length == 8) {
			DBG_MED("=== ACTION_SET_ROI length not equal 8 ===\n");
			return ACT_FAILURE;
		}
		d_idx = input_array[1];
		c_idx = input_array[2];
		w_idx = input_array[3];

		g_app_cfgs.win_idx = MPI_VIDEO_WIN(d_idx, c_idx, w_idx);
		g_app_cfgs.cfg_idx = 0;
		g_app_cfgs.var_cfg.roi.sx = input_array[4];
		g_app_cfgs.var_cfg.roi.sy = input_array[5];
		g_app_cfgs.var_cfg.roi.ex = input_array[6];
		g_app_cfgs.var_cfg.roi.ey = input_array[7];

		MPI_DEV_addIspVarCfg(g_app_cfgs.win_idx, &g_app_cfgs.var_cfg, &g_app_cfgs.cfg_idx);

		DBG_MED("=== MPI_DEV_addIspVarCfg ===\n");
		DBG_MED("win_idx(%d, %d, %d), sx = %d, sy = %d, ex = %d, ey = %d\n", g_app_cfgs.win_idx.dev,
		        g_app_cfgs.win_idx.chn, g_app_cfgs.win_idx.win, g_app_cfgs.var_cfg.roi.sx,
		        g_app_cfgs.var_cfg.roi.sy, g_app_cfgs.var_cfg.roi.ex, g_app_cfgs.var_cfg.roi.ey);

		break;
	case ACTION_QUERY_STATUS:
		if (length == 1) {
			DBG_MED("=== ACTION_QUERY_STATUS length not equal 1 ===\n");
			return ACT_FAILURE;
		}

		for (i = 0; i < g_app_cfgs.roi_cnt; i++) {
			MPI_DEV_getIspVar(MPI_VIDEO_WIN(0, 0, i), 0, &g_var[i]);
		}

		int total[MAX_FOCUS_ROI_NUM] = { 0 };
		total[0] = g_app_cfgs.roi_cnt;
		totalVar(&g_var[0], g_app_cfgs.roi_cnt, total);

		/* structure to string */
		char ret_value[MAX_DATA_SIZE_BYTES] = { '\0' };
		char value[16];
		sprintf(ret_value, "[%d", total[0]);
		for (i = 1; i < g_app_cfgs.roi_cnt + 1; i++) {
			value[0] = '\0';
			sprintf(value, ",%d", total[i]);
			strcat(ret_value, value);
		}
		strcat(ret_value, "]\0");
		DBG_MED("=== ret_value %s ===\n", ret_value);
		sendData(clientfd, ret_value, MAX_DATA_SIZE_BYTES);

		//usleep(1000 * 1000);

		break;
	default:
		abort();
	}

	return ACT_SUCCESS;
}

int runFocusMpiSys(char *type, int clientfd)
{
	int ret = ACT_SUCCESS;

	MPI_DIP_STAT_S stat;
	MPI_PATH idx = { { .dev = 0, .path = 0, .dummy1 = 0, .dummy0 = 0 } };

	ret = MPI_getStatistics(idx, &stat);
	if (ret != MPI_SUCCESS) {
		DBG_HIGH("Failed to MPI_getStatistics!");
		exitMpiSys();
		return ACT_FAILURE;
	}

	/* structure to string */
	char ret_value[MAX_FILE_SIZE_BYTES] = { '\0' };
	if (strcmp(type, "FOCUS") == 0) {
		sprintf(ret_value, "%d", stat.focus_stat.hor_var_sum);
		sendData(clientfd, ret_value, MAX_FILE_SIZE_BYTES);

		sprintf(ret_value, "%d", stat.focus_stat.ver_var_sum);
		sendData(clientfd, ret_value, MAX_FILE_SIZE_BYTES);
	} else {
		DBG_HIGH("Error!! DIP statistics is no %s type\n", type);
		return ACT_FAILURE;
	}

	return ACT_SUCCESS;
}

int runPanoramaMpiSys(char *jstr, int clientfd)
{
	int ret = ACT_SUCCESS;
	char *ctrl = NULL;
	char string[MAX_DATA_SIZE_BYTES] = { '\0' };

	MPI_PANORAMA_ATTR_S attr;
	UINT32 dev_idx = 0;
	UINT32 chn_idx = 0;
	UINT32 win_idx = 0;
	MPI_WIN idx = MPI_VIDEO_WIN(dev_idx, chn_idx, win_idx);

	ret = MPI_DEV_getPanoramaAttr(idx, &attr);
	if (ret != MPI_SUCCESS) {
		DBG_HIGH("Failed to MPI_DEV_getPanoramaAttr!");
		exitMpiSys();
		return ACT_FAILURE;
	}

	ctrl = unicorn_json_get_string(jstr, "ctrl", strlen(jstr));
	if (ctrl == NULL) {
		if (strstr(jstr, "center_offset_x") != NULL) {
			attr.center_offset.x = unicorn_json_get_int(jstr, "center_offset_x", strlen(jstr));
		} else if (strstr(jstr, "center_offset_y") != NULL) {
			attr.center_offset.y = unicorn_json_get_int(jstr, "center_offset_y", strlen(jstr));
		} else if (strstr(jstr, "radius") != NULL) {
			attr.radius = unicorn_json_get_int(jstr, "radius", strlen(jstr));
		}
		DBG_MED("**************** x %d, y %d , radius %d\n", attr.center_offset.x, attr.center_offset.y,
		        attr.radius);

		ret = MPI_DEV_setPanoramaAttr(idx, &attr);
		if (ret != MPI_SUCCESS) {
			DBG_HIGH("Failed to runPanoramaMpiSys!");
			exitMpiSys();
			return ACT_FAILURE;
		}
	} else {
		if (strstr(ctrl, "GET") != NULL) {
			sprintf(string, "[%d,%d,%d]", attr.center_offset.x, attr.center_offset.y, attr.radius);
			DBG_MED("=== string %s ===\n", string);
			sendData(clientfd, string, MAX_DATA_SIZE_BYTES);
		}
	}

	if (ctrl) {
		free(ctrl);
	}
	return ACT_SUCCESS;
}


int initMpiSys(void)
{
	if (MPI_SYS_init() != MPI_SUCCESS) {
		ERR("Failed to MPI_SYS_init!");
		return ACT_FAILURE;
	}

	return ACT_SUCCESS;
}

int exitMpiSys(void)
{
	MPI_SYS_exit();

	return ACT_SUCCESS;
}

int setMpiSetting(char *jstr, int len, int cmd_id, int clientfd)
{
	AGTX_UNUSED(len);

	MPI_WRITE_S write_func;
	MPI_READ_S read_func;
	PARSE_FUNC_S parse_func;
	COMP_FUNC_S comp_func;
	char buf[JSON_STR_LEN];
	int ret = 0;
	struct json_object *jobj;
	int pathIndex = unicorn_json_get_int(jstr, "video_path_idx", strlen(jstr));
	MPI_PATH path_idx = MPI_INPUT_PATH(0, pathIndex < 0 ? 0 : pathIndex);

	ret = getMpiFunc(&read_func, &write_func, cmd_id);
	if (ret) {
		return ACT_FAILURE;
	}

	ret = determine_func(&parse_func, &comp_func, cmd_id);
	if (ret) {
		return ACT_FAILURE;
	}

	/* FIXME: dose not support different win_idx for setWinNr and setWinShp */
	jobj = json_tokener_parse(jstr);
	if (!jobj) {
		return ACT_FAILURE;
	}

	if (AGTX_CMD_ITEM(cmd_id) == AGTX_ITEM_VIDEO_DIP_PCA_TABLE) {
		int fSize = unicorn_json_get_int(jstr, "current_table", strlen(jstr));
		int recvDataSize = fSize / 2;
		int16_t *recvData = (int16_t *)malloc(recvDataSize * sizeof(int16_t));
		if (!recvData) {
			goto err;
		}
		json_object_object_del(jobj, "current_table");
		ret = recvBinaryData(clientfd, fSize, recvData);
		if (ret < 0) {
			DBG_MED("fail to get table\n");
			free(recvData);
			goto err;
		}

		struct json_object *tmp_obj = json_object_new_array();
		if (!tmp_obj) {
			free(recvData);
			goto err;
		}
		for (int i = 0; i < recvDataSize; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(recvData[i]));
		}
		json_object_object_add(jobj, "current_table", tmp_obj);
		free(recvData);
	}

	ret = read_func(path_idx.value, buf);
	if (ret) {
		goto err;
	}
	parse_func(buf, jobj);
	ret = write_func(path_idx.value, buf);
	if (ret) {
		goto err;
	}
	json_object_put(jobj);

	return ACT_SUCCESS;

err:
	json_object_put(jobj);

	return ACT_FAILURE;
}

int getMpiSetting(char *params, char **jstr, int cmd_id, char **table)
{
	MPI_WRITE_S write_func;
	MPI_READ_S read_func;
	PARSE_FUNC_S parse_func;
	COMP_FUNC_S comp_func;
	char buf[JSON_STR_LEN];
	int ret = 0;
	struct json_object *jobj;
	int request_path_idx = 0;
	if (params == NULL) {
		ERR("Invalid argument.\n");
		return 0;
	}
	request_path_idx = unicorn_json_get_int(params, "video_path_idx", strlen(params));
	MPI_PATH path_idx = MPI_INPUT_PATH(0, request_path_idx < 0 ? 0 : request_path_idx);

	ret = getMpiFunc(&read_func, &write_func, cmd_id);
	if (ret) {
		return ACT_FAILURE;
	}

	ret = determine_func(&parse_func, &comp_func, cmd_id);
	if (ret) {
		return ACT_FAILURE;
	}

	/* FIXME: dose not support different win_idx for setWinNr and setWinShp */
	jobj = json_object_new_object();
	ret = read_func(path_idx.value, buf);
	if (ret) {
		goto err;
	}

	comp_func(jobj, buf);

	if (AGTX_CMD_ITEM(cmd_id) == AGTX_ITEM_VIDEO_DIP_PCA_TABLE) {
		struct json_object *tmp_obj;

		if (json_object_object_get_ex(jobj, "current_table", &tmp_obj)) {
			int length = json_object_array_length(tmp_obj), count = 0;
			*table = (char *)malloc(length * sizeof(int16_t) + 1);
			if (table == NULL) {
				goto err;
			}
			for (int i = 0; i < length; i++) {
				struct json_object *tmp_obj_n = json_object_array_get_idx(tmp_obj, i);
				int tmp = json_object_get_int(tmp_obj_n);
				(*table)[count++] = (tmp & 0x00FF);
				(*table)[count++] = (tmp & 0xFF00) >> 8;
				// DBG_MED("set %d, value = %d, table = %02x, %02x\n", count, tmp, (*table)[count - 2], (*table)[count - 1]);
				json_object_put(tmp_obj_n);
			}
			json_object_object_del(jobj, "current_table");
			tmp_obj = json_object_new_int(length * sizeof(int16_t));
			json_object_object_add(jobj, "current_table", tmp_obj);
		}
	}
	const char *target = json_object_get_string(jobj);
	long target_length = strlen(target);
	*jstr = malloc(target_length + 1);
	ret = snprintf(*jstr, target_length + 1, "%s", target);
	json_object_put(jobj);
	return ret;

err:
	json_object_put(jobj);
	return ACT_FAILURE;
}
