#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

typedef enum gpio_direction {
	GPIO_IN = 0, /**< set GPIO to input */
	GPIO_OUT, /**< set GPIO to output, but don't care the current state */
	GPIO_OUT_HIGH, /**< set GPIO to output and high immediately */
	GPIO_OUT_LOW, /**< set GPIO to output and low immediately */
	GPIO_DIR_NUM,
} GpioDirection;

typedef enum gpio_value {
	GPIO_VAL_LOW = 0,
	GPIO_VAL_HIGH = 1,
	GPIO_VAL_NUM,
} GpioValue;

/**
 * @struct Gpio
 * @brief Structure for storing GPIO configuration
 */
typedef struct gpio {
	int id; /**< GPIO id */
	GpioDirection direction; /**< GPIO direction */
	GpioValue value; /**< GPIO value */
	GpioValue activate_value; /** GPIO activate value is HIGH or LOW*/
} Gpio;

int GPIO_initGpio(Gpio *gpio);
void GPIO_releaseGpio(Gpio *gpio);
int GPIO_setGpioDirection(Gpio *gpio);
int GPIO_getGpioDirection(Gpio *gpio);
int GPIO_setGpioValue(Gpio *gpio);
int GPIO_getGpioValue(Gpio *gpio);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //GPIO_H
