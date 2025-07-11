#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>

#include "hiber_demo.h"
#include "log.h"

#define DEBUG

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

/**
 * @brief parse pwr_mgmt node from config
 * @details
 * @param[in] *tmp_obj the json object of pwr_mgmt node
 * @param[in] *param   the parameter structure that is updated by config
 * @return int
 * @retval 0     parse success
 * @retval Not 0 parse failure
 * @see
 */
static int parsePwrMgmt(json_object *tmp_obj, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	char *key = NULL;
	json_bool is_valid = (json_bool)0;
	json_object *tmp_sub_obj = NULL;

	/* get 'gpio' node */
	key = "gpio";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		goto error;
	}
	param->pwr_mgmt.gpio = json_object_get_int(tmp_sub_obj);

	/* get 'active' node */
	key = "active";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		goto error;
	}
	param->pwr_mgmt.active_status = json_object_get_int(tmp_sub_obj);

	/* get 'adc_ch' node */
	key = "adc_ch";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		goto error;
	}
	param->pwr_mgmt.adc_ch = json_object_get_int(tmp_sub_obj);

	/* get 'off_thre' node */
	key = "off_thre";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		goto error;
	}
	param->pwr_mgmt.off_thre = json_object_get_int(tmp_sub_obj);

	/* get 'on_thre' node */
	key = "on_thre";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		goto error;
	}
	param->pwr_mgmt.on_thre = json_object_get_int(tmp_sub_obj);

	goto success;

error:
	DBG("Cannot get %s object.\n", key);
	ret = -EINVAL;
success:
	json_object_put(tmp_obj);
	return ret;
}

/**
 * @brief parse tcp node from config
 * @details
 * @param[in] *tmp_obj the json object of tcp node
 * @param[in] *param   the parameter structure that is updated by config
 * @return int
 * @retval 0     parse success
 * @retval Not 0 parse failure
 * @see
 */
