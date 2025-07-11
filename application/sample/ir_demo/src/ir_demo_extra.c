#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/io.h>
#include <syslog.h>
#include <unistd.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dip_types.h"
#include "mpi_dip_alg.h"
#include "ir_control.h"

#define ENABLE_PWM
#ifdef ENABLE_PWM
#include "pwm.h"
#endif

#include "light.h"
#include "gpio.h"

#define ir_control_demo_log_debug(fmt, args...) \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[IR conrtrol demo][Debug] " fmt, ##args)
#define ir_control_demo_log_err(fmt, args...) \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[IR conrtrol demo][Error] " fmt, ##args)

#define DAY_MODE_FLAG ("/tmp/day")
#define FORCE_DAY_MODE_FLAG ("/tmp/force_day")
#define NIGHT_MODE_FLAG ("/tmp/night")

#define IR_CUT_ID (15) /*tb008 ir cut id 0*/
#ifdef ENABLE_PWM
#define IR_LIGHT_ID (1)
PWM g_ir_pwm = { 0 };
#endif

#define WAIT_DCC_APPLY_USEC (250000)
int g_run_flag = 0;

Gpio g_ir_cut;

int switch_ir_cut_cb(int enabled, void *pData __attribute__((unused)))
{
	if (enabled) {
		setGpioValue(&g_ir_cut, 1);
	} else {
		setGpioValue(&g_ir_cut, 0);
	}
	return 0;
}

int __switch_day_color_via_mpi()
{
	INT32 ret;
	MPI_PATH path_idx = MPI_INPUT_PATH(0, 0);

	MPI_CAL_ATTR_S cal_attr;
	MPI_DIP_ATTR_S dip_attr;
	MPI_CSM_ATTR_S csm_attr;

	ret = MPI_getCalAttr(path_idx, &cal_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getCalAttr! %d", ret);
		return ret;
	}

	cal_attr.dcc_en = 1;
	ret = MPI_setCalAttr(path_idx, &cal_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setCalAttr! %d", ret);
		return ret;
	}

	/* waitting dcc apply */
	//usleep(WAIT_DCC_APPLY_USEC);
	ret = MPI_getDipAttr(path_idx, &dip_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getDipAttr! %d", ret);
		return ret;
	}

	dip_attr.is_awb_en = 1;
	ret = MPI_setDipAttr(path_idx, &dip_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setDipAttr! %d", ret);
		return ret;
	}

	ret = MPI_getCsmAttr(path_idx, &csm_attr);
	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getCsmAttr! %d", ret);
		return ret;
	}

	csm_attr.bw_en = 0;
	ret = MPI_setCsmAttr(path_idx, &csm_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setCsmAttr! %d", ret);
		return ret;
	}

	return ret;
}

int __switch_night_color_via_mpi()
{
	INT32 ret;
	MPI_PATH path_idx = MPI_INPUT_PATH(0, 0);

	MPI_CSM_ATTR_S csm_attr;
	MPI_DIP_ATTR_S dip_attr;
	MPI_CAL_ATTR_S cal_attr;

	ret = MPI_getCsmAttr(path_idx, &csm_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getCsmAttr! %d", ret);
		return ret;
	}

	csm_attr.bw_en = 1;

	ret = MPI_setCsmAttr(path_idx, &csm_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setCsmAttr! %d", ret);
		return ret;
	}

	ret = MPI_getDipAttr(path_idx, &dip_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getDipAttr! %d", ret);
		return ret;
	}

	dip_attr.is_awb_en = 0;

	ret = MPI_setDipAttr(path_idx, &dip_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setDipAttr! %d", ret);
		return ret;
	}

	ret = MPI_getCalAttr(path_idx, &cal_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_getCalAttr! %d", ret);
		return ret;
	}

	cal_attr.dcc_en = 0;

	ret = MPI_setCalAttr(path_idx, &cal_attr);

	if (ret != MPI_SUCCESS) {
		ir_control_demo_log_err("Failed to MPI_setCalAttr! %d", ret);
		return ret;
	}

	return ret;
}

void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		printf("Caught SIGTKILL\n");
	} else if (signo == SIGQUIT) {
		printf("Caught SIGQUIT!\n");
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}

	g_run_flag = 0;
}

int init_cb(void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("init ir light");
	/*Init IR cut gpio*/
	exportGpio(g_ir_cut.id);
	setGpioDirection(&g_ir_cut, "out");
	setGpioValue(&g_ir_cut, 0);

#ifdef ENABLE_PWM
	g_ir_pwm.period = 1000000;
	exportPWM(g_ir_pwm.id);
	setPWMPeriod(&g_ir_pwm, g_ir_pwm.period);
	setPWMDutyCycle(&g_ir_pwm, g_ir_pwm.period);
	enabledPWM(&g_ir_pwm, 1);
	ir_control_demo_log_debug("finish enable PWM");
#endif
	return 0;
}

int uninit_cb(void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("uninit ir light");
	unexportGpio(g_ir_cut.id);
#ifdef ENABLE_PWM
	unexportPWM(g_ir_pwm.id);
#endif
	return 0;
}

int enable_ir_light_cb(int enabled, void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("enabled pwm ? %s", enabled == 0 ? "no" : "yes");
#ifdef ENABLE_PWM
	enabledPWM(&g_ir_pwm, enabled);
#endif
	return 0;
}

IrModeReturn extra_night2day_cb(void *data __attribute__((unused)))
{
	IrModeReturn ret = IR_MODE_UNKNOWN;

	if (access(FORCE_DAY_MODE_FLAG, 0) == 0) {
		ret = IR_FORCE_DAY;
	}

	if (access(DAY_MODE_FLAG, 0) == 0) {
		ret = IR_DAY;
	}

	if (access(NIGHT_MODE_FLAG, 0) == 0) {
		ret = IR_NIGHT;
	}

	return ret;
}

