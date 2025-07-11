#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>

#include "mpi_sys.h"
#include "mpi_dip_alg.h"

#include "dn_alias_util.h"

#include "agtx_cmd.h"
#include "agtx_types.h"
#include "agtx_common.h"

#include "agtx_adv_img_pref.h"
#include "agtx_color_conf.h"
#include "json.h"

#define CC_SOCKET_PATH "/tmp/ccUnxSkt"
#define COLOR_CTRL_NAME "COLOR_CTRL"
#define WAIT_DCC_APPLY_USEC (250000)

#define DAY_NIGHT_FILE_PATH "/tmp/dn_attr.dat"
#define DAY "\"DAY\""
#define NIGHT "\"NIGHT\""
#define ACTIVE "active"
#define REMOVE "remove"
#define ON "on"
#define OFF "off"
#define JSON_STR_BUF_SIZE          16384

typedef enum {
	IMG_RESULT_DAY,
	IMG_RESULT_NIGHT,
} IMG_RESULT_E;

typedef enum {
	ICR_RESULT_ACTIVE,
	ICR_RESULT_REMOVE,
} ICR_RESULT_E;

typedef enum {
	LED_RESULT_OFF,
	LED_RESULT_ON,
} LED_RESULT_E;

typedef struct {
	int switch_by_light_sensor;
	IMG_RESULT_E color_conf;
	ICR_RESULT_E icr_conf;
	LED_RESULT_E led_conf;
} DN_RESULT_S;

typedef struct {
	char *cmd;
	int  pin[2];
	char *mode;
} IR_CUT_PARAMS_S;

typedef struct {
	char *cmd;
	int  pin;
	char *mode;
} IR_LED_PARAMS_S;

typedef struct day_night_attr_s {
	AGTX_ADV_IMG_PREF_S adv_img_pref;
	AGTX_INT32 icr_pin[2];
	AGTX_INT32 ir_led_pin;
} DAY_NIGHT_ATTR_S;

typedef struct {
	int choice;
	AGTX_COLOR_MODE_E color_mode;
} APP_CFG_S;

static char g_json_buf[JSON_STR_BUF_SIZE]; // or bigger array to append data to handle pkt defrag
static int  g_jbuf_size;      // val holds the string len , adds up when handle pkt defrag
static DAY_NIGHT_ATTR_S g_day_night_attr;
static APP_CFG_S g_app_cfgs;
static IR_CUT_PARAMS_S g_ir_cut_params;
static IR_LED_PARAMS_S g_ir_led_params;
static char *g_color_conf;

/* function declartion */
void ctrl_ir_cut(IR_CUT_PARAMS_S *args);
int exec_cmd(char *str);
void help(char *str);
void parse(int argc, char **argv, APP_CFG_S *cfg);
int switch_day_color_via_mpi();
int switch_night_color_via_mpi();
int try
	();
int update_color_conf(const int client_fd, const char *str);
int request_ctrl_grant(const int client_fd);
int start_client_connect(const char *socket_path);
int register_to_cent_ctrl(const int client_fd);
int connect_to_cent_ctrl(const char *cc_socket_path);
int set_color_conf_via_cc(const char *cc_socket_path, const char *color_str);
int get_day_night_attr(const char *dn_file_path, DAY_NIGHT_ATTR_S *attr);
int det_day_night_mode(const DAY_NIGHT_ATTR_S *attr, const AGTX_COLOR_MODE_E color_mode, DN_RESULT_S *result);
int get_icr_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode,ICR_RESULT_E *icr_conf);
int get_led_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode,LED_RESULT_E *led_conf);
int get_img_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode,IMG_RESULT_E *color_conf);
int check_json_string(struct json_object **json_obj, char *buf, int strlen);
int parse_return_value(struct json_object *cmd_obj);

void ctrl_ir_cut(IR_CUT_PARAMS_S *args)
{
	if ((args->pin[0] < 0) && (args->pin[1] < 0)) {
		return;
	}

	char buf[128];

	sprintf(buf, "%s %d %d %s", args->cmd, args->pin[0], args->pin[1], args->mode);
	syslog(LOG_INFO, "[%s] %s\n", __func__, buf);
	system(buf);
}

