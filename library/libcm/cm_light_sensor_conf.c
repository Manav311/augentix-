#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_light_sensor_conf.h"


const char * agtx_light_sensor_mode_e_map[] = {
	"NONE",
	"ADC",
	"MPI"
};

void parse_ir_led_ctrl(AGTX_IR_LED_CTRL_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "duty_cycle", &tmp_obj)) {
		data->duty_cycle = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "light_strength", &tmp_obj)) {
		data->light_strength = json_object_get_int(tmp_obj);
	}
}

void parse_light_sensor_mpi(AGTX_LIGHT_SENSOR_MPI_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "day_delay", &tmp_obj)) {
		data->day_delay = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "day_th", &tmp_obj)) {
		data->day_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dev", &tmp_obj)) {
		data->dev = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ev_response_usec", &tmp_obj)) {
		data->ev_response_usec = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "force_day_th", &tmp_obj)) {
		data->force_day_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iir_current_weight", &tmp_obj)) {
		data->iir_current_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ir_amplitude_ratio", &tmp_obj)) {
		data->ir_amplitude_ratio = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "ir_led_ctrl", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_LIGHT_SENSOR_MPI_S_IR_LED_CTRL_SIZE; i++) {
			parse_ir_led_ctrl(&(data->ir_led_ctrl[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "night_delay", &tmp_obj)) {
		data->night_delay = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "night_th", &tmp_obj)) {
		data->night_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "path", &tmp_obj)) {
		data->path = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "polling_period_usec", &tmp_obj)) {
		data->polling_period_usec = json_object_get_int(tmp_obj);
	}
}

void parse_light_sensor_adc(AGTX_LIGHT_SENSOR_ADC_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "day_th", &tmp_obj)) {
		data->day_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "night_th", &tmp_obj)) {
		data->night_th = json_object_get_int(tmp_obj);
	}
}

void parse_light_sensor_conf(AGTX_LIGHT_SENSOR_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "adc", &tmp_obj)) {
		parse_light_sensor_adc(&(data->adc), tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_light_sensor_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_light_sensor_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_LIGHT_SENSOR_MODE_E) i;
				break;
			}
		}
	}

	fprintf(stderr, "%s = %d\n", "mode", data->mode);
	fprintf(stderr, "%s = %d\n", "night_th", data->adc.night_th);
	fprintf(stderr, "%s = %d\n", "day_th", data->adc.day_th);

	if (json_object_object_get_ex(cmd_obj, "mpi", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_LIGHT_SENSOR_CONF_S_MPI_SIZE; i++) {
			parse_light_sensor_mpi(&(data->mpi[i]), json_object_array_get_idx(tmp_obj, i));

			fprintf(stderr, "%s = %d\n", "dev", data->mpi[i].dev);
			fprintf(stderr, "%s = %d\n", "path", data->mpi[i].path);
			fprintf(stderr, "%s = %d\n", "polling_period_usec", data->mpi[i].polling_period_usec);
			fprintf(stderr, "%s = %d\n", "force_day_th", data->mpi[i].force_day_th);
			fprintf(stderr, "%s = %d\n", "day_th", data->mpi[i].day_th);
			fprintf(stderr, "%s = %d\n", "night_th", data->mpi[i].night_th);
			fprintf(stderr, "%s = %d\n", "day_delay", data->mpi[i].day_delay);
			fprintf(stderr, "%s = %d\n", "night_delay", data->mpi[i].night_delay);
			fprintf(stderr, "%s = %d\n", "iir_current_weight", data->mpi[i].iir_current_weight);
			fprintf(stderr, "%s = %d\n", "ir_amplitude_ratio", data->mpi[i].ir_amplitude_ratio);
			fprintf(stderr, "%s = %d\n", "ev_response_usec", data->mpi[i].ev_response_usec);
			for (int k = 0; k < MAX_AGTX_LIGHT_SENSOR_MPI_S_IR_LED_CTRL_SIZE; k++) {
				fprintf(stderr, "%s[%d] = %d\n", "duty_cycle", k, data->mpi[i].ir_led_ctrl[k].duty_cycle);
				fprintf(stderr, "%s[%d] = %d\n", "light_strength", k, data->mpi[i].ir_led_ctrl[k].light_strength);
			}

		}
	}
}

void comp_ir_led_ctrl(struct json_object *array_obj, AGTX_IR_LED_CTRL_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->duty_cycle);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "duty_cycle", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "duty_cycle");
		}

		tmp1_obj = json_object_new_int(data->light_strength);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "light_strength", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "light_strength");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_light_sensor_mpi(struct json_object *array_obj, AGTX_LIGHT_SENSOR_MPI_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->day_delay);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "day_delay", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "day_delay");
		}

		tmp1_obj = json_object_new_int(data->day_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "day_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "day_th");
		}

		tmp1_obj = json_object_new_int(data->dev);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dev", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dev");
		}

		tmp1_obj = json_object_new_int(data->ev_response_usec);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ev_response_usec", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ev_response_usec");
		}

		tmp1_obj = json_object_new_int(data->force_day_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "force_day_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "force_day_th");
		}

		tmp1_obj = json_object_new_int(data->iir_current_weight);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "iir_current_weight", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "iir_current_weight");
		}

		tmp1_obj = json_object_new_int(data->ir_amplitude_ratio);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ir_amplitude_ratio", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ir_amplitude_ratio");
		}

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_LIGHT_SENSOR_MPI_S_IR_LED_CTRL_SIZE; i++) {
				comp_ir_led_ctrl(tmp1_obj, &(data->ir_led_ctrl[i]));
			}
			json_object_object_add(tmp_obj, "ir_led_ctrl", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "ir_led_ctrl");
		}

		tmp1_obj = json_object_new_int(data->night_delay);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "night_delay", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "night_delay");
		}

		tmp1_obj = json_object_new_int(data->night_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "night_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "night_th");
		}

		tmp1_obj = json_object_new_int(data->path);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "path", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "path");
		}

		tmp1_obj = json_object_new_int(data->polling_period_usec);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "polling_period_usec", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "polling_period_usec");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_light_sensor_adc(struct json_object *ret_obj, AGTX_LIGHT_SENSOR_ADC_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->day_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "day_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "day_th");
	}

	tmp_obj = json_object_new_int(data->night_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "night_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "night_th");
	}

}

void comp_light_sensor_conf(struct json_object *ret_obj, AGTX_LIGHT_SENSOR_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_light_sensor_adc(tmp_obj, &(data->adc));
		json_object_object_add(ret_obj, "adc", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "adc");
	}

	const char *str;
	str = agtx_light_sensor_mode_e_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_LIGHT_SENSOR_CONF_S_MPI_SIZE; i++) {
			comp_light_sensor_mpi(tmp_obj, &(data->mpi[i]));
		}
		json_object_object_add(ret_obj, "mpi", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "mpi");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