int switch_mode_cb1(IR_Mode mode, void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("switch mode %d", (int)mode);
	return 0;
}

int switch_mode_cb2(IR_Mode mode, void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("switch mode %d", (int)mode);
	return 0;
}

int switch_day_night_cb1(DayNIghtMode mode, void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("enabled? %s", mode == DAY ? "DAY" : "NIGHT");
	if (mode == DAY) {
		__switch_day_color_via_mpi();
	} else if (mode == NIGHT) {
		__switch_night_color_via_mpi();
	} else {
		ir_control_demo_log_err("Unknown day/night mode: %d", (int)mode);
		return -1;
	}
	return 0;
}

int switch_day_night_cb2(DayNIghtMode mode, void *data __attribute__((unused)))
{
	ir_control_demo_log_debug("enabled? %s", mode == DAY ? "DAY" : "NIGHT");
	if (mode == DAY) {
		__switch_day_color_via_mpi();
	} else if (mode == NIGHT) {
		__switch_night_color_via_mpi();
	} else {
		ir_control_demo_log_err("Unknown day/night mode %d", (int)mode);
		return -1;
	}
	return 0;
}

void help()
{
	printf("Usage:\n"
	       "\t-m <IR Mode> 1:Auto,2:ON,3:OFF\n"
	       "\t-i <IR cut gpio ip>\n"
	       "\t-p <PWM index>\n"
	       "Options:\n"
	       "\t-h help()\n");
}

int main(int argc, char **argv)
{
	ir_control_demo_log_debug("enter at_ir_control");

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		ir_control_demo_log_err("Cannot handle SIGINT!");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		ir_control_demo_log_err("Cannot handle SIGTERM!");
		exit(1);
	}

	if (argc == 1) {
		help();
		return 0;
	}

	int ir_cut_id = IR_CUT_ID;
#ifdef ENABLE_PWM
	int pwm_idx = IR_LIGHT_ID;
#else
	int pwm_idx;
#endif
	int customer_ir_mode = IR_AUTO;
	int c;

	while ((c = getopt(argc, argv, "hi:m:p:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'i':
			ir_cut_id = atoi(argv[optind - 1]);
			if (ir_cut_id <= 0) {
				ir_control_demo_log_err("Invalid gpio num:%d", ir_cut_id);
				exit(1);
			}
			ir_control_demo_log_debug("get ir cut id :%d", ir_cut_id);
			break;
		case 'p':
			pwm_idx = atoi(argv[optind - 1]);
			if (pwm_idx < 0) {
				ir_control_demo_log_err("Invalid pwm idx :%d", pwm_idx);
				exit(1);
			}
			ir_control_demo_log_debug("get ir light pwm : pwm%d", pwm_idx);
			break;
		case 'm':
			customer_ir_mode = (IR_Mode)atoi(argv[optind - 1]);
			if ((customer_ir_mode > 3) || (customer_ir_mode < 0)) {
				ir_control_demo_log_err("Invalid IR mode num:%d", customer_ir_mode);
				exit(1);
			}
			ir_control_demo_log_debug("get mode: %d", customer_ir_mode);
			break;
		default:
			help();
			exit(1);
		}
	}

	int light_strength[5] = { 1024, 973, 922, 256, 10 };

	IRConfig config = { .sleepIntervalus = 10000,
		            .force_day_th = 3300,
		            .day_th = 1800,
		            .night_th = 230,
		            .day_delay = 3,
		            .night_delay = 3,
		            .iir_current_weight = 40,
		            .ir_amplitude_ratio = 31, /*unit 1/1024*/
		            .ir_led_luma_ratio_thr = 23,
		            .total_luma_delta_thr = 6,
		            .check_ir_sur_luma_thr = 20, /*unit 1/32*/
		            .cloth_thr = 11, /*unit 1/32*/
		            .ev_responce_us = 1000000,
		            .first_night_ev_response_us = 3000000 };

	memcpy(&config.ir_light_strength[0], light_strength, sizeof(light_strength));

	IrControl Ir_control = { .mode = customer_ir_mode,
		                 .initCb = init_cb,
		                 .uninitCb = uninit_cb,
		                 .switchIRcutCb = switch_ir_cut_cb,
		                 .switchModeCb = switch_mode_cb1,
		                 .enabledLightCb = enable_ir_light_cb,
		                 .switchDayNightCb = switch_day_night_cb1,
		                 .extraNighttoDayCb = extra_night2day_cb,
		                 .conf = config,
		                 .pData = NULL };

	MPI_SYS_init();
	g_run_flag = 1;
	g_ir_cut.id = ir_cut_id;
#ifdef ENABLE_PWM
	g_ir_pwm.id = pwm_idx;
#endif

	IR_init();
	IR_setConfig(&Ir_control);

	sleep(5);

	/*test set mode*/
	IR_Mode mode;
	IR_getMode(&mode);
	if (mode != (IR_Mode)customer_ir_mode) {
		ir_control_demo_log_err("failed to set/get IR mode:%d", customer_ir_mode);
	}

	IR_run();

	DayNIghtMode tmpMode;
	while (g_run_flag) {
		sleep(20);
		IR_getDayNightMode(&tmpMode);
		ir_control_demo_log_debug("mode: %s", tmpMode == DAY ? "DAY" : "NIGHT");
	}
	IR_exit();

	MPI_SYS_exit();

	ir_control_demo_log_debug("exit at_ir_control");

	return 0;
}
