#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "linux/limits.h"
#include <errno.h>
#include <signal.h>

#include <json.h>
#include "agtx_lpw.h"

typedef struct {
	unsigned int adc_val;
	unsigned int level;
} BatteryLevel;

#define GPIO_OUTPUT 1
#define MAX_LEVEL_CNT 10

typedef struct tutk_configs {
	unsigned int gpio_id;
	unsigned int adc_id;
	BatteryLevel battery_level[MAX_LEVEL_CNT];
} Configs;

Configs g_configs;

char g_config_path[PATH_MAX] = { 0 };
lpw_handle g_wifi_hd = (lpw_handle)((intptr_t)NULL);

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
}

void help()
{
	printf("lpwio is for measuring battery remaining, adc value, gpio direction, gpio value\n\n");
	printf("usage: -h help\n -i <battery config> -a <adc channel number>\n -g <gpio id> [gpio value]\n -d <gpio id> [gpio direction]\n -a <adc channel number>\n");
	printf("------------------------------\n");
	printf("-Options:\n \
			-i <battery level config.json>\t must followed by -a for measuring battery remaining\n \
			-a <adc channel number>\t read the adc value from adc chanel number\n\
			-d <gpio id> [direction]\t three parameters for reading the gpio direction from gpio id; four parameters for setting the [direction] to <gpio id>\n \
			-g <gpio id> [value] \t three parameters for reading the gpio value from <gpio id> ; four parameters for setting the [vlaue] to <gpio id>\n");
	printf("Example:\n \
			lpwio -i /mnt/sdcard/batteryjson.json -a 3\n \
			lpwio -a 3\n \
			lpwio -d 2\n \
			lpwio -g 2\n \
			Notice:\n\
			Make sure the gpio pin you are measuring is valid in wifi module; the valid gpio pin list in Hi3861L are 2,5,6,7,9,10,14\n \
			the adc channel number for battery measuring is 3\n");
}

static int parseConfigs()
{
	json_object *root = NULL;

	json_object *battery_level_child = NULL;
	json_object *adc_val_child = NULL;
	json_object *level_percentage_child = NULL;

	// Error is logged by json-c.
	root = (g_config_path[0] != '\0') ? json_object_from_file(g_config_path) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	/* ADC battery level configs */
	if (!json_object_object_get_ex(root, "battery_level", &battery_level_child)) {
		perror("battery_level parameter is not found!\n");
		goto parse_end;
	}

	if (json_object_object_get_ex(battery_level_child, "adc_val", &adc_val_child) &&
	    json_object_object_get_ex(battery_level_child, "level", &level_percentage_child)) {
		memset(&g_configs.battery_level[0], 0x00, sizeof(g_configs.battery_level));

		int val_cnt = json_object_array_length(adc_val_child);
		int level_cnt = json_object_array_length(level_percentage_child);
		if (level_cnt != val_cnt) {
			fprintf(stderr, "Warning: val_cnt:%d != level_cnt: %d\n", val_cnt, level_cnt);
		}

		int array_cnt = level_cnt < val_cnt ? level_cnt : val_cnt;
		for (int i = 0; i < array_cnt; i++) {
			g_configs.battery_level[i].adc_val =
			        json_object_get_int(json_object_array_get_idx(adc_val_child, i));
			g_configs.battery_level[i].level =
			        json_object_get_int(json_object_array_get_idx(level_percentage_child, i));
		}

	} else {
		perror("not found adc val or level_percentage array");
	}

parse_end:
	json_object_put(root);

	/*TODO*/
	printf("get battery level table:\n");
	for (unsigned int i = 0; i < sizeof(g_configs.battery_level) / sizeof(g_configs.battery_level[0]); i++) {
		printf("adc val: %d == level:%d\n", g_configs.battery_level[i].adc_val,
		       g_configs.battery_level[i].level);
	}

	return 0;
}

int getAdcLevelInterpolation(int adc, int level_idx)
{
	if (adc < 0 || level_idx >= MAX_LEVEL_CNT) {
		return -EINVAL;
	}

	float level = 0;
	float roof_adc = g_configs.battery_level[level_idx].adc_val;
	float floor_adc = g_configs.battery_level[level_idx + 1].adc_val;
	float roof_level = g_configs.battery_level[level_idx].level;
	float floor_level = g_configs.battery_level[level_idx + 1].level;

	level = floor_level + ((adc - floor_adc) * (roof_level - floor_level)) / (roof_adc - floor_adc);

	if (level > g_configs.battery_level[0].level) {
		level = g_configs.battery_level[0].level;
	}

	if (level < 0) {
		level = 0;
	}

	return (int)level;
}

