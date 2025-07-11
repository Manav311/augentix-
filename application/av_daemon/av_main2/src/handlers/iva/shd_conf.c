#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iva_shd_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Shd)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_SHD_CONF);
	/* parse list of object from Json Object */
	parse_iva_shd_conf(&g_conf.shd, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Shd_conf from database and use channel:  %d.", g_conf.shd.video_chn_idx);
	return 0;
}

int WRITE_DB(Shd)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Shd)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IVA_SHD_CONF_S *shd = (AGTX_IVA_SHD_CONF_S *)data;
	if (len != sizeof(AGTX_IVA_SHD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IVA_SHD_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.shd, shd, sizeof(AGTX_IVA_SHD_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, SHD_ATTR, shd);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.shd, sizeof(AGTX_CMD_SHD_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler shd_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_SHD_CONF, Shd);

__attribute__((constructor)) void registerShd(void)
{
	HANDLERS_registerHandlers(&shd_ops);
}