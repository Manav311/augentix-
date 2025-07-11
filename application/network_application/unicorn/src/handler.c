#include "handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#include "ccclient.h"
#include "utils.h"
#include "frame.h"
#include "gpio.h"
#include "json.h"
#include "action.h"
#include "stream.h"
#include "agtx_types.h"

extern GpioFrame g_gpio;
extern AdcFrame g_adc;

#define CFG_CMD_MAX_LEN 64
enum unicorn_cfg_type { CFG_PID = 0, CFG_UUID = 1, CFG_AUTHKEY = 2, CFG_NUM };
static const char unicorn__cfg_token[CFG_NUM][16] = { "pid", "uuid", "auzkey" };
static const char agtx_cfg_token[CFG_NUM][16] = { "IPC_APP_PID", "IPC_APP_UUID", "IPC_APP_AUTHKEY" };

#define for_each_cfg_type(t) for ((t) = 0; (t) < CFG_NUM; ++(t))

int g_fd;
char *g_fPath = NULL;
unsigned long int g_fSize;
unsigned long int g_fRead = 0;

int getGpioInfo(void)
{
	int ret = 0;
	char jstr[JSON_STR_LEN];

	if (g_gpio.flag == 0) {
		sprintf(jstr, "{\"module\":\"gpio_conf\"}");
		ret = ccClientGet(jstr, CC_GET_GPIO);
		if (ret != 0) {
			ERR("gpio_conf");
			ret = -1;
		}
	}

	return ret;
}

int getAdcInfo(void)
{
	int ret = 0;
	char jstr[JSON_STR_LEN];

	if (g_adc.flag == 0) {
		sprintf(jstr, "{\"module\":\"evt_conf\"}");
		ret = ccClientGet(jstr, CC_GET_ADC);
		if (ret != 0) {
			ERR("evt_conf");
			ret = -1;
		}
	}

	return ret;
}

int captureJpeg(char *file, int chn)
{
	//"/system/bin/mpi_snapshot jpeg 0 /tmp/snap_shot.jpeg"
	int ret = 0;
	char bin[] = "/system/bin/mpi_snapshot";
	char type[] = "jpeg";
	char cmd[128];

	sprintf(cmd, "%s %s %d %s", bin, type, chn, file);
	ret = executeSystemCommand(cmd);
	if ((ret != 0) || (fileExists(file) == false)) {
		ret = -1;
	}
	return ret;
}

int changeDB(char *db_file, char *cc_script, char *av_script)
{
	int ret = 0;
	char cmd[128];

	sprintf(cmd, "%s stop", av_script);
	ret = executeSystemCommand(cmd);
	if (ret >= 0) {
		sprintf(cmd, "%s stop", cc_script);
		ret = executeSystemCommand(cmd);
		if (ret == 0) {
			sprintf(cmd, "cp %s /tmp/ini.db", db_file);
			ret = executeSystemCommand(cmd);
			if (ret == 0) {
				sprintf(cmd, "%s start", cc_script);
				ret = executeSystemCommand(cmd);
				if (ret == 0) {
					sprintf(cmd, "%s start", av_script);
					ret = executeSystemCommand(cmd);
					if (ret != 0) {
						ERR("start av_main2 fail\n");
					}
				} else {
					ERR("start ccserver fail\n");
				}
			} else {
				ERR("copy .db fail\n");
			}
		} else {
			ERR("stop av_main2 fail\n");
		}
	} else {
		ERR("stop ccserver fail\n");
	}

	return ret;
}

int checkStreamStatus(int chn)
{
	FILE *fp;
	int ret = 0;
	char str[256] = { 0 };
	char cmd[128];
	time_t start, end;
	double seconds = 0.0;

	time(&start);
	while (1) {
		sprintf(cmd, "(cat /dev/enc | awk '/State/{print substr($2, 1)}') | awk 'NR==%d{print $1}'", chn);
		fp = popen(cmd, "r");
		if (fp == NULL) {
			ERR("popen error.");
			pclose(fp);
			return -1;
		}
		if (fgets(str, 256, fp) != NULL) {
			if (strcmp(str, "RUNNING") > 0) {
				ret = pclose(fp);
				if (ret == -1) {
					ERR("pclose error.");
					return -1;
				}
				ret = 0;
				break;
			}
		} else {
			ERR("Cant get reture string on stdout.");
			return -1;
		}

		time(&end);
		seconds = difftime(end, start);
		if (seconds > 20) {
			ERR("time out.");
			return -1;
		}
		sleep(3);
	}

	return ret;
}

int unicorn_write_cfg(UnicornFrame *frame, char *buf, int sockfd)
{
	int ret = 0;
	char *jstr = (char *)frame->data;

#ifdef CCSERVER_SUPPORT
	ret = ccClientSet(jstr);
	if (ret == 0)
		DBG_MED("success setting to cc\n");
	else
		DBG_MED("fail setting to cc\n");

	snprintf(buf, sizeof(ret), "%d", ret);
#else
	AGTX_UNUSED(buf);

	ret = unicorn_json_validation(jstr, frame->data_len);
	if (ret) {
		ERR("Invalid JSON string\n");
		return ret;
	}

	char *module_name = unicorn_json_get_string(jstr, "module", strlen(jstr));
	if (module_name == NULL) {
		DBG_MED("Unable to get module name, %s\n", jstr);
		return -1;
	}
	int cmd_id = get_command_id(module_name);
	if (module_name) {
		free(module_name);
	}

	if (ret < 0) {
		return ret;
	}

	ret = setMpiSetting(jstr, frame->data_len, cmd_id, sockfd);
#endif

	return ret;
}

