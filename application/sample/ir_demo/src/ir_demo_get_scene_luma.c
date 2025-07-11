#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/io.h>
#include <syslog.h>
#include <unistd.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dip_types.h"
#include "mpi_dip_alg.h"
#include "ir_control.h"

#define ENABLE_PWM
#ifdef ENABLE_PWM
#include "pwm.h"
#endif

#include "light.h"
#include "gpio.h"

#define ir_control_demo_log_debug(fmt, args...) \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[IR conrtrol demo][Debug] " fmt, ##args)
#define ir_control_demo_log_err(fmt, args...) \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[IR conrtrol demo][Error] " fmt, ##args)

int main(void)
{
	MPI_SYS_init();
	int scene_luma = calcMpiSceneLuma(0, 0);
	ir_control_demo_log_debug("scene luma: %d ", scene_luma);
	MPI_SYS_exit();

	return 0;
}