void ctrl_ir_led(IR_LED_PARAMS_S *args)
{
	if (args->pin < 0) {
		return;
	}

	char buf[128];

	sprintf(buf, "%s %d %s", args->cmd, args->pin, args->mode);
	syslog(LOG_INFO, "[%s] %s\n", __func__, buf);
	system(buf);
}

int exec_cmd(char *action_args)
{
	openlog("EXEC_CMD", LOG_PID, LOG_USER);
	int status;
	char buf[128];

	sprintf(buf, "%s", action_args);
	status = system(buf);

	if (status == -1) {
		DN_ERR("System error!\n");
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				syslog(LOG_INFO, "(%s) be excuted successfully.\n", buf);
			} else {
				DN_ERR("Run cmd fail and exit code is %d (%s)!\n", WEXITSTATUS(status), buf);
			}
		} else {
			DN_ERR("exit status is %d (%s)!\n", WEXITSTATUS(status), buf);
		}
	}

	closelog();
	return status;
}

int switch_day_color_via_mpi()
{
	int ret;

	if (MPI_SYS_init() != MPI_SUCCESS) {
		ret = -1;
		return ret;
	}

	UINT32 dev;
	UINT32 path;
	MPI_PATH path_idx;

	MPI_CAL_ATTR_S cal_attr;
	MPI_DIP_ATTR_S dip_attr;
	MPI_CSM_ATTR_S csm_attr;

	dev = 0;

	for (path = 0; path < MPI_MAX_INPUT_PATH_NUM; path++) {
		path_idx.dev = dev;
		path_idx.path = path;

		ret = MPI_getCalAttr(path_idx, &cal_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getCalAttr!");
			return ret;
		}

		cal_attr.dcc_en = 1;
		ret = MPI_setCalAttr(path_idx, &cal_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setCalAttr!");
			return ret;
		}

		/* waitting dcc apply */
		usleep(WAIT_DCC_APPLY_USEC);

		ret = MPI_getDipAttr(path_idx, &dip_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getDipAttr!");
			return ret;
		}

		dip_attr.is_awb_en = 1;

		ret = MPI_setDipAttr(path_idx, &dip_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setDipAttr!");
			return ret;
		}

		ret = MPI_getCsmAttr(path_idx, &csm_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getCsmAttr!");
			return ret;
		}

		csm_attr.bw_en = 0;

		ret = MPI_setCsmAttr(path_idx, &csm_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setCsmAttr!");
			return ret;
		}
	}

	MPI_SYS_exit();

	return ret;
}

int switch_night_color_via_mpi()
{
	int ret;

	if (MPI_SYS_init() != MPI_SUCCESS) {
		ret = -1;
		return ret;
	}

	UINT32 dev;
	UINT32 path;
	MPI_PATH path_idx;

	MPI_CSM_ATTR_S csm_attr;
	MPI_DIP_ATTR_S dip_attr;
	MPI_CAL_ATTR_S cal_attr;

	dev = 0;

	for (path = 0; path < MPI_MAX_INPUT_PATH_NUM; path++) {
		path_idx.dev = dev;
		path_idx.path = path;

		ret = MPI_getCsmAttr(path_idx, &csm_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getCsmAttr!");
			return ret;
		}

		csm_attr.bw_en = 1;

		ret = MPI_setCsmAttr(path_idx, &csm_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setCsmAttr!");
			return ret;
		}

		ret = MPI_getDipAttr(path_idx, &dip_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getDipAttr!");
			return ret;
		}

		dip_attr.is_awb_en = 0;

		ret = MPI_setDipAttr(path_idx, &dip_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setDipAttr!");
			return ret;
		}

		ret = MPI_getCalAttr(path_idx, &cal_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_getCalAttr!");
			return ret;
		}

		cal_attr.dcc_en = 0;

		ret = MPI_setCalAttr(path_idx, &cal_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Failed to MPI_setCalAttr!");
			return ret;
		}
	}

	MPI_SYS_exit();

	return ret;
}

int try
	()
	{
		int ret;
		MPI_PATH path_idx = { { .dev = 0, .path = 0, .dummy1 = 0, .dummy0 = 0 } };
		MPI_DIP_ATTR_S dip_attr;

		if (MPI_SYS_init() != MPI_SUCCESS) {
			ret = -1;
			return ret;
		}

		ret = MPI_getDipAttr(path_idx, &dip_attr);

		if (ret != MPI_SUCCESS) {
			DN_ERR("Video device is not existed!");
			return ret;
		}

		MPI_SYS_exit();

		return ret;
	}