int unicorn_read_cfg(UnicornFrame *frame, char *buf)
{
	int ret = 0;

	strncpy(buf, (const char *)frame->data, strlen((const char *)frame->data));

#ifdef CCSERVER_SUPPORT
	ret = ccClientGet(buf, CC_GET_REG);
	DBG_MED("unicorn_read_cfg returns %d\n", ret);
#else
	char *module_name = unicorn_json_get_string(buf, "module", frame->data_len);
	if (module_name == NULL) {
		DBG_MED("Unable to get module name, %s\n", buf);
		return -1;
	}
	int cmd_id = get_command_id(module_name);
	if (module_name) {
		free(module_name);
	}
	if (ret < 0) {
		return ret;
	}
	char *buf2;
	ret = getMpiSetting(buf, &buf2, cmd_id, NULL);
	if (buf2 == NULL) {
		return -1;
	}
	strncpy(buf, buf2, UNICORN_FRAME_MAX_LEN);
	buf[UNICORN_FRAME_MAX_LEN - 1] = '\0';
	free(buf2);

#endif
	return ret;
}

int unicorn_access_mode(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	char cmd[256] = { 0 };
	char *mode;
	char *isReboot;

	if (strcmp(type, "READ") == 0) {
		sprintf(cmd, "cat /usrdata/mode");
		ret = captureStdoutStr(cmd, buf);
	} else if (strcmp(type, "WRITE") == 0) {
		mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
		isReboot = unicorn_json_get_string(jStr, "isReboot", strlen(jStr));
		if (mode == NULL || isReboot == NULL) {
			ERR("Error: Unable to get mode or isReboot, %s\n", jStr);
			ret = -1;
		} else {
			sprintf(cmd, "/system/bin/mode %s", mode);
			ret = executeSystemCommand(cmd);
		}

		if (ret == 0) {
			if (strcmp(isReboot, "TRUE") == 0) {
				ret = executeSystemCommand("reboot");
			}
		}
		sprintf(buf, "%d", ret);
		if (mode) {
			free(mode);
		}
		if (isReboot) {
			free(isReboot);
		}
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
		sprintf(buf, "%d", ret);
	}

	return ret;
}

int unicorn_factory_reset(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	char cmd[] = "touch /usrdata/reset_file";
	char cmd_reboot[] = "reboot";

	ret = executeSystemCommand(cmd);
	if (ret == 0)
		ret = executeSystemCommand(cmd_reboot);

	sprintf(buf, "%d", ret);

	return ret;
}

int unicorn_wifi_mac_addr(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char cmd[128];
	char *jStr = (char *)frame->data;
	char iface[] = "wlan0";
	char key[] = "wifiaddr";
	char *mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
	char *mac_addr = unicorn_json_get_string(jStr, "mac_addr", strlen(jStr));

	if (mode == NULL || mac_addr == NULL) {
		ERR("Error: Unable to get mode or mac_arrd, %s\n", jStr);
		ret = -1;
	} else if (strcmp(mode, "DYNAMIC") == 0) {
		if (strcmp(type, "READ") == 0) {
			sprintf(cmd, "cat /sys/class/net/%s/address", iface);
			ret = captureStdoutStr(cmd, buf);
		} else if (strcmp(type, "WRITE") == 0) {
			sprintf(cmd, "/sbin/ifdown %s", iface);
			ret = executeSystemCommand(cmd);
			if (ret == 0) {
				sprintf(cmd, "ifconfig %s hw ether %s", iface, mac_addr);
				ret = executeSystemCommand(cmd);
				if (ret == 0) {
					sprintf(cmd, "/sbin/ifup %s", iface);
					ret = executeSystemCommand(cmd);
				}
			}
			sprintf(buf, "%d", ret);
		}
	} else if (strcmp(mode, "STATIC") == 0) {
		ret = accessUbootReg(key, mac_addr, buf, type);
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
		sprintf(buf, "%d", ret);
	}

	if (mode) {
		free(mode);
	}
	if (mac_addr) {
		free(mac_addr);
	}
	return ret;
}

int unicorn_bt_mac_addr(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char cmd[128];
	char *jStr = (char *)frame->data;
	char iface[] = "bt0";
	char key[] = "btaddr";
	char *mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
	char *mac_addr = unicorn_json_get_string(jStr, "mac_addr", strlen(jStr));

	if (mode == NULL || mac_addr == NULL) {
		ERR("Error: Unable to get mode or mac_addr, %s\n", jStr);
		ret = -1;
	} else if (strcmp(mode, "DYNAMIC") == 0) {
		if (strcmp(type, "READ") == 0) {
			sprintf(cmd, "cat /sys/class/net/%s/address", iface);
			ret = captureStdoutStr(cmd, buf);
		} else if (strcmp(type, "WRITE") == 0) {
			sprintf(cmd, "/sbin/ifdown %s", iface);
			ret = executeSystemCommand(cmd);
			if (ret == 0) {
				sprintf(cmd, "ifconfig %s hw ether %s", iface, mac_addr);
				ret = executeSystemCommand(cmd);
				if (ret == 0) {
					sprintf(cmd, "/sbin/ifup %s", iface);
					ret = executeSystemCommand(cmd);
				}
			}
			sprintf(buf, "%d", ret);
		}
	} else if (strcmp(mode, "STATIC") == 0) {
		ret = accessUbootReg(key, mac_addr, buf, type);
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
		sprintf(buf, "%d", ret);
	}
	if (mode) {
		free(mode);
	}
	if (mac_addr) {
		free(mac_addr);
	}
	return ret;
}

