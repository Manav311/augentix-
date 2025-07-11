#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_event_conf.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


const char * agtx_sw_event_socket_path_e_map[] = {
	"NONE",
	"IVA_MD",
	"IVA_TD",
	"NUM"
};

const char * agtx_sw_event_trig_type_e_map[] = {
	"NONE",
	"IVA_MD_NEGATIVE",
	"IVA_MD_POSITIVE",
	"IVA_TD_NEGATIVE",
	"IVA_TD_POSITIVE",
	"NUM"
};

const char * agtx_event_source_e_map[] = {
	"GPIO",
	"SW",
	"EINTC",
	"ADC",
	"LED",
	"MPI",
	"NUM"
};

const char * agtx_event_name_e_map[] = {
	"NONE",
	"PUSH_BUTTON_IN",
	"LIGHT_SENSOR_IN",
	"PIR_IN",
	"SD_CARD_IN",
	"EINTC_PIR",
	"IVA_MD",
	"IVA_TD",
	"LIGHT_SENSOR_ADC",
	"LED_INFORM",
	"LIGHT_SENSOR_MPI",
	"NUM"
};

const char * agtx_led_event_trig_type_e_map[] = {
	"NONE",
	"Wifi_Pairing",
	"Wifi_Connecting",
	"Cloud_Connecting",
	"Connecting_Fail",
	"Wifi_Connected",
	"Motion_Detected",
	"Live_view",
	"Low_Signal",
	"Disconnected",
	"OTA",
	"Critical_Error",
	"Card_Upgrade",
	"DEBUG_MODE",
	"DEBUG_INFO_DUMP",
	"Reset_INFO_Slow",
	"Reset_INFO_Fast",
	"LED_OFF",
	"NUM"
};

const char * agtx_gpio_event_trig_type_e_map[] = {
	"NONE",
	"LEVEL",
	"EDGE",
	"NUM"
};

const char * agtx_eintc_event_device_path_e_map[] = {
	"NONE",
	"EINTC_PIR",
	"NUM"
};

const char * agtx_eintc_event_trig_type_e_map[] = {
	"NONE",
	"EINTC_PIR_NEGATIVE",
	"EINTC_PIR_POSITIVE",
	"NUM"
};

const char * agtx_adc_event_trig_type_e_map[] = {
	"NONE",
	"HYS",
	"NUM"
};

const char * agtx_event_action_cb_e_map[] = {
	"NONE",
	"PRINT",
	"EXEC_CMD",
	"PAR_STRING",
	"NUM"
};

void parse_sw_event_rule(AGTX_SW_EVENT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_sw_event_trig_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_sw_event_trig_type_e_map[i], str) == 0) {
				data->trigger_type = (AGTX_SW_EVENT_TRIG_TYPE_E) i;
				break;
			}
		}
	}
}

void parse_sw_event(AGTX_SW_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_SW_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "rule", &tmp_obj)) {
		parse_sw_event_rule(&(data->rule), tmp_obj);
	}
}

void parse_sw_event_list(AGTX_SW_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_sw_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "socket_path", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_sw_event_socket_path_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_sw_event_socket_path_e_map[i], str) == 0) {
				data->socket_path = (AGTX_SW_EVENT_SOCKET_PATH_E) i;
				break;
			}
		}
	}
}

void parse_mpi_event(AGTX_MPI_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_MPI_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
}

void parse_mpi_event_list(AGTX_MPI_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_MPI_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_mpi_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_led_event_rule(AGTX_LED_EVENT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_led_event_trig_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_led_event_trig_type_e_map[i], str) == 0) {
				data->trigger_type = (AGTX_LED_EVENT_TRIG_TYPE_E) i;
				break;
			}
		}
	}
}

void parse_led_event(AGTX_LED_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_LED_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "in_use", &tmp_obj)) {
		data->in_use = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "rule", &tmp_obj)) {
		parse_led_event_rule(&(data->rule), tmp_obj);
	}
}