int update_color_conf(const int client_fd, const char *str)
{
	int ret;
	int read_ret;
	int rval;
	char buf[128] = { 0 };
	char update_cmd_buf[128] = { 0 };
	struct json_object *json_obj = NULL;

	ret = 0;
	read_ret = 0;
	rval = -1;

	/* TO-DO cmd_id*/
	sprintf(update_cmd_buf, "{\"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"set\", \"color_mode\": %s}",
	        AGTX_CMD_COLOR_CONF, str);

	/* Update color config */
	if (write(client_fd, &update_cmd_buf, strlen(update_cmd_buf)) < 0) {
		DN_ERR("Failed to update color config to CC!\n");
		ret = -1;
	}

	while (!ret) {
		read_ret = read(client_fd, buf, sizeof(buf));
		if (read_ret != (int)strlen(buf)) {
			DN_ERR("Failed to read socket!\n");
			continue;
		}

		ret = check_json_string(&json_obj, buf, strlen(buf));

		if (ret >= 0) {
			rval = parse_return_value(json_obj);
			json_object_put(json_obj);
		}

		if (rval == 0) {
			syslog(LOG_INFO, "Return ACK from CC after updating color config.\n");
			ret = 0;
			break;
		} else {
			sleep(1);
			DN_NOTICE("Wating CC return ACK!\n");
			DN_NOTICE("%s\n", buf);
			continue;
		}
	}

	return ret;
}