static int parseTcp(json_object *tmp_obj, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	int ipv4[4] = { 0 };
	struct array_list *arr = NULL;
	json_object *tmp_sub_obj = NULL;
	json_object *tmp_arr_obj = NULL;
	unsigned char *tmp_str = NULL;
	int tmp_hex = 0;
	json_bool is_valid = (json_bool)0;
	char *key = NULL;

	if (!json_object_array_length(tmp_obj)) {
		goto not_enable;
	}
	param->tcp = (SLEEP_TCP *)malloc(sizeof(SLEEP_TCP));

	/* get 'ser_ip' node */
	key = "ser_ip";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		DBG("Cannot get %s object.\n", key);
		ret = -EINVAL;
		goto error;
	} else {
		/* update IP address */
		if (!json_object_array_length(tmp_sub_obj)) {
			DBG("%s can't be empty.\n", key);
			ret = -EINVAL;
			goto error;
		}
		arr = json_object_get_array(tmp_sub_obj);
		for (int i = 0; i < json_object_array_length(tmp_sub_obj); i++) {
			tmp_arr_obj = (json_object *)array_list_get_idx(arr, i);
			ipv4[i] = json_object_get_int(tmp_arr_obj);
		}
		param->tcp->ser_ip = IPV4(ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
	}

	/* get 'ser_port' node */
	key = "ser_port";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		DBG("Cannot get %s object.\n", key);
		ret = -EINVAL;
		goto error;
	} else {
		param->tcp->ser_port = json_object_get_int(tmp_sub_obj);
	}

	/* get 'heartbeat_pkg_str' node */
	key = "heartbeat_pkg_str";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		//if "heartbeat_pkg_hex" exists, this isn't an error
		DBG("Cannot get %s object. Set %s as NULL.\n", key, key);
		param->tcp->heartbeat_pkg = NULL;
	} else {
		tmp_str = (unsigned char *)json_object_get_string(tmp_sub_obj);
		if (strlen((const char *)tmp_str) > 0) {
			param->tcp->heartbeat_pkg_len = strlen((const char *)tmp_str);
			param->tcp->heartbeat_pkg =
			        (unsigned char *)malloc(sizeof(unsigned char) * param->tcp->heartbeat_pkg_len);
			memcpy(param->tcp->heartbeat_pkg, (const char *)tmp_str, param->tcp->heartbeat_pkg_len);
		} else {
			DBG("%s object is NULL.\n", key);
			param->tcp->wakeup_pkg = NULL;
			param->tcp->heartbeat_pkg = NULL;
		}
	}

	/* get 'heartbeat_pkg_hex' node */
	key = "heartbeat_pkg_hex";
	//if 'heartbeat_pkg_str' exists, then heartbeat_pkg_hex won't be set
	if (!param->tcp->heartbeat_pkg) {
		is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
		if (!is_valid) {
			//both heartbeat_pkg are invalid, goto error
			DBG("Cannot get %s object.\nThere's no heartbeat pkg setting.\n", key);
			ret = -EINVAL;
			goto error;
		} else {
			if (!json_object_array_length(tmp_sub_obj)) {
				DBG("%s object is NULL.\nThere's no heartbeat pkg setting.\n", key);
				ret = -EINVAL;
				goto error;
			}
			arr = json_object_get_array(tmp_sub_obj);
			param->tcp->heartbeat_pkg_len = json_object_array_length(tmp_sub_obj);
			param->tcp->heartbeat_pkg =
			        (unsigned char *)malloc(sizeof(unsigned char) * param->tcp->heartbeat_pkg_len);

			/* get each bit */
			for (int i = 0; i < param->tcp->heartbeat_pkg_len; i++) {
				tmp_arr_obj = (json_object *)array_list_get_idx(arr, i);
				sscanf((char *)json_object_get_string(tmp_arr_obj), "%02x", &tmp_hex);
				param->tcp->heartbeat_pkg[i] = (unsigned char)tmp_hex;
			}
		}
	}

	/* get 'heartbeat_period' node */
	key = "heartbeat_period";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		DBG("Cannot get %s object.\n", key);
		ret = -EINVAL;
		goto error;
	} else {
		param->tcp->heartbeat_period = json_object_get_int(tmp_sub_obj);
	}

	/* get 'wakeup_pkg_str' node */
	key = "wakeup_pkg_str";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	if (!is_valid) {
		//if "wakeup_pkg_hex" exists, this isn't an error
		DBG("Cannot get %s object. Set %s as NULL.\n", key, key);
		param->tcp->wakeup_pkg = NULL;
	} else {
		tmp_str = (unsigned char *)json_object_get_string(tmp_sub_obj);
		if (strlen((const char *)tmp_str) > 0) {
			param->tcp->wakeup_pkg_len = strlen((const char *)tmp_str);
			param->tcp->wakeup_pkg =
			        (unsigned char *)malloc(sizeof(unsigned char) * param->tcp->wakeup_pkg_len);
			memcpy(param->tcp->wakeup_pkg, (const char *)tmp_str, param->tcp->wakeup_pkg_len);
		} else {
			DBG("%s object is NULL.\n", key);
			param->tcp->wakeup_pkg = NULL;
		}
	}

	/* get 'wakeup_pkg_hex' node */
	key = "wakeup_pkg_hex";
	//if 'wakeup_pkg_str' exists, then wakeup_pkg_hex won't be set
	if (!param->tcp->wakeup_pkg) {
		is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
		if (!is_valid) {
			//both wakeup_pkg are invalid, goto error
			DBG("Cannot get %s object.\nThere's no wakeup pkg setting.\n", key);
			ret = -EINVAL;
			goto error;
		} else {
			if (!json_object_array_length(tmp_sub_obj)) {
				DBG("%s object is NULL.\nThere's no wakeup pkg setting.\n", key);
				ret = -EINVAL;
				goto error;
			}
			arr = json_object_get_array(tmp_sub_obj);
			param->tcp->wakeup_pkg_len = json_object_array_length(tmp_sub_obj);
			param->tcp->wakeup_pkg =
			        (unsigned char *)malloc(sizeof(unsigned char) * param->tcp->wakeup_pkg_len);
			/* get each bit */
			for (int i = 0; i < param->tcp->wakeup_pkg_len; i++) {
				tmp_arr_obj = (json_object *)array_list_get_idx(arr, i);
				sscanf((char *)json_object_get_string(tmp_arr_obj), "%02x", &tmp_hex);
				param->tcp->wakeup_pkg[i] = (unsigned char)tmp_hex;
			}
		}
	}

	/* get 'bad_conn_handle' node */
	key = "bad_conn_handle";
	is_valid = json_object_object_get_ex(tmp_obj, key, &tmp_sub_obj);
	//default will be set as waking up when bad connection occurred
	if (!is_valid) {
		DBG("Cannot get %s object. Set %s as 1.\n", key, key);
		param->tcp->bad_conn_handle = 1;
	} else {
		param->tcp->bad_conn_handle = json_object_get_int(tmp_sub_obj);
	}

	/* update 'sleep_tcp_wake' if all tcp related param. are ready */
	if (param->tcp->ser_ip && param->tcp->ser_port && param->tcp->heartbeat_pkg && param->tcp->heartbeat_period &&
	    param->tcp->wakeup_pkg) {
		param->sleep_tcp_wake = 1;
	}

