/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_PRODUCT_OPTION_LIST_H_
#define AGTX_PRODUCT_OPTION_LIST_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_SIZE 16
#define MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_STR_SIZE 65

typedef struct {
	AGTX_UINT8 option[MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_SIZE][MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_STR_SIZE];
} AGTX_PRODUCT_OPTION_LIST_S;


#endif /* !AGTX_PRODUCT_OPTION_LIST_H_ */
