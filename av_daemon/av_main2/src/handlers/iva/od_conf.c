#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iva_od_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Od)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_OD_CONF);
	/* parse list of object from Json Object */
	parse_iva_od_conf(&g_conf.od, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Od_conf from database and use channel: %d.", g_conf.od.video_chn_idx);
	return 0;
}

int WRITE_DB(Od)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Od)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IVA_OD_CONF_S *od = (AGTX_IVA_OD_CONF_S *)data;
	if (len != sizeof(AGTX_IVA_OD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IVA_OD_CONF_S));
		return -EINVAL;
	}

	UINT8 old_od_version = g_conf.od.version;

	saveOldConftoTmp(&g_conf.od, od, sizeof(AGTX_IVA_OD_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node)); // Node *ptr = &((Node *)node)[DEV];

	if (g_conf.od.version != old_od_version) {
		id = VB; // Tried VB/DEV/CHN/IVA and failed
		ptr = ((int)node) + (id * sizeof(Node));
		ret = NODES_execRestart((Node *)ptr);
	} else {
		ret = NODES_execSet((Node *)ptr, OD_ATTR, od);
		if (ret != 0) {
			recoverOldConfFromTmp(&g_conf.od, sizeof(AGTX_CMD_OD_CONF));
			return ret;
		}
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler od_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_OD_CONF, Od);

__attribute__((constructor)) void registerOd(void)
{
	HANDLERS_registerHandlers(&od_ops);
}