error:
not_enable:
	json_object_put(tmp_obj);
	return ret;
}

/**
 * @brief parse sleep_gpio_det node from config
 * @details
 * @param[in] *tmp_obj the json object of sleep_gpio_det node
 * @param[in] *param   the parameter structure that is updated by config
 * @return int
 * @retval 0     parse success
 * @retval Not 0 parse failure
 * @see
 */
static int parseSleepGpioDet(json_object *tmp_obj, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	json_object *tmp_sub_obj = NULL;
	struct json_object_iter tmp_iter;
	char *tmp_str = NULL;
	int tmp_str_len = 0;
	int tmp_cnt = 0;
	json_bool is_valid = (json_bool)0;
	char *key = NULL;

	/* update 'sleep_gpio_det_cnt' */
	param->sleep_gpio_det_cnt = 0;
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		param->sleep_gpio_det_cnt += 1;
	}

	/* memory allocation for 'sleep_gpio_det' which is a pointer of SLEEP_GPIO_DET array*/
	param->sleep_gpio_det = (SLEEP_GPIO_DET *)malloc(sizeof(SLEEP_GPIO_DET) * param->sleep_gpio_det_cnt);

	/* loop for each gpio */
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		/* get 'gpio' node */
		key = "gpio";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->sleep_gpio_det[tmp_cnt].gpio = json_object_get_int(tmp_sub_obj);

		/* get 'event' node */
		key = "event";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		tmp_str = (char *)json_object_get_string(tmp_sub_obj);
		assert(strlen(tmp_str) == EVENT_LEN);
		tmp_str_len = strlen(tmp_str) + 1;
		snprintf(param->sleep_gpio_det[tmp_cnt].event, tmp_str_len, tmp_str);
		tmp_cnt += 1;
	}

	goto success;

error:
	DBG("Cannot get %s object.\n", key);
	ret = -EINVAL;
success:
	json_object_put(tmp_obj);
	return ret;
}

/**
 * @brief parse sleep_adc_det node from config
 * @details
 * @param[in] *tmp_obj the json object of sleep_adc_det node
 * @param[in] *param   the parameter structure that is updated by config
 * @return int
 * @retval 0     parse success
 * @retval Not 0 parse failure
 * @see
 */