int unicorn_eth_mac_addr(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char cmd[128];
	char *jStr = (char *)frame->data;
	char iface[] = "eth0";
	char key[] = "ethaddr";
	char *mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
	char *mac_addr = unicorn_json_get_string(jStr, "mac_addr", strlen(jStr));

	if (mode == NULL || mac_addr == NULL) {
		ERR("Error: Unable to get mode or mac_addr, %s\n", jStr);
		ret = -1;
	} else if (strcmp(mode, "DYNAMIC") == 0) {
		if (strcmp(type, "READ") == 0) {
			sprintf(cmd, "cat /sys/class/net/%s/address", iface);
			ret = captureStdoutStr(cmd, buf);
		} else if (strcmp(type, "WRITE") == 0) {
			sprintf(cmd, "/sbin/ifdown %s", iface);
			ret = executeSystemCommand(cmd);
			if (ret == 0) {
				sprintf(cmd, "ifconfig %s hw ether %s", iface, mac_addr);
				ret = executeSystemCommand(cmd);
				if (ret == 0) {
					sprintf(cmd, "/sbin/ifup %s", iface);
					ret = executeSystemCommand(cmd);
				}
			}
			sprintf(buf, "%d", ret);
		}
	} else if (strcmp(mode, "STATIC") == 0) {
		ret = accessUbootReg(key, mac_addr, buf, type);
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
		sprintf(buf, "%d", ret);
	}

	if (mode) {
		free(mode);
	}
	if (mac_addr) {
		free(mac_addr);
	}
	return ret;
}

int unicorn_usb_mac_addr(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char cmd[128];
	char *jStr = (char *)frame->data;
	char iface[] = "usb0";
	char key[] = "usbaddr";
	char *mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
	char *mac_addr = unicorn_json_get_string(jStr, "mac_addr", strlen(jStr));

	if (mode == NULL || mac_addr == NULL) {
		ERR("Error: Unable to get mode or mac_addr, %s\n", jStr);
		ret = -1;
	} else if (strcmp(mode, "DYNAMIC") == 0) {
		if (strcmp(type, "READ") == 0) {
			sprintf(cmd, "cat /sys/class/net/%s/address", iface);
			ret = captureStdoutStr(cmd, buf);
		} else if (strcmp(type, "WRITE") == 0) {
			sprintf(cmd, "/sbin/ifdown %s", iface);
			ret = executeSystemCommand(cmd);
			if (ret == 0) {
				sprintf(cmd, "ifconfig %s hw ether %s", iface, mac_addr);
				ret = executeSystemCommand(cmd);
				if (ret == 0) {
					sprintf(cmd, "/sbin/ifup %s", iface);
					ret = executeSystemCommand(cmd);
				}
			}
			sprintf(buf, "%d", ret);
		}
	} else if (strcmp(mode, "STATIC") == 0) {
		ret = accessUbootReg(key, mac_addr, buf, type);
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
		sprintf(buf, "%d", ret);
	}

	if (mode) {
		free(mode);
	}
	if (mac_addr) {
		free(mac_addr);
	}
	return ret;
}

int unicorn_read_fw_version(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	char cmd[] = "cat /etc/sw-version";

	ret = captureStdoutStr(cmd, buf);

	return ret;
}

int unicorn_mb_number(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char key[] = "mb_number";
	char reg[128];
	sprintf(reg, "%s", frame->data);

	ret = accessUbootReg(key, reg, buf, type);

	return ret;
}

int unicorn_serial_number(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char key[] = "serial_number";
	char reg[128];
	sprintf(reg, "%s", frame->data);

	ret = accessUbootReg(key, reg, buf, type);

	return ret;
}

int unicorn_product_id(UnicornFrame *frame, char *buf, const char *type)
{
	int ret = 0;
	char key[] = "model";
	char reg[128];
	sprintf(reg, "%s", frame->data);

	ret = accessUbootReg(key, reg, buf, type);

	return ret;
}

int unicorn_sd_test(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	char filename[128];
	const char test_buffer[] = "Hello world!";
	snprintf(filename, sizeof(filename), "/mnt/sdcard/sd_test.txt");
	int len = strlen(test_buffer) + 1; // null-terminated

	FILE *test_fp = fopen(filename, "w+");
	DBG_MED("ard write, size = %d\n", len);
	if (test_fp) {
		int ret_w = fwrite(test_buffer, 1, sizeof(test_buffer), test_fp);
		DBG_MED("Card write, size = %d\n", ret_w);

		fclose(test_fp);
		test_fp = NULL;

		ret = (ret_w == len) ? 0 : -1;
	} else {
		ret = -1;
	}

	sprintf(buf, "%d", ret);

	return ret;
}

int unicorn_ir_cut_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *ctrl;
	int gpio0 = -1, gpio1 = -1;
	char *jStr = (char *)frame->data;
	char script[] = "ir_cut.sh";
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	ctrl = unicorn_json_get_string(jStr, "ctrl", strlen(jStr));

	if (ctrl == NULL) {
		ERR("Unable to get ctrl, %s\n", jStr);
		return -1;
	}

	gpio0 = unicorn_json_get_int(jStr, "gpio0", strlen(jStr));
	gpio1 = unicorn_json_get_int(jStr, "gpio1", strlen(jStr));
	if ((gpio0 == -1) || (gpio1 == -1)) {
		ret = getGpioInfo();
		if (ret == 0) {
			gpio0 = g_gpio.ir_cut[0].pin_num;
			gpio1 = g_gpio.ir_cut[1].pin_num;
		}
	}

	if ((gpio0 != -1) && (gpio1 != -1)) {
		if (strcmp(ctrl, "ON") == 0) {
			sprintf(cmd, "%s/%s %d %d active", path, script, gpio0, gpio1);
			ret = executeSystemCommand(cmd);
		} else if (strcmp(ctrl, "OFF") == 0) {
			sprintf(cmd, "%s/%s %d %d remove", path, script, gpio0, gpio1);
			ret = executeSystemCommand(cmd);
		} else {
			ERR("There is no %s ctrl for control ir cut.", ctrl);
			ret = -1;
		}
	} else {
		ERR("There can't get gpio of ir cut.");
		ret = -1;
	}

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	if (ctrl) {
		free(ctrl);
	}
	return ret;
}