int findAdcLevel(unsigned int adc)
{
	unsigned int idx;
	for (idx = 0; idx < MAX_LEVEL_CNT; idx++) {
		if (adc >= g_configs.battery_level[idx].adc_val) {
			break;
		}

		if (adc < g_configs.battery_level[MAX_LEVEL_CNT - 1].adc_val) {
			idx = MAX_LEVEL_CNT - 1;
			break;
		}

		if (adc <= g_configs.battery_level[idx].adc_val && adc > g_configs.battery_level[idx + 1].adc_val) {
			break;
		}
	}

	printf("[%d]comp[%d] %d %d\n", idx, adc, g_configs.battery_level[idx].adc_val,
	       g_configs.battery_level[idx + 1].adc_val);

	return idx;
}

int adc_get()
{
	int adc = 0;
	adc = lpw_adc_get(g_wifi_hd, g_configs.adc_id);
	return adc;
}

int calBatteryRemain()
{
	int adc;
	int level_idx;

	adc = adc_get();

	printf("Get ADC[%d] value: %d.\n", g_configs.adc_id, adc);
	if (adc < 0) {
		printf("Invalid ADC[%d] val: %d\n", g_configs.adc_id, adc);
		return -1;
	}

	level_idx = findAdcLevel(adc);

	printf("sort level: %d, level: %d\n", level_idx, getAdcLevelInterpolation(adc, level_idx));
	// adc -= 5;

	return 0;
}

static int setGpio()
{
	int val = lpw_gpio_get_input(g_wifi_hd, g_configs.gpio_id);
	if (val == 0) {
		printf("gpio %d is 0 : low\n", g_configs.gpio_id);
	} else if (val == 1) {
		printf("gpio %d is 1 : high\n", g_configs.gpio_id);
	} else {
		printf("setGpio() error\n");
		return -1;
	}
	return 0;
}

static int gpio_set(int out)
{
	int ret = lpw_gpio_set_output(g_wifi_hd, g_configs.gpio_id, out);
	if (ret != 0) {
		printf("gpio_set() error\n");
		return -1;
	}
	printf("gpio %d is set to %d\n", g_configs.gpio_id, out);
	return 0;
}

static int getGpioDir()
{
	int dir = -1;
	dir = lpw_gpio_get_dir(g_wifi_hd, g_configs.gpio_id);
	if (dir > 0) {
		printf("gpio %d is output direction\n", g_configs.gpio_id);
	} else if (dir == 0) {
		printf("gpio %d is input direction\n", g_configs.gpio_id);
	} else {
		printf("getGpioDir() error\n");
		return -1;
	}
	return 0;
}

static int gpio_dir_set(int dir, int out)
{
	int ret = lpw_gpio_set_dir(g_wifi_hd, g_configs.gpio_id, dir, out);
	if (ret != 0) {
		printf("gpio_dir_set() error\n");
		return -1;
	} else {
		printf("gpio %d is set to %d direction (0 for input ;others for output)\n", g_configs.gpio_id, dir);
	}
	return 0;
}

static int parseInputOptions(int argc, char *argv[])
{
	int option = 0;
	int ret = 0;
	while ((option = getopt(argc, argv, "hi:a:g:d:")) > 0) {
		switch (option) {
		case 'i':
			strncpy(g_config_path, optarg, sizeof(g_config_path));
			printf("JSON config %s\n", g_config_path);
			parseConfigs();
			g_configs.adc_id = atoi(argv[4]);
			ret = calBatteryRemain();
			break;
		case 'g':
			g_configs.gpio_id = atoi(optarg);
			if (argc == 3) {
				ret = setGpio();
			} else if (argc == 4) {
				ret = gpio_set(atoi(argv[3]));
			}
			break;
		case 'd': // 0 for input;others for output
			g_configs.gpio_id = atoi(optarg);
			if (argc == 3) {
				ret = getGpioDir();
			} else if (argc == 4) {
				ret = gpio_dir_set(atoi(argv[3]), atoi(argv[3]));
			}
			break;
		case 'a':
			g_configs.adc_id = atoi(optarg);
			printf("adc value is : %d\n", adc_get());
			break;
		case 'h':
			help();
			break;
		default:
			help();
			return -EINVAL;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	if (g_wifi_hd == (lpw_handle)((intptr_t)NULL)) {
		g_wifi_hd = lpw_open();
	}

	int ret = 0;
	ret = parseInputOptions(argc, argv);
	if (ret < 0) {
		printf("parseInputOptions ret[%d] Error\n", ret);
		return -1;
	}

	return 0;
}