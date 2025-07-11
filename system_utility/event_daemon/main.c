#define _GNU_SOURCE
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <eirq-pir-hc18xx.h>

#include <errno.h>
#include <pthread.h>

#include "adc.h"
#include "agtx_color_conf.h"
#include "agtx_floodlight_conf.h"
#include "agtx_light_sensor_conf.h"
#include "agtx_pir_conf.h"
#include "agtx_pwm_conf.h"
#include "event.h"
#include "event_utils.h"
#include "gpio.h"
#include "led.h"
#include "ledevt.h"
#include "pwm.h"
#include "sw.h"
#include "agtx_types.h"

#define GET_ADC_FREQ (5)
#define MAX_TRIG_RANGE (2)
#define DAY_NIGHT_FILE_PATH "/tmp/dn_attr.dat"

typedef enum {
	STATE_FAIL,
	STATE_NOT_READY,
	STATE_SUCCESS,
} STATE_E;

typedef struct {
	pthread_t tid;
	pthread_mutex_t lock;
	int id;
	char *name;
	int create_thread;
	AGTX_EVENT_ATTR_S attr;
	AGTX_EVENT_GROUP_S group;
	Gpio gpio;
	char *socket_path;
	char *adc_path;
	char *device_path;
	int dft_adc_value;
} EVENT_CTX_S;

typedef struct {
	AGTX_BOOL is_updated;
	AGTX_ADV_IMG_PREF_S pref;
} ADV_IMG_CTX_S;

typedef struct {
	AGTX_ADV_IMG_PREF_S img_pref;
	AGTX_INT32 icr_pin[2];
	AGTX_INT32 ir_led_pin;
} DAY_NIGHT_ATTR_S;

typedef struct {
	Gpio gpio;
	Value curr_v;
	Value prev_v;
	struct timeval curr_tv;
	struct timeval trig_tv[MAX_TRIG_RANGE];
	struct timeval up_tv, down_tv;
	struct timeval diff_tv;
} PIR_ATTR_S;

typedef struct {
	pthread_mutex_t lock;
	AGTX_BOOL is_updated;
	AGTX_PIR_CONF_S pir_conf;
	PWM pwm[MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE];
} PIR_CTX_S;

typedef struct {
	pthread_mutex_t lock;
	AGTX_BOOL is_updated;
	AGTX_FLOODLIGHT_CONF_S floodlight_conf;
	PWM pwm;
} FLOODLIGHT_CTX_S;

typedef struct {
	pthread_mutex_t lock;
	AGTX_BOOL is_updated;
	AGTX_LIGHT_SENSOR_CONF_S light_sensor_conf;
} LIGHT_SENSOR_CTX_S;

typedef struct {
	pthread_mutex_t lock;
	AGTX_BOOL is_updated;
	AGTX_COLOR_CONF_S color_mode_conf;
} COLOR_MODE_CTX_S;

static EVENT_CTX_S g_event_ctx[MAX_AGTX_EVENT_CONF_S_EVENT_SIZE];
pthread_t g_adj_evt_cfg_tid;
static ADV_IMG_CTX_S g_adv_img_ctx;
static PIR_CTX_S g_pir_ctx;
static FLOODLIGHT_CTX_S g_floodlight_ctx;
static LIGHT_SENSOR_CTX_S g_light_sensor_ctx;
static COLOR_MODE_CTX_S g_color_mode_ctx;
static AGTX_GPIO_ALIAS_S g_gpio_alias[MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE];
static AGTX_PWM_ALIAS_S g_pwm_alias[MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE];
static DAY_NIGHT_ATTR_S g_day_night_attr;
static int g_icr_pin_num;
struct LED_EVENT_TABLE_S g_levt;
struct LED_EVENT_TABLE_S curr_levt;

#define EVENT_GET_CTX(id, ptr)          \
	do {                            \
		ptr = &g_event_ctx[id]; \
	} while (0)

/* Internal function prototype */
static int EVENT_connectCentCtrl(const char *cc_socket_path);
static int EVENT_getConf(const int client_fd, void *dft_cfg, size_t cfg_len, AGTX_UINT32 cid);
static void EVENT_initGPIO(const int num_of_elms, const AGTX_GPIO_ALIAS_S *gpio_alias);
static void EVENT_exitGPIO(const int num_of_elms, const AGTX_GPIO_ALIAS_S *gpio_alias);
static int EVENT_initPWM(const int num_of_elms, const AGTX_PWM_ALIAS_S *pwm_alias);
static void EVENT_exitPWM(const int num_of_elms, const AGTX_PWM_ALIAS_S *pwm_alias);
static int EVENT_setEvtAttr(const int num_of_event, const AGTX_EVENT_ATTR_S *attr);
static int EVENT_initGlobalCtx(const int num_of_event, const AGTX_EVENT_GROUP_S *data);
static void EVENT_getTimeInfos(const int curr_v, const AGTX_GPIO_EVENT_LIST_S *dft_gpio, struct timeval *curr_tv,
                               struct timeval *trig_tv);
static int EVENT_TimeCompare(struct timeval *curr_tv, int time_sec_start, int time_sec_end);
static int handleSpAdvImgPref(const AGTX_ADV_IMG_PREF_S *adv_img_pref, AGTX_EVENT_ACTION_CB action_cb,
                              void *action_args, const char *DAY_NIGHT_PATH);
static int handlePirDatabase(AGTX_PIR_CONF_S *pir, AGTX_FLOODLIGHT_CONF_S *floodlight, AGTX_COLOR_CONF_S *color_conf);
static int handleButtonTime(struct timeval *curr_tv, struct timeval trig_tv, int *button_state, int *button_reset,
                            int level_time_sec);
static void handlePirAction(FLOODLIGHT_CTX_S *floodlight, AGTX_PIR_CONF_S *pir, AGTX_COLOR_CONF_S *color_conf,
                            struct timeval *curr_tv, struct timeval *stop_tv, int *pir_trig);
static void *EVENT_GPIO_daemon(void *arg);
static void *EVENT_SW_daemon(void *arg);
static void *EVENT_ADC_daemon(void *arg);
static void *EVENT_EINTC_PIR_daemon(void *arg);
static void *EVENT_LED_daemon(void *arg);
static int EVENT_parseCtrlCmd(const int client_fd, const AGTX_MSG_HEADER_S *cmd_header);
static int EVENT_sendReplyCmd(const int client_fd, const AGTX_MSG_HEADER_S *cmd_reply, const int ret);
static void *EVENT_CC_daemon(void *arg);
static int initAdvImgPref(const int client_fd);
static void EVENT_printEvtGroup(const AGTX_EVENT_GROUP_S *group);
static void EVENT_printEvtCtx(const int max_idx, const EVENT_CTX_S *ctx);
static void EVENT_printEvtCfgList(const int max_idx, const EVENT_CTX_S *ctx);
static void EVENT_printGpioConf(const int max_idx, const AGTX_GPIO_ALIAS_S *gpio_alias);

static void handleSigno(int signo)
{
	if (signo == SIGINT) {
		EVT_NOTICE("Caught %s!\n", strsignal(signo));
	} else if (signo == SIGTERM) {
		EVT_NOTICE("Caught %s!\n", strsignal(signo));
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}

	EVENT_exitGPIO(MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, &g_gpio_alias[0]);
	exit(0);
}

static void __attribute__((unused)) switchIrCut(int icr_pin_num, int *icr_pin, int mode)
{
	Gpio ir_cut0;
	Gpio ir_cut1;

	ir_cut0.id = icr_pin[0];
	ir_cut1.id = icr_pin[1];

	if (icr_pin_num == 1) {
		if (mode) {
			//active
			setGpioValue(&ir_cut0, 0);
		} else {
			//remove
			setGpioValue(&ir_cut0, 1);
		}
	} else if (icr_pin_num == 2) {
		if (mode) {
			//active
			setGpioValue(&ir_cut0, 0);
			setGpioValue(&ir_cut1, 1);
			usleep(300000);
			setGpioValue(&ir_cut0, 0);
			setGpioValue(&ir_cut1, 0);
		} else {
			//remove
			setGpioValue(&ir_cut0, 1);
			setGpioValue(&ir_cut1, 0);
			usleep(300000);
			setGpioValue(&ir_cut0, 0);
			setGpioValue(&ir_cut1, 0);
		}
	} else {
		EVT_ERR("Invalid parameter to switch Ir Cut\n");
		return;
	}
}

static int calcIcrNum(int *icr_pin)
{
	int num = 0;

	if (icr_pin[0] > 0) {
		num++;
	}

	if (icr_pin[1] > 0) {
		num++;
	}

	return num;
}

static int registerToCentCtrl(int client_fd)
{
	int ret;
	int read_ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	ret = 0;
	read_ret = 0;

	sprintf(reg_buf,
	        "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", "
	        "\"name\":\"%s\"}",
	        AGTX_CMD_REG_CLIENT, EVT_NAME);
	sprintf(ret_cmd,
	        "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": "
	        "\"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/* Send register information */
	if (write(client_fd, &reg_buf, strlen(reg_buf)) < 0) {
		EVT_ERR("Failed to send register information to CC!\n");
		ret = -1;
	}

	while (!ret) {
		read_ret = read(client_fd, buf, strlen(ret_cmd));
		if (read_ret != (int)strlen(ret_cmd)) {
			EVT_ERR("Failed to read socket!\n");
			continue;
		}

		if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
			sleep(1);
			EVT_NOTICE("Wating CC return ACK for an valid client!\n");
			continue;
		} else {
			syslog(LOG_INFO, "Registered to CC from %s.\n", EVT_NAME);
			break;
		}
	}

	return ret;
}

static int EVENT_connectCentCtrl(const char *cc_socket_path)
{
	int client_fd;
	int ret = 0;

	client_fd = startClientConnect(cc_socket_path);

	if (client_fd == SW_FAILURE) {
		ret = -1;
		return ret;
	}

	ret = registerToCentCtrl(client_fd);

	if (ret < 0) {
		EVT_ERR("Failed to register to central control!\n");
		return ret;
	}

	return client_fd;
}