int unicorn_ir_led_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	int gpio;
	char *ctrl;
	char *jStr = (char *)frame->data;
	char script[] = "/system/mpp/script/ir_led.sh";
	char cmd[128];

	ctrl = unicorn_json_get_string(jStr, "ctrl", strlen(jStr));
	if (ctrl == NULL) {
		ERR("Error: Unable to get ctrl, %s\n", jStr);
		return -1;
	}

	gpio = unicorn_json_get_int(jStr, "gpio", strlen(jStr));
	if (gpio == -1) {
		ret = getGpioInfo();
		if (ret == 0) {
			gpio = g_gpio.ir_led.pin_num;
		}
	}

	if (gpio != -1) {
		if (strcmp(ctrl, "ON") == 0) {
			sprintf(cmd, "%s %d on", script, gpio);
			ret = executeSystemCommand(cmd);
		} else if (strcmp(ctrl, "OFF") == 0) {
			sprintf(cmd, "%s %d off", script, gpio);
			ret = executeSystemCommand(cmd);
		} else {
			DBG_MED("There is no %s ctrl for control ir led.", ctrl);
			ret = -1;
		}
	} else {
		ERR("There can't get gpio of ir led.");
		ret = -1;
	}

	sprintf(buf, "%d", ret);

	if (ctrl) {
		free(ctrl);
	}
	return ret;
}

int unicorn_led_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *option;
	char *jStr = (char *)frame->data;
	char bin[] = "ledctrl";
	char conf[] = "led.conf";
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	option = unicorn_json_get_string(jStr, "option", strlen(jStr));
	if (option == NULL) {
		ERR("Error: Unable to get option, %s\n", jStr);
		return -1;
	}
	sprintf(cmd, "%s/%s %s %s/%s", path, bin, option, path, conf);
	ret = executeSystemCommand(cmd);

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	if (option) {
		free(option);
	}
	return ret;
}

int unicorn_rssi(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	char cmd[] = "iwconfig wlan0 | awk '/Signal level/{print substr($4,7)}'";

	ret = captureStdoutStr(cmd, buf);

	return ret;
}

int unicorn_light_sensor(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	int chn = 0;
	char *jStr = (char *)frame->data;
	char cmd[128];

	chn = unicorn_json_get_int(jStr, "chn", strlen(jStr));
	if (chn == -1) {
		ret = getAdcInfo();
		if (ret == 0) {
			chn = g_adc.light_sensor.chn;
		}
	}

	if (chn != -1) {
		sprintf(cmd, "cat /sys/bus/iio/devices/iio:device0/in_voltage%d_raw", chn);
		ret = captureStdoutStr(cmd, buf);
	} else {
		ERR("There can't get gpio of ir cut.");
		ret = -1;
	}

	return ret;
}

int unicorn_audio_ctrl(const char *path, const char *file, const char *type, const int volume)
{
	int ret = 0;
	char cmd[128];
	char bin[] = "audioctrl";

	if (strcmp(type, "START") == 0) {
		if (fileExists(file) == true) {
			ret = remove(file);
		}
		if (ret == 0) {
			sprintf(cmd, "%s/%s -s %s &", path, bin, file);
			DBG_MED("cmd %s", cmd);
			ret = executeSystemCommand(cmd);
			DBG_MED("=== Start record ... ===");
		}
	} else if (strcmp(type, "END") == 0) {
		sprintf(cmd, "%s/%s -e", path, bin);
		DBG_MED("cmd %s", cmd);
		ret = executeSystemCommand(cmd);
		DBG_MED("=== Finish record ===");
	} else if (strcmp(type, "PLAY") == 0) {
		if (volume == -1) {
			sprintf(cmd, "%s/%s -p %s", path, bin, file);
			DBG_MED("cmd %s", cmd);
		} else {
			sprintf(cmd, "%s/%s -p %s -v %d", path, bin, file, volume);
			DBG_MED("cmd %s", cmd);
		}
		DBG_MED("=== %s ===", cmd);
		ret = executeSystemCommand(cmd);
		DBG_MED("=== Finish play ===");
	} else {
		ERR("Error: Unknown command %s\n", type);
		ret = -1;
	}

	return ret;
}

int unicorn_audio_speaker(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	int volume = 100;
	char *file;
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if ((strcmp(path, "") == 0) || (path == NULL)) {
		sprintf(path, "%s", "/tmp");
	}

	file = unicorn_json_get_string(jStr, "file", strlen(jStr));
	if (file == NULL) {
		ERR("Error: Unable to get file, %s\n", jStr);
		return -1;
	}

	volume = unicorn_json_get_int(jStr, "volume", strlen(jStr));

	if (volume < 0)
		volume = -1;
	else if (volume > 100)
		volume = -1;

	ret = unicorn_audio_ctrl(path, file, "PLAY", volume);
	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	if (file) {
		free(file);
	}
	return ret;
}

