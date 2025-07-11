#ifndef TRACKING_H_
#define TRACKING_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "libmotor.h"
#include "mpi_base_types.h"
#include "mpi_iva.h"

#define INIT_TRACKING_ID (0xffff)

typedef enum { TRACKING_TYPE_OD = 0, TRACKING_TYPE_GMV_OD, TRACKING_TYPE_NUM } TRACKING_TYPE_E;

typedef struct tracking_param {
	TRACKING_TYPE_E type; /** tracking type for auto-tracking */
	MPI_WIN win_idx; /** window index which perform auto-tracking */

	MPI_IVA_OD_PARAM_S od_param;
	MPI_IVA_OD_MOTOR_PARAM_S od_motor_param;

	int obj_life_th; /** remove object whose life is less than this value */
	float detect_boundary[AXIS_NUM]; /** specify detect boundary in percentage of the FOV */
	float max_move_time; /** max move time per motor movement for satisfying GMV OD (unit: ms) */
	int reset_time; /** reset motor time (unit: s) */
	int track_delay_time; /** delay time to start tracking after a motor movement (unit: ms) */

	MPI_SIZE_S boundary; /** for result output */
	int mirr_en; /** indicate whether camera mirror is enable */
	int flip_en; /** indicate whether camera flip is enable */
	int enable_debug_osd; /** show OSD for track obj and boundary */
} TrackingParam;

typedef struct tracking_instance TrackingInstance;

TrackingInstance *newTrackingInstance(const TrackingParam *param, MotorData *motor_data);
void deleteTrackingInstance(TrackingInstance *instance);
int runAutoTracking(TrackingInstance *instance);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif /** TRACKING_H_ */
