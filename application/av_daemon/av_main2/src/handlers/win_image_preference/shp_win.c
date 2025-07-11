#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_dip_shp_win_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(ShpWin)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_DIP_SHP_WIN);
	/* parse list of object from Json Object */
	parse_dip_shp_win_conf(&g_conf.shp_win, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("shp_win  w:%d.", g_conf.shp_win.win_shp_en);
	return 0;
}

int WRITE_DB(ShpWin)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(ShpWin)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_UNUSED(data);
	AGTX_UNUSED(len);
	AGTX_UNUSED(node);

	avmain2_log_info("this conf can't modified");
	return 0;
}

static JsonConfHandler shp_win_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_DIP_SHP_WIN, ShpWin);

__attribute__((constructor)) void registerShpWin(void)
{
	HANDLERS_registerHandlers(&shp_win_ops);
}