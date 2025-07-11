#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_awb_pref.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(AwbPref)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_AWB_PREF);
	/* parse list of object from Json Object */
	parse_awb_pref(&g_conf.awb, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("awb_pref mode:%d.", g_conf.awb.mode);

	return 0;
}

int WRITE_DB(AwbPref)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(AwbPref)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_AWB_PREF_S *awb_pref = (AGTX_AWB_PREF_S *)data;
	if (len != sizeof(AGTX_AWB_PREF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_AWB_PREF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.awb, awb_pref, sizeof(AGTX_AWB_PREF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, AWB_PREF, awb_pref);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.awb, sizeof(AGTX_CMD_AWB_PREF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler awb_pref_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_AWB_PREF, AwbPref);

__attribute__((constructor)) void registerAwbPref(void)
{
	HANDLERS_registerHandlers(&awb_pref_ops);
}