int unicorn_audio_mic(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	char *file;
	int rec_time = 5;
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	file = unicorn_json_get_string(jStr, "file", strlen(jStr));
	if (file == NULL) {
		ERR("Error: Unable to get file, %s\n", jStr);
		return -1;
	}
	rec_time = unicorn_json_get_int(jStr, "rec_time", strlen(jStr));

	ret = unicorn_audio_ctrl(path, file, "START", 0);
	if (ret == 0) {
		sleep(rec_time);
		ret = unicorn_audio_ctrl(path, file, "END", 0);
	}

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	if (file) {
		free(file);
	}
	return ret;
}

int unicorn_button_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0, gpio = 0;
	char *jStr = (char *)frame->data;
	char *ctrl;
	char bin[] = "btndetect";
	char config[] = "btn.conf";
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	ctrl = unicorn_json_get_string(jStr, "ctrl", strlen(jStr));
	if (ctrl == NULL) {
		ERR("Error: Unable to get ctrl, %s\n", jStr);
		return -1;
	}
	gpio = unicorn_json_get_int(jStr, "gpio", strlen(jStr));

	if (strcmp(ctrl, "EXPORT") == 0) {
		sprintf(cmd, "%s/%s -e %s/%s", path, bin, path, config);
		ret = executeSystemCommand(cmd);
		sprintf(buf, "%d", ret);
	} else if (strcmp(ctrl, "VALUE") == 0) {
		sprintf(cmd, "cat /sys/class/gpio/gpio%d/value", gpio);
		ret = captureStdoutStr(cmd, buf);
	} else if (strcmp(ctrl, "UNEXPORT") == 0) {
		sprintf(cmd, "%s/%s %s/%s", path, bin, path, config);
		ret = executeSystemCommand(cmd);
		sprintf(buf, "%d", ret);
	}

	if (path) {
		free(path);
	}
	if (ctrl) {
		free(ctrl);
	}
	return ret;
}

int unicorn_lvds_test(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	char kill_bin[] = "killall -s TERM av_main2";
	char bin[] = "lvds_test";
	char config[] = "case_config_lvds.conf";
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	ret = executeSystemCommand(kill_bin);
	if (ret != -1) {
		ret = executeSystemCommand("/system/mpp/script/load_mpp.sh -i");
		if (ret != -1) {
			sprintf(cmd, "%s/%s -d %s/%s", path, bin, path, config);
			ret = executeSystemCommand(cmd);
		}
	}

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	return ret;
}

int unicorn_sensor_test(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	char *file;
	int chn = 0;

	file = unicorn_json_get_string(jStr, "file", strlen(jStr));
	if (file == NULL) {
		ERR("Error: Unable to get file, %s\n", jStr);
		return -1;
	}
	ret = captureJpeg(file, chn);
	sprintf(buf, "%d", ret);

	if (file) {
		free(file);
	}
	return ret;
}

int unicorn_send_file_cmd(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0;
	char *jStr = (char *)frame->data;
	char *cmd;
	bool fExists;

	g_fPath = NULL;
	g_fRead = 0;
	g_fSize = 0;
	frame->frame_type = FRAME_TYPE_ACK;

	g_fPath = unicorn_json_get_string(jStr, "file", strlen(jStr));
	cmd = unicorn_json_get_string(jStr, "cmd", strlen(jStr));

	if (g_fPath == NULL || cmd == NULL) {
		ERR("Error: Unable to get file or cmd, %s\n", jStr);
		goto FAILURE;
	}

	DBG_MED("g_fPath : %s\n", g_fPath);
	DBG_MED("cmd : %s\n", cmd);

	if (strcmp(cmd, "START") == 0) {
		g_fd = -1;
		fExists = fileExists(g_fPath);
		if (fExists == true) {
			g_fSize = getFileSize(g_fPath);
			DBG_MED("fSize : %ld\n", g_fSize);
			ret = g_fSize;
		} else {
			DBG_MED("No file exist: %s\n", g_fPath);
			goto FAILURE;
		}
		g_fd = open(g_fPath, O_RDONLY, 0); // perms always set to 0
		if (g_fd == -1) {
			ERR("open file to send");
			goto FAILURE;
		}
	} else if (strcmp(cmd, "TERMINATE") == 0) {
		close(g_fd);
		g_fd = -1;
		ret = 0;
	}

	sprintf(buf, "%d", ret);
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	if (cmd) {
		free(cmd);
	}
	return ret;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	if (cmd) {
		free(cmd);
	}
	return ret;
}

int unicorn_send_file_ack(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0, ret_str = 0;
	int single_send_size = 16384; // * 2 - 256;

	frame->frame_type = FRAME_TYPE_DATA;

	if ((g_fRead < g_fSize) && (g_fd != -1)) {
		ret = read(g_fd, buf, single_send_size);
		if (ret < 0) {
			DBG_MED("Error !!! fread. \n");
			ERR("fread");
			goto FAILURE;
		}

		g_fRead += single_send_size;
		DBG_MED("fRead = %ld, fSize = %ld\n", g_fRead, g_fSize);
		if ((g_fRead >= g_fSize)) {
			ret_str = single_send_size - (g_fRead - g_fSize);
		} else {
			ret_str = single_send_size;
		}
	}

	return ret_str;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	return strlen(buf);
}