void parse_led_event_list(AGTX_LED_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_led_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "fast_flash_period_usec", &tmp_obj)) {
		data->fast_flash_period_usec = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "init_light_on_pin_num", &tmp_obj)) {
		data->init_light_on_pin_num = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "init_light_on_value", &tmp_obj)) {
		data->init_light_on_value = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "polling_period_usec", &tmp_obj)) {
		data->polling_period_usec = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "slow_flash_period_usec", &tmp_obj)) {
		data->slow_flash_period_usec = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "trigger_type_level", &tmp_obj)) {
		data->trigger_type_level = json_object_get_int(tmp_obj);
	}
}

void parse_gpio_init_rule(AGTX_GPIO_INIT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_GPIO_INIT_RULE_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
}

void parse_gpio_event_rule(AGTX_GPIO_EVENT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "edge", &tmp_obj)) {
		data->edge = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "edge_time_sec_end", &tmp_obj)) {
		data->edge_time_sec_end = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "edge_time_sec_start", &tmp_obj)) {
		data->edge_time_sec_start = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "level_time_sec", &tmp_obj)) {
		data->level_time_sec = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "level_value", &tmp_obj)) {
		data->level_value = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_gpio_event_trig_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_gpio_event_trig_type_e_map[i], str) == 0) {
				data->trigger_type = (AGTX_GPIO_EVENT_TRIG_TYPE_E) i;
				break;
			}
		}
	}
}

void parse_gpio_event(AGTX_GPIO_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_GPIO_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "rule", &tmp_obj)) {
		parse_gpio_event_rule(&(data->rule), tmp_obj);
	}
}

void parse_gpio_event_list(AGTX_GPIO_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_gpio_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "init_level", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_GPIO_EVENT_LIST_S_INIT_LEVEL_SIZE; i++) {
			parse_gpio_init_rule(&(data->init_level[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "polling_period_usec", &tmp_obj)) {
		data->polling_period_usec = json_object_get_int(tmp_obj);
	}
}

void parse_eintc_event_rule(AGTX_EINTC_EVENT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_eintc_event_trig_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_eintc_event_trig_type_e_map[i], str) == 0) {
				data->trigger_type = (AGTX_EINTC_EVENT_TRIG_TYPE_E) i;
				break;
			}
		}
	}
}

void parse_eintc_event(AGTX_EINTC_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_EINTC_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "rule", &tmp_obj)) {
		parse_eintc_event_rule(&(data->rule), tmp_obj);
	}
}

void parse_eintc_event_list(AGTX_EINTC_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "device_path", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_eintc_event_device_path_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_eintc_event_device_path_e_map[i], str) == 0) {
				data->device_path = (AGTX_EINTC_EVENT_DEVICE_PATH_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_eintc_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "jiffies_timeout", &tmp_obj)) {
		data->jiffies_timeout = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
}

void parse_adc_init_rule(AGTX_ADC_INIT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_ADC_INIT_RULE_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
}

void parse_adc_event_rule(AGTX_ADC_EVENT_RULE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "hys_th", &tmp_obj)) {
		data->hys_th = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_adc_event_trig_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_adc_event_trig_type_e_map[i], str) == 0) {
				data->trigger_type = (AGTX_ADC_EVENT_TRIG_TYPE_E) i;
				break;
			}
		}
	}
}

void parse_adc_event(AGTX_ADC_EVENT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "action", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_action_cb_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_action_cb_e_map[i], str) == 0) {
				data->action = (AGTX_EVENT_ACTION_CB_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "action_args", &tmp_obj)) {
		i = min(MAX_AGTX_ADC_EVENT_S_ACTION_ARGS_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->action_args, json_object_get_string(tmp_obj), i);
		data->action_args[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "rule", &tmp_obj)) {
		parse_adc_event_rule(&(data->rule), tmp_obj);
	}
}

