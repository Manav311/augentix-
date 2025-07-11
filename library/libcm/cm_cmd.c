#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_cmd.h"


typedef struct {
	char         *cmd_name;
	unsigned int  cmd_id;
} AGTX_CMD_PAIR_S;


static const AGTX_CMD_PAIR_S product_option[] = {
	{ "AGTX_CMD_RES_OPTION",         AGTX_CMD_RES_OPTION         },
	{ "AGTX_CMD_VENC_OPTION",        AGTX_CMD_VENC_OPTION        },
	{ "AGTX_CMD_SYS_FEATURE_OPTION", AGTX_CMD_SYS_FEATURE_OPTION },
};


unsigned int map_product_option_list(char *str)
{
	int i;
	unsigned int cmd_id = AGTX_CMD_INVALID;

	for (i = 0; (unsigned long)i < sizeof(product_option) / sizeof(AGTX_CMD_PAIR_S); i++) {
		if (strcmp(product_option[i].cmd_name, str) == 0) {
			cmd_id = product_option[i].cmd_id;
			break;
		}
	}

	return cmd_id;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
