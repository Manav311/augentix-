
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/limits.h>
#include <pthread.h>
#include <errno.h>

#include "tutk_pir.h"
#include "tutk_init.h"
#include "tutk_define.h"
#include "log_define.h"
#include <unistd.h>

#include "agtx_lpw.h"

extern bool gProgressRun;
extern bool gSettingsExist;
extern char gDefaultSettingsFilePath[PATH_MAX];
extern char gUdid[MAX_UDID_LENGTH + 1];
extern NebulaDeviceCtx *gDeviceCtx;
extern lpw_handle gWifihd;
extern TutkConfigs gConfigs;

int getPIRStatus()
{
	if (gWifihd == (lpw_handle)NULL) {
		gWifihd = lpw_open();
	}

	int adc_val = lpw_adc_get(gWifihd, gConfigs.pir_adc.adc_chn);

	if (adc_val > gConfigs.pir_adc.thres) {
		return adc_val;
	}

	return 0;
}

#define GPIO_OUTPUT 1
#define GPIO_INPUT 0
#define GPIO_LOW 0
#define GPIO_HIGH 1

static int resetPIRStatus()
{
	if (gWifihd == (lpw_handle)NULL) {
		gWifihd = lpw_open();
	}

	int ret = lpw_gpio_get_input(gWifihd, gConfigs.pir_adc.detect_event_gpio);
	if (ret == 1) {
		/* reset detect_event_gpio to low*/
		lpw_gpio_set_dir(gWifihd, gConfigs.pir_adc.detect_event_gpio, GPIO_OUTPUT, GPIO_LOW);
		lpw_gpio_set_dir(gWifihd, gConfigs.pir_adc.detect_event_gpio, GPIO_INPUT, GPIO_LOW);
	}

	return 0;
}

static void *ThreadCheckPIR(void *arg)
{
	tutkservice_log_info("%s start\n", __func__);
	NebulaDeviceCtx *device_ctx = (NebulaDeviceCtx *)arg;
	int ret = 0;
	NebulaJsonObject *notification_obj = NULL;
	unsigned int push_abort = 0;

	ret = Nebula_Json_Obj_Create_From_String("{\"type\":\"pir\"}", &notification_obj);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Nebula_Json_Obj_Create_From_String[%d]\n", ret);
		goto end;
	}

	sleep(3);

	while (gProgressRun) {
		ret = getPIRStatus();

		if (ret > 0) {
			/*PIR trigger*/
			ResetNoSessionTime();

			ret = Nebula_Device_Push_Notification(device_ctx, notification_obj, 10000, &push_abort);
			tutkservice_log_info("Nebula_Device_Push_Notification[%d], adc[%d]: %d\n", ret,
			                     gConfigs.pir_adc.adc_chn, getPIRStatus());
		} else if (ret < 0) {
			tutkservice_log_err("failed to get pir: %d\n", ret);
		}

		resetPIRStatus();

		sleep(1);
	}
end:
	Nebula_Json_Obj_Release(notification_obj);
	tutkservice_log_info("%s exit\n", __func__);
	pthread_exit(0);
}

int TUTK_startPIRnotificationThread(void *device_ctx)
{
	pthread_t adc_check_thread_id;
	if (pthread_create(&adc_check_thread_id, NULL, ThreadCheckPIR, (void *)device_ctx) < 0) {
		tutkservice_log_err("create ThreadCheckADC failed!\n");
	} else {
		pthread_detach(adc_check_thread_id);
	}

	return 0;
}