static int EVENT_getConf(const int client_fd, void *dft_cfg, size_t cfg_len, AGTX_UINT32 cid)
{
	int ret = STATE_NOT_READY;
	int read_ret = 0;
	int cnt = 0;
	AGTX_MSG_HEADER_S cmd_header = { 0 };

	/* Send config request */
	cmd_header.cid = cid;

	if (write(client_fd, &cmd_header, sizeof(cmd_header)) < 0) {
		EVT_ERR("Failed to request cmd header!\n");
		ret = STATE_FAIL;
		return ret;
	}

	while (ret == STATE_NOT_READY) {
		cnt++;

		if (cnt > 5) {
			EVT_ERR("Failed to get default config many times from CC!\n");
			ret = STATE_FAIL;
			break;
		}

		EVT_TRACE("Try to get default config from CC...(%d)\n", cnt);

		read_ret = read(client_fd, &cmd_header, sizeof(cmd_header));

		if (read_ret == 0) {
			continue;
		}

		if (read_ret < 0) {
			EVT_ERR("Failed to read cmd header!\n");
			continue;
		}

		if (read_ret != sizeof(cmd_header)) {
			EVT_WARN("Incorrect size. read_ret / cmd_header = (%d / %d)\n", read_ret, sizeof(cmd_header));
			continue;
		}


		switch (cmd_header.cid) {
		case AGTX_CMD_EVT_CONF:
			assert(sizeof(AGTX_EVENT_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_EVENT_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_EVENT_CONF_S = (%d / %d)\n",
				         cmd_header.len, sizeof(AGTX_EVENT_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_EVENT_CONF_S));

			if (read_ret != sizeof(AGTX_EVENT_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_EVENT_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_EVENT_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_EVT_PARAM:
			assert(sizeof(AGTX_EVENT_PARAM_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_EVENT_PARAM_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_EVENT_PARAM_S = (%d / %d)\n",
				         cmd_header.len, sizeof(AGTX_EVENT_PARAM_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_EVENT_PARAM_S));

			if (read_ret != sizeof(AGTX_EVENT_PARAM_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_EVENT_PARAM_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_EVENT_PARAM_S));
				continue;
			}
			break;

		case AGTX_CMD_GPIO_CONF:
			assert(sizeof(AGTX_GPIO_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_GPIO_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_GPIO_CONF_S = (%d / %d)\n",
				         cmd_header.len, sizeof(AGTX_GPIO_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_GPIO_CONF_S));

			if (read_ret != sizeof(AGTX_GPIO_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_GPIO_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_GPIO_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_PWM_CONF:
			assert(sizeof(AGTX_PWM_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_PWM_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_PWM_CONF_S = (%d / %d)\n", cmd_header.len,
				         sizeof(AGTX_PWM_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_PWM_CONF_S));

			if (read_ret != sizeof(AGTX_PWM_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_PWM_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_PWM_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_ADV_IMG_PREF:
			assert(sizeof(AGTX_ADV_IMG_PREF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_ADV_IMG_PREF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_ADV_IMG_PREF_S = (%d / %d)\n",
				         cmd_header.len, sizeof(AGTX_ADV_IMG_PREF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_ADV_IMG_PREF_S));

			if (read_ret != sizeof(AGTX_ADV_IMG_PREF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_ADV_IMG_PREF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_ADV_IMG_PREF_S));
				continue;
			}
			break;

		case AGTX_CMD_FLOODLIGHT_CONF:
			assert(sizeof(AGTX_FLOODLIGHT_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_FLOODLIGHT_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_FLOODLIGHT_CONF_S = (%d "
				         "/ %d)\n",
				         cmd_header.len, sizeof(AGTX_FLOODLIGHT_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_FLOODLIGHT_CONF_S));

			if (read_ret != sizeof(AGTX_FLOODLIGHT_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_FLOODLIGHT_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_FLOODLIGHT_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_PIR_CONF:
			assert(sizeof(AGTX_PIR_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_PIR_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_PIR_CONF_S = (%d "
				         "/ %d)\n",
				         cmd_header.len, sizeof(AGTX_PIR_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_PIR_CONF_S));

			if (read_ret != sizeof(AGTX_PIR_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_PIR_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_PIR_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_LIGHT_SENSOR_CONF:
			assert(sizeof(AGTX_LIGHT_SENSOR_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_LIGHT_SENSOR_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_LIGHT_SENSOR_CONF_S = (%d "
				         "/ %d)\n",
				         cmd_header.len, sizeof(AGTX_LIGHT_SENSOR_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_LIGHT_SENSOR_CONF_S));

			if (read_ret != sizeof(AGTX_LIGHT_SENSOR_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_LIGHT_SENSOR_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_LIGHT_SENSOR_CONF_S));
				continue;
			}
			break;

		case AGTX_CMD_COLOR_CONF:
			assert(sizeof(AGTX_COLOR_CONF_S) == cfg_len);
			if (cmd_header.len != sizeof(AGTX_COLOR_CONF_S)) {
				EVT_WARN("Invaild size. cmd_header.len / AGTX_COLOR_CONF_S = (%d "
				         "/ %d)\n",
				         cmd_header.len, sizeof(AGTX_COLOR_CONF_S));
				continue;
			}

			read_ret = read(client_fd, dft_cfg, sizeof(AGTX_COLOR_CONF_S));

			if (read_ret != sizeof(AGTX_COLOR_CONF_S)) {
				EVT_WARN("Incorrect size. read_ret / AGTX_COLOR_CONF_S = (%d /%d)\n", read_ret,
				         sizeof(AGTX_COLOR_CONF_S));
				continue;
			}
			break;

		default:
			return STATE_FAIL;
			break;
		}

		ret = STATE_SUCCESS;
	}

	return ret;
}

static void EVENT_initGPIO(const int num_of_elms, const AGTX_GPIO_ALIAS_S *gpio_alias)
{
	int i;
	int ret;
	Gpio gpio;

	ret = 0;

	syslog(LOG_INFO, "#--- GPIO list (init)\n");

	for (i = 0; i < num_of_elms; i++) {
		if ((strcmp((char *)gpio_alias[i].name, "") != 0) && (gpio_alias[i].pin_num >= 0)) {
			ret = exportGpio(gpio_alias[i].pin_num);

			if (ret < 0) {
				EVT_ERR("Failed to exportGpio(%2d)!\n", gpio_alias[i].pin_num);
				continue;
			}

			gpio.id = gpio_alias[i].pin_num;

			switch (gpio_alias[i].dir) {
			case AGTX_GPIO_DIR_IN:
				setGpioDirection(&gpio, "in");
				break;
			case AGTX_GPIO_DIR_OUT:
				if ((Value)gpio_alias[i].value == GPIO_HIGH) {
					setGpioDirection(&gpio, "high");
				} else {
					setGpioDirection(&gpio, "low");
				}
				break;
			default:
				EVT_WARN("GPIO %d: Invalid GPIO direction!\n", gpio.id);
				break;
			}

			syslog(LOG_INFO, "pin_num = %2d, name = %s\n", gpio_alias[i].pin_num, gpio_alias[i].name);
		}
	}

	return;
}

static void EVENT_exitGPIO(const int num_of_elms, const AGTX_GPIO_ALIAS_S *gpio_alias)
{
	int i;
	int ret;

	ret = 0;

	syslog(LOG_INFO, "#--- GPIO list (exit)\n");

	for (i = 0; i < num_of_elms; i++) {
		if ((strcmp((char *)gpio_alias[i].name, "") != 0) && (gpio_alias[i].pin_num > 0)) {
			ret = unexportGpio(gpio_alias[i].pin_num);

			if (ret < 0) {
				EVT_ERR("Failed to unexportGpio(%2d)!\n", gpio_alias[i].pin_num);
				continue;
			}

			syslog(LOG_INFO, "pin_num = %2d, name = %s\n", gpio_alias[i].pin_num, gpio_alias[i].name);
		}
	}

	return;
}

static int EVENT_initPWM(const int num_of_elms, const AGTX_PWM_ALIAS_S *pwm_alias)
{
	int i;
	int ret = STATE_NOT_READY;
	PWM pwm;

	syslog(LOG_INFO, "#--- PWM list (init)\n");

	for (i = 0; i < num_of_elms; i++) {
		if (pwm_alias[i].pin_num >= 0) {
			ret = exportPWM(pwm_alias[i].pin_num);

			if (ret < 0) {
				EVT_ERR("Failed to export PWM(%2d)!\n", pwm_alias[i].pin_num);
				return ret;
			}

			pwm.id = pwm_alias[i].pin_num;
			ret = enabledPWM(&pwm, pwm_alias[i].enabled);

			if (ret < 0) {
				EVT_ERR("Failed to enable PWM(%2d)!\n", pwm_alias[i].pin_num);
				return ret;
			}

			setPWMPeriod(&pwm, pwm_alias[i].period);
			setPWMDutyCycle(&pwm, pwm_alias[i].duty_cycle);

			syslog(LOG_INFO, "pwm: pin_num = %d, init_enabled = %d, period = %d, duty_cycle = %d",
			       pwm_alias[i].pin_num, pwm_alias[i].enabled, pwm_alias[i].period,
			       pwm_alias[i].duty_cycle);
		}
	}

	return STATE_SUCCESS;
}

static void EVENT_exitPWM(const int num_of_elms, const AGTX_PWM_ALIAS_S *pwm_alias)
{
	int i;
	int ret;

	ret = 0;

	syslog(LOG_INFO, "#--- PWM list (exit)\n");

	for (i = 0; i < num_of_elms; i++) {
		if (pwm_alias[i].pin_num > 0) {
			ret = unexportPWM(pwm_alias[i].pin_num);

			if (ret < 0) {
				EVT_ERR("Failed to unexportPWM(%2d)!\n", pwm_alias[i].pin_num);
				continue;
			}

			syslog(LOG_INFO, "Unexport pin_num = %2d\n", pwm_alias[i].pin_num);
		}
	}

	return;
}

static int EVENT_setEvtAttr(const int num_of_event, const AGTX_EVENT_ATTR_S *attr)
{
	int i;
	int idx;
	EVENT_CTX_S *ctx;

	for (i = 0; i < MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE; i++) {
		// syslog(LOG_INFO, "attr[i = %d].name = %s\n", i, attr[i].name);
		/* find attr in which ctx */
		for (idx = 0; idx < num_of_event; idx++) {
			EVENT_GET_CTX(idx, ctx);

			// syslog(LOG_INFO, "ctx[idx = %d]->name = %s\n", idx, ctx->name);

			if (strcmp((char *)attr[i].name, ctx->name) == 0) {
				/* update ctx */
				pthread_mutex_lock(&ctx->lock);
				memcpy(&ctx->attr, &attr[i], sizeof(AGTX_EVENT_ATTR_S));
				pthread_mutex_unlock(&ctx->lock);
				break;
			}
		}
	}

	syslog(LOG_INFO, "After updating attributes...\n");
	EVENT_printEvtCfgList(MAX_AGTX_EVENT_CONF_S_EVENT_SIZE, &g_event_ctx[0]);

	return 0;
}

static int EVENT_initGlobalCtx(const int num_of_event, const AGTX_EVENT_GROUP_S *data)
{
	int i;
	int ret;
	int pin;
	char *direction;

	ret = 0;
	direction = "in";

	for (i = 0; i < num_of_event; i++) {
		g_event_ctx[i].lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		g_event_ctx[i].id = i;

		memcpy(&g_event_ctx[i].group, &data[i], sizeof(AGTX_EVENT_GROUP_S));

		g_event_ctx[i].name =
		        findNamefromAliasTable(k_event_alias_table, AGTX_EVENT_NAME_NUM + 1, g_event_ctx[i].group.name);

		g_event_ctx[i].create_thread = g_event_ctx[i].group.in_use;

		if (g_event_ctx[i].name == NULL) {
			g_event_ctx[i].create_thread = 0;
			continue;
		}

		EVT_TRACE("g_event_ctx[%d].name = %s\n", i, g_event_ctx[i].name);

		if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_GPIO) {
			pin = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, g_event_ctx[i].name);

			if (pin >= 0) {
				g_event_ctx[i].gpio.id = pin;
				setGpioDirection(&g_event_ctx[i].gpio, direction);
				getGpioValue(&g_event_ctx[i].gpio);
			} else {
				g_event_ctx[i].create_thread = 0;
			}

			g_event_ctx[i].socket_path = NULL;
			g_event_ctx[i].adc_path = NULL;
			g_event_ctx[i].device_path = NULL;
		} else if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_SW) {
			g_event_ctx[i].socket_path = findNamefromAliasTable(k_socket_path_alias_table,
			                                                    AGTX_SW_EVENT_SOCKET_PATH_NUM + 1,
			                                                    g_event_ctx[i].group.sw.socket_path);

			g_event_ctx[i].adc_path = NULL;
			g_event_ctx[i].device_path = NULL;
			EVT_TRACE("g_event_ctx[%d].socket_path = %s\n", i, g_event_ctx[i].socket_path);

		} else if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_ADC) {
			g_event_ctx[i].socket_path = NULL;
			g_event_ctx[i].adc_path = findNamefromAliasTable(k_adc_path_alias_table, ADC_CHANNEL_NUM,
			                                                 g_event_ctx[i].group.adc.chn);
			g_event_ctx[i].device_path = NULL;
			getAdcValue(g_event_ctx[i].adc_path, &g_event_ctx[i].dft_adc_value);

		} else if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_LED) {
			g_event_ctx[i].socket_path = NULL;
			g_event_ctx[i].adc_path = NULL;
			g_event_ctx[i].device_path = NULL;

		} else if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_MPI) {
			g_event_ctx[i].socket_path = NULL;
			g_event_ctx[i].adc_path = NULL;
			g_event_ctx[i].device_path = NULL;

		} else if (g_event_ctx[i].group.source == AGTX_EVENT_SOURCE_EINTC) {
			g_event_ctx[i].device_path = findNamefromAliasTable(k_eintc_device_path_alias_table,
			                                                    AGTX_EINTC_EVENT_DEVICE_PATH_NUM + 1,
			                                                    g_event_ctx[i].group.eintc.device_path);
			g_event_ctx[i].socket_path = NULL;
			g_event_ctx[i].adc_path = NULL;

		} else {
			EVT_ERR("Unexpected on initialize global context!\n");
			ret = -1;
			return ret;
		}
	}

	EVT_TRACE("#--- Event List\n");
	for (i = 0; i < num_of_event; i++) {
		EVT_TRACE("[%d] %s\n", g_event_ctx[i].id, g_event_ctx[i].name);
	}
	EVT_TRACE("\n");

	return ret;
}

static void EVENT_getTimeInfos(const int curr_v, const AGTX_GPIO_EVENT_LIST_S *dft_gpio, struct timeval *curr_tv,
                               struct timeval *trig_tv)
{
	int get_time_info;
	int n;
	int tmp_time_sec;

	get_time_info = 0;

	for (n = 0; n < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; n++) {
		if (dft_gpio->event[n].rule.trigger_type == AGTX_GPIO_EVENT_TRIG_TYPE_LEVEL &&
		    (dft_gpio->event[n].rule.level_time_sec > 0 || dft_gpio->event[n].rule.level_time_sec < 0)) {
			tmp_time_sec = dft_gpio->event[n].rule.level_time_sec;

			if (curr_v == dft_gpio->event[n].rule.level_value) {
				gettimeofday(curr_tv, NULL);

				if (trig_tv[1].tv_sec == 0 && trig_tv[1].tv_usec == 0) {
					trig_tv[0].tv_sec = curr_tv->tv_sec;
					trig_tv[0].tv_usec = curr_tv->tv_usec;

					tmp_time_sec = (tmp_time_sec > 0) ? tmp_time_sec : -tmp_time_sec;

					trig_tv[1].tv_sec = curr_tv->tv_sec + tmp_time_sec;
					trig_tv[1].tv_usec = curr_tv->tv_usec;
				}

				get_time_info = 1;

			} else {
				memset(curr_tv, 0, sizeof(struct timeval));
				memset(trig_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
			}
		}
	}

	if (get_time_info == 1) {
		EVT_TRACE("[EVENT_getTimeInfos]\n");
		EVT_TRACE("trig_tv[0].tv_sec = %ld\n", trig_tv[0].tv_sec);
		EVT_TRACE("trig_tv[0].tv_usec = %ld\n", trig_tv[0].tv_usec);
		EVT_TRACE("curr_tv->tv_sec = %ld\n", curr_tv->tv_sec);
		EVT_TRACE("curr_tv->tv_usec = %ld\n", curr_tv->tv_usec);
		EVT_TRACE("trig_tv[1].tv_sec = %ld\n", trig_tv[1].tv_sec);
		EVT_TRACE("trig_tv[1].tv_usec = %ld\n", trig_tv[1].tv_usec);
	}

	return;
}

static int EVENT_TimeCompare(struct timeval *diff_tv, int time_sec_start, int time_sec_end)
{
	int ret = 0;

	if (diff_tv->tv_sec >= time_sec_start && diff_tv->tv_sec < time_sec_end) {
		if (diff_tv->tv_usec != 0) {
			ret = 1;
		}
	} else {
		ret = 0;
	}

	return ret;
}

//im166
#define SHM_NAME "/agt903_pir"
#define SHM_SIZE 16
#define FILE_SIZE 16
static int shm_res = 0;
int fd;
void *add_r = NULL;

void __exit_share_mem__(void)
{
	/* close shared memory */
	munmap(add_r, FILE_SIZE);
	shm_unlink(SHM_NAME);
	close(fd);
}

static int __init_share_mem__(void)
{
	/* create shared memory */
	fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("[EVT] shm_open() through....................................2\n");
		goto failed;
	}

	if (ftruncate(fd, SHM_SIZE) == -1) {
		printf("[EVT] shm_open() through....................................3\n");
		goto failed;
	}

	add_r = mmap(NULL, FILE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (add_r == MAP_FAILED) {
		printf("[EVT] shm_open() through....................................4\n");
		shm_unlink(SHM_NAME);
		goto failed;
	}

	return 0;

failed:
	close(fd);
	return -1;
}

static void *EVENT_GPIO_daemon(void *arg)
{
	int ret;
	EVENT_CTX_S *ctx;
	AGTX_GPIO_EVENT_LIST_S *dft_gpio;
	Value curr_v;
	Value prev_v;
	int polling_period_usec;
	int n;
	int i;
	struct timeval curr_tv;
	struct timeval trig_tv[MAX_TRIG_RANGE];
	AGTX_EVENT_ACTION_CB action_cb[MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE];
	struct timeval up_tv, down_tv;
	struct timeval diff_tv;
	AGTX_BOOL curr_detection_status;
	AGTX_BOOL prev_detection_status;
	AGTX_EVENT_ACTION_CB exec_cmd_cb[0];
	int time_tr_sec = 0;
	int button_state = AGTX_BUTTON_PRESS_TYPE_NONE;
	int button_reset = AGTX_BUTTON_RESET_TYPE_OFF;
	int edge_trig_flag = 0;
	int edge_trig_on = 0;
	PIR_ATTR_S pir_attr[MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE];
	AGTX_PIR_CONF_S pir;
	FLOODLIGHT_CTX_S floodlight;
	AGTX_COLOR_CONF_S color_conf;
	struct timeval pir_curr_tv;
	struct timeval pir_stop_tv[MAX_TRIG_RANGE];
	int pir_trig = FLOODLIGHT_NONE;
	int led_thread_enabled = 0;

	curr_detection_status = AGTX_TRUE;
	prev_detection_status = AGTX_TRUE;
	ret = 0;
	ctx = (EVENT_CTX_S *)arg;
	dft_gpio = &ctx->group.gpio;
	polling_period_usec = ctx->group.gpio.polling_period_usec;
	prev_v = ctx->gpio.value;

	memset(&curr_tv, 0, sizeof(struct timeval));
	memset(trig_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
	memset(&up_tv, 0, sizeof(struct timeval));
	memset(&down_tv, 0, sizeof(struct timeval));
	memset(&diff_tv, 0, sizeof(struct timeval));
	memset(&pir_attr, 0, sizeof(PIR_ATTR_S));

	for (n = 0; n < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; n++) {
		action_cb[n] = *(findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1,
		                                         dft_gpio->event[n].action));
	}

	/* get led thread status */
	for (n = 0; n < MAX_AGTX_EVENT_CONF_S_EVENT_SIZE; n++) {
		if (strcmp((char *)g_event_ctx[n].attr.name, "LED_INFORM") == 0) {
			led_thread_enabled = g_event_ctx[n].attr.enabled;
		}
	}

	exec_cmd_cb[0] = *(
	        findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1, AGTX_EVENT_ACTION_CB_EXEC_CMD));

	/* handle init status */
	curr_v = getGpioValue(&ctx->gpio);

	EVT_TRACE("[%d, %s] curr_v = %d (init status)\n", ctx->id, ctx->name, curr_v);

	if (strcmp((char *)dft_gpio->init_level[curr_v].action_args, "")) {
		syslog(LOG_INFO, "%s is triggered by rule #[%d] (init status).\n", ctx->name, curr_v);

		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_IN].name) == 0) {
			handleSpAdvImgPref(&g_adv_img_ctx.pref, exec_cmd_cb[0],
			                   dft_gpio->init_level[curr_v].action_args, DAY_NIGHT_FILE_PATH);
		} else {
			AGTX_EVENT_execCmd(dft_gpio->init_level[curr_v].action_args, NULL);
		}
	}

	if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_PIR_IN].name) == 0) {
		/* get pir gpio, maximum pir support number = 3.*/
		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_PIR_IN].name) == 0) {
			pir_attr[0].gpio.id = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "PIR_IN");
			pir_attr[1].gpio.id = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "PIR2_IN");
			pir_attr[2].gpio.id = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "PIR3_IN");
		}

		/* get pir config */
		memcpy(&pir, &g_pir_ctx.pir_conf, sizeof(AGTX_PIR_CONF_S));

		/* get colar mode config*/
		memcpy(&color_conf, &g_color_mode_ctx.color_mode_conf, sizeof(AGTX_COLOR_CONF_S));

		/* handle floodlight enabled status*/
		memcpy(&floodlight, &g_floodlight_ctx, sizeof(FLOODLIGHT_CTX_S));
		for (n = 0; n < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; n++) {
			if (strcmp((char *)g_pwm_alias[n].name, "FLOODLIGHT") == 0) {
				pir_trig = FLOODLIGHT_ACTION_MODE;
				handlePirAction(&floodlight, &pir, &color_conf, &pir_curr_tv, pir_stop_tv, &pir_trig);
				break;
			}
		}
	}

	//im166
	shm_res = __init_share_mem__();

	/* gpio event detection and handling */
	EVT_TRACE("Ready to detect and handle event(%d, %s).\n", ctx->id, ctx->name);
	while (!ret) {
		//im166
		int pin_trigger = 0; //Check pin result: Default:0 = not trigger
		prev_detection_status = curr_detection_status;
		/*********************************/
		/* check enabled or not */
		/*********************************/
		pthread_mutex_lock(&ctx->lock);
		curr_detection_status = ((ctx->attr.enabled || ctx->group.always_enabled) == 0) ? AGTX_FALSE :
		                                                                                  AGTX_TRUE;
		pthread_mutex_unlock(&ctx->lock);

		EVT_TRACE("%s ctx->attr.enabled %d\n", ctx->name, ctx->attr.enabled);
		EVT_TRACE("%s ctx->group.always_enabled %d\n", ctx->name, ctx->group.always_enabled);

		if (curr_detection_status == AGTX_FALSE) {
			usleep(polling_period_usec);
			continue;
		}

		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_PIR_IN].name) == 0) {
			/*When pir_conf, floodlight_conf and color_conf change*/
			n = handlePirDatabase(&pir, &floodlight.floodlight_conf, &color_conf);
			if (n != FLOODLIGHT_NONE) {
				pir_trig = n;
				handlePirAction(&floodlight, &pir, &color_conf, &pir_curr_tv, pir_stop_tv, &pir_trig);
			}

			/*get gpio value*/
			for (n = 0; n < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; n++) {
				if (pir_attr[n].gpio.id >= 0) {
					pir_attr[n].curr_v = getGpioValue(&pir_attr[n].gpio);
				}
			}
		} else {
			curr_v = getGpioValue(&ctx->gpio);
		}

		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_IN].name) == 0) {
			if (curr_detection_status != prev_detection_status || g_adv_img_ctx.is_updated == AGTX_TRUE) {
				syslog(LOG_INFO, "%s is triggered by rule #[%d] (status change).\n", ctx->name, curr_v);

				handleSpAdvImgPref(&g_adv_img_ctx.pref, exec_cmd_cb[0],
				                   dft_gpio->init_level[curr_v].action_args, DAY_NIGHT_FILE_PATH);
				// AGTX_EVENT_execCmd(dft_gpio->init_level[curr_v].action_args);
			}
		}

		EVT_TRACE("[%d, %s]  gpio id = %d, curr_v = %d\n", ctx->id, ctx->name, ctx->gpio.id, curr_v);

		/*get trigger time and monitor the status*/
		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_PIR_IN].name) == 0) {
			for (n = 0; n < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; n++) {
				if (pir_attr[n].gpio.id >= 0) {
					EVENT_getTimeInfos(pir_attr[n].curr_v, dft_gpio, &pir_attr[n].curr_tv,
					                   pir_attr[n].trig_tv);

					if ((pir_attr[n].curr_v - pir_attr[n].prev_v) == 1) {
						gettimeofday(&pir_attr[n].up_tv, NULL);
					} else if ((pir_attr[n].curr_v - pir_attr[n].prev_v) == -1) {
						gettimeofday(&pir_attr[n].down_tv, NULL);
					}
				}
			}
		} else {
			EVENT_getTimeInfos(curr_v, dft_gpio, &curr_tv, trig_tv);
			if ((curr_v - prev_v) == 1) {
				gettimeofday(&up_tv, NULL);
				edge_trig_flag = 1;
			} else if ((curr_v - prev_v) == -1) {
				gettimeofday(&down_tv, NULL);
				edge_trig_flag = 1;
			} else {
				edge_trig_flag = 0;
			}
		}

		for (n = 0; n < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; n++) {
			switch (dft_gpio->event[n].rule.trigger_type) {
			case AGTX_GPIO_EVENT_TRIG_TYPE_NONE:
				syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name, n);
				continue;
				break;
			case AGTX_GPIO_EVENT_TRIG_TYPE_LEVEL:
				/* Inform LED daemon the pressing time for reset button */
				if ((strcmp(ctx->name, "PUSH_BUTTON_IN") == 0) && led_thread_enabled) {
					time_tr_sec = handleButtonTime(&curr_tv, trig_tv[1], &button_state,
					                               &button_reset,
					                               dft_gpio->event[n].rule.level_time_sec);
				}
				/* trigger based on current level, trigger based on time_sec */
				if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_PIR_IN].name) == 0) {
					/*Update floodlight status and tracing pir trigger condition*/
					for (i = 0; i < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; i++) {
						/* trigger by floodlight operation */
						if (pir.pir_alias[i].enabled && (pir_attr[i].gpio.id >= 0)) {
							if (pir_attr[i].curr_v == dft_gpio->event[n].rule.level_value &&
							    dft_gpio->event[n].rule.level_time_sec > 0 &&
							    timercmp(&pir_attr[i].curr_tv, &pir_attr[i].trig_tv[1],
							             >)) {
								syslog(LOG_INFO,
								       "%s is triggered by rule #[%d], trigger pir_gpio is %d.\n",
								       ctx->name, n, pir_attr[i].gpio.id);

								//No matter which pin is triggered
								pin_trigger = 1;

								// action_cb[n](dft_gpio->event[n].action_args, NULL);
								pir_trig = PIR_TRIGGER_MODE;
								handlePirAction(&floodlight, &pir, &color_conf,
								                &pir_curr_tv, pir_stop_tv, &pir_trig);

								memset(&pir_attr[i].curr_tv, 0, sizeof(struct timeval));
								memset(pir_attr[i].trig_tv, 0,
								       sizeof(struct timeval) * MAX_TRIG_RANGE);
							}
						}
					}

					//im166
					if (pin_trigger && (shm_res != -1)) {
						memcpy(add_r, "pir_triggered", sizeof("pir_triggered"));
						//printf("arr_r = %s\n", (char *)add_r);
					} else if (!pin_trigger && (shm_res != -1)) {
						memcpy(add_r, "non_triggered", sizeof("non_triggered"));
						//printf("arr_r = %s\n", (char *)add_r);
					}
					/* handle the floodlight time out */
					handlePirAction(&floodlight, &pir, &color_conf, &pir_curr_tv, pir_stop_tv,
					                &pir_trig);
					continue;
				} else {
					if (curr_v == dft_gpio->event[n].rule.level_value &&
					    dft_gpio->event[n].rule.level_time_sec == 0) {
						syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name, n);

						action_cb[n](dft_gpio->event[n].action_args, NULL);
						continue;

					} else if (curr_v == dft_gpio->event[n].rule.level_value &&
					           dft_gpio->event[n].rule.level_time_sec > 0 &&
					           timercmp(&curr_tv, &trig_tv[1], >)) {
						syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name, n);

						if ((strcmp(ctx->name, "PUSH_BUTTON_IN") == 0) && led_thread_enabled) {
							button_reset = AGTX_BUTTON_RESET_TYPE_OFF;
							handleButtonState(time_tr_sec,
							                  dft_gpio->event[n].rule.level_time_sec *
							                          1000000,
							                  curr_levt.curr_client, curr_levt.prev_client,
							                  button_reset);
						}

						action_cb[n](dft_gpio->event[n].action_args, NULL);

						memset(&curr_tv, 0, sizeof(struct timeval));
						memset(trig_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
						continue;

					} else if (curr_v == dft_gpio->event[n].rule.level_value &&
					           dft_gpio->event[n].rule.level_time_sec < 0 &&
					           (timercmp(&curr_tv, &trig_tv[0], >) &&
					            timercmp(&curr_tv, &trig_tv[1], <))) {
						syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name, n);

						action_cb[n](dft_gpio->event[n].action_args, NULL);

						memset(&curr_tv, 0, sizeof(struct timeval));
						memset(trig_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
						continue;

					} else {
						continue;
					}
				}

				break;
			case AGTX_GPIO_EVENT_TRIG_TYPE_EDGE:
				/* trigger based on edge */
				if ((curr_v - prev_v) == dft_gpio->event[n].rule.edge || edge_trig_on == 1) {
					if ((curr_v - prev_v) == 1) {
						timersub(&up_tv, &down_tv, &diff_tv);
					} else if ((curr_v - prev_v) == -1) {
						timersub(&down_tv, &up_tv, &diff_tv);
					}
					/*Trigger event only at the button be released*/
					if (edge_trig_flag == 0 && edge_trig_on == 1) {
						if (EVENT_TimeCompare(&diff_tv,
						                      dft_gpio->event[n].rule.edge_time_sec_start,
						                      dft_gpio->event[n].rule.edge_time_sec_end)) {
							syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name,
							       n);

							action_cb[n](dft_gpio->event[n].action_args, NULL);
							edge_trig_on = 0;
							continue;
						} else {
							if (n == (MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE - 1)) {
								if (down_tv.tv_sec == 0 && down_tv.tv_usec == 0) {
									memset(&up_tv, 0, sizeof(struct timeval));
								}
								memset(&down_tv, 0, sizeof(struct timeval));
								edge_trig_on = 0;
							}
							continue;
						}
					} else { // dft_gpio->event[n].rule.edge_time_sec_end == 0
						/* Check AV main ready or not */
						if (strcmp(ctx->name,
						           k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_IN].name) ==
						    0) {
							syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name,
							       n);
							handleSpAdvImgPref(&g_adv_img_ctx.pref, action_cb[n],
							                   dft_gpio->event[n].action_args,
							                   DAY_NIGHT_FILE_PATH);

							action_cb[n](dft_gpio->event[n].action_args, NULL);
							continue;
						}
					}
					if (edge_trig_flag == 0 || edge_trig_on == 0) {
						edge_trig_on = 1;
					}
				} else {
					continue;
				}
				break;
			default:
				break;
			}
		}

		prev_v = curr_v;
		usleep(polling_period_usec);
	}

	__exit_share_mem__();
	return NULL;
}

