#ifndef SAMPLE_LIGHT_SRC_H_
#define SAMPLE_LIGHT_SRC_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include <stdbool.h>
#include <stdio.h>
#include <linux/limits.h>
#include "gpio.h"
#include "adc.h"
#include "sample_venc_extend.h"

#include "sample_dip.h"
#define RG_BG_RATIO_MAX (16)
#define RG_BG_RATIO_MIN (0)

typedef struct light_src_detection LightSrcDetection;
typedef enum { DETECT_FILE = 0, DETECT_SW_LIGHT_SENSOR, DETECT_GPIO, DETECT_ADC, DETECT_TYPE_NUM } DetectionType;

/**
 * @struct IrHwConfig
 * @brief Structure for storing IR hareware configuration, include IR cut & IR led
 */
typedef struct ir_hw {
	Gpio ir_cut[2];
	Gpio ir_led;
} IrHwConfig;

/**
 * @struct LightHwConfig
 * @brief Structure for storing white led hareware configuration
 */
typedef struct white_led_hw {
	Gpio w_led;
} LightHwConfig;

/**
 * @struct ReadLightSrcFromFile
 * @brief Read file to detect light source change
 */
typedef struct {
	char filename[PATH_MAX];
} ReadLightSrcFromFileConfig;

/**
 * @struct SwLightSensorParam 
 * @brief Parameters for SW light sensing algorithm 
 */
typedef struct software_light_sensor_param {
	char detect_name[NAME_MAX];
	UINT32 day2ir_th; /** read iso to detect switch day to ir*/
	UINT32 ir2day_th;
	UINT16 rg_ratio_min; /**  r * 256 / g */
	UINT16 rg_ratio_max;
	UINT16 bg_ratio_min;
	UINT16 bg_ratio_max;
} SwLightSensorParam;

/**
 * @struct GpioLightSensorParam
 * @brief Parameters for GPIO light sensing
 */
typedef struct gpio_light_sensor_param {
	Gpio sensor_gpio;
	unsigned char night_value;
} GpioLightSensorParam;

/**
 * @struct AdcLightSensorParam
 * @brief Parameters for ADC light sensing
 */
typedef struct adc_light_sensor_param {
	Adc sensor_adc;
	unsigned char night_value;
} AdcLightSensorParam;

/**
 * @struct light_src_detection
 * @brief Each input path has unique light source detect object
 */
struct light_src_detection {
	DetectionType method;
	/* Parameters for each method */
	union {
		ReadLightSrcFromFileConfig file_config;
		SwLightSensorParam sw_light_sensor_param;
		GpioLightSensorParam gpio_light_sensor_param;
		AdcLightSensorParam adc_light_sensor_param;
	} method_param;

	/**
	 * @param[in] idx Input path index.
	 * @param[in] detection Light source detection method.
	 * @param[out] req_name output light source name.
	 * @param[out] buffer size of req_name
	 * @retval
	 *    Positive values indicate string length of req_name. Negative values
	 * indicate an error occur. Zero means there is no change.
	 */
	int (*detectLightSrc)(MPI_PATH idx, LightSrcDetection *detection, char *req_name, size_t count);
	/* curr_name light source name */
	char curr_name[NAME_MAX];
	/* detection interval in us */
	int detect_interval_us;
};

/**
 * @brief Method to create light source
 */
LightSrc *SAMPLE_newDayLightSrc(const char *name, const char *path);
LightSrc *SAMPLE_newIrLightSrc(const char *name, const char *path, IrHwConfig *config);
LightSrc *SAMPLE_newWhiteLightSrc(const char *name, const char *path, LightHwConfig *config);

/**
 * @brief Method to create light source detection
 * An input path can only use a LightSrcDetection method at the same time 
 */
LightSrcDetection *SAMPLE_newExternalFileControl(const char *filename);
LightSrcDetection *SAMPLE_newSwLightSensor(SwLightSensorParam *param);
LightSrcDetection *SAMPLE_newGpioLightSensor(GpioLightSensorParam *param);
LightSrcDetection *SAMPLE_newAdcLightSensor(AdcLightSensorParam *param);

void SAMPLE_deleteLightSrc(LightSrc *src);
void SAMPLE_deleteLightSrcDetection(LightSrcDetection *detection);

/**
 * @brief Create detection threads for switching day/night modes
 */
void SAMPLE_detectLightSrcOnce(MPI_PATH path_idx, LightSrcDetection *detection, char *req_name, size_t count);
int SAMPLE_createLightSrcDetectionThread(MPI_PATH path_idx, LightSrc *head, LightSrcDetection *detection);
int SAMPLE_destroyLightSrcDetectionThread(MPI_PATH path_idx);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif // SAMPLE_LIGHT_SRC_H_
