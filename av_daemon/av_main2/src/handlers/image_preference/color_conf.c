#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_color_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(ColorConf)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_COLOR_CONF);
	/* parse list of object from Json Object */
	parse_color_conf(&g_conf.color, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("color mode:%d.", g_conf.color.color_mode);
	for (int i = 0; i < MAX_AGTX_COLOR_CONF_S_PARAMS_SIZE; i++) {
		avmain2_log_info("path[%d]= (%s, %d, %d, %d, %d, %d, %d)", i, g_conf.color.params[i].detect_name,
		                 g_conf.color.params[i].day2ir_th, g_conf.color.params[i].ir2day_th,
		                 g_conf.color.params[i].bg_ratio_max, g_conf.color.params[i].bg_ratio_min,
		                 g_conf.color.params[i].rg_ratio_max, g_conf.color.params[i].rg_ratio_min);
	}
	return 0;
}

int WRITE_DB(ColorConf)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(ColorConf)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_COLOR_CONF_S *color_conf = (AGTX_COLOR_CONF_S *)data;
	if (len != sizeof(AGTX_COLOR_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_COLOR_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.color, color_conf, sizeof(AGTX_COLOR_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, COLOR_CONF, color_conf);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.color, sizeof(AGTX_CMD_COLOR_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler color_conf_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_COLOR_CONF, ColorConf);

__attribute__((constructor)) void registerColorConf(void)
{
	HANDLERS_registerHandlers(&color_conf_ops);
}