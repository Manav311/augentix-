/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_GPIO_CONF_H_
#define AGTX_GPIO_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef enum {
	AGTX_GPIO_DIR_IN,
	AGTX_GPIO_DIR_OUT,
	AGTX_GPIO_DIR_NUM
} AGTX_GPIO_DIR_E;

#define MAX_AGTX_GPIO_ALIAS_S_NAME_SIZE         129
#define MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE    12

typedef struct {
	AGTX_GPIO_DIR_E dir; /* Name of GPIO direction. */
	AGTX_UINT8 name[MAX_AGTX_GPIO_ALIAS_S_NAME_SIZE]; /* GPIO event name. */
	AGTX_INT32 pin_num; /* Pin number. */
	AGTX_INT32 value; /* GPIO output value; don't care for GPIO input. */
} AGTX_GPIO_ALIAS_S;

typedef struct {
	AGTX_GPIO_ALIAS_S gpio_alias[MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE];
} AGTX_GPIO_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_GPIO_CONF_H_ */
