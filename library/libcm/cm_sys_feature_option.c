#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_sys_feature_option.h"


void parse_sys_feature_option(AGTX_SYS_FEATURE_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "stitch_support", &tmp_obj)) {
		data->stitch_support = json_object_get_int(tmp_obj);
	}
}

void comp_sys_feature_option(struct json_object *ret_obj, AGTX_SYS_FEATURE_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->stitch_support);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "stitch_support", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "stitch_support");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
