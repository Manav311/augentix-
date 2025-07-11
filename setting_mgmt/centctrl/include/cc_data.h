/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CC_DATA_H_
#define CC_DATA_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include <json.h>

typedef struct {
	unsigned int cmd_id;
	char str[64];
	long size;
	char is_cal_data;
} CC_CMD_INFO_S;

extern const CC_CMD_INFO_S cmd_table[];
extern const long cmd_table_size;

typedef void (*PARSE_FUNC_S)(void *data, struct json_object *obj);
typedef void (*COMP_FUNC_S)(struct json_object *obj, void *data);

int determine_func(PARSE_FUNC_S *parse_func, COMP_FUNC_S *comp_func, int cmd_id);
void list_cmd_table(void);
int get_cmd_id(const char *str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CC_DATA_H_ */
