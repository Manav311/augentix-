/**
 * @file PelcoD.c
 * @brief Construct command in PELCO-D protocol.
 * @see motor.c
 */

// Complete PELCO D commands
// See: https://www.epiphan.com/userguides/LUMiO12x/Content/UserGuides/PTZ/3-operation/PELCODcommands.htm

#include "motor.h"

#include <errno.h>
#include <stdio.h>

#define set_bit(p, n) ((p) |= (1 << (n)))
#define clear_bit(p, n) ((p) &= ~(1 << (n)))

#define abs(a) (((a) < 0) ? -(a) : a)
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
#define clamp(a, _min, _max) (min(max(a, _min), _max))

// Debugger.
char *PelcoD_getCmdString(const PelcoDCmd *cmd, char *buf)
{
	char *ptr = buf;

	for (unsigned int i = 0; i < 7; ++i, ptr += 3) {
		sprintf(ptr, "%02x ", cmd->data[i]);
	}

	*ptr = '\0';

	return buf;
}

// PELCO-D supports address code from 0x01 to 0x1F
void PelcoD_initCmd(PelcoDCmd *cmd, uint8_t addr)
{
	cmd->sync = 0xFF;
	cmd->addr = addr;
	cmd->cmnd1 = cmd->cmnd2 = cmd->data1 = cmd->data2 = cmd->cksm = 0x00;
}

/**
 * @brief Set camera move speed
 * @param[in] pan_speed     Positive value for clockwise rotation.
 * @param[in] tilt_speed    Positive value for raising up.
 */
void PelcoD_setSpeed(PelcoDCmd *cmd, int8_t pan_speed, int8_t tilt_speed)
{
	PelcoD_clear(cmd);

	uint8_t cmnd2 = 0;

	cmnd2 |= (pan_speed > 0) ? 0x2 : (pan_speed < 0) ? 0x4 : 0x0;
	cmnd2 |= (tilt_speed > 0) ? 0x8 : (tilt_speed < 0) ? 0x10 : 0x0;

	cmd->cmnd2 = cmnd2;
	cmd->data1 = clamp(abs(pan_speed), 0, 0x3f);
	cmd->data2 = clamp(abs(tilt_speed), 0, 0x3f);
}

void PelcoD_clear(PelcoDCmd *cmd)
{
	cmd->cmnd1 = cmd->cmnd2 = cmd->data1 = cmd->data2 = cmd->cksm = 0x00;
}

void PelcoD_calcChecksum(PelcoDCmd *cmd)
{
	uint16_t sum = 0;

	for (unsigned int i = 1; i < 6; ++i) {
		sum += cmd->data[i];
	}

	sum = sum % 256;
	cmd->data[6] = (uint8_t)sum;
}
