#ifndef EVENT_UTILS_H
#define EVENT_UTILS_H

#include "event.h"

typedef struct {
    int     id;
    char    *name;
} ALIAS_TABLE_S;

typedef struct {
	int                     id;
	AGTX_EVENT_ACTION_CB    cb;
} CB_ALIAS_TABLE_S;
// clang-format off
static ALIAS_TABLE_S k_event_alias_table[AGTX_EVENT_NAME_NUM + 1] = {
    { AGTX_EVENT_NAME_NONE ,             NULL },
    { AGTX_EVENT_NAME_PUSH_BUTTON_IN,    "PUSH_BUTTON_IN" },
    { AGTX_EVENT_NAME_LIGHT_SENSOR_IN,   "LIGHT_SENSOR_IN" },
    { AGTX_EVENT_NAME_PIR_IN,            "PIR_IN" },
    { AGTX_EVENT_NAME_SD_CARD_IN,        "SD_CARD_IN" },
    { AGTX_EVENT_NAME_EINTC_PIR,         "EINTC_PIR" },
    { AGTX_EVENT_NAME_IVA_MD,            "IVA_MD" },
    { AGTX_EVENT_NAME_IVA_TD,            "IVA_TD" },
    { AGTX_EVENT_NAME_LIGHT_SENSOR_ADC,  "LIGHT_SENSOR_ADC" },
    { AGTX_EVENT_NAME_LED_INFORM,        "LED_INFORM" },
    { AGTX_EVENT_NAME_LIGHT_SENSOR_MPI,  "LIGHT_SENSOR_MPI" },
    { AGTX_EVENT_NAME_NUM,               "NUM" },
};

static ALIAS_TABLE_S k_socket_path_alias_table[AGTX_SW_EVENT_SOCKET_PATH_NUM + 1] = {
    { AGTX_SW_EVENT_SOCKET_PATH_NONE,     NULL},
    { AGTX_SW_EVENT_SOCKET_PATH_IVA_MD,   "/tmp/iva_md_skt"},
    { AGTX_SW_EVENT_SOCKET_PATH_IVA_TD,   "/tmp/iva_td_skt"},
    { AGTX_SW_EVENT_SOCKET_PATH_NUM,      NULL},
};

static ALIAS_TABLE_S k_eintc_device_path_alias_table[AGTX_EINTC_EVENT_DEVICE_PATH_NUM + 1] = {
    { AGTX_EINTC_EVENT_DEVICE_PATH_NONE,        NULL},
    { AGTX_EINTC_EVENT_DEVICE_PATH_EINTC_PIR,   "/dev/eint_pir"},
    { AGTX_EINTC_EVENT_DEVICE_PATH_NUM,         NULL},
};

static CB_ALIAS_TABLE_S k_cb_alias_table[AGTX_EVENT_ACTION_CB_NUM + 1] = {
    { AGTX_EVENT_ACTION_CB_NONE,           NULL},
    { AGTX_EVENT_ACTION_CB_PRINT,          AGTX_EVENT_print},
    { AGTX_EVENT_ACTION_CB_EXEC_CMD,       AGTX_EVENT_execCmd},
    { AGTX_EVENT_ACTION_CB_PARSE_STRING,   AGTX_EVENT_parseString},
    { AGTX_EVENT_ACTION_CB_NUM,            NULL},
};

static ALIAS_TABLE_S k_sw_trig_alias_table[AGTX_SW_EVENT_TRIG_TYPE_NUM + 1] = {
    { AGTX_SW_EVENT_TRIG_TYPE_NONE,            "NONE"},
    { AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_NEGATIVE, "IVA_MD_NEGATIVE"},
    { AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE, "IVA_MD_POSITIVE"},
    { AGTX_SW_EVENT_TRIG_TYPE_IVA_TD_NEGATIVE, "IVA_TD_NEGATIVE"},
    { AGTX_SW_EVENT_TRIG_TYPE_IVA_TD_POSITIVE, "IVA_TD_POSITIVE"},
    { AGTX_SW_EVENT_TRIG_TYPE_NUM,             NULL},
};

static ALIAS_TABLE_S k_eintc_pir_trig_alias_table[AGTX_EINTC_EVENT_TRIG_TYPE_NUM + 1] = {
    { AGTX_EINTC_EVENT_TRIG_TYPE_NONE,               "NONE"},
    { AGTX_EINTC_EVENT_TRIG_TYPE_EINTC_PIR_NEGATIVE, "EINTC_PIR_NEGATIVE"},
    { AGTX_EINTC_EVENT_TRIG_TYPE_EINTC_PIR_POSITIVE, "EINTC_PIR_POSITIVE"},
    { AGTX_EINTC_EVENT_TRIG_TYPE_NUM,                NULL},
};

static ALIAS_TABLE_S k_adc_path_alias_table[3] = {
    { 0,   "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"},
    { 1,   "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"},
    { 2,   "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"},
};