static void *EVENT_SW_daemon(void *arg)
{
	EVENT_CTX_S *ctx;
	AGTX_SW_EVENT_LIST_S *dft_sw;
	AGTX_SW_EVENT_RULE_S curr_child_rule;
	AGTX_EVENT_ACTION_CB action_cb[MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE];
	AGTX_BOOL detection_status;

	int listen_fd;
	int child_fd;
	int read_ret;
	int ret;
	int n;
	int is_child_fd_exist;
	int cnt = 0;

	detection_status = AGTX_TRUE;
	ctx = (EVENT_CTX_S *)arg;
	dft_sw = &ctx->group.sw;
	ret = 0;

	for (n = 0; n < MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE; n++) {
		action_cb[n] = *(findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1,
		                                         dft_sw->event[n].action));
	}

	listen_fd = createServerListen(ctx->socket_path);

	EVT_TRACE("listen_fd = %d\n", listen_fd);

	if (listen_fd < 0) {
		EVT_ERR("createServerListen() failure (listen_fd = %d)!\n", listen_fd);
		close(listen_fd);
		ret = -1;
	}

	EVT_TRACE("Ready to detect and handle event(%d, %s).\n", ctx->id, ctx->name);
	while (!ret) {
		EVT_TRACE("waitServerAccept()...\n");
		child_fd = waitServerAccept(listen_fd);

		EVT_TRACE("child_fd = %d\n", child_fd);

		if (child_fd == SW_FAILURE) {
			EVT_ERR("waitServerAccept() failure (child_fd = %d)!\n", child_fd);
			close(child_fd);
			ret = -1;
		}

		is_child_fd_exist = 0;

		while (!is_child_fd_exist) {
			/*********************************/
			/* check enabled or not */
			/*********************************/
			pthread_mutex_lock(&ctx->lock);
			detection_status = ((ctx->attr.enabled || ctx->group.always_enabled) == 0) ? AGTX_FALSE :
			                                                                             AGTX_TRUE;
			pthread_mutex_unlock(&ctx->lock);

			if (detection_status == AGTX_FALSE) {
				usleep(2000000);
				continue;
			}

			EVT_TRACE("[Loop: %d] read()...\n", cnt);
			read_ret = read(child_fd, &curr_child_rule, sizeof(curr_child_rule));

			if (read_ret == 0) {
				close(child_fd);
				is_child_fd_exist = -1;
				continue;
			}

			if (read_ret < 0) {
				EVT_ERR("read() failure!\n");
				close(child_fd);
				is_child_fd_exist = -1;
				break;
			}

			if (read_ret == sizeof(curr_child_rule)) {
				syslog(LOG_INFO, "[%d , %s] Recieve message: %s\n", ctx->id, ctx->name,
				       k_sw_trig_alias_table[curr_child_rule.trigger_type].name);
			}

			for (n = 0; n < MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE; n++) {
				if (curr_child_rule.trigger_type == dft_sw->event[n].rule.trigger_type &&
				    strcmp((char *)dft_sw->event[n].action_args, "") != 0) {
					syslog(LOG_INFO, "%s is triggered by rule #[%d]: '%s'.\n", ctx->name, n,
					       k_sw_trig_alias_table[curr_child_rule.trigger_type].name);

					if (strcmp(ctx->name, "IVA_MD") == 0) {
						setLEDInform("Motion_Detected", 1);
					} else {
						action_cb[n](dft_sw->event[n].action_args, NULL);
					}

				} else {
					continue;
				}
			}

			cnt++;
		}
	}

	close(listen_fd);

	return NULL;
}

