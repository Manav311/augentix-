#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdbool.h>
#include <stdint.h>

#include "mpi_index.h"
#include "mpi_iva.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_CTRL_C 3
#define KEY_CTRL_D 4
#define KEY_ESC    0x1B

typedef union pelco_d_cmd {
	uint8_t data[7];
	struct {
		uint8_t sync;
		uint8_t addr;
		uint8_t cmnd1;
		uint8_t cmnd2;
		uint8_t data1;
		uint8_t data2;
		uint8_t cksm;
	};
} PelcoDCmd;

typedef enum motor_state {
	MOTOR_EXIT = 0,
	MOTOR_IDLE,
	MOTOR_START,
	MOTOR_SET,
	MOTOR_MOVE_L,
	MOTOR_MOVE_R,
	MOTOR_MOVE_U,
	MOTOR_MOVE_D,
	MOTOR_STOP,
	// MOTOR_EXCEPTION,
	MOTOR_STATE_NUM,
} MotorState;

typedef struct motor {
	int fd;
	PelcoDCmd cmd; // Only supports PELCO-D protocol for now.
	int8_t pan_speed;
	int8_t tilt_speed;
} Motor;

typedef struct controller {
	// Control field for state STOP_N
	enum motor_state curr_state;
	unsigned int delay; // unit: frame
	unsigned int count;

	// Data field
	MPI_WIN idx;
	MPI_IVA_OD_PARAM_S param;
	MPI_IVA_OD_MOTOR_PARAM_S motor_param;
	Motor motor;

	// Teletypewriter (tty) node and communication mode
	const char *tty_node;
	int baud;
} Controller;

typedef void (*StateF)(Controller *controller, MotorState target);

char *PelcoD_getCmdString(const PelcoDCmd *cmd, char *buf);
void PelcoD_initCmd(PelcoDCmd *cmd, uint8_t addr);
void PelcoD_setSpeed(PelcoDCmd *cmd, int8_t pan_speed, int8_t tilt_speed);
void PelcoD_calcChecksum(PelcoDCmd *cmd);
void PelcoD_clear(PelcoDCmd *cmd);
#define PelcoD_stop(cmd) PelcoD_setSpeed(cmd, 0, 0)

int Motor_init(Motor *motor, const char *tty_node, int baudrate);
int Motor_apply(Motor *motor);
void Motor_exit(Motor *motor);

int toBaudrate(int key);
MotorState Motor_parseCmd(Controller *controller, int c);
extern StateF state_map[MOTOR_STATE_NUM];

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_H_ */
