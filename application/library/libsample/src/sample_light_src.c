#define _GNU_SOURCE /** for pthread_create() */

#include "sample_light_src.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>

#include "mpi_limits.h"
#include "mpi_index.h"
#include "mpi_dip_alg.h"
#include "sample_venc_extend.h"

#include "utlist.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define FILE_MAX_LEN (128)

/**
 * @struct detect_thr_info
 */
typedef struct detect_thr_info {
	MPI_PATH idx;
	LightSrc *head;
	LightSrcDetection detection;
} DetectThrInfo;

/* Data of detect lightSrc threads */
static pthread_t g_detect_tid[MPI_MAX_INPUT_PATH_NUM]; /*record for join*/
static int g_detect_thread_run[MPI_MAX_INPUT_PATH_NUM] = { 0 };

/**
 * @brief Create the target file, and its parent directories if needed
 *
 * @param[in] filepath the path to the file
 * @return Execution result
 * @retval 0 Succeeded
 * @retval -1 Failed, errno might be set if it failed at a system call
 */
static int createFile(const char *filepath)
{
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;
	int ret;

	len = strlen(filepath);
	if (len >= PATH_MAX) {
		ret = -EINVAL;
		goto failed;
	}

	strncpy(tmp, filepath, sizeof(tmp));

	while (len > 0 && tmp[len - 1] == '/') {
		tmp[len - 1] = '\0';
		len--;
	}

	if (len == 0) {
		return -1;
	}

	for (p = tmp + 1; *p != '\0'; p++) {
		if (*p == '/') {
			*p = 0;
			ret = mkdir(tmp, 0777);
			if (ret != 0 && errno != EEXIST) {
				goto failed;
			}
			*p = '/';
		}
	}

	ret = open(tmp, O_RDWR | O_CREAT | O_EXCL, 0666);
	if (ret >= 0) {
		close(ret);
	} else if (errno != EEXIST) {
		goto failed;
	}

	return 0;
failed:
	return ret;
}

/**
 * @brief Write provided texts to the target file
 *
 * @param[in] filename the file to write
 * @param[in] str string to rwite
 * @param[in] len number of characters to write
 * @return Execution result
 * @retval 0 Succeeded, all of the characters are written
 * @retval -1 Failed, may or may not has written anything
 */
static int writeFile(const char *filename, const char *str, int len)
{
	FILE *fp;
	size_t write_num = 0;

	fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "failed to open the file.\n");
		return -EINVAL;
	}

	write_num = fwrite(str, sizeof(char), len, fp);
	if ((int)write_num != len) {
		fprintf(stderr, "failed to write %s file: %s\n", filename, str);
	}
	fclose(fp);

	return 0;
}

/**
 * @brief Get Light source type from given file
 *
 * @param[in] filename Path to detect.
 * @param[out] req_name output request characters
 * @param[in] count max length of req_name
 * @return Execution result or the IQ mode written in file
 * @retval -1             Failed
 */
static int readFile(const char *filename, char *req_name, size_t count)
{
	int fd;
	int ch_read = 0;
	int total_ch = 0;
	int ret = 0;
	char buf[FILE_MAX_LEN] = { 0 };

	if (filename == NULL) {
		fprintf(stderr, "Invalid attributes for function %s().\n", __func__);
		return -1;
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		ret = errno;
		fprintf(stderr, "Unable to open file %s for IQ mode, err = %d.\n", filename, ret);
		return -1;
	}

	total_ch = sizeof(buf) - 1;
	do {
		ret = read(fd, buf + ch_read, total_ch - ch_read);
		if (ret == -1 && errno != EINTR) {
			ret = errno;
			fprintf(stderr, "Unable to read file %s for IQ mode, err = %d.\n", filename, ret);
			close(fd);
			return -1;
		}
		if (ret > 0) {
			ch_read += ret;
		}
	} while (ret != 0);
	close(fd);

	for (int i = 0; i < FILE_MAX_LEN; i++) {
		if (buf[i] == '\n') {
			/** Only try to parse the first line */
			buf[i] = '\0';
		}
		if (buf[i] == '\0') {
			break;
		}
		buf[i] = tolower(buf[i]);
	}

	memset(req_name, 0x00, count);
	snprintf(req_name, count, "%s", buf);

	return 0;
}

