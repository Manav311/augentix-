
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <linux/limits.h>
#include <pthread.h>
#include <errno.h>

#include "tutk_adc.h"
#include "tutk_define.h"
#include "log_define.h"
#include <unistd.h>

#include "agtx_lpw.h"
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) > (b) ? (b) : (a))

extern bool gProgressRun;
extern bool gSettingsExist;
extern char gDefaultSettingsFilePath[PATH_MAX];
extern char gUdid[MAX_UDID_LENGTH + 1];
extern NebulaDeviceCtx *gDeviceCtx;
extern lpw_handle gWifihd;
extern TutkConfigs gConfigs;
unsigned int ginterpolation_level;

int sortAdcLevelIdx(unsigned int adc_val)
{
	unsigned int level_idx = 0;
	for (level_idx = 0; level_idx < MAX_LEVEL_CNT; level_idx++) {
		if (adc_val >= gConfigs.battery_level[level_idx].adc_val) {
			break;
		}

		if (adc_val < gConfigs.battery_level[MAX_LEVEL_CNT - 1].adc_val) {
			level_idx = MAX_LEVEL_CNT - 1;
			break;
		}

		if (adc_val <= gConfigs.battery_level[level_idx].adc_val &&
		    adc_val > gConfigs.battery_level[level_idx + 1].adc_val) {
			break;
		}
	}

	return level_idx;
}

int getAdcLevelInterpolation(int adc, int level_idx)
{
	if (adc < 0 || level_idx >= MAX_LEVEL_CNT) {
		return -EINVAL;
	}

	float level = 0;
	float roof_adc = gConfigs.battery_level[level_idx].adc_val;
	float floor_adc = gConfigs.battery_level[level_idx + 1].adc_val;
	float roof_level = gConfigs.battery_level[level_idx].level;
	float floor_level = gConfigs.battery_level[level_idx + 1].level;

	level = floor_level + ((adc - floor_adc) * (roof_level - floor_level)) / (roof_adc - floor_adc);

	if (level > gConfigs.battery_level[0].level) {
		level = gConfigs.battery_level[0].level;
	}

	if (level < 0) {
		level = 0;
	}

	return (int)level;
}

int getAdcValueInterpolation(unsigned int interpolation_level)
{
	if (interpolation_level == 0) {
		return -EINVAL;
	}

	float adc_interpolation_value = 0;
	int level_idx = 0;

	for (int i = 0; i < MAX_LEVEL_CNT; i++) {
		printf("sort %d%% - %d%%\n", gConfigs.battery_level[i].level, gConfigs.battery_level[i + 1].level);
		if (interpolation_level <= gConfigs.battery_level[i].level &&
		    interpolation_level >= gConfigs.battery_level[i + 1].level) {
			level_idx = i;
			break;
		}

		if (interpolation_level <= gConfigs.battery_level[MAX_LEVEL_CNT - 1].level) {
			level_idx = MAX_LEVEL_CNT - 2;
		}
	}

	float roof_adc = gConfigs.battery_level[level_idx].adc_val;
	float floor_adc = gConfigs.battery_level[level_idx + 1].adc_val;
	float roof_level = gConfigs.battery_level[level_idx].level;
	float floor_level = gConfigs.battery_level[level_idx + 1].level;

	printf("\n%d (%f %f%% - %f %f%%)\n", interpolation_level, roof_adc, roof_level, floor_adc, floor_level);

	adc_interpolation_value =
	        floor_adc + ((interpolation_level - floor_level) * (roof_adc - floor_adc)) / (roof_level - floor_level);

	if (adc_interpolation_value > gConfigs.battery_level[0].adc_val) {
		adc_interpolation_value = gConfigs.battery_level[0].adc_val;
	}

	return (int)adc_interpolation_value;
}