static void *EVENT_LED_daemon(void *arg)
{
	EVENT_CTX_S *ctx;
	AGTX_LED_EVENT_LIST_S *dft_led;
	AGTX_EVENT_ACTION_CB action_cb[MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE];
	AGTX_BOOL curr_detection_status;

	AGTX_INT32 led0_gpio;
	AGTX_INT32 led1_gpio;

	curr_detection_status = AGTX_TRUE;
	int curr_state = 0;
	int prev_state = 0;
	char *curr_client = calloc(sizeof(char), LED_CLIENT_LENGTH);
	char *prev_client = calloc(sizeof(char), LED_CLIENT_LENGTH);
	pthread_mutex_init(&g_levt.lock, NULL);
	int ret = 0;
	int t_ret = 0;
	int update = AGTX_LED_TYPE_STATUS_NOT_CHANGE;
	int update_client_list = AGTX_LED_CLIENT_LIST_OFF;
	int n;
	int k;
	pthread_t t;
	int err = 0;
	Gpio gpio;
	int critical_flag = 0;
	int system_update_flag = 0;
	int led_in_used = 0;

	struct sockaddr_un serun, cliun;
	socklen_t cliun_len;
	int listenfd, connfd, size;
	char buf[LED_CLIENT_LENGTH];
	char *led_socket_path = LED_SERVER_SOCKET_PATH;

	ctx = (EVENT_CTX_S *)arg;
	dft_led = &ctx->group.led;
	curr_levt.polling_period_usec = ctx->group.led.polling_period_usec;

	for (n = 0; n < MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE; n++) {
		action_cb[n] = *(findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1,
		                                         dft_led->event[n].action));
	}

	/*handle init status*/
	led0_gpio = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "LED0_OUT");
	led1_gpio = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "LED1_OUT");

	curr_levt.led[0].pin = led0_gpio;
	curr_levt.led[1].pin = led1_gpio;
	strcpy(curr_levt.led[0].pin_name, "LED0_OUT");
	strcpy(curr_levt.led[1].pin_name, "LED1_OUT");
	curr_levt.led_init_list.pin = ctx->group.led.init_light_on_pin_num;
	curr_levt.led_init_list.value = ctx->group.led.init_light_on_value;
	if (ctx->group.led.trigger_type_level == 0 || ctx->group.led.trigger_type_level == 1) {
		curr_levt.led_init_list.trigger_type_level = ctx->group.led.trigger_type_level;
	} else {
		/*If trigger_type undefined, set default value*/
		curr_levt.led_init_list.trigger_type_level = 1;
	}

	/*Set initial led on*/
	if (curr_levt.led_init_list.pin >= 0) {
		for (n = 0; n < LED_PIN_NUM_SIZE; n++) {
			if (curr_levt.led_init_list.pin == curr_levt.led[n].pin) {
				gpio.id = curr_levt.led_init_list.pin;
				gpio.direction = GPIO_OUT;
				setGpioValue(&gpio, curr_levt.led_init_list.value);
			}
		}
	}

	EVT_TRACE("led0_gpio=%d, led1_gpio=%d.\n", curr_levt.led[0].pin, curr_levt.led[1].pin);
	EVT_TRACE("LED0_OUT_name=%s, LED1_OUT_name=%s.\n", curr_levt.led[0].pin_name, curr_levt.led[1].pin_name);

	/*Initialize LED client list*/
	for (n = 0; n < LED_CLIENT_LIST_SIZE; n++) {
		strncpy(curr_levt.led_client_list[n].client, "NONE", LED_CLIENT_LENGTH);
		curr_levt.led_client_list[n].enabled = 0;
	}

	/*Create LED client socket*/
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		syslog(LOG_INFO, "LED Daemon: socket error");
		perror("LED Daemon: socket error");
	}
	memset(&serun, 0, sizeof(serun));
	serun.sun_family = AF_UNIX;
	strcpy(serun.sun_path, led_socket_path);
	size = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);
	unlink(led_socket_path);
	if (bind(listenfd, (struct sockaddr *)&serun, size) < 0) {
		syslog(LOG_INFO, "LED Daemon: bind error");
		perror("LED Daemon: bind error");
	}
	syslog(LOG_INFO, "LED client socket bound!\n");

	if (listen(listenfd, 20) < 0) {
		syslog(LOG_INFO, "LED Daemon: listen error");
		perror("LED Daemon: listen error");
	}
	syslog(LOG_INFO, "LED client: Accepting connections ...\n");

	//****************
	EVT_TRACE("[%d, %s] curr_led = %s (init status)\n", ctx->id, ctx->name, curr_client);
	//****************

	/* led event detection and handling */
	EVT_TRACE("attr.enabled = %d, always_enabled = %d.\n", ctx->attr.enabled, ctx->group.always_enabled);

	while (!ret) {
		/*********************************/
		/* check enabled or not */
		/*********************************/
		pthread_mutex_lock(&ctx->lock);
		curr_detection_status = ((ctx->attr.enabled || ctx->group.always_enabled) == 0) ? AGTX_FALSE :
		                                                                                  AGTX_TRUE;
		pthread_mutex_unlock(&ctx->lock);

		EVT_TRACE("%s ctx->attr.enabled %d\n", ctx->name, ctx->attr.enabled);
		EVT_TRACE("%s ctx->group.always_enabled %d\n", ctx->name, ctx->group.always_enabled);

		if (curr_detection_status == AGTX_FALSE) {
			usleep(2000000);
			continue;
		}

		/*Get old client from LED client list*/
		if (update_client_list != AGTX_LED_CLIENT_LIST_OFF) {
			for (n = 0; n < LED_CLIENT_LIST_SIZE; n++) {
				if (strcmp(curr_levt.led_client_list[n].client, "NONE") == 0) {
					if (n == 0) {
						update_client_list = AGTX_LED_CLIENT_LIST_RESET;
						break;
					}
					setLEDInform((char *)(&curr_levt.led_client_list[n - 1].client),
					             curr_levt.led_client_list[n - 1].enabled);
					break;
				}
			}
		}

		/*Get LED client socket*/
		cliun_len = sizeof(cliun);
		if ((connfd = accept(listenfd, (struct sockaddr *)&cliun, &cliun_len)) < 0) {
			syslog(LOG_INFO, "LED Daemon: accept error");
			continue;
		} else {
			while (1) {
				n = read(connfd, buf, sizeof(buf));
				if (n < 0) {
					syslog(LOG_INFO, "LED Daemon: read error");
					break;
				} else if (n == 0) {
					syslog(LOG_INFO, "LED Daemon: EOF\n");
					break;
				}
				strncpy(curr_client, (const char *)(&buf), LED_CLIENT_LENGTH);
				curr_state = getLEDInform(&curr_client);
				break;
			}
			close(connfd);
		}

		if (curr_state < 0) {
			usleep(curr_levt.polling_period_usec);
			continue;
		}

		for (n = 0; n < MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE; n++) {
			if ((strcmp(curr_client, k_led_trig_alias_table[dft_led->event[n].rule.trigger_type].name) ==
			     0) &&
			    (dft_led->event[n].in_use == 0)) {
				syslog(LOG_INFO, "%s is un-used #[%d]: '%s'.\n", ctx->name, n,
				       k_led_trig_alias_table[dft_led->event[n].rule.trigger_type].name);
				led_in_used = 1;
				break;
			}
		}
		if (led_in_used) {
			led_in_used = 0;
			continue;
		}

		if (strcmp(prev_client, curr_client) == 0) {
			if (curr_state == prev_state) {
				update = AGTX_LED_TYPE_STATUS_NOT_CHANGE;
			} else {
				if (update_client_list == AGTX_LED_CLIENT_LIST_RESET) {
					update = AGTX_LED_TYPE_CLIENT_CHANGE;
				} else {
					update = AGTX_LED_TYPE_STATE_CHANGE;
				}
			}
		} else {
			if (curr_state == 1) {
				update = AGTX_LED_TYPE_CLIENT_CHANGE;
			} else {
				update = AGTX_LED_TYPE_STATUS_NOT_CHANGE;
			}
		}

		/* Update current state */
		for (n = 0; n < LED_CLIENT_LIST_SIZE; n++) {
			if (strcmp(curr_client, curr_levt.led_client_list[n].client) == 0) {
				curr_levt.led_client_list[n].enabled = curr_state;
				break;
			}
		}

		if (update != AGTX_LED_TYPE_STATUS_NOT_CHANGE) {
			/*When Critical error occur, limit the led client list*/
			if (critical_flag == 0) {
				if (strcmp(curr_client, "Critical_Error") == 0) {
					critical_flag = 1;
				}
			}
			if (critical_flag == 1) {
				for (n = 0; n < AGTX_LED_EVENT_CRITICAL_TYPE_NUM; n++) {
					if (strcmp(curr_client, k_led_critical_alias_table[n].name) == 0) {
						if ((strcmp(prev_client, curr_client) == 0) &&
						    (curr_state != prev_state)) {
							update = AGTX_LED_TYPE_STATE_CHANGE;
						} else {
							update = AGTX_LED_TYPE_CLIENT_CHANGE;
						}
						break;
					} else {
						update = AGTX_LED_TYPE_STATUS_NOT_CHANGE;
					}
				}
			}

			/*When system update, ignore other led clients except critical error*/
			if (system_update_flag == 0) {
				if (strcmp(curr_client, "OTA") == 0 || strcmp(curr_client, "Card_Upgrade") == 0) {
					system_update_flag = 1;
				}
			} else {
				if (strcmp(curr_client, "Critical_Error") == 0) {
					update = AGTX_LED_TYPE_CLIENT_CHANGE;
				} else if (strcmp(curr_client, "OTA") == 0 && curr_state == 0) {
					system_update_flag = 0;
				} else if (strcmp(curr_client, "Card_Upgrade") == 0 && curr_state == 0) {
					system_update_flag = 0;
				} else {
					update = AGTX_LED_TYPE_STATUS_NOT_CHANGE;
				}
			}
		}

		/* Insert new client */
		if (update == AGTX_LED_TYPE_CLIENT_CHANGE || strcmp(curr_levt.led_client_list[0].client, "NONE") == 0) {
			if (update_client_list != AGTX_LED_CLIENT_LIST_ON) {
				for (n = 0; n < LED_CLIENT_LIST_SIZE; n++) {
					if (curr_state == 0) {
						break;
					}
					if (strcmp(curr_levt.led_client_list[n].client, "NONE") == 0) {
						strncpy(curr_levt.led_client_list[n].client, curr_client,
						        LED_CLIENT_LENGTH);
						curr_levt.led_client_list[n].enabled = curr_state;
						break;
					} else if (strcmp(curr_levt.led_client_list[n].client, curr_client) == 0) {
						for (k = n + 1; k < LED_CLIENT_LIST_SIZE; k++) {
							if (strcmp(curr_levt.led_client_list[k].client, "NONE") == 0) {
								strncpy(curr_levt.led_client_list[k].client,
								        curr_client, LED_CLIENT_LENGTH);
								curr_levt.led_client_list[k].enabled = curr_state;
								curr_levt.led_client_list[n].enabled = 0;
								break;
							}
						}
						break;
					}
				}
			}
			update_client_list = AGTX_LED_CLIENT_LIST_OFF;
		}

		/* Full up the hole */
		for (n = 0; n < LED_CLIENT_LIST_SIZE; n++) {
			if (curr_levt.led_client_list[n].enabled == 0) {
				for (k = n; k < LED_CLIENT_LIST_SIZE - 1; k++) {
					strncpy(curr_levt.led_client_list[k].client,
					        curr_levt.led_client_list[k + 1].client, LED_CLIENT_LENGTH);
					curr_levt.led_client_list[k].enabled = curr_levt.led_client_list[k + 1].enabled;
					if (k == LED_CLIENT_LIST_SIZE) {
						strncpy(curr_levt.led_client_list[k].client, "NONE", LED_CLIENT_LENGTH);
						curr_levt.led_client_list[k].enabled = 0;
					}
					if (strcmp(curr_levt.led_client_list[k].client, "NONE") == 0) {
						break;
					}
				}
			}
		}

		/*Handle the client update*/
		if (update != AGTX_LED_TYPE_STATUS_NOT_CHANGE) {
			prev_state = curr_state;
			strncpy(prev_client, curr_client, LED_CLIENT_LENGTH);
			curr_levt.clear_flag = 1;
			curr_levt.enabled = 0;
			if (update == AGTX_LED_TYPE_STATE_CHANGE) {
				update_client_list = AGTX_LED_CLIENT_LIST_ON;
			}

			for (n = 0; n < MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE; n++) {
				if ((strcmp(curr_client,
				            k_led_trig_alias_table[dft_led->event[n].rule.trigger_type].name) == 0) &&
				    strcmp((char *)dft_led->event[n].action_args, "") != 0) {
					syslog(LOG_INFO, "%s is triggered by rule #[%d]: '%s', Status: %d.\n",
					       ctx->name, n,
					       k_led_trig_alias_table[dft_led->event[n].rule.trigger_type].name,
					       curr_state);

					curr_levt.pin[0] = 0;
					curr_levt.pin[1] = 0;
					action_cb[n](dft_led->event[n].action_args, (void *)&curr_levt);
					curr_levt.enabled = curr_state;
					pthread_mutex_lock(&g_levt.lock);
					curr_levt.led_switch_enabled = &ctx->attr.enabled;
					strncpy(curr_levt.curr_client, curr_client, LED_CLIENT_LENGTH);
					strncpy(curr_levt.prev_client, prev_client, LED_CLIENT_LENGTH);
					memcpy(&g_levt, &curr_levt, sizeof(curr_levt));
					pthread_mutex_unlock(&g_levt.lock);

					if (t_ret == 0) {
						ret = pthread_create(&t, NULL, handleLEDaction, (void *)&g_levt);
						if (ret != 0) {
							EVT_ERR("Failed to create EVENT_LED_SUB_daemon(%d)!\n", n);
							syslog(LOG_INFO, "Failed to create EVENT_LED_SUB_daemon(%d)!\n",
							       n);
						}
						t_ret = 1;
						err = pthread_setname_np(t, "EVENT_LED_SUB");
						if (err != 0) {
							syslog(LOG_INFO,
							       "Create object detection thread to config failed. err = %d\n",
							       err);
						}
						syslog(LOG_INFO, "Create EVENT_LED_SUB_daemon(%d, %s).\n", n,
						       curr_client);
					}
					EVT_TRACE("%s, curr_client:%s, curr_state:%d\n", ctx->name, curr_client,
					          curr_state);
				} else {
					continue;
				}
				syslog(LOG_INFO, "Change client EVENT_LED_SUB_daemon(%d, %s).\n", n, curr_client);
			}
			EVT_TRACE("%s, Check update: curr_client:%s, curr_state:%d\n", ctx->name, curr_client,
			          curr_state);
		}
		// usleep(curr_levt.polling_period_usec);
	}
	close(listenfd);
	free(curr_client);
	free(prev_client);

	ret = pthread_join(t, NULL);

	if (ret == 0) {
		syslog(LOG_INFO, "Join event EVENT_LED_SUB_daemon.\n");
	} else {
		EVT_ERR("Failed to join event EVENT_LED_SUB_daemon!\n");
	}

	return NULL;
}