static int parseSleepAdcDet(json_object *tmp_obj, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	json_object *tmp_sub_obj = NULL;
	struct json_object_iter tmp_iter;
	int tmp_cnt = 0;
	json_bool is_valid = (json_bool)0;
	char *key = NULL;

	/* update 'sleep_adc_cnt' */
	param->sleep_adc_det_cnt = 0;
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		param->sleep_adc_det_cnt += 1;
	}
	param->sleep_adc_det = (SLEEP_ADC_DET *)malloc(sizeof(SLEEP_ADC_DET) * param->sleep_adc_det_cnt);

	/* loop for each adc */
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		/* get 'adc' node */
		key = "adc";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->sleep_adc_det[tmp_cnt].adc = json_object_get_int(tmp_sub_obj);

		/* get 'threshold' node */
		key = "threshold";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->sleep_adc_det[tmp_cnt].threshold = json_object_get_int(tmp_sub_obj);

		/* get 'above_under' node */
		key = "above_under";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->sleep_adc_det[tmp_cnt].above_under = json_object_get_int(tmp_sub_obj);
		tmp_cnt += 1;
	}
	goto success;

error:
	DBG("Cannot get %s object.\n", key);
	ret = -EINVAL;
success:
	json_object_put(tmp_obj);
	return ret;
}

/**
 * @brief parse gpio_dir node from config
 * @details
 * @param[in] *tmp_obj the json object of gpio_dir node
 * @param[in] *param   the parameter structure that is updated by config
 * @return int
 * @retval 0     parse success
 * @retval Not 0 parse failure
 * @see
 */
static int parseGpioDir(json_object *tmp_obj, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	json_object *tmp_sub_obj = NULL;
	struct json_object_iter tmp_iter;
	int tmp_cnt = 0;
	json_bool is_valid = (json_bool)0;
	char *key = NULL;

	/* get 'gpio_dir_cnt' node */
	param->gpio_dir_cnt = 0;
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		param->gpio_dir_cnt += 1;
	}
	param->gpio_dir = (GPIO_DIR *)malloc(sizeof(GPIO_DIR) * param->gpio_dir_cnt);

	/* loop for each gpio */
	json_object_object_foreachC(tmp_obj, tmp_iter)
	{
		/* get 'gpio' node */
		key = "gpio";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->gpio_dir[tmp_cnt].gpio = json_object_get_int(tmp_sub_obj);

		/* get 'dir' node */
		key = "dir";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!is_valid) {
			goto error;
		}
		param->gpio_dir[tmp_cnt].dir = json_object_get_int(tmp_sub_obj);

		/* get 'out_val' node */
		key = "out_val";
		is_valid = json_object_object_get_ex(tmp_iter.val, key, &tmp_sub_obj);
		if (!tmp_sub_obj) {
			goto error;
		}
		param->gpio_dir[tmp_cnt].out_val = json_object_get_int(tmp_sub_obj);
		tmp_cnt += 1;
	}
	goto success;

error:
	DBG("Cannot get %s object.\n", key);
	ret = -EINVAL;
success:
	json_object_put(tmp_obj);
	return ret;
}

/**
 * @brief read config json file
 * @details
 * @param[in] *file_name the file name of the config
 * @param[in] *param     the parameter structure that is updated by config
 * @return int
 * @retval 0     read success
 * @retval Not 0 read failure
 * @see
 */