unsigned int parseWarningAdcLevel(unsigned int interpolation_level)
{

	unsigned int warning_adc_level = gConfigs.low_battery_warning;
	FILE *fp;
	if (access(ADC_WARING_FILE, 0) != 0) {
		/*file not exist*/
		fp = fopen(ADC_WARING_FILE, "w");
		fwrite(&gConfigs.low_battery_warning, sizeof(gConfigs.low_battery_warning), 1, fp);
		goto end;
	}

	fp = fopen(ADC_WARING_FILE, "w");
	warning_adc_level = (interpolation_level / 5) * 5;
	unsigned int next_warning_level = max((warning_adc_level), 1);
	next_warning_level = min(next_warning_level, gConfigs.low_battery_warning);
	fseek(fp, 0, SEEK_SET);
	fwrite(&next_warning_level, sizeof(next_warning_level), 1, fp);
	fclose(fp);
	fp = NULL;

end:
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
		return warning_adc_level;
	}
	return next_warning_level;
}

static void *ThreadCheckADC(void *arg)
{
	tutkservice_log_info("%s start\n", __func__);
	NebulaDeviceCtx *device_ctx = (NebulaDeviceCtx *)arg;
	int ret = 0;
	NebulaJsonObject *notification_obj = NULL;
	unsigned int push_abort = 0;

	ret = Nebula_Json_Obj_Create_From_String("{\"type\":\"lowBattery\"}", &notification_obj);
	if (ret != NEBULA_ER_NoERROR) {
		tutkservice_log_err("Nebula_Json_Obj_Create_From_String[%d]\n", ret);
		return NULL;
	}

	if (gWifihd == (lpw_handle)NULL) {
		gWifihd = lpw_open();
	}

	sleep(3);

	while (gProgressRun) {
		/* get adc_val*/
		int adc_val = 0;
		int level_idx = 0;
		static unsigned int interpolation_level = 0;

		adc_val = lpw_adc_get(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch);
		level_idx = sortAdcLevelIdx(adc_val);
		ginterpolation_level = getAdcLevelInterpolation(adc_val, level_idx);

		if (gSettingsExist) {
			ret = Nebula_Device_Push_Notification(device_ctx, notification_obj, 5000, &push_abort);
			printf("Nebula_Device_Push_Notification[%d]\n", ret);

			tutkservice_log_info("Get ADC[%d] value: %d= %d%%. set warning at: %d%%\n",
			                     gConfigs.pwr_mgmt.battery_adc_ch, adc_val, ginterpolation_level,
			                     parseWarningAdcLevel(ginterpolation_level));
			if (ginterpolation_level <= 1) {
				tutkservice_log_info(
				        "\nBattery Level <= 1, gProgress will be set to 0 and quit all the function\n");
				gProgressRun = false;
				break;
			} else {
				printf("Battery Level = %d\n", ginterpolation_level);
			}
		}

		sleep(3);
	}
	Nebula_Json_Obj_Release(notification_obj);
	tutkservice_log_info("%s exit\n", __func__);
	pthread_exit(0);
}

int TUTK_startADCnotificationThread(void *device_ctx)
{
	char *settings = NULL;

	FILE *fp = NULL;
	int ret = 0;
	// get notification settings
	fp = fopen(gDefaultSettingsFilePath, "r");
	if (fp != NULL) {
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		settings = calloc(1, fsize + 1);
		ret = fread(settings, 1, fsize, fp);
		fclose(fp);
		fp = NULL;
		if (ret <= 0) {
			tutkservice_log_err("read %s fail, ret=[%d]\n", gDefaultSettingsFilePath, ret);
			return -1;
		}
		gSettingsExist = true;
	} else {
		tutkservice_log_err("%s not exist\n", gDefaultSettingsFilePath);
	}

	ret = Nebula_Device_Load_Settings(device_ctx, settings);
	tutkservice_log_info("Nebula_Device_Load_Settings[%d]\n", ret);
	if (ret == NEBULA_ER_NoERROR) {
		gSettingsExist = true;
	}
	free(settings);

	pthread_t adc_check_thread_id;
	if (pthread_create(&adc_check_thread_id, NULL, ThreadCheckADC, (void *)device_ctx) < 0) {
		tutkservice_log_err("create ThreadCheckADC failed!\n");
	} else {
		pthread_detach(adc_check_thread_id);
	}

	return 0;
}