int main(int argc, char *argv[])
{
	openlog("switch_day_night_mode", LOG_PID, LOG_USER);

	DN_RESULT_S dn_result;

	/* parse config */
	parse(argc, argv, &g_app_cfgs);

	/* select mode */
	switch (g_app_cfgs.choice) {
	case TRY:

		if (try () == -1) {
			DN_ERR("Failed to try. Video device is not existed!");
			goto err;
		}

		syslog(LOG_INFO, "Video device is existed!\n");

		break;
	case DAY_COLOR_SETTING_VIA_MPI:

		if (switch_day_color_via_mpi() == -1) {
			DN_ERR("Failed to switch day color setting via mpi!");
			goto err;
		}

		syslog(LOG_INFO, "Switch to day color setting via mpi!\n");

		break;
	case NIGHT_COLOR_SETTING_VIA_MPI:

		if (switch_night_color_via_mpi() == -1) {
			DN_ERR("Failed to switch night color setting via mpi!\n");
			goto err;
		}

		syslog(LOG_INFO, "Switch to night color setting via mpi!\n");

		break;
	case DAY_COLOR_SETTING_VIA_CC:

		if (set_color_conf_via_cc(CC_SOCKET_PATH, DAY) == -1) {
			DN_ERR("Failed to switch day color setting via cc!\n");
			goto err;
		}

		syslog(LOG_INFO, "Switch to day color setting via cc!\n");

		break;
	case NIGHT_COLOR_SETTING_VIA_CC:

		if (set_color_conf_via_cc(CC_SOCKET_PATH, NIGHT) == -1) {
			DN_ERR("Failed to switch day color setting via cc!\n");
			goto err;
		}

		syslog(LOG_INFO, "Switch to night color setting via cc!\n");

		break;
	case READ_DAY_NIGHT_ATTR:

		if (get_day_night_attr(DAY_NIGHT_FILE_PATH, &g_day_night_attr) == -1) {
			DN_ERR("Failed to get day_night_attr!\n");
			goto err;
		}

		g_ir_cut_params.cmd = CTRL_IR_CUT_CMD;
		g_ir_cut_params.pin[0] = g_day_night_attr.icr_pin[0];
		g_ir_cut_params.pin[1] = g_day_night_attr.icr_pin[1];

		g_ir_led_params.cmd = CTRL_IR_LED_CMD;
		g_ir_led_params.pin = g_day_night_attr.ir_led_pin;

		det_day_night_mode(&g_day_night_attr, g_app_cfgs.color_mode, &dn_result);

		if (dn_result.switch_by_light_sensor == 0) {
			return 0;
		} else if (dn_result.switch_by_light_sensor == 1) {
			g_color_conf = (dn_result.color_conf == IMG_RESULT_DAY ? DAY : NIGHT);
			g_ir_cut_params.mode = (dn_result.icr_conf == ICR_RESULT_ACTIVE ? ACTIVE : REMOVE);
			g_ir_led_params.mode = (dn_result.led_conf == LED_RESULT_ON ? ON : OFF);
		} else {
			DN_ERR("Impossible dn_result.switch_by_light_sensor = %d!\n", dn_result.switch_by_light_sensor);
			goto err;
		}

		/* Handle execute order icr_conf and color_conf  */
		if (strcmp(g_color_conf, DAY) == 0) {
			ctrl_ir_led(&g_ir_led_params);
			ctrl_ir_cut(&g_ir_cut_params);

			if (set_color_conf_via_cc(CC_SOCKET_PATH, g_color_conf) == -1) {
				DN_ERR("Failed to send color config to cc!\n");
				goto err;
			}

		} else if (strcmp(g_color_conf, NIGHT) == 0) {
			if (set_color_conf_via_cc(CC_SOCKET_PATH, g_color_conf) == -1) {
				DN_ERR("Failed to send color config to cc!\n");
				goto err;
			}

			ctrl_ir_cut(&g_ir_cut_params);

			usleep(500000);

			ctrl_ir_led(&g_ir_led_params);
		} else {
			DN_ERR("Impossible case under handling icr_conf and color_conf!\n");
		}

		syslog(LOG_INFO, "#--- <%s> ICR, <%s> color!\n", g_color_conf, g_ir_cut_params.mode);

		break;
	case DIRECT_SWITCH_BY_READ_DAY_NIGHT_ATTR:
		if (get_day_night_attr(DAY_NIGHT_FILE_PATH, &g_day_night_attr) == -1) {
			DN_ERR("Failed to get day_night_attr!\n");
			goto err;
		}

		g_ir_cut_params.cmd = CTRL_IR_CUT_CMD;
		g_ir_cut_params.pin[0] = g_day_night_attr.icr_pin[0];
		g_ir_cut_params.pin[1] = g_day_night_attr.icr_pin[1];

		g_ir_led_params.cmd = CTRL_IR_LED_CMD;
		g_ir_led_params.pin = g_day_night_attr.ir_led_pin;

		det_day_night_mode(&g_day_night_attr, g_app_cfgs.color_mode, &dn_result);

		if (dn_result.switch_by_light_sensor == 0) {
			if (g_app_cfgs.color_mode == AGTX_COLOR_MODE_DAY) {
				g_ir_led_params.mode = OFF;
				ctrl_ir_led(&g_ir_led_params);

				g_ir_cut_params.mode = ACTIVE;
				ctrl_ir_cut(&g_ir_cut_params);

				if (set_color_conf_via_cc(CC_SOCKET_PATH, DAY) == -1) {
					DN_ERR("Failed to send color config to cc!\n");
					goto err;
				}
			} else if (g_app_cfgs.color_mode == AGTX_COLOR_MODE_NIGHT) {
				g_ir_led_params.mode = ON;
				ctrl_ir_led(&g_ir_led_params);

				if (set_color_conf_via_cc(CC_SOCKET_PATH, NIGHT) == -1) {
					DN_ERR("Failed to send color config to cc!\n");
					goto err;
				}
				g_ir_cut_params.mode = REMOVE;
				ctrl_ir_cut(&g_ir_cut_params);
			} else {
				DN_ERR("Impossible g_app_cfgs.color_mode = %d!\n", g_app_cfgs.color_mode);
				goto err;
			}
		} else if (dn_result.switch_by_light_sensor == 1) {
			DN_ERR("This command only support for auto switch by time, dn_result.switch_by_light_sensor = %d!\n", dn_result.switch_by_light_sensor);
			goto err;
		} else {
			DN_ERR("Impossible dn_result.switch_by_light_sensor = %d!\n", dn_result.switch_by_light_sensor);
			goto err;
		}

		break;
	default:
		abort();
	}

	closelog();

	return 0;
err:
	return -1;
}

