#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iva_td_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Td)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_TD_CONF);
	/* parse list of object from Json Object */
	parse_iva_td_conf(&g_conf.td, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Td_conf from database and use channel: %d.", g_conf.td.video_chn_idx);
	return 0;
}

int WRITE_DB(Td)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Td)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IVA_TD_CONF_S *td = (AGTX_IVA_TD_CONF_S *)data;
	if (len != sizeof(AGTX_IVA_TD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IVA_TD_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.td, td, sizeof(AGTX_IVA_TD_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, TD_ATTR, td);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.td, sizeof(AGTX_CMD_TD_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler td_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_TD_CONF, Td);

__attribute__((constructor)) void registerTd(void)
{
	HANDLERS_registerHandlers(&td_ops);
}