void parse_adc_event_list(AGTX_ADC_EVENT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "chn", &tmp_obj)) {
		data->chn = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_ADC_EVENT_LIST_S_EVENT_SIZE; i++) {
			parse_adc_event(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "init_hys", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_ADC_EVENT_LIST_S_INIT_HYS_SIZE; i++) {
			parse_adc_init_rule(&(data->init_hys[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "polling_period_usec", &tmp_obj)) {
		data->polling_period_usec = json_object_get_int(tmp_obj);
	}
}

void parse_event_group(AGTX_EVENT_GROUP_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "adc", &tmp_obj)) {
		parse_adc_event_list(&(data->adc), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "always_enabled", &tmp_obj)) {
		data->always_enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "eintc", &tmp_obj)) {
		parse_eintc_event_list(&(data->eintc), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gpio", &tmp_obj)) {
		parse_gpio_event_list(&(data->gpio), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "in_use", &tmp_obj)) {
		data->in_use = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "led", &tmp_obj)) {
		parse_led_event_list(&(data->led), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mpi", &tmp_obj)) {
		parse_mpi_event_list(&(data->mpi), tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_name_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_name_e_map[i], str) == 0) {
				data->name = (AGTX_EVENT_NAME_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "source", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_event_source_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_event_source_e_map[i], str) == 0) {
				data->source = (AGTX_EVENT_SOURCE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "sw", &tmp_obj)) {
		parse_sw_event_list(&(data->sw), tmp_obj);
	}
}

void parse_event_conf(AGTX_EVENT_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_EVENT_CONF_S_EVENT_SIZE; i++) {
			parse_event_group(&(data->event[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_sw_event_rule(struct json_object *ret_obj, AGTX_SW_EVENT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_sw_event_trig_type_e_map[data->trigger_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type");
	}

}

void comp_sw_event(struct json_object *array_obj, AGTX_SW_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_sw_event_rule(tmp1_obj, &(data->rule));
			json_object_object_add(tmp_obj, "rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_sw_event_list(struct json_object *ret_obj, AGTX_SW_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_SW_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_sw_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

	const char *str;
	str = agtx_sw_event_socket_path_e_map[data->socket_path];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "socket_path", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "socket_path");
	}

}

void comp_mpi_event(struct json_object *array_obj, AGTX_MPI_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_mpi_event_list(struct json_object *ret_obj, AGTX_MPI_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_MPI_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_mpi_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

}

void comp_led_event_rule(struct json_object *ret_obj, AGTX_LED_EVENT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_led_event_trig_type_e_map[data->trigger_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type");
	}

}

void comp_led_event(struct json_object *array_obj, AGTX_LED_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		tmp1_obj = json_object_new_int(data->in_use);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "in_use", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "in_use");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_led_event_rule(tmp1_obj, &(data->rule));
			json_object_object_add(tmp_obj, "rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_led_event_list(struct json_object *ret_obj, AGTX_LED_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_LED_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_led_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

	tmp_obj = json_object_new_int(data->fast_flash_period_usec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "fast_flash_period_usec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "fast_flash_period_usec");
	}

	tmp_obj = json_object_new_int(data->init_light_on_pin_num);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "init_light_on_pin_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "init_light_on_pin_num");
	}

	tmp_obj = json_object_new_int(data->init_light_on_value);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "init_light_on_value", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "init_light_on_value");
	}

	tmp_obj = json_object_new_int(data->polling_period_usec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "polling_period_usec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "polling_period_usec");
	}

	tmp_obj = json_object_new_int(data->slow_flash_period_usec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "slow_flash_period_usec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "slow_flash_period_usec");
	}

	tmp_obj = json_object_new_int(data->trigger_type_level);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type_level", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type_level");
	}

}

void comp_gpio_init_rule(struct json_object *array_obj, AGTX_GPIO_INIT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_gpio_event_rule(struct json_object *ret_obj, AGTX_GPIO_EVENT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->edge);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "edge", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "edge");
	}

	tmp_obj = json_object_new_int(data->edge_time_sec_end);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "edge_time_sec_end", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "edge_time_sec_end");
	}

	tmp_obj = json_object_new_int(data->edge_time_sec_start);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "edge_time_sec_start", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "edge_time_sec_start");
	}

	tmp_obj = json_object_new_int(data->level_time_sec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "level_time_sec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "level_time_sec");
	}

	tmp_obj = json_object_new_int(data->level_value);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "level_value", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "level_value");
	}

	const char *str;
	str = agtx_gpio_event_trig_type_e_map[data->trigger_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type");
	}

}