void help(char *str)
{
	printf("USAGE:\n");
	printf("\t%s [OPTION]\n", str);
	printf("\n");
	printf("EXAMPLE:\n");
	printf("\t%s -c1\n", str);
	printf("\n");
	printf("OPTION:\n");
	printf("\t'-c' select color setting\n");
	printf("\t     -c1: day color setting via cc\n");
	printf("\t     -c2: night color setting via cc\n");
	printf("\t     -c3: day color setting via mpi\n");
	printf("\t     -c4: night color setting via mpi\n");
	printf("\t'-t' try\n");
	printf("\t'-r' determine icr_conf & color_conf after parsing 'DAY_NIGHT_FILE_PATH' data \n");
	printf("\t     -r1: light_sensor_result <day>\n");
	printf("\t     -r2: light_sensor_result <night>\n");
	printf("\t'-d' switch day/night directly for auto time switching\n");
	printf("\t     -d1: color setting via cc and control ir cut <day>\n");
	printf("\t     -d2: color setting via cc and control ir cut <night>\n");
	printf("\t'-h' help\n");
	printf("\n");
}

void parse(int argc, char **argv, APP_CFG_S *config)
{
	int c;

	memset(config, 0, sizeof(APP_CFG_S));

	while ((c = getopt(argc, argv, "c:tr:d:h")) != -1) {
		switch (c) {
		case 'c':
			if (atoi(optarg) == 1) {
				config->choice = DAY_COLOR_SETTING_VIA_CC;
			} else if (atoi(optarg) == 2) {
				config->choice = NIGHT_COLOR_SETTING_VIA_CC;
			} else if (atoi(optarg) == 3) {
				config->choice = DAY_COLOR_SETTING_VIA_MPI;
			} else if (atoi(optarg) == 4) {
				config->choice = NIGHT_COLOR_SETTING_VIA_MPI;
			} else {
				printf("Wrong color setting!n");
				return;
			}
			break;
		case 't':
			config->choice = TRY;
			break;
		case 'r':
			config->choice = READ_DAY_NIGHT_ATTR;
			if (atoi(optarg) == 1) {
				config->color_mode = AGTX_COLOR_MODE_DAY;   //light_sensor_result
			} else if (atoi(optarg) == 2) {
				config->color_mode = AGTX_COLOR_MODE_NIGHT; //light_sensor_result
			} else {
				printf("Wrong color setting!n");
				return;
			}
			break;
		case 'd':
			config->choice = DIRECT_SWITCH_BY_READ_DAY_NIGHT_ATTR;
			if (atoi(optarg) == 1) {
				config->color_mode = AGTX_COLOR_MODE_DAY;   //DAY
			} else if (atoi(optarg) == 2) {
				config->color_mode = AGTX_COLOR_MODE_NIGHT; //NIGHT
			} else {
				printf("Wrong color setting!n");
				return;
			}
			break;
		case 'h':
			help(argv[0]);
			exit(1);
			break;
		case '?':
			if ((optopt == 'c') || (optopt == 't') || (optopt == 'r') || (optopt == 'd') || (optopt == 'h')) {
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			} else if (isprint(optopt)) {
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			} else {
				fprintf(stderr, "Unknow option character '\\x%x'.\n", optopt);
			}
			return;
		default:
			abort();
		}
	}

	return;
}

int request_ctrl_grant(const int client_fd)
{
	int ret;
	int read_ret;
	int rval;
	char buf[128] = { 0 };
	char ctrl_grant_buf[128] = { 0 };
	struct json_object *json_obj = NULL;

	ret = 0;
	read_ret = 0;
	rval = -1;

	sprintf(ctrl_grant_buf, "{\"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\"}", AGTX_CMD_SESS_START);

	/* Request control grant */
	if (write(client_fd, &ctrl_grant_buf, strlen(ctrl_grant_buf)) < 0) {
		DN_ERR("Failed to request control grant to CC!\n");
		ret = -1;
	}

	while (!ret) {
		read_ret = read(client_fd, buf, sizeof(buf));

		if (read_ret != (int)strlen(buf)) {
			DN_ERR("Failed to read socket!\n");
			continue;
		}

		ret = check_json_string(&json_obj, buf, strlen(buf));

		if (ret >= 0) {
			rval = parse_return_value(json_obj);
			json_object_put(json_obj);
		}

		if (rval == 0) {
			syslog(LOG_INFO, "Ready to control from %s.\n", COLOR_CTRL_NAME);
			ret = 0;
			break;
		} else {
			sleep(1);
			DN_NOTICE("Wating CC return ACK and master id!\n");
			DN_NOTICE("%s\n", buf);
			continue;
		}
	}

	return ret;
}

