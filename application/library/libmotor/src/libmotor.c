#include "libmotor.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

#include "gpio_motor.h"

int MOTOR_init(MotorData *data)
{
	if (data->init != NULL) {
		data->init(data);
		return 0;
	}

	return -EACCES;
}

int MOTOR_deinit(MotorData *data)
{
	if (data->deinit != NULL) {
		data->deinit(data);
		return 0;
	}

	return -EACCES;
}

int MOTOR_reset(MotorData *data)
{
	if (data->reset != NULL) {
		data->reset(data);
		return 0;
	}

	return -EACCES;
}

int MOTOR_alignCenter(MotorData *data)
{
	if (data->alignCenter != NULL) {
		data->alignCenter(data);
		return 0;
	}

	return -EACCES;
}

void MOTOR_stop(MotorData *data)
{
	data->status.is_rotate = 0;
}

int MOTOR_rotateXAxis(MotorData *data, float *theta)
{
	if (data->rotateXAxis != NULL) {
		data->rotateXAxis(data, theta);
		return 0;
	}

	return -EACCES;
}

int MOTOR_rotateYAxis(MotorData *data, float *theta)
{
	if (data->rotateYAxis != NULL) {
		data->rotateYAxis(data, theta);
		return 0;
	}

	return -EACCES;
}

int MOTOR_rotateZAxis(MotorData *data, float *theta)
{
	if (data->rotateZAxis != NULL) {
		data->rotateZAxis(data, theta);
		return 0;
	}

	return -EACCES;
}

int MOTOR_rotateXAxisSec(MotorData *data, MOTOR_CMD_E cmd, float timeout_seconds)
{
	if (timeout_seconds == 0) {
		return 0;
	}

	/** calc theta from timeout_seconds & direction */
	float sign, theta;

	if (cmd == MOTOR_CMD_ROTATE_LEFT) {
		sign = 1;
	} else if (cmd == MOTOR_CMD_ROTATE_RIGHT) {
		sign = -1;
	} else {
		return -EINVAL;
	}

	theta = sign * timeout_seconds / (data->limit.attr[X_AXIS].max_velocity * 8.f / 1000.f);

	/** rotate motor */
	MOTOR_rotateXAxis(data, &theta);

	return 0;
}

int MOTOR_rotateYAxisSec(MotorData *data, MOTOR_CMD_E cmd, float timeout_seconds)
{
	if (timeout_seconds == 0) {
		return 0;
	}

	/** calc theta from timeout_seconds & direction */
	float sign, theta;

	if (cmd == MOTOR_CMD_ROTATE_UP) {
		sign = 1;
	} else if (cmd == MOTOR_CMD_ROTATE_DOWN) {
		sign = -1;
	} else {
		return -EINVAL;
	}

	theta = sign * timeout_seconds / (data->limit.attr[Y_AXIS].max_velocity * 8.f / 1000.f);

	/** rotate motor */
	MOTOR_rotateYAxis(data, &theta);

	return 0;
}

int MOTOR_rotateZAxisSec(MotorData *data, MOTOR_CMD_E cmd, float timeout_seconds)
{
	if (timeout_seconds == 0) {
		return 0;
	}

	/** calc theta from timeout_seconds & direction */
	float sign, theta;

	if (cmd == MOTOR_CMD_ROTATE_FORWARD) {
		sign = 1;
	} else if (cmd == MOTOR_CMD_ROTATE_BACKWARD) {
		sign = -1;
	} else {
		return -EINVAL;
	}

	theta = sign * timeout_seconds / (data->limit.attr[Z_AXIS].max_velocity * 8.f / 1000.f);

	/** rotate motor */
	MOTOR_rotateZAxis(data, &theta);

	return 0;
}

int MOTOR_getStat(const MotorData *data, MotorStatus *status)
{
	if (data != NULL) {
		memcpy(status, &data->status, sizeof(MotorStatus));
		return 0;
	}
	return -EACCES;
}

int MOTOR_setLimit(MotorData *data, MotorLimit *limit)
{
	if (limit == NULL) {
		return -EINVAL;
	}

	if (data != NULL) {
		memcpy(&data->limit, limit, sizeof(MotorLimit));
		return 0;
	}
	return -EACCES;
}

int MOTOR_getLimit(const MotorData *data, MotorLimit *limit)
{
	if (limit == NULL) {
		return -EINVAL;
	}

	if (data != NULL) {
		memcpy(limit, &data->limit, sizeof(MotorLimit));
		return 0;
	}
	return -EACCES;
}

int MOTOR_setSpeed(MotorData *data, const AXIS_TYPE_E axis, const float speed)
{
	memcpy(&data->status.state[axis].curr_velocity, &speed, sizeof(speed));

	return 0;
}

int MOTOR_getSpeed(const MotorData *data, const AXIS_TYPE_E axis, float *speed)
{
	*speed = data->status.state[axis].curr_velocity;

	return 0;
}

int MOTOR_getPtzSpeedFactor(const MotorData *data, const AXIS_TYPE_E axis, float *ptz_speed_factor)
{
	switch (data->type) {
	case MOTOR_TYPE_GPIO:
		*ptz_speed_factor = ((GpioMotor *)(data->private_data))->ptz_speed_factor[axis];
		break;
	default:
		return -EINVAL;
	}

	return 0;
}
