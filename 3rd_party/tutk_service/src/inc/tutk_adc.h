
#ifndef TUTK_ADC_H_
#define TUTK_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

int sortAdcLevelIdx(unsigned int adc_val);
int getAdcLevelInterpolation(int adc, int level_idx);
int getAdcValueInterpolation(unsigned int interpolation_level);
unsigned int parseWarningAdcLevel(unsigned int interpolation_level);
int TUTK_startADCnotificationThread(void *deviceCtx);

#ifdef __cplusplus
}
#endif

#endif