int start_client_connect(const char *socket_path)
{
	openlog("SW", LOG_PID, LOG_USER);
	size_t path_len;
	int fd;
	struct sockaddr_un client_un;
	int ret;

	path_len = strlen(socket_path);

	if (path_len == 0) {
		DN_ERR("Socket path can't be empty!\n");
		return -1;
	}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0) {
		DN_ERR("Socket() failure!\n");
		return -1;
	}

	memset(&client_un, 0, sizeof(client_un));
	client_un.sun_family = AF_UNIX;
	strcpy(client_un.sun_path, socket_path);

	ret = connect(fd, (struct sockaddr *)&client_un, sizeof(client_un));

	if (ret < 0) {
		DN_ERR("Connect() failure!\n");
		close(fd);
		return -1;
	}

	closelog();
	return fd;
}

int register_to_cent_ctrl(const int client_fd)
{
	int ret;
	int read_ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	ret = 0;
	read_ret = 0;

	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"%s\"}",
	        AGTX_CMD_REG_CLIENT, COLOR_CTRL_NAME);
	sprintf(ret_cmd, "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": \"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/* Send register information */
	if (write(client_fd, &reg_buf, strlen(reg_buf)) < 0) {
		DN_ERR("Failed to send register information to CC!\n");
		ret = -1;
	}

	while (!ret) {
		read_ret = read(client_fd, buf, strlen(ret_cmd));
		if (read_ret != (int)strlen(ret_cmd)) {
			DN_ERR("Failed to read socket!\n");
			continue;
		}

		if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
			sleep(1);
			DN_NOTICE("Wating CC return ACK for an valid client!\n");
			continue;
		} else {
			syslog(LOG_INFO, "Registered to CC from %s.\n", COLOR_CTRL_NAME);
			break;
		}
	}

	return ret;
}

int connect_to_cent_ctrl(const char *cc_socket_path)
{
	int client_fd;
	int ret = 0;

	client_fd = start_client_connect(cc_socket_path);

	if (client_fd == -1) {
		ret = -1;
		return ret;
	}

	ret = register_to_cent_ctrl(client_fd);

	if (ret < 0) {
		DN_ERR("Failed to register to central control!\n");
		return ret;
	}

	return client_fd;
}

int set_color_conf_via_cc(const char *cc_socket_path, const char *color_str)
{
	int client_fd;
	int ret;

	client_fd = connect_to_cent_ctrl(cc_socket_path);

	if (client_fd < 0) {
		DN_ERR("Failed to connect central control!\n");
		close(client_fd);
		return -1;
	}

	ret = request_ctrl_grant(client_fd);

	if (ret != 0) {
		DN_ERR("Failed to request control grant!\n");
		close(client_fd);
		return -1;
	}

	ret = update_color_conf(client_fd, color_str);

	if (ret != 0) {
		DN_ERR("Failed to send color comand to cc!");
		close(client_fd);
		return -1;
	}

	close(client_fd);

	return ret;
}

int get_day_night_attr(const char *dn_file_path, DAY_NIGHT_ATTR_S *attr)
{
	FILE *infile;

	/* Open file for reading */
	infile = fopen(dn_file_path, "r");
	if (infile == NULL) {
		fprintf(stderr, "\nError opening file\n");
		return -1;
	}

	/* read file contents till end of file */
	fread(attr, sizeof(DAY_NIGHT_ATTR_S), 1, infile);

	/* close file */
	fclose(infile);

	syslog(LOG_INFO, "night_mode = %d, ir_led_mode = %d, icr_mode = %d, img_mode = %d\n", attr->adv_img_pref.night_mode, attr->adv_img_pref.ir_led_mode,
	attr->adv_img_pref.icr_mode, attr->adv_img_pref.image_mode);
	syslog(LOG_INFO, "ir_led_pin = %d\n", attr->ir_led_pin);
	syslog(LOG_INFO, "icr_pin = %d, %d\n", attr->icr_pin[0], attr->icr_pin[1]);

	return 0;
}