int unicorn_send_file(UnicornFrame *frame, char *buf, int sockfd)
{
	int ret = 0;
	int frame_type = frame->frame_type;
	char *jStr = (char *)frame->data;
	char *cmd = NULL;
	bool fExists;
	int sendybtes = 0;

	if (frame_type == FRAME_TYPE_CMD) {
		g_fPath = NULL;
		g_fRead = 0;
		g_fSize = 0;

		g_fPath = unicorn_json_get_string(jStr, "file", strlen(jStr));
		cmd = unicorn_json_get_string(jStr, "cmd", strlen(jStr));

		if (g_fPath == NULL || cmd == NULL) {
			ERR("Error: Unable to get g_fPath or cmd, %s\n", jStr);
			goto FAILURE;
		}
		DBG_MED("g_fPath : %s\n", g_fPath);
		DBG_MED("cmd : %s\n", cmd);
		if (strcmp(cmd, "TERMINATE") == 0) {
			close(g_fd);
			g_fd = -1;
			ret = 0;
			sprintf(buf, "%d", ret);
			if (g_fPath) {
				free(g_fPath);
				g_fPath = NULL;
			}
			if (cmd) {
				free(cmd);
			}
			return ret;
		} else if (strcmp(cmd, "START") == 0) {
			g_fd = -1;
			fExists = fileExists(g_fPath);
			if (fExists == true) {
				g_fSize = getFileSize(g_fPath);
				DBG_MED("fSize : %ld\n", g_fSize);
				ret = g_fSize;
			} else {
				DBG_MED("No file exist: %s\n", g_fPath);
				goto FAILURE;
			}
			g_fd = open(g_fPath, O_RDONLY, 0); // perms always set to 0
			if (g_fd == -1) {
				ERR("open file to send");
				goto FAILURE;
			}
		}

		if (g_fPath) {
			free(g_fPath);
			g_fPath = NULL;
		}
		if (cmd) {
			free(cmd);
		}
	}

	if (frame_type == FRAME_TYPE_ACK) {
		while (g_fRead != g_fSize) {
			sendybtes = sendfile(sockfd, g_fd, 0, BUFSIZE);
			if (sendybtes < 0) {
				DBG_MED("Error !!! sent file. \n");
				ERR("sendfile");
				goto FAILURE;
			}

			g_fRead += sendybtes;
			DBG_MED("fRead = %ld, fSize = %ld\n", g_fRead, g_fSize);
		}

		if (g_fRead == g_fSize) {
			ret = 0;
		}
	}

	sprintf(buf, "%d", ret);
	return ret;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	if (cmd) {
		free(cmd);
	}
	return ret;
}

int unicorn_recv_file_cmd(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0;
	int frame_type = frame->frame_type;
	char *jStr = (char *)frame->data;
	char *cmd;

	DBG_MED("frame_type = %d, %d\n", frame_type, FRAME_TYPE_CMD);

	frame->frame_type = FRAME_TYPE_ACK;
	g_fPath = NULL;
	g_fRead = 0;
	g_fSize = 0;
	/** 2. Get file_path, fSize and open file, then ACK **/
	cmd = unicorn_json_get_string(jStr, "cmd", strlen(jStr));
	g_fPath = unicorn_json_get_string(jStr, "file", strlen(jStr));

	if (cmd == NULL || g_fPath == NULL) {
		ERR("Error: Unable to get cmd or file, %s\n", jStr);
		goto FAILURE;
	}

	g_fSize = unicorn_json_get_int(jStr, "fSize", strlen(jStr));
	DBG_MED("cmd = '%s'\n", cmd);
	DBG_MED("file = '%s'\n", g_fPath);
	DBG_MED("fSize = %lu\n", g_fSize);

	if (strcmp(cmd, "START") == 0) {
		/** open file **/
		g_fd = open(g_fPath, O_WRONLY | O_CREAT);
		if (g_fd == -1) {
			ERR("open during recvFile");
			goto FAILURE;
		}
	}
	sprintf(buf, "%d", ret);
	if (cmd) {
		free(cmd);
	}
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	return ret;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	if (cmd) {
		free(cmd);
	}
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	return ret;
}

int unicorn_recv_file_data(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0, ret_len = 0;
	int readbytes = frame->data_len;
	int frame_type = frame->frame_type;
	char *jStr = (char *)frame->data;
	char buffer[BUFSIZE];
	//unsigned long int fRead = 0;
	DBG_MED("frame_type = %d, %d\n", frame_type, FRAME_TYPE_CMD);

	frame->frame_type = FRAME_TYPE_ACK;

	/* write to file */
	strncpy(buffer, jStr, readbytes);
	DBG_MED("%%%%%%%%%%%%%%%% reading %d bytes from socket\n", readbytes);
	ret_len = write(g_fd, buffer, readbytes);
	DBG_MED("reading %d bytes from socket\n", ret_len);
	if (ret_len != readbytes) {
		ERR("write");
		goto FAILURE;
	}

	g_fRead += readbytes;
	DBG_MED("fRead = %ld, fSize = %ld\n", g_fRead, g_fSize);

	if (g_fRead == g_fSize) {
		DBG_MED("close file \n");
		close(g_fd);

		/*ret = system("sync");
		DBG_MED( "sync ret %d\n", ret);
		if (ret == -1) {
			ERR("system sync");
			goto FAILURE;
		}*/

		/*char chmod_cmd[256];
		sprintf(chmod_cmd,"chmod 644 %s", g_fPath);
		ret = system(chmod_cmd);
		DBG_MED( "chmod_cmd ret %d\n", ret);
		if (ret == -1) {
			ERR("system chmod_cmd");
			goto FAILURE;
		}*/

		/*if ((strstr(g_fPath, ".txt") != NULL) || (strstr(g_fPath, ".json") != NULL) || (strstr(g_fPath, ".log") != NULL)) {
			char dos2unix[256];
			sprintf(dos2unix,"dos2unix %s", g_fPath);
			ret = system(dos2unix);
			DBG_MED( "dos2unix ret %d\n", ret);
			if (ret == -1) {
				ERR("system dos2unix");
				goto FAILURE;
			}
		}*/
	}

	sprintf(buf, "%d", ret);
	return ret;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	return ret;
}