/**
 * @brief Function to get scene ISO value
 *
 * @param[in] path_idx input path idx
 * @param[out] iso_val
 * @return int Run success or not
 */
static int getMpiSceneIso(MPI_PATH path_idx, uint32_t *iso_val) {
	MPI_EXPOSURE_INFO_S exp_info;
	int ret = 0;
	MPI_PATH idx = path_idx;

	ret = MPI_queryExposureInfo(idx, &exp_info);

	if (ret != MPI_SUCCESS) {
		return ret;
	}

	if (exp_info.luma_avg > 19275) {
		/** Overexposure can't trust */
		return -EINVAL;
	}

	*iso_val = exp_info.iso;
	return 0;
}

/**
 * @brief Get the Mpi Scene r g b value
 *
 * @param[in] path_idx input path idx
 * @param[out] r scene R statics
 * @param[out] g scene G statics
 * @param[out] b scene B statics
 * @return int Run success or not
 */
static int getMpiSceneRgb(MPI_PATH path_idx, UINT16 *r, UINT16 *g, UINT16 *b)
{
	int ret = 0;
	MPI_DIP_STAT_S stat;

	ret = MPI_getStatistics(path_idx, &stat);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	*r = stat.wb_stat.global_r_avg[0];
	*g = stat.wb_stat.global_g_avg[0];
	*b = stat.wb_stat.global_b_avg[0];

	return 0;
}

static void initLightSrc(const char *name, const char *path, LightSrc *src)
{
	/* set to zero */
	memset(src, 0, sizeof(LightSrc));

	strncpy((char *)src->name, name, sizeof(src->name));
	strncpy((char *)src->sensor_path, path, sizeof(src->sensor_path));
}

static int LIGHTSRC_gpioOn(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	if (((Gpio *)context)->id == -1) {
		return 0;
	}

	Gpio *gpio_ctrl = (Gpio *)context;
	GPIO_initGpio(gpio_ctrl);
	if (gpio_ctrl->direction == GPIO_IN) {
		gpio_ctrl->direction = GPIO_OUT_HIGH;
		if (gpio_ctrl->activate_value == GPIO_VAL_LOW) {
			gpio_ctrl->direction = GPIO_OUT_LOW;
		}
		GPIO_setGpioDirection(gpio_ctrl);
	} else {
		gpio_ctrl->value = gpio_ctrl->activate_value;
		GPIO_setGpioValue(gpio_ctrl);
	}

	GPIO_releaseGpio(gpio_ctrl);

	return 0;
}

static int LIGHTSRC_gpioOff(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	if (((Gpio *)context)->id == -1) {
		return 0;
	}

	Gpio *gpio_ctrl = (Gpio *)context;
	GPIO_initGpio(gpio_ctrl);
	if (gpio_ctrl->direction == GPIO_IN) {
		gpio_ctrl->direction = GPIO_OUT_HIGH;

		if (gpio_ctrl->activate_value == GPIO_VAL_HIGH) {
			gpio_ctrl->direction = GPIO_OUT_LOW;
		}

		GPIO_setGpioDirection(gpio_ctrl);
	} else {
		gpio_ctrl->value = GPIO_VAL_HIGH;
		if (gpio_ctrl->activate_value == GPIO_VAL_HIGH) {
			gpio_ctrl->value = GPIO_VAL_LOW;
		}

		GPIO_setGpioValue(gpio_ctrl);
	}

	GPIO_releaseGpio(gpio_ctrl);

	return 0;
}

static int LIGHTSRC_irOn(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	IrHwConfig *ir_hw = (IrHwConfig *)context;
	LIGHTSRC_gpioOn((void *)&ir_hw->ir_led);

	LIGHTSRC_gpioOff((void *)&ir_hw->ir_cut[0]);
	LIGHTSRC_gpioOff((void *)&ir_hw->ir_cut[1]);

	return 0;
}

static int LIGHTSRC_irOff(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	IrHwConfig *ir_hw = (IrHwConfig *)context;
	LIGHTSRC_gpioOff((void *)&ir_hw->ir_led);

	LIGHTSRC_gpioOn((void *)&ir_hw->ir_cut[0]);
	LIGHTSRC_gpioOn((void *)&ir_hw->ir_cut[1]);

	return 0;
}