static ALIAS_TABLE_S k_led_trig_alias_table[AGTX_LED_EVENT_TRIG_TYPE_NUM + 1] = {
	{ AGTX_LED_EVENT_TRIG_TYPE_NONE,              "NONE"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Wifi_Pairing,      "Wifi_Pairing"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Wifi_Connecting,   "Wifi_Connecting"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Cloud_Connecting,  "Cloud_Connecting"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Connecting_Fail,   "Connecting_Fail"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Wifi_Connected,    "Wifi_Connected"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Motion_Detected,   "Motion_Detected"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Live_view,         "Live_view"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Low_Signal,        "Low_Signal"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Disconnected,      "Disconnected"},
	{ AGTX_LED_EVENT_TRIG_TYPE_OTA,               "OTA"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Critical_Error,    "Critical_Error"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Card_Upgrade,      "Card_Upgrade"},
	{ AGTX_LED_EVENT_TRIG_TYPE_DEBUG_MODE,        "DEBUG_MODE"},
	{ AGTX_LED_EVENT_TRIG_TYPE_DEBUG_INFO_DUMP,   "DEBUG_INFO_DUMP"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Reset_INFO_Slow,   "Reset_INFO_Slow"},
	{ AGTX_LED_EVENT_TRIG_TYPE_Reset_INFO_Fast,   "Reset_INFO_Fast"},
	{ AGTX_LED_EVENT_TRIG_TYPE_LED_OFF,           "LED_OFF"},
	{ AGTX_LED_EVENT_TRIG_TYPE_NUM,               NULL},
};

typedef enum {
	AGTX_LED_EVENT_CRITICAL_TYPE_NONE,
	AGTX_LED_EVENT_CRITICAL_TYPE_Critical_Error,
	AGTX_LED_EVENT_CRITICAL_TYPE_DEBUG_MODE,
	AGTX_LED_EVENT_CRITICAL_TYPE_DEBUG_INFO_DUMP,
	AGTX_LED_EVENT_CRITICAL_TYPE_Reset_INFO_Slow,
	AGTX_LED_EVENT_CRITICAL_TYPE_Reset_INFO_Fast,
	AGTX_LED_EVENT_CRITICAL_TYPE_NUM
} AGTX_LED_EVENT_CRITICAL_TYPE_E;

static ALIAS_TABLE_S k_led_critical_alias_table[AGTX_LED_EVENT_CRITICAL_TYPE_NUM + 1] = {
	{ AGTX_LED_EVENT_CRITICAL_TYPE_NONE,              "NONE"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_Critical_Error,    "Critical_Error"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_DEBUG_MODE,        "DEBUG_MODE"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_DEBUG_INFO_DUMP,   "DEBUG_INFO_DUMP"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_Reset_INFO_Slow,   "Reset_INFO_Slow"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_Reset_INFO_Fast,   "Reset_INFO_Fast"},
	{ AGTX_LED_EVENT_CRITICAL_TYPE_NUM,               NULL},
};
// clang-format on
static inline int findIdfromAliasTable(ALIAS_TABLE_S tab[], const int len, const char *str)
{
    int i;
    int ret;

    ret = -1;
    for (i = 0; i < len; i++) {
        if (strcmp(str, tab[i].name) == 0) {
            ret = tab[i].id;
            return ret;
        }
    }

    if (ret < 0) {
        EVT_ERR("(str = %s) no match\n", str);
    }

    return ret;
}

static inline char *findNamefromAliasTable(ALIAS_TABLE_S tab[], const int len, const int id)
{
    int i;
    char *str;

    str = NULL;
    for (i = 0; i < len; i++) {
        if (tab[i].id == id) {
            str = tab[i].name;
            return str;
        }
    }

    if (str == NULL) {
        EVT_ERR("(id = %d) no match!\n", id);
    }

    return str;
}

static inline AGTX_EVENT_ACTION_CB *findFucfromCbAliasTable(CB_ALIAS_TABLE_S tab[], const int len, const int id)
{
    int i;
    AGTX_EVENT_ACTION_CB *cb;

    cb = NULL;
    for (i = 0; i < len; i++) {
        if (tab[i].id == id) {
            cb = &(tab[i].cb);
            return cb;
        }
    }

    if (cb == NULL) {
        EVT_ERR("(id = %d) no match!\n", id);
    }

    return cb;
}

static inline int name2pin(AGTX_GPIO_ALIAS_S gpio_alias[], const int num_of_elm, const char *str)
{
    int i;
    int ret;

    if (num_of_elm <= 0) {
        EVT_ERR("Invaild element size(%d)!\n", num_of_elm);
        ret = -1;
        return ret;
    }

    ret = -1;
    for (i = 0; i < num_of_elm; i++) {
        if (strcmp((const char *)str, (const char *)gpio_alias[i].name) == 0) {
            ret = gpio_alias[i].pin_num;
            return ret;
        }
    }

    if (ret < 0) {
        EVT_WARN("Name(%s) no match\n", str);
    }

    return ret;
}

static inline int name2pwmidx(AGTX_PWM_ALIAS_S gpio_alias[], const int num_of_elm, const char *str)
{
    int i;
    int ret;

    if (num_of_elm <= 0) {
        EVT_ERR("Invaild element size(%d)!\n", num_of_elm);
        ret = -1;
        return ret;
    }

    ret = -1;
    for (i = 0; i < num_of_elm; i++) {
        if (strcmp((const char *)str, (const char *)gpio_alias[i].name) == 0) {
            return i;
        }
    }

    if (ret < 0) {
        EVT_WARN("Name(%s) no match\n", str);
    }

    return ret;
}

#endif /* EVENT_UTILS_H */