static void *EVENT_EINTC_PIR_daemon(void *arg)
{
	EVENT_CTX_S *ctx;
	AGTX_EINTC_EVENT_LIST_S *dft_pir;
	AGTX_EVENT_ACTION_CB action_cb[MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE];
	AGTX_EINTC_EVENT_RULE_S curr_child_rule;

	AGTX_BOOL curr_detection_status;
	curr_detection_status = AGTX_TRUE;

	int fd;
	int ret = 0;
	int *stat = calloc(sizeof(int), 4);
	int n;

	ctx = (EVENT_CTX_S *)arg;
	dft_pir = &ctx->group.eintc;
	curr_child_rule.trigger_type = AGTX_EINTC_EVENT_TRIG_TYPE_EINTC_PIR_NEGATIVE;

	for (n = 0; n < MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE; n++) {
		action_cb[n] = *(findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1,
		                                         dft_pir->event[n].action));
	}

	/*Open PIR character device*/

	fd = open(ctx->device_path, O_RDWR);
	if (fd < 0) {
		syslog(LOG_INFO, "Can't not open PIR character device: %s.", ctx->device_path);
		return NULL;
	}

	if (ioctl(fd, IOC_PIR_SETTIMEOUT, &ctx->group.eintc.jiffies_timeout) < 0) {
		perror("failed to set timeout.");
	}

	if (ioctl(fd, IOC_PIR_SENSITIVITY, &ctx->group.eintc.sensitivity) < 0) {
		perror("failed to set timeout.");
	}

	while (!ret) {
		/*********************************/
		/* check enabled or not */
		/*********************************/
		pthread_mutex_lock(&ctx->lock);
		curr_detection_status = ((ctx->attr.enabled || ctx->group.always_enabled) == 0) ? AGTX_FALSE :
		                                                                                  AGTX_TRUE;
		pthread_mutex_unlock(&ctx->lock);

		EVT_TRACE("%s ctx->attr.enabled %d\n", ctx->name, ctx->attr.enabled);
		EVT_TRACE("%s ctx->group.always_enabled %d\n", ctx->name, ctx->group.always_enabled);

		if (curr_detection_status == AGTX_FALSE) {
			usleep(2000000);
			continue;
		}

		/*Get external irq status*/
		if (read(fd, stat, sizeof(stat)) < 0) {
			perror("failed to get pir status.");
		}

		if (*stat == 0) {
			/* pir timeout */
			curr_child_rule.trigger_type = AGTX_EINTC_EVENT_TRIG_TYPE_EINTC_PIR_NEGATIVE;
			continue;
		} else if (*stat > 0) {
			/* pir detect */
			curr_child_rule.trigger_type = AGTX_EINTC_EVENT_TRIG_TYPE_EINTC_PIR_POSITIVE;
		}

		for (n = 0; n < MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE; n++) {
			if (curr_child_rule.trigger_type == dft_pir->event[n].rule.trigger_type &&
			    strcmp((char *)dft_pir->event[n].action_args, "") != 0) {
				syslog(LOG_INFO, "%s is triggered by rule #[%d]: '%s'.\n", ctx->name, n,
				       k_eintc_pir_trig_alias_table[curr_child_rule.trigger_type].name);

				action_cb[n](dft_pir->event[n].action_args, NULL);

			} else {
				continue;
			}
		}
	}

	close(fd);
	return NULL;
}

int getMedian(int n, int x[])
{
	int temp;
	int i, j;
	// the following two loops sort the array x in ascending order
	for (i = 0; i < n - 1; i++) {
		for (j = i + 1; j < n; j++) {
			if (x[j] < x[i]) {
				// swap elements
				temp = x[i];
				x[i] = x[j];
				x[j] = temp;
			}
		}
	}

	if (n % 2 == 0) {
		// if there is an even number of elements, return mean of the two elements in the middle
		return ((x[n >> 1] + x[(n >> 1) - 1] + 1) >> 1);
	} else {
		// else return the element in the middle
		return x[n >> 1];
	}
}

static void *EVENT_ADC_daemon(void *arg)
{
	EVENT_CTX_S *ctx;
	AGTX_ADC_EVENT_LIST_S *dft_adc;
	AGTX_EVENT_ACTION_CB action_cb[MAX_AGTX_ADC_EVENT_LIST_S_EVENT_SIZE];
	AGTX_BOOL curr_detection_status;
	AGTX_BOOL prev_detection_status;
	AGTX_EVENT_ACTION_CB exec_cmd_cb[0];

	int curr_adc;
	int prev_adc;
	int polling_period_usec;
	int day_th;
	int night_th;
	int ret;
	int curr_state;
	int prev_state;
	int n;
	curr_detection_status = AGTX_TRUE;
	prev_detection_status = AGTX_TRUE;
	ctx = (EVENT_CTX_S *)arg;
	dft_adc = &ctx->group.adc;
	polling_period_usec = ctx->group.adc.polling_period_usec;
	ret = 0;
	prev_adc = ctx->dft_adc_value;

	if ((dft_adc->event[0].rule.trigger_type != AGTX_ADC_EVENT_TRIG_TYPE_HYS) &&
	    dft_adc->event[1].rule.trigger_type != AGTX_ADC_EVENT_TRIG_TYPE_HYS) {
		EVT_ERR("ADC_EVENT_TRIG_TYPE only support HYS type!, rule[0] = %d, rule[1] "
		        "= %d\n",
		        dft_adc->event[0].rule.trigger_type, dft_adc->event[1].rule.trigger_type);
		return NULL;
	}

	EVT_TRACE("[%d, %s] prev_adc = %d \n", ctx->id, ctx->name, prev_adc);

	for (n = 0; n < MAX_AGTX_ADC_EVENT_LIST_S_EVENT_SIZE; n++) {
		action_cb[n] = *(findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1,
		                                         dft_adc->event[n].action));
	}

	exec_cmd_cb[0] = *(
	        findFucfromCbAliasTable(k_cb_alias_table, AGTX_EVENT_ACTION_CB_NUM + 1, AGTX_EVENT_ACTION_CB_EXEC_CMD));

	EVT_TRACE("%s(): Ready to get ADC value on chn : %d (%s)\n", __func__, dft_adc->chn, ctx->adc_path);

	if (g_light_sensor_ctx.light_sensor_conf.mode == AGTX_LIGHT_SENSOR_MODE_ADC) {
		night_th = g_light_sensor_ctx.light_sensor_conf.adc.night_th;
		day_th = g_light_sensor_ctx.light_sensor_conf.adc.day_th;
	} else {
		night_th = dft_adc->event[0].rule.hys_th;
		day_th = dft_adc->event[1].rule.hys_th;
	}

	/* handle init hys */
	getAdcValue(ctx->adc_path, &curr_adc);

	if (curr_adc <= night_th && prev_adc < night_th) {
		curr_state = 0;
		prev_state = 0;
	} else {
		curr_state = 1;
		prev_state = 1;
	}

	EVT_TRACE("[%d, %s] curr_adc = %d (init hys)\n", ctx->id, ctx->name, curr_adc);

	if (strcmp((char *)dft_adc->init_hys[n].action_args, "")) {
		syslog(LOG_INFO, "%s is triggered by rule #[%d] (init hys).\n", ctx->name, curr_state);

		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_ADC].name) == 0) {
			handleSpAdvImgPref(&g_adv_img_ctx.pref, exec_cmd_cb[0],
			                   dft_adc->init_hys[curr_state].action_args, DAY_NIGHT_FILE_PATH);
			prev_state = curr_state;
		}
	}

	/* adc event detection and handling */
	EVT_TRACE("Ready to detect and handle event(%d, %s).\n", ctx->id, ctx->name);
	while (!ret) {
		prev_detection_status = curr_detection_status;

		if (g_light_sensor_ctx.is_updated == AGTX_TRUE) {
			pthread_mutex_lock(&g_light_sensor_ctx.lock);
			if (g_light_sensor_ctx.light_sensor_conf.mode == AGTX_LIGHT_SENSOR_MODE_ADC) {
				night_th = g_light_sensor_ctx.light_sensor_conf.adc.night_th;
				day_th = g_light_sensor_ctx.light_sensor_conf.adc.day_th;
			}
			pthread_mutex_unlock(&g_light_sensor_ctx.lock);
		}

		/*********************************/
		/* check enabled or not */
		/*********************************/
		pthread_mutex_lock(&ctx->lock);
		curr_detection_status = ((ctx->attr.enabled || ctx->group.always_enabled) == 0) ? AGTX_FALSE :
		                                                                                  AGTX_TRUE;
		pthread_mutex_unlock(&ctx->lock);

		EVT_TRACE("%s ctx->attr.enabled %d\n", ctx->name, ctx->attr.enabled);
		EVT_TRACE("%s ctx->group.always_enabled %d\n", ctx->name, ctx->group.always_enabled);

		if (curr_detection_status == AGTX_FALSE) {
			usleep(polling_period_usec);
			continue;
		}

		int i;
		int tmp_adc[GET_ADC_FREQ];
#if 1
		// obtaion curr_adc by median of adc value (5 times) to avoid impulse
		for (i = 0; i < GET_ADC_FREQ; i++) {
			getAdcValue(ctx->adc_path, &tmp_adc[i]);
		}

		curr_adc = getMedian(GET_ADC_FREQ, tmp_adc);
#else
		getAdcValue(ctx->adc_path, &curr_adc);
#endif

		// TO-DO lock
		if (curr_adc <= night_th) {
			curr_state = 0;
		} else if (curr_adc > day_th) {
			curr_state = 1;
		} else {
			// keep curr_state
		}

		if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_ADC].name) == 0) {
			if (curr_detection_status != prev_detection_status || g_adv_img_ctx.is_updated == AGTX_TRUE ||
			    g_light_sensor_ctx.is_updated == AGTX_TRUE) {
				syslog(LOG_INFO, "%s is triggered by rule #[%d] (status change).\n", ctx->name,
				       curr_state);

				handleSpAdvImgPref(&g_adv_img_ctx.pref, exec_cmd_cb[0],
				                   dft_adc->init_hys[curr_state].action_args, DAY_NIGHT_FILE_PATH);
				g_light_sensor_ctx.is_updated = AGTX_FALSE;
				prev_state = curr_state;
			}
		}

		EVT_TRACE(
		        "[%d, %s] night_th = %4d, day_th = %4d, prev_adc = %4d, curr_adc = %4d ( %4d, %4d, %4d, %4d, %4d).\n",
		        ctx->id, ctx->name, night_th, day_th, prev_adc, curr_adc, tmp_adc[0], tmp_adc[1], tmp_adc[2],
		        tmp_adc[3], tmp_adc[4]);

		if (curr_state != prev_state) {
			if (strcmp(ctx->name, k_event_alias_table[AGTX_EVENT_NAME_LIGHT_SENSOR_ADC].name) == 0) {
				syslog(LOG_INFO, "%s is triggered by rule #[%d].\n", ctx->name, curr_state);
				syslog(LOG_INFO,
				       "[%d, %s] th0 = %4d, th1 = %4d, prev_adc = %4d, curr_adc = %4d ( %4d, %4d, %4d, %4d, %4d).\n",
				       ctx->id, ctx->name, night_th, day_th, prev_adc, curr_adc, tmp_adc[0], tmp_adc[1],
				       tmp_adc[2], tmp_adc[3], tmp_adc[4]);
				handleSpAdvImgPref(&g_adv_img_ctx.pref, action_cb[curr_state],
				                   dft_adc->event[curr_state].action_args, DAY_NIGHT_FILE_PATH);
			} else {
				EVT_ERR("Only support LIGHT_SENSOR_ADC event\n");
				return NULL;
			}
		}

		prev_adc = curr_adc;
		prev_state = curr_state;
		usleep(polling_period_usec);
	}

	return NULL;
}