int det_day_night_mode(const DAY_NIGHT_ATTR_S *attr, const AGTX_COLOR_MODE_E color_mode, DN_RESULT_S *result)
{
	DN_TRACE("night_mode = %d, ir_led_mode = %d, icr_mode = %d, img_mode = %d, color_mode = %d\n", attr->adv_img_pref.night_mode, attr->adv_img_pref.ir_led_mode,
	attr->adv_img_pref.icr_mode, attr->adv_img_pref.image_mode, color_mode);

	result->switch_by_light_sensor = 1;

	if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTOSWITCH) {
		result->switch_by_light_sensor = 0;
		return 0;
	}

	DN_TRACE("<before> result->led_conf = %d\n", result->led_conf);
	DN_TRACE("<before> result->icr_conf = %d\n", result->icr_conf);
	DN_TRACE("<before> result->color_conf = %d\n", result->color_conf);
#if 0
	if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTO && attr->adv_img_pref.icr_mode == AGTX_ICR_MODE_AUTO && attr->adv_img_pref.image_mode == AGTX_IMAGE_MODE_AUTO) {

		DN_TRACE("222 in\n");

		result->color_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? IMG_RESULT_DAY : IMG_RESULT_NIGHT;
		result->icr_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? ICR_RESULT_ACTIVE : ICR_RESULT_REMOVE;

	} else if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTO &&attr->adv_img_pref.icr_mode == AGTX_ICR_MODE_AUTO &&
	          attr->adv_img_pref.image_mode == AGTX_IMAGE_MODE_COLOR) {

				DN_TRACE("220 in\n");
				result->color_conf = IMG_RESULT_DAY;
				result->icr_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? ICR_RESULT_ACTIVE : ICR_RESULT_REMOVE;

	} else if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTO && attr->adv_img_pref.icr_mode == AGTX_ICR_MODE_AUTO &&
	           attr->adv_img_pref.image_mode == AGTX_IMAGE_MODE_GRAYSCALE) {

				DN_TRACE("221 in\n");
				result->color_conf = IMG_RESULT_NIGHT;
				result->icr_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? ICR_RESULT_ACTIVE : ICR_RESULT_REMOVE;

	} else if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTO && attr->adv_img_pref.icr_mode == AGTX_ICR_MODE_ON &&
	           attr->adv_img_pref.image_mode == AGTX_IMAGE_MODE_AUTO) {

				DN_TRACE("212 in\n");
				result->color_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? IMG_RESULT_DAY : IMG_RESULT_NIGHT;
				result->icr_conf = ICR_RESULT_ACTIVE;


	} else if (attr->adv_img_pref.night_mode == AGTX_NIGHT_MODE_AUTO && attr->adv_img_pref.icr_mode == AGTX_ICR_MODE_OFF &&
	           attr->adv_img_pref.image_mode == AGTX_IMAGE_MODE_AUTO) {

				DN_TRACE("202 in\n");
				result->color_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? IMG_RESULT_DAY : IMG_RESULT_NIGHT;
				result->icr_conf = ICR_RESULT_REMOVE;

	} else {

		DN_TRACE("<22> in\n");
		get_led_conf(&attr->adv_img_pref, &result->led_conf);
		get_icr_conf(&attr->adv_img_pref, &result->icr_conf);
		get_img_conf(&attr->adv_img_pref, &result->color_conf);

	}
#else
	get_led_conf(&attr->adv_img_pref, color_mode, &result->led_conf);
	get_icr_conf(&attr->adv_img_pref, color_mode, &result->icr_conf);
	get_img_conf(&attr->adv_img_pref, color_mode, &result->color_conf);
#endif
	DN_TRACE("#--- result->led_conf = %d\n", result->led_conf);
	DN_TRACE("#--- result->icr_conf = %d\n", result->icr_conf);
	DN_TRACE("#--- result->color_conf = %d\n", result->color_conf);

	return 0;
}