static int readJsonFromFile(const char *file_name, HIBER_DEMO_PARAM *param)
{
	int ret = 0;
	json_object *obj = NULL;
	json_object *tmp_obj = NULL;
	json_bool is_valid = (json_bool)0;
	char *key = NULL;

	/* load json config from file */
	obj = (file_name != NULL) ? json_object_from_file(file_name) : json_object_new_object();
	if (!obj) {
		log_err("Cannot open %s.\n", file_name);
		ret = -EBADF;
		goto error;
	}

	/* get param: pwr_mgmt */
	key = "pwr_mgmt";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	ret = parsePwrMgmt(tmp_obj, param);
	if (ret != 0) {
		goto error;
	}

	/* get param: tcp */
	key = "tcp";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	ret = parseTcp(tmp_obj, param);
	if (ret != 0) {
		goto error;
	}

	/* get param: sleep_gpio_det */
	key = "sleep_gpio_det";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	ret = parseSleepGpioDet(tmp_obj, param);
	if (ret != 0) {
		goto error;
	}

	/* get param: sleep_adc_det */
	key = "sleep_adc_det";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	ret = parseSleepAdcDet(tmp_obj, param);
	if (ret != 0) {
		goto error;
	}

	/* get param: sleep_timeout */
	key = "sleep_timeout";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	param->sleep_timeout = json_object_get_int(tmp_obj);

	/* get param: sleep_response */
	key = "sleep_response";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	param->sleep_response = json_object_get_int(tmp_obj);

	/* get param: gpio_dir */
	key = "gpio_dir";
	is_valid = json_object_object_get_ex(obj, key, &tmp_obj);
	if (!is_valid) {
		goto invalid_obj;
	}
	ret = parseGpioDir(tmp_obj, param);
	if (ret != 0) {
		goto error;
	}

	goto success;

invalid_obj:
	DBG("Cannot get %s object.\n", key);
	ret = -EINVAL;
error:
	DBG("Parse %s failed.\n", key);
success:
	json_object_put(obj);
	return ret;
}

/**
 * @brief print member of structure HIBER_DEMO_PARAM
 * @details
 * @param[in] param a HIBER_DEMO_PARAM from main function 
 * @return NULL
 * @retval NULL
 * @see
 */
static void printParam(HIBER_DEMO_PARAM param)
{
	/* print power control gpio */
	printf("[HIBER_DEMO_PARAM] pwr_mgmt.gpio: %d\n", param.pwr_mgmt.gpio);
	printf("[HIBER_DEMO_PARAM] pwr_mgmt.active_status: %d\n", param.pwr_mgmt.active_status);
	printf("[HIBER_DEMO_PARAM] pwr_mgmt.adc_ch: %d\n", param.pwr_mgmt.adc_ch);
	printf("[HIBER_DEMO_PARAM] pwr_mgmt.off_thre: %d\n", param.pwr_mgmt.off_thre);
	printf("[HIBER_DEMO_PARAM] pwr_mgmt.on_thre: %d\n", param.pwr_mgmt.on_thre);

	/* print tcp */
	printf("[HIBER_DEMO_PARAM] tcp.ser_ip: %s\n", inet_ntoa(*((struct in_addr *)&param.tcp->ser_ip)));
	printf("[HIBER_DEMO_PARAM] tcp.ser_port: %d\n", param.tcp->ser_port);
	printf("[HIBER_DEMO_PARAM] tcp.heartbeat_pkg: %s with length: %d\n", param.tcp->heartbeat_pkg,
	       param.tcp->heartbeat_pkg_len);
	printf("[HIBER_DEMO_PARAM] tcp.heartbeat_period: %d\n", param.tcp->heartbeat_period);
	printf("[HIBER_DEMO_PARAM] tcp.wakeup_pkg: %s with length: %d\n", param.tcp->wakeup_pkg,
	       param.tcp->wakeup_pkg_len);
	printf("[HIBER_DEMO_PARAM] tcp.bad_conn_handle: %d\n", param.tcp->bad_conn_handle);
	printf("[HIBER_DEMO_PARAM] sleep_tcp_wake: %d\n", param.sleep_tcp_wake);

	/* print sleep detection gpio */
	printf("[HIBER_DEMO_PARAM] sleep_gpio_det_cnt: %d\n", param.sleep_gpio_det_cnt);
	for (int i = 0; i < param.sleep_gpio_det_cnt; i++) {
		printf("[HIBER_DEMO_PARAM] sleep_gpio_det[%d].gpio: %d\n", i, param.sleep_gpio_det[i].gpio);
		printf("[HIBER_DEMO_PARAM] sleep_gpio_det[%d].event: %s\n", i, param.sleep_gpio_det[i].event);
	}

	/* print sleep detection adc */
	printf("[HIBER_DEMO_PARAM] sleep_adc_det_cnt: %d\n", param.sleep_adc_det_cnt);
	for (int i = 0; i < param.sleep_adc_det_cnt; i++) {
		printf("[HIBER_DEMO_PARAM] sleep_adc_det[%d].adc: %d\n", i, param.sleep_adc_det[i].adc);
		printf("[HIBER_DEMO_PARAM] sleep_adc_det[%d].threshold: %d\n", i, param.sleep_adc_det[i].threshold);
		printf("[HIBER_DEMO_PARAM] sleep_adc_det[%d].above_under: %d\n", i, param.sleep_adc_det[i].above_under);
	}

	/* print sleep detection timeout */
	printf("[HIBER_DEMO_PARAM] sleep_timeout: %d\n", param.sleep_timeout);

	/* print sleep response level */
	printf("[HIBER_DEMO_PARAM] sleep_response: %d\n", param.sleep_response);

	/* print gpio direction */
	printf("[HIBER_DEMO_PARAM] gpio_dir_cnt: %d\n", param.gpio_dir_cnt);
	for (int i = 0; i < param.gpio_dir_cnt; i++) {
		printf("[HIBER_DEMO_PARAM] gpio_dir[%d].gpio: %d\n", i, param.gpio_dir[i].gpio);
		printf("[HIBER_DEMO_PARAM] gpio_dir[%d].dir: %d\n", i, param.gpio_dir[i].dir);
		printf("[HIBER_DEMO_PARAM] gpio_dir[%d].out_val: %d\n", i, param.gpio_dir[i].out_val);
	}
}