int unicorn_recv_file(UnicornFrame *frame, char *buf, int sockfd)
{
	int ret = 0;
	int readbytes = frame->data_len;
	int frame_type = frame->frame_type;
	char *jStr = (char *)frame->data;
	unsigned long int fRead = 0;
	char buffer[BUFSIZE];
	DBG_MED("frame_type = %d, %d\n", frame_type, FRAME_TYPE_CMD);

	/** 2. Get file_path, fSize and open file, then ACK **/
	g_fPath = unicorn_json_get_string(jStr, "file", strlen(jStr));
	if (g_fPath == NULL) {
		ERR("Error: Unable to get file, %s\n", jStr);
		goto FAILURE;
	}
	g_fSize = unicorn_json_get_int(jStr, "fSize", strlen(jStr));
	DBG_MED("file = '%s'\n", g_fPath);
	DBG_MED("fSize = %lu\n", g_fSize);

	/** open file **/
	g_fd = open(g_fPath, O_WRONLY | O_CREAT);
	if (g_fd == -1) {
		ERR("open during recvFile");
		goto FAILURE;
	}
	sprintf(buf, "%d", ret);
	ret = unicorn_put_frame(sockfd, 0x25, frame_type, buf, strlen(buf));

	while (fRead != g_fSize) {
		/* read form socket */
		readbytes = read(sockfd, buffer, BUFSIZE);
		if (readbytes == -1) {
			ERR("read sockfd");
			goto FAILURE;
		}

		DBG_MED("reading %d bytes from socket\n", readbytes);
		/* write to file */
		if (write(g_fd, buffer, readbytes) == -1) {
			ERR("write");
			goto FAILURE;
		}

		fRead += readbytes;
		DBG_MED("fRead = %ld, fSize = %ld\n", fRead, g_fSize);
	}
	close(g_fd);
	ret = 0;
	/*ret = system("sync");
	DBG_MED( "sync ret %d\n", ret);
	if (ret == -1) {
		ERR("system sync");
		goto FAILURE;
	}*/

	/*char chmod_cmd[256];
	sprintf(chmod_cmd,"chmod 644 %s", g_fPath);
	ret = system(chmod_cmd);
	DBG_MED( "chmod_cmd ret %d\n", ret);
	if (ret == -1) {
		ERR("system chmod_cmd");
		goto FAILURE;
	}*/

	/*if ((strstr(g_fPath, ".txt") != NULL) || (strstr(g_fPath, ".json") != NULL) || (strstr(g_fPath, ".log") != NULL)) {
		char dos2unix[256];
		sprintf(dos2unix,"dos2unix %s", g_fPath);
		ret = system(dos2unix);
		DBG_MED( "dos2unix ret %d\n", ret);
		if (ret == -1) {
			ERR("system dos2unix");
			goto FAILURE;
		}
	}*/

	sprintf(buf, "%d", ret);
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	return ret;

FAILURE:
	ret = -1;
	sprintf(buf, "%d", ret);
	if (g_fPath) {
		free(g_fPath);
		g_fPath = NULL;
	}
	return ret;
}

int unicorn_oc_adjust_preview(UnicornFrame *frame, char *buf)
{
	char *jstr = (char *)frame->data;
	int ret = ccClientSet(jstr);
	if (ret == 0)
		DBG_MED("success setting to cc\n");
	else
		DBG_MED("fail setting to cc\n");

	snprintf(buf, sizeof(ret), "%d", ret);

	return ret;
}

int unicorn_oc_adjust_apply(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0;
	char upd_cmd[256];
	char file_path[256];
	char upd_file_path[256];
	char cc_buf[256] = "";
	char *string = (char *)frame->data;
	int int_array[3] = { 0 };

	parseIntArray(string, int_array);
	sprintf(cc_buf,
	        "{\"cmd_id\": \"AGTX_CMD_PANORAMA_CONF\",\"json_content\": {\"center_offset_x\":%d,\"center_offset_y\":%d, \"radius\":%d}}",
	        int_array[0], int_array[1], int_array[2]);

	ret = getCalibFilePath("PANORAMA", file_path);
	DBG_MED("file_path: %s\n", file_path);
	if (ret == 0) {
		/* file open then write data */
		ret = writeJsonDataToFile(file_path, cc_buf);

		ret = getCalibFilePath("PANORAMA_UPD", upd_file_path);
		DBG_MED("upd_file_path: %s\n", upd_file_path);

		if (ret == 0) {
			sprintf(upd_cmd, "touch %s", upd_file_path);
			ret = executeSystemCommand(upd_cmd);
			if ((ret != 0) || (fileExists(upd_file_path) == false)) {
				ret = -1;
			}
		}
	}

	sprintf(buf, "%d", ret);

	return ret;
}

int unicorn_ac_freq_preview(UnicornFrame *frame, char *buf)
{
	char *jstr = (char *)frame->data;
	int ret = ccClientSet(jstr);
	if (ret == 0)
		DBG_MED("success setting to cc\n");
	else
		DBG_MED("fail setting to cc\n");

	snprintf(buf, sizeof(ret), "%d", ret);

	return ret;
}

