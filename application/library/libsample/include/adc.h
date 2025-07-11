#ifndef ADC_H
#define ADC_H

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

typedef enum adc_value {
	ADC_VAL_LOW = 0,
	ADC_VAL_HIGH = 1,
	ADC_VAL_NUM,
} AdcHighLow;

/**
 * @struct Adc
 * @brief Structure for storing Adc configuration
 */
typedef struct adc {
	int id; /**< ADC id */
	unsigned short int value; /**< ADC value, 0-4096 */
	unsigned short int threshold_hl; /*beyond the threshold, adc_hl is high*/
	AdcHighLow adc_hl;
} Adc;

int ADC_initAdc(Adc *adc);
void ADC_releaseAdc(Adc *adc);
int ADC_getAdcValue(Adc *adc);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //ADC_H
