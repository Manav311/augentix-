#ifndef ANR_H_
#define ANR_H_

#include "pcm_interfaces.h"

int anr_en(int en);
int anr_get_en(int *en);
int anr_set_params(AnrParams params);
int anr_get_params(AnrParams *params);

#endif /* ANR_H_ */