int get_icr_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode, ICR_RESULT_E *icr_conf)
{
	if (adv_img_pref->icr_mode == AGTX_ICR_MODE_AUTO) {
		if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_OFF) {
			*icr_conf = ICR_RESULT_ACTIVE;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_ON) {
			*icr_conf = ICR_RESULT_REMOVE;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_AUTO) {
			*icr_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? ICR_RESULT_ACTIVE : ICR_RESULT_REMOVE;
		} else {
			return -1;
		}
	} else if (adv_img_pref->icr_mode == AGTX_ICR_MODE_ON) {
		*icr_conf = ICR_RESULT_ACTIVE;
	} else if (adv_img_pref->icr_mode == AGTX_ICR_MODE_OFF) {
		*icr_conf = ICR_RESULT_REMOVE;
	} else {
		return -1;
	}

	return 0;
}

int get_led_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode,LED_RESULT_E *led_conf)
{
	if (adv_img_pref->ir_led_mode == AGTX_IR_LED_MODE_AUTO) {
		if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_OFF) {
			*led_conf = LED_RESULT_OFF;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_ON) {
			*led_conf = LED_RESULT_ON;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_AUTO) {
			*led_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? LED_RESULT_OFF : LED_RESULT_ON;
		} else {
			return -1;
		}
	} else if (adv_img_pref->ir_led_mode == AGTX_IR_LED_MODE_ON) {
		*led_conf = LED_RESULT_ON;
	} else if (adv_img_pref->ir_led_mode == AGTX_IR_LED_MODE_OFF) {
		*led_conf = LED_RESULT_OFF;
	} else {
		return -1;
	}

	return 0;
}

int get_img_conf(const AGTX_ADV_IMG_PREF_S *adv_img_pref, const AGTX_COLOR_MODE_E color_mode,IMG_RESULT_E *color_conf)
{
	if (adv_img_pref->image_mode == AGTX_IMAGE_MODE_AUTO) {
		if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_OFF) {
			*color_conf = IMG_RESULT_DAY;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_ON) {
			*color_conf = IMG_RESULT_NIGHT;
		} else if (adv_img_pref->night_mode == AGTX_NIGHT_MODE_AUTO) {
			*color_conf = (color_mode == AGTX_COLOR_MODE_DAY) ? IMG_RESULT_DAY : IMG_RESULT_NIGHT;
		} else {
			return -1;
		}
	} else if (adv_img_pref->image_mode == AGTX_IMAGE_MODE_GRAYSCALE) {
		*color_conf = IMG_RESULT_NIGHT;
	} else if (adv_img_pref->image_mode == AGTX_IMAGE_MODE_COLOR) {
		*color_conf = IMG_RESULT_DAY;
	} else {
		return -1;
	}

	return 0;
}

int check_json_string(struct json_object **json_obj, char *buf, int strlen)
{
	struct json_object  *obj;
	struct json_tokener *tok = json_tokener_new();
	enum   json_tokener_error jerr;

	/* Parse the buf */
	if (strlen > 0) {
		strcat(g_json_buf, buf);
	//	g_jbuf_size += strlen;  // size needed for pkt re-assembly
		g_jbuf_size = strlen;
		obj = json_tokener_parse_ex(tok, g_json_buf, g_jbuf_size);
	} else {
		/* TODO: check if needed */
		obj = json_tokener_parse_ex(tok, (char *)buf, g_jbuf_size);
	}

	jerr = json_tokener_get_error(tok);

	if (jerr != json_tokener_success) {
		fprintf(stderr, "JSON Tokener errNo: %d \n", jerr);
		fprintf(stderr, "JSON Parsing errorer %s \n", json_tokener_error_desc(jerr));

		bzero(g_json_buf, JSON_STR_BUF_SIZE);
		json_tokener_free(tok);
		*json_obj = NULL;

		return -1;
	} else if (jerr == json_tokener_success) {
		//TODO: pass to sub-func's which parse a given key data type

		bzero(g_json_buf, JSON_STR_BUF_SIZE);
		g_jbuf_size = 0;
	}

	json_tokener_free(tok);
	*json_obj = obj;

	return 0;
}

int parse_return_value(struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int rval = -1;

	if (json_object_object_get_ex(cmd_obj, "rval", &tmp_obj)) {
		rval = json_object_get_int(tmp_obj);
	}

	return rval;
}
