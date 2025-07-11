#ifndef LIBMOTOR_H_
#define LIBMOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include <pthread.h>

typedef struct motor_data MotorData;

typedef enum { X_AXIS = 0, Y_AXIS, Z_AXIS, AXIS_NUM } AXIS_TYPE_E;

typedef enum {
	LEFT_DIR = 0,
	RIGHT_DIR,
	UP_DIR,
	DOWN_DIR,
	FORWARD_DIR,
	BACKWARD_DIR,
	NONE_DIR,
	DIRECTION_NUM
} DIRECTION_TYPE_E;

typedef enum { MOTOR_TYPE_GPIO = 0, MOTOR_TYPE_NUM } MOTOR_TYPE_E;

typedef enum {
	MOTOR_CMD_NONE = 0,
	MOTOR_CMD_ROTATE_LEFT,
	MOTOR_CMD_ROTATE_RIGHT,
	MOTOR_CMD_ROTATE_UP,
	MOTOR_CMD_ROTATE_DOWN,
	MOTOR_CMD_ROTATE_FORWARD,
	MOTOR_CMD_ROTATE_BACKWARD,
	MOTOR_CMD_ALIGN_CENTER,
	MOTOR_CMD_RESET,
	MOTOR_CMD_NUM
} MOTOR_CMD_E;

typedef struct motor_axis_attr {
	float min_theta; /*< max theta for reverse operation */
	float max_theta; /*< max theta for forward operation */
	int min_velocity; /*< The number of microseconds required to rotate one degree. */
	int max_velocity; /*< The number of microseconds required to rotate one degree. */
	int default_velocity;
	float center_position; /*< The initial positioning point at the center of the screen */
	DIRECTION_TYPE_E plus_coordinates; /*< means which side is '+' in this axis*/
} MotorAxisAttr;

typedef struct motor_limit {
	MotorAxisAttr attr[AXIS_NUM];
} MotorLimit;

typedef struct motor_state {
	float actual_theta; /*< motor actually move */
	float ideal_theta; /*< motor cmd by user */
	float curr_velocity; /*< unit: micro-seconds per degree */
	float velocity; /*< scalar */
} MotorState;

typedef struct motor_status {
	int is_rotate; /*< indicate whether motor is rotating */
	int is_reset; /*< indicate whether motor is resetting to default position */
	MotorState state[AXIS_NUM];
} MotorStatus;

struct motor_data {
	int (*init)(MotorData *data); /*< init the motor */
	int (*deinit)(MotorData *data); /*< deinit the motor */
	int (*reset)(MotorData *data); /*< reset the motor */
	int (*alignCenter)(MotorData *data); /*< align the motor position to center */
	int (*rotateXAxis)(MotorData *data, float *theta); /*< X axis rotate */
	int (*rotateYAxis)(MotorData *data, float *theta); /*< Y axis rotate */
	int (*rotateZAxis)(MotorData *data, float *theta); /*< Z axis rotate */
	MOTOR_TYPE_E type;
	void *private_data; /*< store the data structures of current motor type such as gpio motor */
	MotorLimit limit; /*< motor limit info of eash axis */
	MotorStatus status; /*< movement status of the motor */
	pthread_mutex_t lock; /*< for enabling re-entraint of motor rotate APIs */
};

int MOTOR_init(MotorData *data);
int MOTOR_deinit(MotorData *data);
int MOTOR_reset(MotorData *data);
int MOTOR_alignCenter(MotorData *data);
void MOTOR_stop(MotorData *data);
int MOTOR_rotateXAxis(MotorData *data, float *theta);
int MOTOR_rotateYAxis(MotorData *data, float *theta);
int MOTOR_rotateZAxis(MotorData *data, float *theta);
int MOTOR_rotateXAxisSec(MotorData *data, MOTOR_CMD_E cmd,
                         float timeout_seconds); /*< motor rotate in one direction, limit by max time*/
int MOTOR_rotateYAxisSec(MotorData *data, MOTOR_CMD_E cmd, float timeout_seconds);
int MOTOR_rotateZAxisSec(MotorData *data, MOTOR_CMD_E cmd, float timeout_seconds);
int MOTOR_getStat(const MotorData *data, MotorStatus *stat);
int MOTOR_setLimit(MotorData *data, MotorLimit *limit);
int MOTOR_getLimit(const MotorData *data, MotorLimit *limit);
int MOTOR_setSpeed(MotorData *data, const AXIS_TYPE_E axis, const float speed);
int MOTOR_getSpeed(const MotorData *data, const AXIS_TYPE_E axis, float *speed);
int MOTOR_getPtzSpeedFactor(const MotorData *data, const AXIS_TYPE_E axis, float *ptz_speed_factor);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif