#ifndef GPIO_H
#define GPIO_H

#define GPIO_IN 0
#define GPIO_OUT 1

#define GPIO_LOW 0
#define GPIO_HIGH 1

typedef unsigned char Direction;
typedef unsigned char Value;

/*
 * GpioAttr: GPIO attributes for sysfs
 *
 * @id: GPIO index
 * @direction: GPIO direction
 * @value: GPIO value
 */
typedef struct {
	int id;
	Direction direction;
	Value value;
} Gpio;

/* initGpio(): Request a specific GPIO
 * @id: GPIO index
 */
Gpio * initGpio(int id);
void releaseGpio(Gpio * gpio);

void setGpioDirection(Gpio * gpio, char * direction);
Direction getGpioDirection(Gpio * gpio);

void setGpioValue(Gpio * gpio, Value v);
Value getGpioValue(Gpio * gpio);

#endif /* GPIO_H */
