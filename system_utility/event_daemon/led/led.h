#ifndef LED_H
#define LED_H

#include <pthread.h>
#include "agtx_types.h"
#define LED_SERVER_SOCKET_PATH "/tmp/led_info"
#define LED_NAME_LENGTH 24
#define LED_CLIENT_LENGTH 24
#define LED_FLASH_LENGTH 16
#define LED_PIN_NUM_SIZE 6
#define LED_CLIENT_LIST_SIZE 17
#define RESET_TRIGGER_SLOW 250000
#define RESET_TRIGGER_FAST 5000000

typedef enum {
	AGTX_BUTTON_PRESS_TYPE_PRESSING,
	AGTX_BUTTON_PRESS_TYPE_NONE,
	AGTX_BUTTON_PRESS_TYPE_RELEASE,
} AGTX_BUTTON_PRESS_EVENT_TYPE_E;

typedef enum {
	AGTX_BUTTON_RESET_TYPE_OFF,
	AGTX_BUTTON_RESET_TYPE_ON,
} AGTX_BUTTON_RESET_EVENT_TYPE_E;

typedef enum {
	AGTX_LED_TYPE_STATUS_NOT_CHANGE,
	AGTX_LED_TYPE_CLIENT_CHANGE,
	AGTX_LED_TYPE_STATE_CHANGE,
} AGTX_LED_CLIENT_EVENT_TYPE_E;

typedef enum {
	AGTX_LED_CLIENT_LIST_OFF,
	AGTX_LED_CLIENT_LIST_ON,
	AGTX_LED_CLIENT_LIST_RESET,
} AGTX_LED_CLIENT_LIST_TYPE_E;

struct LED_GPIO_TABLE_S {
	AGTX_INT32 pin;
	char pin_name[LED_NAME_LENGTH];
};

struct LED_LIST_TABLE_S {
	int enabled;
	char client[LED_NAME_LENGTH];
};

struct LED_INIT_TABLE_S {
	AGTX_INT32 pin;
	int value;
	int trigger_type_level;
};

struct LED_EVENT_TABLE_S {
	pthread_mutex_t lock;
	int *led_switch_enabled;
	char curr_client[LED_CLIENT_LENGTH];
	char prev_client[LED_CLIENT_LENGTH];
	int enabled;
	int led_num;
	int clear_flag;
	char flash_type[LED_FLASH_LENGTH];
	AGTX_INT32 flash_period;
	AGTX_INT32 pin[LED_PIN_NUM_SIZE];
	AGTX_INT32 polling_period_usec;
	struct LED_GPIO_TABLE_S led[LED_PIN_NUM_SIZE];
	struct LED_LIST_TABLE_S led_client_list[LED_CLIENT_LIST_SIZE];
	struct LED_INIT_TABLE_S led_init_list;
};

int getLEDInform(char **);
void *handleLEDaction(void *rsv);
int handleButtonState(int currtime, int resettime, char *curr_client, char *prev_client, int button_reset);
#endif /* LED_H */