static int LIGHTSRC_whiteLightOn(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	LightHwConfig *w_led = (LightHwConfig *)context;

	LIGHTSRC_gpioOn((void *)&w_led->w_led);

	return 0;
}

static int LIGHTSRC_whiteLightOff(void *context)
{
	if (context == NULL) {
		fprintf(stderr, "context is null\n");
		return -EINVAL;
	}

	LightHwConfig *w_led = (LightHwConfig *)context;
	LIGHTSRC_gpioOff((void *)&w_led->w_led);

	return 0;
}

/**
 * @brief software light sensing method to select day or ir light source
 *
 * @param[in] detection include algo params
 * @param[out] idx Input path to detect
 * @param[out] len Length of request name characters
 * @param[out] req_name Request LightSrc name pointer.
 * @return int 0 means success, 1 means an event occurs, a negative value means an error occurs
 */
static int SWLIGHTSENSOR_detect(MPI_PATH idx, LightSrcDetection *detection, char *req_name, size_t count)
{
	int ret = 0;
	UINT32 iso_val = 0;
	UINT16 r, g, b;
	UINT16 output_rg_ratio, output_bg_ratio;
	SwLightSensorParam *param = &(detection->method_param.sw_light_sensor_param);

	ret = getMpiSceneIso(idx, &iso_val);
	if (ret != 0) {
		fprintf(stderr, "failed to get ISO val\n");
		return -EACCES;
	}

	ret = getMpiSceneRgb(idx, &r, &g, &b);
	if (ret != 0) {
		fprintf(stderr, "failed to get scene RGB val\n");
		return -EACCES;
	}

	output_rg_ratio = (r * 256 + (g >> 1)) / MAX(g, 1);
	output_bg_ratio = (b * 256 + (g >> 1)) / MAX(g, 1);

	/** Algorithm to select LightSrc */
	if ((strcmp(detection->curr_name, "day") == 0) &&
	    (iso_val > param->day2ir_th)) {
		/** switch to ir light src */
		snprintf(req_name, count, "%s", &param->detect_name[0]);
		return 1;
	}

	if ((strcmp(detection->curr_name, param->detect_name) == 0) &&
	    ((iso_val < param->ir2day_th) || (output_rg_ratio >= param->rg_ratio_max) ||
	     (output_rg_ratio <= param->rg_ratio_min) || (output_bg_ratio >= param->bg_ratio_max) ||
	     (output_bg_ratio <= param->bg_ratio_min))) {
		/*switch to day light src */
		snprintf(req_name, count, "%s", "day");
		return 1;
	}

	return 0;
}

/**
 * @brief Read detection file to check which LightSrc to use in this input path
 * 
 * @param[in] detection lightSrc detection object
 * @param[out] idx Input path to detect
 * @param[out] req_name Request LightSrc name pointer.
 * @param[in] len Size of req_name 
 * @return int 0 means success, 1 means an event occurs, a negative value means an error occurs
 */
static int FILEREAD_detect(MPI_PATH idx, LightSrcDetection *detection, char *req_name, size_t len)
{
	(void)(idx);

	static struct stat tmp_status, old_status;
	ReadLightSrcFromFileConfig *config = &(detection->method_param.file_config);
	int ret = 0;
	ret = stat(config->filename, &tmp_status);
	if (ret != 0) {
		fprintf(stderr, "failed to stat %s\n", config->filename);
		return ret;
	}

	if (tmp_status.st_mtime == old_status.st_mtime) {
		/** file content not changed */
		return 0;
	}

	readFile(config->filename, req_name, len);

	if (strcmp(req_name, detection->curr_name) != 0) {
		return 1;
	}

	return 0;
}

/**
 * @brief Read gpio value to check which LightSrc to use in this input path
 * 
 * @param[in] detection lightSrc detection object
 * @param[out] idx Input path to detect
 * @param[out] req_name Request LightSrc name pointer.
 * @param[in] len Size of req_name 
 * @return int 0 means success, 1 means an event occurs, a negative value means an error occurs
 */
