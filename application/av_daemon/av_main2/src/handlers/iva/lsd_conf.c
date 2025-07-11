#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iaa_lsd_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Lsd)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_LSD_CONF);
	/* parse list of object from Json Object */
	parse_iaa_lsd_conf(&g_conf.lsd, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Lsd_conf from database and volume: %d.", g_conf.lsd.volume);
	return 0;
}

int WRITE_DB(Lsd)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Lsd)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IAA_LSD_CONF_S *lsd = (AGTX_IAA_LSD_CONF_S *)data;
	if (len != sizeof(AGTX_IAA_LSD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IAA_LSD_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.lsd, lsd, sizeof(AGTX_IAA_LSD_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, LSD_ATTR, lsd);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.lsd, sizeof(AGTX_CMD_LSD_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler lsd_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_LSD_CONF, Lsd);

__attribute__((constructor)) void registerLsd(void)
{
	HANDLERS_registerHandlers(&lsd_ops);
}