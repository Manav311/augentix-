#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_sys_info.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


void parse_sys_info(AGTX_SYS_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "dev_name", &tmp_obj)) {
		i = min(MAX_AGTX_SYS_INFO_S_DEV_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->dev_name, json_object_get_string(tmp_obj), i);
		data->dev_name[i] = '\0';
	}
}

void comp_sys_info(struct json_object *ret_obj, AGTX_SYS_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = (const char *)data->dev_name;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "dev_name", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "dev_name");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