static int handleSpAdvImgPref(const AGTX_ADV_IMG_PREF_S *adv_img_pref, AGTX_EVENT_ACTION_CB action_cb,
                              void *action_args, const char *DAY_NIGHT_PATH)
{
	int cnt = 0;

	/*get image WEB setting*/
	g_day_night_attr.img_pref = *adv_img_pref;

	FILE *file = fopen(DAY_NIGHT_PATH, "w+");
	if (file == 0) {
		syslog(LOG_INFO, "Light sensor setting failure.");
	}

	else {
		fwrite(&g_day_night_attr, sizeof(DAY_NIGHT_ATTR_S), 1, file);
		while (fflush(file) != 0) {
			syslog(LOG_INFO, "Try to set light Sensor data again...");
		}
		syslog(LOG_INFO, "Light Sensor data setting success.");
	}
	fclose(file);

	g_adv_img_ctx.is_updated = AGTX_FALSE;
	while (action_cb(action_args, NULL) != 0) {
		cnt++;
		if (cnt > 5) {
			EVT_ERR("Calling DAY_NIGHT_MODE fail!\n");
			break;
		}
	}

	return 0;
}

static int handlePirDatabase(AGTX_PIR_CONF_S *pir, AGTX_FLOODLIGHT_CONF_S *floodlight, AGTX_COLOR_CONF_S *color_conf)
{
	int n;
	int ret = FLOODLIGHT_NONE;

	if (g_pir_ctx.is_updated == AGTX_TRUE) {
		pthread_mutex_lock(&g_pir_ctx.lock);
		memcpy(pir, &g_pir_ctx.pir_conf, sizeof(AGTX_PIR_CONF_S));
		g_pir_ctx.is_updated = AGTX_FALSE;
		pthread_mutex_unlock(&g_pir_ctx.lock);

		for (n = 0; n < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; n++) {
			if (g_pir_ctx.pwm[n].enabled == AGTX_PIR_ENABLED_ON) {
				setPWMDutyCycle(&g_pir_ctx.pwm[n], pir->duty_cycle);
			}
		}
		for (n = 0; n < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; n++) {
			syslog(LOG_INFO,
			       "PIR Daemon: Get pir conf from CC, pir_%d(enabled = %d, "
			       "%s), duty_cycle = %d.\n",
			       n, pir->pir_alias[n].enabled, pir->pir_alias[n].name, pir->duty_cycle);
		}
		ret = PIR_ACTION_MODE;
	}
	if (g_floodlight_ctx.is_updated == AGTX_TRUE) {
		pthread_mutex_lock(&g_floodlight_ctx.lock);
		if (floodlight->enabled != g_floodlight_ctx.floodlight_conf.enabled) {
			ret = FLOODLIGHT_ACTION_MODE;
		}
		if (floodlight->lightness != g_floodlight_ctx.floodlight_conf.lightness) {
			if (g_floodlight_ctx.floodlight_conf.enabled) {
				ret = FLOODLIGHT_ACTION_MODE;
			} else {
				ret = PIR_ACTION_MODE;
			}
		}
		memcpy(floodlight, &g_floodlight_ctx.floodlight_conf, sizeof(AGTX_FLOODLIGHT_CONF_S));
		g_floodlight_ctx.is_updated = AGTX_FALSE;
		pthread_mutex_unlock(&g_floodlight_ctx.lock);

		syslog(LOG_INFO,
		       "PIR Daemon: Get floodlight conf from CC, enabled = "
		       "%d, lightness = %d, bright_mode = %d, warn_switch = "
		       "%d, warn_time = %d.\n",
		       floodlight->enabled, floodlight->lightness, floodlight->bright_mode, floodlight->warn_switch,
		       floodlight->warn_time);
		if (ret != FLOODLIGHT_ACTION_MODE)
			ret = PIR_ACTION_MODE;
	}
	if (g_color_mode_ctx.is_updated == AGTX_TRUE) {
		pthread_mutex_lock(&g_color_mode_ctx.lock);
		memcpy(color_conf, &g_color_mode_ctx.color_mode_conf, sizeof(AGTX_COLOR_CONF_S));
		g_color_mode_ctx.is_updated = AGTX_FALSE;
		pthread_mutex_unlock(&g_color_mode_ctx.lock);

		syslog(LOG_INFO, "Color mode = %d.\n", g_color_mode_ctx.color_mode_conf.color_mode);
	}

	return ret;
}

static int handleButtonTime(struct timeval *curr_tv, struct timeval trig_tv, int *button_state, int *button_reset,
                            int level_time_sec)
{
	int time_tr_sec = 0;
	int time_tr_usec = 0;

	if (curr_tv->tv_sec != 0) {
		time_tr_sec = trig_tv.tv_sec - curr_tv->tv_sec;
		time_tr_usec = trig_tv.tv_usec - curr_tv->tv_usec;
		if (time_tr_usec < 0) {
			time_tr_usec = -time_tr_usec;
			time_tr_sec -= 1;
		}
		time_tr_sec = (level_time_sec - time_tr_sec) * 1000000 - time_tr_usec;

		if (time_tr_usec != 0) {
			*button_state = AGTX_BUTTON_PRESS_TYPE_PRESSING;
		}
	} else {
		time_tr_sec = 0;
	}
	if (*button_state == AGTX_BUTTON_PRESS_TYPE_PRESSING || *button_reset != 0) {
		*button_state = handleButtonState(time_tr_sec, level_time_sec * 1000000, curr_levt.curr_client,
		                                  curr_levt.prev_client, *button_reset);
		if (*button_state == AGTX_BUTTON_PRESS_TYPE_NONE && *button_reset != AGTX_BUTTON_RESET_TYPE_OFF) {
			*button_reset -= 1;
		} else if (*button_state == AGTX_BUTTON_PRESS_TYPE_PRESSING) {
			*button_reset = AGTX_BUTTON_RESET_TYPE_ON;
		} else if (*button_state == AGTX_BUTTON_PRESS_TYPE_RELEASE) {
			*button_reset = AGTX_BUTTON_RESET_TYPE_OFF;
		}
	}

	return time_tr_sec;
}

static void handlePirAction(FLOODLIGHT_CTX_S *floodlight, AGTX_PIR_CONF_S *pir, AGTX_COLOR_CONF_S *color_conf,
                            struct timeval *curr_tv, struct timeval *stop_tv, int *pir_trig)
{
	int tmp_time_sec;
	int duty_cycle = 0;
	int i, pir_enabled = 0;

