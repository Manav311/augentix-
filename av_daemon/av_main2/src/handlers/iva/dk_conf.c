#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iva_dk_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Dk)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_DK_CONF);
	/* parse list of object from Json Object */
	parse_iva_dk_conf(&g_conf.dk, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Dk_conf from database and use channel: %d.", g_conf.dk.video_chn_idx);

	return 0;
}

int WRITE_DB(Dk)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Dk)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IVA_DK_CONF_S *dk = (AGTX_IVA_DK_CONF_S *)data;
	if (len != sizeof(AGTX_IVA_DK_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IVA_DK_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.dk, dk, sizeof(AGTX_IVA_DK_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, DK_ATTR, dk);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.dk, sizeof(AGTX_CMD_DK_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler dk_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_DK_CONF, Dk);

__attribute__((constructor)) void registerDk(void)
{
	HANDLERS_registerHandlers(&dk_ops);
}