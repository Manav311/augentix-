/**
 * @file motor.c
 * @brief Implement GL-602 motor control functions.
 */

#include "motor.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "log.h"

/**
 * @brief Mapping baud rate from integer type to macro.
 * @return The execution result.
 * @retval 0          Success
 * @retval -EINVAL    Target baudrate is not supported.
 */
int toBaudrate(int key)
{
	struct {
		int key;
		int value;
	} dict[] = {
		{ 50, B50 },       { 75, B75 },         { 110, B110 },       { 134, B134 },     { 150, B150 },
		{ 200, B200 },     { 300, B300 },       { 600, B600 },       { 1200, B1200 },   { 1800, B1800 },
		{ 2400, B2400 },   { 4800, B4800 },     { 9600, B9600 },     { 19200, B19200 }, { 38400, B38400 },
		{ 57600, B57600 }, { 115200, B115200 }, { 230400, B230400 },
	};

	for (size_t i = 0; i < sizeof(dict) / sizeof(dict[0]); ++i) {
		if (key == dict[i].key) {
			return dict[i].value;
		}
	}

	return -EINVAL;
}

// Communication attribute specified by GL-602
static int GL602_setSerialAttributes(int fd, int baudrate)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) != 0) {
		return -errno;
	}

	cfsetspeed(&tty, baudrate);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag |= (CLOCAL | CREAD);             // Enable receiver (seems that GL-602 does not
	                                             // feedback any info to host)
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag &= ~CSTOPB;

	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 5;                         // Timeout. As mentioned above, read operator is
	                                             // not supported for now.

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		return -errno;
	}

	return 0;
}

// We want to defer the motor stopping notification between motor
// and object detection until the motor stills and stabled.
//
// To do so, a STOP_N state is introduced into the FSM. It counts
// and transits to IDLE when number of frames to delay is reached.
//
// Complete transition graph is shown below:
//
//         start
//           |
//           ∨                     <--
// EXIT <-- IDLE --> START --> SET     MOVING
//           ^                  ^  -->
//           |                  |
//           |----- STOP_N <----|
//

// Parse control char to command.
MotorState Motor_parseCmd(Controller *controller, int c)
{
	c = tolower(c);
	switch (c) {
	case 'w':
		controller->motor.pan_speed = 0;
		controller->motor.tilt_speed = 0x20;
		return MOTOR_MOVE_U;

	case 's':
		controller->motor.pan_speed = 0;
		controller->motor.tilt_speed = -0x20;
		return MOTOR_MOVE_D;

	case 'd':
		controller->motor.pan_speed = 0x3f;
		controller->motor.tilt_speed = 0;
		return MOTOR_MOVE_R;

	case 'a':
		controller->motor.pan_speed = -0x3f;
		controller->motor.tilt_speed = 0;
		return MOTOR_MOVE_L;

	case KEY_ESC:
		controller->motor.pan_speed = controller->motor.tilt_speed = 0;
		return MOTOR_EXIT;

	case ' ':
		controller->motor.pan_speed = controller->motor.tilt_speed = 0;
		return MOTOR_IDLE;

	default:
		// Read nothing or other not registered commands.
		return controller->curr_state;
	}
}

// MPI_IVA_setObjParam() supports object detection when global motion
// exists. This feature needs user notifying OD whether motor is moving
// by setting MPI_IVA_OD_PARAM_S::en_motor as MPI_IVA_OD_DISABLE_MOTOR
// or MPI_IVA_OD_ENABLE_MOTOR.
//
// User should indicate OD algorithm before and after motor rotating.
// Here we assume OD keeps running on the background. Thus error
// checking is skipped.
static void State_start(Controller *controller, MotorState target)
{
	controller->motor_param.en_motor = 1;
	MPI_IVA_setObjMotorParam(controller->idx, &controller->motor_param);

	state_map[MOTOR_SET](controller, target);

	return;
}

static void State_stop(Controller *controller, MotorState target)
{
	// If target is set as MOVE
	if (target != MOTOR_IDLE && target != MOTOR_EXIT && target != MOTOR_STOP) {
		state_map[MOTOR_SET](controller, target);
		return;
	}

	if (controller->count) {
		controller->count--;
		return;
	}

	controller->motor_param.en_motor = 0;
	MPI_IVA_setObjMotorParam(controller->idx, &controller->motor_param);

	controller->curr_state = MOTOR_IDLE;
	state_map[MOTOR_IDLE](controller, MOTOR_IDLE);

	return;
}

static void State_set(Controller *controller, MotorState target)
{
	Motor_apply(&controller->motor);

	// If motor movement is disabled, prepare to go to state STOP.
	if (target == MOTOR_IDLE || target == MOTOR_EXIT) {
		controller->count = controller->delay;
		controller->curr_state = MOTOR_STOP;
		state_map[MOTOR_STOP](controller, target);
		return;
	}

	// Otherwise, set as MOVE and terminate transition.
	controller->curr_state = target;

	return;
}

static void State_idle(Controller *controller, MotorState target)
{
	if (target == MOTOR_IDLE) {
		return;
	}

	if (target == MOTOR_EXIT) {
		controller->curr_state = MOTOR_EXIT;
		return;
	}

	state_map[MOTOR_START](controller, target);

	return;
}

// Conclude MOVE_L, MOVE_R, MOVE_U and MOVE_D
static void State_moving(Controller *controller, MotorState target)
{
	if (target == controller->curr_state) {
		return;
	}

	state_map[MOTOR_SET](controller, target);

	return;
}

static void State_exit(Controller *controller __attribute__((unused)), MotorState target __attribute__((unused)))
{
	return;
}

StateF state_map[MOTOR_STATE_NUM] = {
	[MOTOR_EXIT] = State_exit,
	[MOTOR_IDLE] = State_idle,
	[MOTOR_START] = State_start,
	[MOTOR_SET] = State_set,
	[MOTOR_MOVE_L] = State_moving,
	[MOTOR_MOVE_R] = State_moving,
	[MOTOR_MOVE_U] = State_moving,
	[MOTOR_MOVE_D] = State_moving,
	[MOTOR_STOP] = State_stop,
};

/**
 * @brief Initialize connection to motor.
 * @param[in] motor
 * @param[in] tty_node
 * @return The execution result.
 * @retval 0         Success.
 * @retval others    Device node cannot be opened.
 *                   Check return code to learn the reason of failure.
 */
int Motor_init(Motor *motor, const char *tty_node, int rate)
{
	// The application supports operating without a motor.
	// Just return error code to indicate the event.
	motor->fd = open(tty_node, O_RDWR);
	if (motor->fd == -1) {
		return -errno;
	}

	// Hard-coding address code as 0x01.
	PelcoD_initCmd(&motor->cmd, 0x01);
	motor->pan_speed = motor->tilt_speed = 0;

	GL602_setSerialAttributes(motor->fd, rate);

	return 0;
}

int Motor_apply(Motor *motor)
{
	if (motor->fd == -1) {
		return 0;
	}

	PelcoD_setSpeed(&motor->cmd, motor->pan_speed, motor->tilt_speed);
	PelcoD_calcChecksum(&motor->cmd);

	if (write(motor->fd, motor->cmd.data, 7) != 7) {
		log_err("Send message error. %s", strerror(errno));
		return -errno;
	}

	return 0;
}

void Motor_exit(Motor *motor)
{
	if (motor->fd == -1) {
		return;
	}

	close(motor->fd);
	motor->fd = -1;
}
