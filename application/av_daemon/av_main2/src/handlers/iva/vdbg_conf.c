#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_vdbg_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Vdbg)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_VDBG_CONF);
	/* parse list of object from Json Object */
	parse_vdbg_conf(&g_conf.vdbg, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read vdbg and enable context:%d.", g_conf.vdbg.ctx);

	return 0;
}

int WRITE_DB(Vdbg)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Vdbg)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_VDBG_CONF_S *vdbg = (AGTX_VDBG_CONF_S *)data;
	if (len != sizeof(AGTX_VDBG_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_VDBG_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.vdbg, vdbg, sizeof(AGTX_VDBG_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, VDBG_ATTR, vdbg);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.vdbg, sizeof(AGTX_CMD_VDBG_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler vdbg_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_VDBG_CONF, Vdbg);

__attribute__((constructor)) void registerVdbg(void)
{
	HANDLERS_registerHandlers(&vdbg_ops);
}