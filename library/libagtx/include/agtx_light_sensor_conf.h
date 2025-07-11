#ifndef AGTX_LIGHT_SENSOR_CONF_H_
#define AGTX_LIGHT_SENSOR_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_LIGHT_SENSOR_MPI_S_IR_LED_CTRL_SIZE 5
typedef enum {
	AGTX_LIGHT_SENSOR_MODE_NONE,
	AGTX_LIGHT_SENSOR_MODE_ADC,
	AGTX_LIGHT_SENSOR_MODE_MPI
} AGTX_LIGHT_SENSOR_MODE_E;

#define MAX_AGTX_LIGHT_SENSOR_CONF_S_MPI_SIZE 1

typedef struct {
	AGTX_INT32 duty_cycle;
	AGTX_INT32 light_strength;
} AGTX_IR_LED_CTRL_S;

typedef struct {
	AGTX_INT32 day_delay;
	AGTX_INT32 day_th;
	AGTX_INT32 dev;
	AGTX_INT32 ev_response_usec;
	AGTX_INT32 force_day_th;
	AGTX_INT32 iir_current_weight;
	AGTX_INT32 ir_amplitude_ratio;
	AGTX_IR_LED_CTRL_S ir_led_ctrl[MAX_AGTX_LIGHT_SENSOR_MPI_S_IR_LED_CTRL_SIZE];
	AGTX_INT32 night_delay;
	AGTX_INT32 night_th;
	AGTX_INT32 path;
	AGTX_INT32 polling_period_usec;
} AGTX_LIGHT_SENSOR_MPI_S;

typedef struct {
	AGTX_INT32 day_th;
	AGTX_INT32 night_th;
} AGTX_LIGHT_SENSOR_ADC_S;

typedef struct {
	AGTX_LIGHT_SENSOR_ADC_S adc;
	AGTX_LIGHT_SENSOR_MODE_E mode;
	AGTX_LIGHT_SENSOR_MPI_S mpi[MAX_AGTX_LIGHT_SENSOR_CONF_S_MPI_SIZE];
} AGTX_LIGHT_SENSOR_CONF_S;


#endif /* AGTX_LIGHT_SENSOR_CONF_H_ */