	switch (*pir_trig) {
	case PIR_TRIGGER_MODE:
		if (floodlight->floodlight_conf.warn_switch) {
			/* handle floodlight by pir trigger */
			gettimeofday(curr_tv, NULL);
			tmp_time_sec = floodlight->floodlight_conf.warn_time;

			if (color_conf->color_mode == AGTX_COLOR_MODE_NIGHT) {
				if (stop_tv[1].tv_sec == 0 && stop_tv[1].tv_usec == 0) {
					duty_cycle =
					        (int)(pow(0.786 * floodlight->floodlight_conf.lightness, 2) + 1150);
					setPWMDutyCycle(&floodlight->pwm, duty_cycle);
					enabledPWM(&floodlight->pwm, 1);
				}
				stop_tv[0].tv_sec = curr_tv->tv_sec;
				stop_tv[0].tv_usec = curr_tv->tv_usec;
				tmp_time_sec = (tmp_time_sec > 0) ? tmp_time_sec : -tmp_time_sec;
				stop_tv[1].tv_sec = curr_tv->tv_sec + tmp_time_sec;
				stop_tv[1].tv_usec = curr_tv->tv_usec;
			}
		} else {
			enabledPWM(&floodlight->pwm, 0);
			if (stop_tv[1].tv_sec != 0 || stop_tv[1].tv_usec != 0) {
				memset(curr_tv, 0, sizeof(struct timeval));
				memset(stop_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
			}
		}
		break;

	case PIR_ACTION_MODE:
		for (i = 0; i < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; i++) {
			pir_enabled = pir_enabled || pir->pir_alias[i].enabled;
		}
		if (!pir_enabled) {
			if (!floodlight->floodlight_conf.enabled) {
				enabledPWM(&floodlight->pwm, 0);
				if (stop_tv[1].tv_sec != 0 || stop_tv[1].tv_usec != 0) {
					memset(curr_tv, 0, sizeof(struct timeval));
					memset(stop_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
				}
			}
		} else {
			duty_cycle = (int)(pow(0.786 * floodlight->floodlight_conf.lightness, 2) + 1150);
			setPWMDutyCycle(&floodlight->pwm, duty_cycle);
		}
		break;

	case FLOODLIGHT_ACTION_MODE:
		if (floodlight->floodlight_conf.enabled) {
			enabledPWM(&floodlight->pwm, floodlight->floodlight_conf.enabled);
			duty_cycle = (int)(pow(0.786 * floodlight->floodlight_conf.lightness, 2) + 1150);
			setPWMDutyCycle(&floodlight->pwm, duty_cycle);
		} else {
			enabledPWM(&floodlight->pwm, 0);
			if (stop_tv[1].tv_sec != 0 || stop_tv[1].tv_usec != 0) {
				memset(curr_tv, 0, sizeof(struct timeval));
				memset(stop_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
			}
		}
		break;

	case FLOODLIGHT_TRACING_MODE:
		/*Trace floodlight time out*/
		gettimeofday(curr_tv, NULL);
		if (timercmp(curr_tv, &stop_tv[1], >)) {
			if (floodlight->floodlight_conf.enabled) {
				duty_cycle = (int)(pow(0.786 * floodlight->floodlight_conf.lightness, 2) + 1150);
				setPWMDutyCycle(&floodlight->pwm, duty_cycle);
			} else {
				enabledPWM(&floodlight->pwm, 0);
			}
			memset(curr_tv, 0, sizeof(struct timeval));
			memset(stop_tv, 0, sizeof(struct timeval) * MAX_TRIG_RANGE);
		}
		break;
	}
	*pir_trig = FLOODLIGHT_TRACING_MODE;

	return;
}

static int EVENT_parseCtrlCmd(const int client_fd, const AGTX_MSG_HEADER_S *cmd_header)
{
	int ret = 0;
	int read_ret = 0;

	AGTX_ADV_IMG_PREF_S adv_img = { 0 };
	AGTX_EVENT_PARAM_S evt_param = { { { 0 } } };
	AGTX_PIR_CONF_S pir = { 0 };
	AGTX_FLOODLIGHT_CONF_S floodlight = { 0 };
	AGTX_LIGHT_SENSOR_CONF_S light_sensor;
	memset(&light_sensor, 0, sizeof(AGTX_LIGHT_SENSOR_CONF_S));
	AGTX_COLOR_CONF_S color_conf = { 0 };

	switch (cmd_header->cid) {
	case AGTX_CMD_EVT_PARAM:

		if (cmd_header->len != sizeof(AGTX_EVENT_PARAM_S)) {
			EVT_WARN("AGTX_EVENT_PARAM_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_EVENT_PARAM_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &evt_param, sizeof(AGTX_EVENT_PARAM_S));

		if (read_ret != sizeof(AGTX_EVENT_PARAM_S)) {
			EVT_WARN("Read size of AGTX_EVENT_PARAM_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		ret = EVENT_setEvtAttr(MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE, &evt_param.event_attr[0]);

		if (ret != 0) {
			EVT_WARN("Failed to set event attributes, ret = %d!\n", ret);
			break;
		}

		break;
	case AGTX_CMD_ADV_IMG_PREF:

		if (cmd_header->len != sizeof(AGTX_ADV_IMG_PREF_S)) {
			EVT_WARN("AGTX_ADV_IMG_PREF_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_ADV_IMG_PREF_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &adv_img, sizeof(AGTX_ADV_IMG_PREF_S));

		if (read_ret != sizeof(AGTX_ADV_IMG_PREF_S)) {
			EVT_WARN("Read size of AGTX_ADV_IMG_PREF_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		// checkVidExisted();
		memcpy(&g_adv_img_ctx.pref, &adv_img, sizeof(adv_img));

		break;
	case AGTX_CMD_PIR_CONF:

		if (cmd_header->len != sizeof(AGTX_PIR_CONF_S)) {
			EVT_WARN("AGTX_PIR_CONF_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_PIR_CONF_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &pir, sizeof(AGTX_PIR_CONF_S));

		if (read_ret != sizeof(AGTX_PIR_CONF_S)) {
			EVT_WARN("Read size of AGTX_PIR_CONF_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		pthread_mutex_lock(&g_pir_ctx.lock);
		memcpy(&g_pir_ctx.pir_conf, &pir, sizeof(pir));
		g_pir_ctx.is_updated = AGTX_TRUE;
		pthread_mutex_unlock(&g_pir_ctx.lock);

		break;
	case AGTX_CMD_FLOODLIGHT_CONF:

		if (cmd_header->len != sizeof(AGTX_FLOODLIGHT_CONF_S)) {
			EVT_WARN("AGTX_FLOODLIGHT_CONF_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_FLOODLIGHT_CONF_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &floodlight, sizeof(AGTX_FLOODLIGHT_CONF_S));

		if (read_ret != sizeof(AGTX_FLOODLIGHT_CONF_S)) {
			EVT_WARN("Read size of AGTX_FLOODLIGHT_CONF_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		pthread_mutex_lock(&g_floodlight_ctx.lock);
		memcpy(&g_floodlight_ctx.floodlight_conf, &floodlight, sizeof(floodlight));
		g_floodlight_ctx.is_updated = AGTX_TRUE;
		pthread_mutex_unlock(&g_floodlight_ctx.lock);

		break;
	case AGTX_CMD_LIGHT_SENSOR_CONF:

		if (cmd_header->len != sizeof(AGTX_LIGHT_SENSOR_CONF_S)) {
			EVT_WARN("AGTX_LIGHT_SENSOR_CONF_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_LIGHT_SENSOR_CONF_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &light_sensor, sizeof(AGTX_LIGHT_SENSOR_CONF_S));

		if (read_ret != sizeof(AGTX_LIGHT_SENSOR_CONF_S)) {
			EVT_WARN("Read size of AGTX_LIGHT_SENSOR_CONF_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		pthread_mutex_lock(&g_light_sensor_ctx.lock);
		memcpy(&g_light_sensor_ctx.light_sensor_conf, &light_sensor, sizeof(light_sensor));
		g_light_sensor_ctx.is_updated = AGTX_TRUE;
		pthread_mutex_unlock(&g_light_sensor_ctx.lock);

		break;
	case AGTX_CMD_COLOR_CONF:

		if (cmd_header->len != sizeof(AGTX_COLOR_CONF_S)) {
			EVT_WARN("AGTX_COLOR_CONF_S size doesn't match %d / %d\n", cmd_header->len,
			         sizeof(AGTX_COLOR_CONF_S));
			ret = -1;
			break;
		}

		read_ret = read(client_fd, &color_conf, sizeof(AGTX_COLOR_CONF_S));

		if (read_ret != sizeof(AGTX_COLOR_CONF_S)) {
			EVT_WARN("Read size of AGTX_COLOR_CONF_S(%d) error!\n", read_ret);
			ret = -1;
			break;
		}

		pthread_mutex_lock(&g_color_mode_ctx.lock);
		memcpy(&g_color_mode_ctx.color_mode_conf, &color_conf, sizeof(color_conf));
		g_color_mode_ctx.is_updated = AGTX_TRUE;
		pthread_mutex_unlock(&g_color_mode_ctx.lock);

		break;
	default:
		EVT_NOTICE("Unknown command!\n");
		break;
	}

	return ret;
}

static int EVENT_sendReplyCmd(const int client_fd, const AGTX_MSG_HEADER_S *cmd_reply, const int ack)
{
	int ret = 0;

	if (write(client_fd, cmd_reply, sizeof(*cmd_reply)) < 0) {
		EVT_ERR("Failed to send reply command!\n");
		ret = -1;
		return ret;
	}

	if (write(client_fd, &ack, sizeof(int)) < 0) {
		EVT_ERR("Failed to return ack!\n");
		ret = -1;
		return ret;
	}

	return ret;
}

static void *EVENT_CC_daemon(void *arg)
{
	int ret = 0;
	int read_ret = 0;
	int client_fd = (int)arg;
	int err_cnt = 0;
	fd_set read_fds;
	struct timeval tv = { 0 };

	AGTX_MSG_HEADER_S cmd_header = { 0 };
	AGTX_MSG_HEADER_S cmd_reply = { 0 };

	while (1) {
		tv.tv_sec = 0;
		tv.tv_usec = 1000000;

		FD_ZERO(&read_fds);
		FD_SET(client_fd, &read_fds);
		ret = select(client_fd + 1, &read_fds, NULL, NULL, &tv);

		if (ret < 0) {
			EVT_ERR("select() failure!\n");
			continue;
		} else if (ret == 0) {
			EVT_TRACE("CC_daemon timeout(ret = %d)!\n", ret);
			continue;
		} else {
			if (err_cnt > 5) {
				EVT_ERR("To many err_cnt on CC_daemon, close client_fd!\n");
				break;
			}

			read_ret = read(client_fd, &cmd_header, sizeof(cmd_header));

			if (read_ret < 0) {
				EVT_ERR("read() failure!\n");
				break;
			}

			if (read_ret != sizeof(cmd_header)) {
				err_cnt++;
				EVT_TRACE("Try to get new config from CC...(%d)!\n", err_cnt);
				continue;
			}

			err_cnt = 0;

			/* CC <-> ED */
			ret = EVENT_parseCtrlCmd(client_fd, &cmd_header);

			cmd_reply.cid = cmd_header.cid;
			cmd_reply.sid = 0;
			cmd_reply.len = sizeof(int);

			ret = EVENT_sendReplyCmd(client_fd, &cmd_reply, ret);

			/* ED <-> DN */
			if (cmd_header.cid == AGTX_CMD_ADV_IMG_PREF) {
				g_adv_img_ctx.is_updated = AGTX_TRUE;
			}
		}
	}

	close(client_fd);

	return NULL;
}

static int initAdvImgPref(const int client_fd)
{
	int ret;
	int get_ret;
	int err_cnt;
	AGTX_ADV_IMG_PREF_S dft_adv_img_pref = { 0 };

	ret = 0;
	get_ret = STATE_NOT_READY;
	err_cnt = 0;

	while (get_ret != STATE_SUCCESS) {
		if (err_cnt > 10) {
			EVT_ERR("Failed to get adv_img_pref many times(%d)!\n", err_cnt);
			ret = -1;
			continue;
		}

		usleep(1000000);
		get_ret =
		        EVENT_getConf(client_fd, &dft_adv_img_pref, sizeof(AGTX_ADV_IMG_PREF_S), AGTX_CMD_ADV_IMG_PREF);

		if (get_ret != STATE_SUCCESS) {
			err_cnt++;
			continue;
		}

		// checkVidExisted();

		if (get_ret == STATE_SUCCESS) {
			memcpy(&g_adv_img_ctx.pref, &dft_adv_img_pref, sizeof(AGTX_ADV_IMG_PREF_S));
			err_cnt = 0;
			break;
		}
	}

	if (get_ret == STATE_SUCCESS) {
		syslog(LOG_INFO, "Update advance image preference setting from CC...\n");
		g_adv_img_ctx.is_updated = AGTX_TRUE;
		ret = 0;
	}

	return ret;
}

static void EVENT_printEvtGroup(const AGTX_EVENT_GROUP_S *group)
{
	int n;

	syslog(LOG_INFO, "group\n");
	syslog(LOG_INFO, "\t in_use = %d\n", group->in_use);
	syslog(LOG_INFO, "\t name = %d\n", group->name);
	syslog(LOG_INFO, "\t gpio\n");
	syslog(LOG_INFO, "\t\t polling_period_usec = %d\n", group->gpio.polling_period_usec);

	for (n = 0; n < MAX_AGTX_GPIO_EVENT_LIST_S_INIT_LEVEL_SIZE; n++) {
		syslog(LOG_INFO, "\t\t init_level[%d] = %s\n", n, group->gpio.init_level[n].action_args);
	}

	for (n = 0; n < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; n++) {
		syslog(LOG_INFO, "\t\t event[%d].rule\n", n);
		syslog(LOG_INFO, "\t\t\t trigger_type = %d\n", group->gpio.event[n].rule.trigger_type);
		syslog(LOG_INFO, "\t\t\t level_value = %d\n", group->gpio.event[n].rule.level_value);
		syslog(LOG_INFO, "\t\t\t level_time_sec = %d\n", group->gpio.event[n].rule.level_time_sec);
		syslog(LOG_INFO, "\t\t\t edge = %d\n", group->gpio.event[n].rule.edge);
		syslog(LOG_INFO, "\t\t\t edge_time_sec_start = %d\n", group->gpio.event[n].rule.edge_time_sec_start);
		syslog(LOG_INFO, "\t\t\t edge_time_sec_end = %d\n", group->gpio.event[n].rule.edge_time_sec_end);
		syslog(LOG_INFO, "\t\t action = %d\n", group->gpio.event[n].action);
		syslog(LOG_INFO, "\t\t action_args = %s\n", group->gpio.event[n].action_args);
	}

	syslog(LOG_INFO, "\t sw\n");
	syslog(LOG_INFO, "\t\t socket_path = %d\n", group->sw.socket_path);

	for (n = 0; n < MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE; n++) {
		syslog(LOG_INFO, "\t\t event[%d].rule\n", n);
		syslog(LOG_INFO, "\t\t\t trigger_type = %d\n", group->sw.event[n].rule.trigger_type);
		syslog(LOG_INFO, "\t\t\t action = %d\n", group->sw.event[n].action);
		syslog(LOG_INFO, "\t\t\t action_args = %s\n", group->sw.event[n].action_args);
	}

	syslog(LOG_INFO, "\t adc\n");
	syslog(LOG_INFO, "\t\t polling_period_usec = %d\n", group->adc.polling_period_usec);
	syslog(LOG_INFO, "\t\t chn = %d\n", group->adc.chn);

	for (n = 0; n < MAX_AGTX_ADC_EVENT_LIST_S_INIT_HYS_SIZE; n++) {
		syslog(LOG_INFO, "\t\t init_hys[%d] = %s\n", n, group->adc.init_hys[n].action_args);
	}

	for (n = 0; n < MAX_AGTX_ADC_EVENT_LIST_S_EVENT_SIZE; n++) {
		syslog(LOG_INFO, "\t\t event[%d].rule\n", n);
		syslog(LOG_INFO, "\t\t\t trigger_type = %d\n", group->adc.event[n].rule.trigger_type);
		syslog(LOG_INFO, "\t\t\t hys_th = %d\n", group->adc.event[n].rule.hys_th);
		syslog(LOG_INFO, "\t\t action = %d\n", group->adc.event[n].action);
		syslog(LOG_INFO, "\t\t action_args = %s\n", group->adc.event[n].action_args);
	}

	syslog(LOG_INFO, "\t eintc_pir\n");
	syslog(LOG_INFO, "\t\t device_path = %d\n", group->eintc.device_path);
	syslog(LOG_INFO, "\t\t jiffies_timeout = %d\n", group->eintc.jiffies_timeout);
	for (n = 0; n < MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE; n++) {
		syslog(LOG_INFO, "\t\t event[%d].rule\n", n);
		syslog(LOG_INFO, "\t\t\t trigger_type = %d\n", group->eintc.event[n].rule.trigger_type);
		syslog(LOG_INFO, "\t\t action = %d\n", group->eintc.event[n].action);
		syslog(LOG_INFO, "\t\t action_args = %s\n", group->eintc.event[n].action_args);
	}
}

static void EVENT_printEvtAttr(const AGTX_EVENT_ATTR_S *attr)
{
	syslog(LOG_INFO, "attr\n");
	syslog(LOG_INFO, "\t name = %s\n", attr->name);
	syslog(LOG_INFO, "\t enabled = %d\n", attr->enabled);
}

static void EVENT_printEvtCtx(const int max_idx, const EVENT_CTX_S *ctx)
{
	int n;

	for (n = 0; n < max_idx; n++) {
		syslog(LOG_INFO, "#----------\n");
		syslog(LOG_INFO, "id = %d\n", ctx[n].id);
		syslog(LOG_INFO, "name = %s\n", ctx[n].name);
		syslog(LOG_INFO, "socket_path = %s\n", ctx[n].socket_path);
		syslog(LOG_INFO, "adc_path = %s\n", ctx[n].adc_path);
		syslog(LOG_INFO, "device_path = %s\n", ctx[n].device_path);
		syslog(LOG_INFO, "create_thread = %d\n", ctx[n].create_thread);
		EVENT_printEvtAttr(&ctx[n].attr);
		EVENT_printEvtGroup(&ctx[n].group);
	}
}

static void EVENT_printEvtCfgList(const int max_idx, const EVENT_CTX_S *ctx)
{
	int n;

	syslog(LOG_INFO, "#--- Event Config\n");
	for (n = 0; n < max_idx; n++) {
		syslog(LOG_INFO, "id = %2d, created_thread = %2d, enabled = %2d, name = %s\n", ctx[n].id,
		       ctx[n].create_thread, ctx[n].attr.enabled, ctx[n].name);
	}
}

static void EVENT_printGpioConf(const int max_idx, const AGTX_GPIO_ALIAS_S *gpio_alias)
{
	int n;

	syslog(LOG_INFO, "#--- GPIO Config\n");
	for (n = 0; n < max_idx; n++) {
		syslog(LOG_INFO, "pin_num = %2d, dir = %1d, value = %1d, name = %s\n", gpio_alias[n].pin_num,
		       gpio_alias[n].dir, gpio_alias[n].value, gpio_alias[n].name);
	}
}

int main(int argc, char **argv)
{
	AGTX_UNUSED(argc);
	AGTX_UNUSED(argv);

	openlog("eventd", LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "Event daemon started.");

	int idx;
	int ret = 0;
	int client_fd;
	int err = 0;
	int n = 0;

	int lock_fd;
	char *lock_name = "/tmp/eventd.lock";

	AGTX_EVENT_CONF_S dft_evt_cfg;
	memset(&dft_evt_cfg, 0, sizeof(AGTX_EVENT_CONF_S));
	AGTX_GPIO_CONF_S dft_gpio_cfg = { { { 0 } } };
	AGTX_EVENT_PARAM_S dft_evt_param = { { { 0 } } };
	AGTX_PWM_CONF_S dft_pwm_cfg = { { { 0 } } };
	AGTX_FLOODLIGHT_CONF_S dft_floodlight_cfg = { 0 };
	AGTX_PIR_CONF_S dft_pir_cfg = { 0 };
	AGTX_LIGHT_SENSOR_CONF_S dft_light_senosr_cfg;
	memset(&dft_light_senosr_cfg, 0, sizeof(AGTX_LIGHT_SENSOR_CONF_S));
	AGTX_COLOR_CONF_S dft_colar_mode_cfg = { 0 };

	if (MAX_AGTX_EVENT_CONF_S_EVENT_SIZE <= 0) {
		EVT_NOTICE("No event be detect, and event daemon be stopped!\n");
		closelog();
		return ret;
	}

	if (signal(SIGINT, handleSigno) == SIG_ERR) {
		EVT_ERR("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigno) == SIG_ERR) {
		EVT_ERR("Cannot handle SIGTERM!\n");
		exit(1);
	}

	lock_fd = open(lock_name, O_CREAT | O_RDWR, 0666);
	if (-1 == lock_fd) {
		perror("Cannot open lock file");
		return -EINVAL;
	}

	if (-1 == flock(lock_fd, LOCK_EX | LOCK_NB)) {
		printf("Failed to lock %s. Multiple instance detected.\n", lock_name);
		close(lock_fd);
		return -EBUSY;
	}

	client_fd = EVENT_connectCentCtrl(SW_CC_SOCKET_PATH);

	if (client_fd < 0) {
		EVT_ERR("Failed to connect central control!\n");
		close(client_fd);
		return ret;
	}

	/* get event config from CC */
	ret = EVENT_getConf(client_fd, &dft_evt_cfg, sizeof(AGTX_EVENT_CONF_S), AGTX_CMD_EVT_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get event config from CC!\n");
	}

	/* get event parameters from CC */
	ret = EVENT_getConf(client_fd, &dft_evt_param, sizeof(AGTX_EVENT_PARAM_S), AGTX_CMD_EVT_PARAM);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get event parameters from CC!\n");
	}

	/* get gpio config from CC */
	ret = EVENT_getConf(client_fd, &dft_gpio_cfg, sizeof(AGTX_GPIO_CONF_S), AGTX_CMD_GPIO_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get gpio config from CC!\n");
		EVENT_printGpioConf(MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, &dft_gpio_cfg.gpio_alias[0]);
	}

	memcpy(g_gpio_alias, &dft_gpio_cfg.gpio_alias[0],
	       sizeof(AGTX_GPIO_ALIAS_S) * MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE);

	EVENT_initGPIO(MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, &g_gpio_alias[0]);

	/* get floodlight config from CC */
	ret = EVENT_getConf(client_fd, &dft_floodlight_cfg, sizeof(AGTX_FLOODLIGHT_CONF_S), AGTX_CMD_FLOODLIGHT_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get floodlight config from CC!\n");
	}

	memcpy(&g_floodlight_ctx.floodlight_conf, &dft_floodlight_cfg, sizeof(AGTX_FLOODLIGHT_CONF_S));

	/* get pwm config from CC */
	ret = EVENT_getConf(client_fd, &dft_pwm_cfg, sizeof(AGTX_PWM_CONF_S), AGTX_CMD_PWM_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get pwm config from CC!\n");
	}

	memcpy(g_pwm_alias, &dft_pwm_cfg.pwm_alias[0], sizeof(AGTX_PWM_ALIAS_S) * MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE);

	ret = EVENT_initPWM(MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE, &g_pwm_alias[0]);

	if (ret != STATE_SUCCESS) {
		EVENT_exitPWM(MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE, &dft_pwm_cfg.pwm_alias[0]);
	}

	/* get pir config from CC */
	ret = EVENT_getConf(client_fd, &dft_pir_cfg, sizeof(AGTX_PIR_CONF_S), AGTX_CMD_PIR_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get pir config from CC!\n");
	}

	memcpy(&g_pir_ctx.pir_conf, &dft_pir_cfg, sizeof(AGTX_PIR_CONF_S));

	for (idx = 0; idx < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; idx++) {
		g_pir_ctx.pwm[idx].id = -1;
	}

	for (idx = 0, n = 0; idx < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; idx++) {
		if (strcmp((char *)g_pwm_alias[idx].name, "PIR") == 0 && g_pwm_alias[idx].pin_num >= 0) {
			g_pir_ctx.pwm[n].enabled = AGTX_PIR_ENABLED_ON;
			g_pir_ctx.pwm[n].id = g_pwm_alias[idx].pin_num;
			n++;
		} else {
			g_pir_ctx.pwm[n].enabled = AGTX_PIR_ENABLED_OFF;
		}
		if (strcmp((char *)g_pwm_alias[idx].name, "FLOODLIGHT") == 0 && g_pwm_alias[idx].pin_num >= 0) {
			g_floodlight_ctx.pwm.enabled = AGTX_PIR_ENABLED_ON;
			g_floodlight_ctx.pwm.id = g_pwm_alias[idx].pin_num;
			g_floodlight_ctx.pwm.period = g_pwm_alias[idx].period;
		} else {
			g_floodlight_ctx.pwm.enabled = AGTX_PIR_ENABLED_OFF;
		}
	}

	/* get light sensor config from CC */
	ret = EVENT_getConf(client_fd, &dft_light_senosr_cfg, sizeof(AGTX_LIGHT_SENSOR_CONF_S),
	                    AGTX_CMD_LIGHT_SENSOR_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get light sensor config from CC!\n");
	}

	memcpy(&g_light_sensor_ctx.light_sensor_conf, &dft_light_senosr_cfg, sizeof(AGTX_LIGHT_SENSOR_CONF_S));

	/* get color mode config from CC */
	ret = EVENT_getConf(client_fd, &dft_colar_mode_cfg, sizeof(AGTX_COLOR_CONF_S), AGTX_CMD_COLOR_CONF);

	if (ret != STATE_SUCCESS) {
		EVT_ERR("Failed to get color mode config from CC!\n");
	}

	memcpy(&g_color_mode_ctx.color_mode_conf, &dft_colar_mode_cfg, sizeof(AGTX_COLOR_CONF_S));

	/* initialize data to pass threads */
	if (ret == STATE_SUCCESS) {
		syslog(LOG_INFO, "Update event config setting from CC...\n");
		ret = EVENT_initGlobalCtx(MAX_AGTX_EVENT_CONF_S_EVENT_SIZE, &dft_evt_cfg.event[0]);
		syslog(LOG_INFO, "Initialize global context success...\n");
		EVENT_printEvtCtx(MAX_AGTX_EVENT_CONF_S_EVENT_SIZE, &g_event_ctx[0]);
	}

	if (ret < 0) {
		EVT_ERR("Failed to initialize global context!\n");
		EVENT_printEvtCtx(MAX_AGTX_EVENT_CONF_S_EVENT_SIZE, &g_event_ctx[0]);
		EVENT_exitGPIO(MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, &g_gpio_alias[0]);
		return ret;
	}

	/* update attr */
	ret = EVENT_setEvtAttr(MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE, &dft_evt_param.event_attr[0]);

	if (ret != 0) {
		EVT_WARN("Failed to set event attributes, ret = %d!\n", ret);
	}

	/* init adv img prev */
	g_day_night_attr.icr_pin[0] = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRCUT0_OUT");
	g_day_night_attr.icr_pin[1] = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRCUT1_OUT");
	g_day_night_attr.ir_led_pin = name2pin(g_gpio_alias, MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE, "IRLED_OUT");
	g_icr_pin_num = calcIcrNum(&g_day_night_attr.icr_pin[0]);

	initAdvImgPref(client_fd);

	EVENT_printEvtCfgList(MAX_AGTX_EVENT_CONF_S_EVENT_SIZE, &g_event_ctx[0]);

	/* create threads based on event in the database */
	for (idx = 0; idx < MAX_AGTX_EVENT_CONF_S_EVENT_SIZE; idx++) {
		if (g_event_ctx[idx].create_thread) {
			if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_GPIO) {
				ret = pthread_create(&g_event_ctx[idx].tid, NULL, EVENT_GPIO_daemon,
				                     (void *)&g_event_ctx[idx]);

				if (ret != 0) {
					EVT_ERR("Failed to create EVENT_GPIO_daemon(%d)!\n", idx);
				}
				err = pthread_setname_np(g_event_ctx[idx].tid, "EVENT_GPIO");
				if (err != 0) {
					syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n",
					       err);
				}
				syslog(LOG_INFO, "Create EVENT_GPIO_daemon(%d, %s).\n", idx, g_event_ctx[idx].name);

			} else if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_SW) {
				ret = pthread_create(&g_event_ctx[idx].tid, NULL, EVENT_SW_daemon,
				                     (void *)&g_event_ctx[idx]);

				if (ret != 0) {
					EVT_ERR("Failed to create EVENT_SW_daemon(%d)!\n", idx);
				}
				err = pthread_setname_np(g_event_ctx[idx].tid, "EVENT_SW");
				if (err != 0) {
					syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n",
					       err);
				}
				syslog(LOG_INFO, "Create EVENT_SW_daemon(%d, %s).\n", idx, g_event_ctx[idx].name);

			} else if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_ADC) {
				ret = pthread_create(&g_event_ctx[idx].tid, NULL, EVENT_ADC_daemon,
				                     (void *)&g_event_ctx[idx]);

				if (ret != 0) {
					EVT_ERR("Failed to create EVENT_ADC_daemon(%d)!\n", idx);
				}
				err = pthread_setname_np(g_event_ctx[idx].tid, "EVENT_ADC");
				if (err != 0) {
					syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n",
					       err);
				}
				syslog(LOG_INFO, "Create EVENT_ADC_daemon(%d, %s).\n", idx, g_event_ctx[idx].name);

			} else if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_LED) {
				ret = pthread_create(&g_event_ctx[idx].tid, NULL, EVENT_LED_daemon,
				                     (void *)&g_event_ctx[idx]);

				if (ret != 0) {
					EVT_ERR("Failed to create EVENT_LED_daemon(%d)!\n", idx);
				}
				err = pthread_setname_np(g_event_ctx[idx].tid, "EVENT_LED");
				if (err != 0) {
					syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n",
					       err);
				}
				syslog(LOG_INFO, "Create EVENT_LED_daemon(%d, %s).\n", idx, g_event_ctx[idx].name);

			} else if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_MPI) {
				/* Do nothing */
			} else if (g_event_ctx[idx].group.source == AGTX_EVENT_SOURCE_EINTC) {
				ret = pthread_create(&g_event_ctx[idx].tid, NULL, EVENT_EINTC_PIR_daemon,
				                     (void *)&g_event_ctx[idx]);

				if (ret != 0) {
					EVT_ERR("Failed to create EVENT_EINTC_PIR_daemon(%d)!\n", idx);
				}
				err = pthread_setname_np(g_event_ctx[idx].tid, "EVENT_EINTC_PIR");
				if (err != 0) {
					syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n",
					       err);
				}
				syslog(LOG_INFO, "Create EVENT_EINTC_PIR_daemon(%d, %s).\n", idx,
				       g_event_ctx[idx].name);

			} else {
				EVT_ERR("Invaild event source, idx = %d!\n", idx);
			}
		}
	}

	if (client_fd > 0) {
		if (pthread_create(&g_adj_evt_cfg_tid, NULL, EVENT_CC_daemon, (void *)client_fd) != 0) {
			EVT_ERR("Failed to create EVENT_CC_daemon!\n");
		}
		err = pthread_setname_np(g_adj_evt_cfg_tid, "EVENT_CC");
		if (err != 0) {
			syslog(LOG_INFO, "Create object detection thread to config failed. err = %d\n", err);
		}
		syslog(LOG_INFO, "Create EVENT_CC_daemon.\n");
	}

	for (idx = 0; idx < MAX_AGTX_EVENT_CONF_S_EVENT_SIZE; idx++) {
		if (g_event_ctx[idx].create_thread) {
			ret = pthread_join(g_event_ctx[idx].tid, NULL);

			if (ret != 0) {
				EVT_ERR("Failed to join event(%d, %s)!\n", idx, g_event_ctx[idx].name);
			}

			syslog(LOG_INFO, "Join event(%d, %s).\n", idx, g_event_ctx[idx].name);
		}
	}

	ret = pthread_join(g_adj_evt_cfg_tid, NULL);

	if (ret == 0) {
		syslog(LOG_INFO, "Join event g_adj_evt_cfg_tid.\n");
	} else {
		EVT_ERR("Failed to join event g_adj_evt_cfg_tid!\n");
	}

	syslog(LOG_INFO, "Event daemon stopped.");
	closelog();

	return 0;
}
