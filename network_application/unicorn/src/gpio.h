#ifndef GPIO_H
#define GPIO_H

typedef struct gpio_type {
	char name[32];
	char dir[8];
	int pin_num;
	int value;
} GpioType;

typedef struct gpio_frame {
	int flag;
	GpioType button;
	GpioType light_sensor;
	GpioType pir;
	GpioType sd_card;
	GpioType ir_cut[2];
	GpioType alarm;
	GpioType ir_led;
	GpioType led[2];
} GpioFrame;

typedef struct adc_type {
	char name[32];
	int chn;
} AdcType;

typedef struct adc_frame {
	int flag;
	AdcType light_sensor;
} AdcFrame;

int parseGpio(char *jstr_in);
int parseAdc(char *jstr_in);

#endif