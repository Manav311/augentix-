#ifndef AGTX_ADV_IMG_PREF_H_
#define AGTX_ADV_IMG_PREF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_NIGHT_MODE_OFF,
	AGTX_NIGHT_MODE_ON,
	AGTX_NIGHT_MODE_AUTO,
	AGTX_NIGHT_MODE_AUTOSWITCH
} AGTX_NIGHT_MODE_E;

typedef enum {
	AGTX_IR_LED_MODE_OFF,
	AGTX_IR_LED_MODE_ON,
	AGTX_IR_LED_MODE_AUTO
} AGTX_IR_LED_MODE_E;

typedef enum {
	AGTX_IMAGE_MODE_COLOR,
	AGTX_IMAGE_MODE_GRAYSCALE,
	AGTX_IMAGE_MODE_AUTO
} AGTX_IMAGE_MODE_E;

typedef enum {
	AGTX_ICR_MODE_OFF,
	AGTX_ICR_MODE_ON,
	AGTX_ICR_MODE_AUTO
} AGTX_ICR_MODE_E;


typedef struct {
	AGTX_INT32 backlight_compensation;
	AGTX_ICR_MODE_E icr_mode;
	AGTX_IMAGE_MODE_E image_mode;
	AGTX_IR_LED_MODE_E ir_led_mode;
	AGTX_INT32 ir_light_suppression;
	AGTX_NIGHT_MODE_E night_mode;
	AGTX_INT32 wdr_en;
	AGTX_INT32 wdr_strength;
} AGTX_ADV_IMG_PREF_S;


#endif /* AGTX_ADV_IMG_PREF_H_ */