static int GPIO_detect(MPI_PATH idx, LightSrcDetection *detection, char *req_name, size_t len)
{
	(void)(idx);
	int ret = 0;

	GpioLightSensorParam *param = &(detection->method_param.gpio_light_sensor_param);
	Gpio *gpio = &(param->sensor_gpio);

	if (0 > GPIO_getGpioValue(gpio)) {
		/*gpio only init once*/
		GPIO_initGpio(gpio);
		printf("init gpio[%d]\n", gpio->id);
		ret = GPIO_getGpioValue(gpio);
	} else {
	}

	if (ret < 0) {
		fprintf(stderr, "failed to get gpio[%d] val\n", gpio->id);
	}

	if (gpio->value == param->night_value) {
		/*uboot open IR*/
		snprintf(req_name, len, "%s", "ir");
	} else {
		/*switch to day light src */
		snprintf(req_name, len, "%s", "day");
	}

	if (strcmp(req_name, detection->curr_name) != 0) {
		return 1;
	}

	return 0;
}

/**
 * @brief Determine using day or night LightSrc based on adc value (photoresistor) in this input path
 * 
 * @param[in] detection lightSrc detection object
 * @param[out] idx Input path to detect
 * @param[out] req_name Request LightSrc name pointer.
 * @param[in] len Size of req_name 
 * @return int 0 means success, 1 means an event occurs, a negative value means an error occurs
 */
static int ADC_detect(MPI_PATH idx, LightSrcDetection *detection, char *req_name, size_t len)
{
	(void)(idx);
	int ret = 0;

	AdcLightSensorParam *param = &(detection->method_param.adc_light_sensor_param);
	Adc *adc = &(param->sensor_adc);

	ret = ADC_initAdc(adc);

	if (ret < 0) {
		fprintf(stderr, "failed to init adc[%d]\n", adc->id);
		return -EINVAL;
	}

	ret = ADC_getAdcValue(adc);

	if (ret < 0) {
		fprintf(stderr, "failed to read adc[%d] val\n", adc->id);
		return -EINVAL;
	}

	if (adc->adc_hl == param->night_value) {
		/*IR mode*/
		snprintf(req_name, len, "%s", "ir");
	} else {
		/*switch to day light src */
		snprintf(req_name, len, "%s", "day");
	}

	if (strcmp(req_name, detection->curr_name) != 0) {
		return 1;
	}

	return 0;
}

LightSrc *SAMPLE_newDayLightSrc(const char *name, const char *path)
{
	LightSrc *light_src = malloc(sizeof(LightSrc));

	if (light_src == NULL) {
		fprintf(stderr, "Unable to malloc light src.");
		return NULL;
	}

	initLightSrc(name, path, light_src);
	light_src->type = SRC_TYPE_DAY;

	return light_src;
}

LightSrc *SAMPLE_newIrLightSrc(const char *name, const char *path, IrHwConfig *config)
{
	LightSrc *light_src = malloc(sizeof(LightSrc));

	if (light_src == NULL) {
		fprintf(stderr, "Unable to malloc light src.");
		return NULL;
	}

	initLightSrc(name, path, light_src);
	light_src->type = SRC_TYPE_IR;
	light_src->on = LIGHTSRC_irOn;
	light_src->off = LIGHTSRC_irOff;
	light_src->private = malloc(sizeof(*config));

	if (light_src->private == NULL) {
		goto free;
	}

	memcpy(light_src->private, (void *)config, sizeof(*config));

	return light_src;

free:
	free(light_src);
	return NULL;
}

LightSrc *SAMPLE_newWhiteLightSrc(const char *name, const char *path, LightHwConfig *config)
{
	LightSrc *light_src = malloc(sizeof(LightSrc));

	if (light_src == NULL) {
		fprintf(stderr, "Unable to malloc light src.");
		return NULL;
	}

	initLightSrc(name, path, light_src);
	light_src->type = SRC_TYPE_LIGHT;
	light_src->on = LIGHTSRC_whiteLightOn;
	light_src->off = LIGHTSRC_whiteLightOff;
	light_src->private = malloc(sizeof(*config));

	if (light_src->private == NULL) {
		goto free;
	}

	memcpy(light_src->private, (void *)config, sizeof(*config));

	return light_src;

free:
	free(light_src);
	return NULL;
}

void SAMPLE_deleteLightSrc(LightSrc *src)
{
	if (src == NULL) {
		return;
	}

	if (src->private) {
		free(src->private);
		src->private = NULL;
	}

	free(src);
}
/**
 * @brief This function create external file light src detection object, also 
 * create external file to control light src 
 * 
 * @param filename file to read/write light src
 * @return LightSrcDetection* object
 */