/**
 * @brief help messages
 * @details
 * @param NULL
 * @return NULL
 * @retval NULL
 * @see
 */
static void help(const char *program_name)
{
	printf("USAGE: %s -i [options] ...\n"
	       "Options:\n"
	       " -i <file> 		Config file with Parameters in .json format.\n"
	       " -v <verbosity level>	Verbose mode option. (Default 0)\n"
	       "                        0: Do not print parameters from config file.\n"
	       "                        1: Print parameters from config file.\n"
	       "\n"
	       "Example:\n"
	       " $ %s -i /etc/config.json\n"
	       " $ %s -i /etc/config.json -v\n",
	       program_name, program_name, program_name);
}

int main(int argc, char **argv)
{
	log_info("Hiber demo start\n");

	/* initlize variables */
	int opt;
	int ret = 0;
	int verbose = 0;
	const char *cfg_file_name = NULL;
	HIBER_DEMO_PARAM hiber_param = {
		.sleep_tcp_wake = 0,
		.sleep_gpio_det_cnt = 0,
		.sleep_adc_det_cnt = 0,
		.sleep_timeout = 0,
		.sleep_response = 0,
	};

	/* get parameters */
	while ((opt = getopt(argc, argv, "i:v:h")) != -1) {
		switch (opt) {
		case 'i':
			cfg_file_name = optarg;
			break;

		case 'v':
			verbose = atoi(optarg);
			break;

		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;

		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	/* parse config */
	ret = readJsonFromFile(cfg_file_name, &hiber_param);
	if (ret != 0) {
		log_err("Fail to read Json from config file: %s.\n", cfg_file_name);
		return EXIT_FAILURE;
	}

	/* print parameter */
	if (verbose == V1) {
		printParam(hiber_param);
	}

	/* sleep process */
	ret = HIBER_appGoToSleep(hiber_param);
	if (ret != 0) {
		log_err("APP fails to sleep. Error no: %d.\n", ret);
		return EXIT_FAILURE;
	}
	free(hiber_param.tcp);
	free(hiber_param.sleep_gpio_det);
	free(hiber_param.sleep_adc_det);
	free(hiber_param.gpio_dir);
	log_info("Hiber demo end.\n");
	return 0;
}