void comp_gpio_event(struct json_object *array_obj, AGTX_GPIO_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_gpio_event_rule(tmp1_obj, &(data->rule));
			json_object_object_add(tmp_obj, "rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_gpio_event_list(struct json_object *ret_obj, AGTX_GPIO_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_GPIO_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_gpio_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_GPIO_EVENT_LIST_S_INIT_LEVEL_SIZE; i++) {
			comp_gpio_init_rule(tmp_obj, &(data->init_level[i]));
		}
		json_object_object_add(ret_obj, "init_level", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "init_level");
	}

	tmp_obj = json_object_new_int(data->polling_period_usec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "polling_period_usec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "polling_period_usec");
	}

}

void comp_eintc_event_rule(struct json_object *ret_obj, AGTX_EINTC_EVENT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_eintc_event_trig_type_e_map[data->trigger_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type");
	}

}

void comp_eintc_event(struct json_object *array_obj, AGTX_EINTC_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_eintc_event_rule(tmp1_obj, &(data->rule));
			json_object_object_add(tmp_obj, "rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_eintc_event_list(struct json_object *ret_obj, AGTX_EINTC_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_eintc_event_device_path_e_map[data->device_path];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "device_path", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "device_path");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_EINTC_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_eintc_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

	tmp_obj = json_object_new_int(data->jiffies_timeout);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "jiffies_timeout", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "jiffies_timeout");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

}

void comp_adc_init_rule(struct json_object *array_obj, AGTX_ADC_INIT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_adc_event_rule(struct json_object *ret_obj, AGTX_ADC_EVENT_RULE_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->hys_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "hys_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "hys_th");
	}

	const char *str;
	str = agtx_adc_event_trig_type_e_map[data->trigger_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_type");
	}

}

void comp_adc_event(struct json_object *array_obj, AGTX_ADC_EVENT_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_event_action_cb_e_map[data->action];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action");
		}

		str = (const char *)data->action_args;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "action_args", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "action_args");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_adc_event_rule(tmp1_obj, &(data->rule));
			json_object_object_add(tmp_obj, "rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_adc_event_list(struct json_object *ret_obj, AGTX_ADC_EVENT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->chn);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "chn", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "chn");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_ADC_EVENT_LIST_S_EVENT_SIZE; i++) {
			comp_adc_event(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_ADC_EVENT_LIST_S_INIT_HYS_SIZE; i++) {
			comp_adc_init_rule(tmp_obj, &(data->init_hys[i]));
		}
		json_object_object_add(ret_obj, "init_hys", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "init_hys");
	}

	tmp_obj = json_object_new_int(data->polling_period_usec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "polling_period_usec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "polling_period_usec");
	}

}

void comp_event_group(struct json_object *array_obj, AGTX_EVENT_GROUP_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_adc_event_list(tmp1_obj, &(data->adc));
			json_object_object_add(tmp_obj, "adc", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "adc");
		}

		tmp1_obj = json_object_new_int(data->always_enabled);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "always_enabled", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "always_enabled");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_eintc_event_list(tmp1_obj, &(data->eintc));
			json_object_object_add(tmp_obj, "eintc", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "eintc");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_gpio_event_list(tmp1_obj, &(data->gpio));
			json_object_object_add(tmp_obj, "gpio", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "gpio");
		}

		tmp1_obj = json_object_new_int(data->in_use);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "in_use", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "in_use");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_led_event_list(tmp1_obj, &(data->led));
			json_object_object_add(tmp_obj, "led", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "led");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_mpi_event_list(tmp1_obj, &(data->mpi));
			json_object_object_add(tmp_obj, "mpi", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mpi");
		}

		const char *str;
		str = agtx_event_name_e_map[data->name];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "name", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "name");
		}

		str = agtx_event_source_e_map[data->source];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "source", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "source");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_sw_event_list(tmp1_obj, &(data->sw));
			json_object_object_add(tmp_obj, "sw", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "sw");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_event_conf(struct json_object *ret_obj, AGTX_EVENT_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_EVENT_CONF_S_EVENT_SIZE; i++) {
			comp_event_group(tmp_obj, &(data->event[i]));
		}
		json_object_object_add(ret_obj, "event", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