int unicorn_ac_freq_apply(UnicornFrame *frame, char *buf, int sockfd)
{
	AGTX_UNUSED(sockfd);

	int ret = 0;
	char upd_cmd[256];
	char file_path[256];
	char upd_file_path[256];
	char cc_buf[256] = "";
	char *string = (char *)frame->data;
	int frequency_idx;
	int frequency_en;
	int int_array[1] = { 0 };
	parseIntArray(string, int_array);
	if (int_array[0] == 50) { // 50Hz
		frequency_en = 1;
		frequency_idx = 0;
	} else if (int_array[0] == 60) { // 60Hz
		frequency_en = 1;
		frequency_idx = 1;
	} else if (int_array[0] == -1) { // off
		frequency_en = 0;
		frequency_idx = 0;
	} else {
		frequency_en = 0;
		frequency_idx = 0;
	}

	sprintf(cc_buf,
	        "{\"cmd_id\":\"AGTX_CMD_ANTI_FLICKER_CONF\", \"json_content\":{\"enable\": %d,\"frequency_idx\": %d }}",
	        frequency_en, frequency_idx);

	ret = getCalibFilePath("ANTI_FLICKER", file_path);
	DBG_MED("file_path: %s\n", file_path);
	if (ret == 0) {
		/* file open then write data */
		ret = writeJsonDataToFile(file_path, cc_buf);

		ret = getCalibFilePath("ANTI_FLICKER_UPD", upd_file_path);
		DBG_MED("upd_file_path: %s\n", upd_file_path);

		if (ret == 0) {
			sprintf(upd_cmd, "touch %s", upd_file_path);
			ret = executeSystemCommand(upd_cmd);
			if ((ret != 0) || (fileExists(upd_file_path) == false)) {
				ret = -1;
			}
		}
	}

	sprintf(buf, "%d", ret);

	return ret;
}

int unicorn_day_night_mode(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *mode;
	char *jStr = (char *)frame->data;
	char cmd[128];
	char *path;
	char bin[] = "daynightctrl";
	char config[] = "day_night.conf";
	char kill_bin[] = "killall -2 daynightctrl";

	ret = getGpioInfo();
	if (ret == 0) {
		ret = executeSystemCommand(kill_bin);

		path = unicorn_json_get_string(jStr, "path", strlen(jStr));
		if (path == NULL) {
			path = "/tmp";
		} else if (strcmp(path, "") == 0) {
			sprintf(path, "%s", "/tmp");
		}

		mode = unicorn_json_get_string(jStr, "mode", strlen(jStr));
		DBG_MED("mode: %s\n", mode);
		if (mode == NULL) {
			ERR("Error: Unable to get mode, %s\n", jStr);
			ret = -1;
		} else if (strcmp(mode, "DAY") == 0) {
			/* daynightctrl -d day_night.json */
			sprintf(cmd, "%s/%s -d %s/%s", path, bin, path, config);
			ret = executeSystemCommand(cmd);
		} else if (strcmp(mode, "NIGHT") == 0) {
			/* daynightctrl -n day_night.json */
			sprintf(cmd, "%s/%s -n %s/%s", path, bin, path, config);
			ret = executeSystemCommand(cmd);
		} else if (strcmp(mode, "AUTO") == 0) {
			sprintf(cmd, "%s/%s -a %s/%s &", path, bin, path, config);
			ret = executeSystemCommand(cmd);
		} else {
			ERR("Error: Unknown command %s\n", mode);
			ret = -1;
		}

		if (path) {
			free(path);
		}
		if (mode) {
			free(mode);
		}
	}

	sprintf(buf, "%d", ret);

	return ret;
}

void unicorn_ota_sysupd_command()
{
	int ret;
	const char *params = "sysupd";

	ret = system(params); // system return 0 as success

	if (ret == -1) {
		ERR("System error!\n");
	} else {
		if (WIFEXITED(ret)) {
			if (WEXITSTATUS(ret) == 0) {
				DBG_MED("(%s) be excuted successfully.\n", params);
			} else {
				ERR("Run cmd fail and exit code is %d (%s)!\n", WEXITSTATUS(ret), params);
			}
		} else {
			ERR("exit status is %d (%s)!\n", WEXITSTATUS(ret), params);
		}
	}
}

int unicorn_ota(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	pthread_t t0;

	ret = pthread_create(&t0, NULL, (void *)unicorn_ota_sysupd_command, NULL);
	if (ret != 0) {
		perror("pthread_create");
	}
	pthread_detach(t0);

	sprintf(buf, "%d", ret);
	return ret;
}

int unicorn_bad_pixel(UnicornFrame *frame, char *buf)
{
	AGTX_UNUSED(frame);

	int ret = 0;
	char file[] = "/tmp/bad_pixel_snap.jpeg";
	int chn = 0;

	ret = captureJpeg(file, chn);
	sprintf(buf, "%d", ret);

	return ret;
}

int unicorn_floodlight_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0;
	char *jStr = (char *)frame->data;
	char bin[] = "pir_test";
	char config[] = "pir_test.conf";
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	sprintf(cmd, "%s/%s %s/%s -f", path, bin, path, config);
	ret = executeSystemCommand(cmd);

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	return ret;
}

int unicorn_pir_ctrl(UnicornFrame *frame, char *buf)
{
	int ret = 0, pir_idx = 0;
	char bin[] = "pir_test";
	char config[] = "pir_test.conf";
	char *jStr = (char *)frame->data;
	char cmd[128];
	char *path;

	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	if (path == NULL) {
		path = "/tmp";
	} else if (strcmp(path, "") == 0) {
		sprintf(path, "%s", "/tmp");
	}

	pir_idx = unicorn_json_get_int(jStr, "pir_idx", strlen(jStr));
	sprintf(cmd, "%s/%s %s/%s -p %d", path, bin, path, config, pir_idx);
	ret = executeSystemCommand(cmd);

	sprintf(buf, "%d", ret);

	if (path) {
		free(path);
	}
	return ret;
}