LightSrcDetection *SAMPLE_newExternalFileControl(const char *filename)
{
	LightSrcDetection *detection = malloc(sizeof(LightSrcDetection));

	if (detection == NULL) {
		fprintf(stderr, "Unable to malloc LightSrcDetection instance.");
		return NULL;
	}

	detection->method = DETECT_FILE;
	strncpy(detection->method_param.file_config.filename, filename,
	        sizeof(detection->method_param.file_config.filename));
	detection->detectLightSrc = FILEREAD_detect;
	detection->detect_interval_us = 500000;

	int ret = 0;
	ret = createFile(filename);
	if (ret) {
		goto failed;
	}

	ret = writeFile(filename, "day", 3);
	if (ret) {
		goto failed;
	}

	return detection;

failed:
	free(detection);
	return NULL;
}

/**
 * @brief creat sw light sensor light src detection object.
 * 
 * @param param input sw light sensor parameters 
 * @return LightSrcDetection*  object
 */
LightSrcDetection *SAMPLE_newSwLightSensor(SwLightSensorParam *param)
{
	LightSrcDetection *detection = malloc(sizeof(LightSrcDetection));

	if (detection == NULL) {
		fprintf(stderr, "Unable to malloc LightSrcDetection instance.");
		return NULL;
	}

	detection->method = DETECT_SW_LIGHT_SENSOR;
	memcpy(&(detection->method_param.sw_light_sensor_param), param, sizeof(*param));
	detection->detectLightSrc = SWLIGHTSENSOR_detect;
	detection->detect_interval_us = 500000;

	return detection;
}

/**
 * @brief creat gpio light sensor light src detection object.
 * 
 * @param param input gpio light sensor parameters 
 * @return LightSrcDetection*  object
 */
LightSrcDetection *SAMPLE_newGpioLightSensor(GpioLightSensorParam *param)
{
	LightSrcDetection *detection = malloc(sizeof(LightSrcDetection));

	if (detection == NULL) {
		fprintf(stderr, "Unable to malloc LightSrcDetection instance.");
		return NULL;
	}

	detection->method = DETECT_GPIO;
	memcpy(&(detection->method_param.gpio_light_sensor_param), param, sizeof(*param));
	detection->detectLightSrc = GPIO_detect;
	detection->detect_interval_us = 500000;

	return detection;
}

/**
 * @brief create adc light sensor light src detection object.
 * 
 * @param param input adc light sensor parameters 
 * @return LightSrcDetection*  object
 */
LightSrcDetection *SAMPLE_newAdcLightSensor(AdcLightSensorParam *param)
{
	LightSrcDetection *detection = malloc(sizeof(LightSrcDetection));

	if (detection == NULL) {
		fprintf(stderr, "Unable to malloc LightSrcDetection instance.");
		return NULL;
	}

	detection->method = DETECT_ADC;
	memcpy(&(detection->method_param.adc_light_sensor_param), param, sizeof(*param));
	detection->detectLightSrc = ADC_detect;
	detection->detect_interval_us = 500000;

	return detection;
}

void SAMPLE_deleteLightSrcDetection(LightSrcDetection *detection)
{
	if (detection == NULL) {
		return;
	}

	free(detection);
}

void SAMPLE_detectLightSrcOnce(MPI_PATH path_idx, LightSrcDetection *detection, char *req_name, size_t count)
{
	if (detection->detectLightSrc != NULL) {
		detection->detectLightSrc(path_idx, detection, &req_name[0], count);
	} else {
		sprintf(req_name, "day");
		fprintf(stderr, "LightSrcDetection is empty use default day mode\n");
	}
}

/**
 * @brief A loop to check LightSrc auto changed or not
 * 
 * @param _args Include input path, LightSrc linked list to select, LightSrcDetect method and params.
 * @return void* 
 */
