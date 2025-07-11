#ifndef SAMPLE_DIP_H_
#define SAMPLE_DIP_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include <linux/limits.h>
#include "mpi_index.h"

#define DIP_FILE_PATH "/system/mpp/script"

typedef enum {
	SRC_TYPE_DAY = 0, /** use only environment light */
	SRC_TYPE_IR, /** Enable Ir cut and Ir LED in night scene */
	SRC_TYPE_LIGHT, /** Enable white light in night scene */
	SRC_TYPE_NUM
} LightSrcType;

/**
 * @struct LightSrc
 * @brief Structure for storing light source configuration
 */
typedef struct light_src {
#define LIGHT_SRC_NAME_MAX (32)
	const char name[LIGHT_SRC_NAME_MAX];
	LightSrcType type;
	const char sensor_path[PATH_MAX]; /**< IQ parameters file path */
	int (*on)(void *); /**< method to on light src hareware */
	int (*off)(void *); /**< method to off light src hareware */
	int (*prepare)(void *); /**< method to prepare light src hareware */
	int (*close)(void *); /**< method to close light src hareware */
	void *private; /**< on/off/prepare/close functions argument */
	struct light_src *next;
	char dip_extend_path[PATH_MAX];
} LightSrc;

int SAMPLE_updateDipAttrOnce(MPI_PATH path_idx, const char *filename, const char *dip_extend_filepath);
int SAMPLE_createDipAttrUpdateThread(MPI_PATH path_idx, const char *sensor_path, const char *dip_extend_filepath);
int SAMPLE_destroyDipAttrUpdateThread(MPI_PATH path_idx);
int SAMPLE_updatePca(MPI_PATH path_idx, const char *filename);
int SAMPLE_switchLightSrc(MPI_PATH path_idx, LightSrc *src_context,
                          LightSrc *dst_context);
int SAMPLE_updateChnDipAttr(MPI_CHN chn_idx, const char *filename);

int SAMPLE_setBrightnessRatio(MPI_PATH path_idx, INT32 ratio);
int SAMPLE_setContrastRatio(MPI_PATH path_idx, INT32 ratio);
int SAMPLE_setSaturationRatio(MPI_PATH path_idx, INT32 ratio);
int SAMPLE_setHueRatio(MPI_PATH path_idx, INT32 ratio);
int SAMPLE_setSharpnessRatio(MPI_PATH path_idx, INT32 ratio);
int SAMPLE_setAntiflicker(MPI_PATH path_idx, INT32 enable, INT32 frequency, INT32 frame_rate);
int SAMPLE_setWdr(MPI_PATH path_idx, INT32 wdr_en, INT32 wdr_strength);
int SAMPLE_setBlc(MPI_PATH path_idx, INT32 backlight_compensation);
int SAMPLE_setAwbPref(MPI_PATH path_idx, INT32 awb_mode, INT32 color_temp, INT32 r_gain_ratio, INT32 b_gain_ratio);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //SAMPLE_DIP_H_