static void *detect_light_src_thread(void *args)
{
	char req_name[NAME_MAX];

	DetectThrInfo *info = (DetectThrInfo *)args;
	LightSrcDetection *detection = &(info->detection);
	LightSrc *item, *tmp, *src = NULL, *dst = NULL;
	int ret = 0;

	if (detection->detectLightSrc == NULL) {
		goto free;
	}

	/* Search curr_name light source */
	LL_FOREACH_SAFE(info->head, item, tmp)
	{
		if (strcmp(item->name, detection->curr_name) == 0) {
			src = item;
		}
	}

	/* If there is no day light src registered */
	if (src == NULL) {
		fprintf(stderr, "failed to find day light source\n");
		goto free;
	}

	while (g_detect_thread_run[info->idx.path]) {
		/* start to detect light src change */
		ret = detection->detectLightSrc(info->idx, detection, &req_name[0], NAME_MAX);
		if (ret == 1) {
			/* If an event occurs */
			LL_FOREACH_SAFE(info->head, item, tmp) {
				if (strcmp(item->name, req_name) == 0) {
					dst = item;
				}
			}

			if (dst == NULL) {
				fprintf(stderr, "failed to find request light src: %s\n", req_name);
				continue;
			}

			/** Close old IQ thread */
			SAMPLE_destroyDipAttrUpdateThread(info->idx);

			/* Switch from src to dst light source */
			ret = SAMPLE_switchLightSrc(info->idx, src, dst);
			if (ret < 0) {
				fprintf(stderr, "failed to switch request light src: %s -> %s\n", detection->curr_name,
				        req_name);
				break;
			}

			/*switch venc_extend*/
			SAMPLE_setVencExtend(info->idx, dst->type);

			/** Reopen IQ thread */
			SAMPLE_createDipAttrUpdateThread(info->idx, dst->sensor_path, dst->dip_extend_path);

			/* Assign new light source to old */
			strncpy(detection->curr_name, req_name, sizeof(detection->curr_name));
			src = dst;
		} else if (ret < 0) {
			/* An error occurs, leave the thread */
			break;
		} else {
			/* Do nothing */
		}

		usleep(detection->detect_interval_us);
	}

free:
	free(args);

	return NULL;
}

/**
 * @brief Create a thread to check one input path LightSrc 
 * 
 * @param path_idx Detect input path to switch LightSrc
 * @param head Linked list of all LightSrc in this input path
 * @param detection LightSrc detection method in this input path
 * @return int Run success or not.
 * @see SAMPLE_destroyLightSrcDetectionThread
 */
int SAMPLE_createLightSrcDetectionThread(MPI_PATH path_idx, LightSrc *head, LightSrcDetection *detection)
{
	char tid_name[32];

	if ((head == NULL || detection == NULL) && (path_idx.path >= MPI_MAX_INPUT_PATH_NUM)) {
		fprintf(stderr, "Invalid arguments\n");
		return -EINVAL;
	}

	/** Generate thread name */
	sprintf(tid_name, "det_%d", path_idx.path);

	DetectThrInfo *info = malloc(sizeof(DetectThrInfo));
	info->idx = path_idx;
	info->head = head;
	memcpy(&(info->detection), detection, sizeof(*detection));

	g_detect_thread_run[path_idx.path] = 1;
	
	if (pthread_create(&g_detect_tid[path_idx.path], NULL, detect_light_src_thread, (void *)info) != 0) {
		fprintf(stderr, "Create thread updateIniThread %d failed.\n", path_idx.path);
		g_detect_thread_run[path_idx.path] = 0;
		return -ENAVAIL;
	}

	if (pthread_setname_np(g_detect_tid[path_idx.path], tid_name) != 0) {
		fprintf(stderr, "Set thread name to updateIniThread %d failed.\n", path_idx.path);
	}

	return 0;
}

/**
 * @brief Destroy the thread to detect input path LightSrc
 * 
 * @param path_idx Detect input path to switch LightSrc.
 * @return int int Run success or not.
 * @see SAMPLE_createLightSrcDetectionThread
 */
int SAMPLE_destroyLightSrcDetectionThread(MPI_PATH path_idx)
{
	int thd_idx = path_idx.path;
	if (thd_idx >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Invalid path index %d.\n", thd_idx);
		return -EINVAL;
	}

	if (g_detect_thread_run[thd_idx]) {
		g_detect_thread_run[thd_idx] = 0;
		if (pthread_join(g_detect_tid[thd_idx], NULL) != 0) {
			fprintf(stderr, "Failed to join thread detectLightSrcThread %d.\n", thd_idx);
			return -EINVAL;
		}
	}

	return 0;
}
