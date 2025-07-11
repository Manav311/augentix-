#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_iva_md_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Md)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_MD_CONF);
	/* parse list of object from Json Object */
	parse_iva_md_conf(&g_conf.md, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Md_conf from database and use channel: %d.", g_conf.md.video_chn_idx);
	return 0;
}

int WRITE_DB(Md)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Md)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IVA_MD_CONF_S *md = (AGTX_IVA_MD_CONF_S *)data;
	if (len != sizeof(AGTX_IVA_MD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IVA_MD_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.md, md, sizeof(AGTX_IVA_MD_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, MD_ATTR, md);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.md, sizeof(AGTX_CMD_MD_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler md_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_MD_CONF, Md);

__attribute__((constructor)) void registerMd(void)
{
	HANDLERS_registerHandlers(&md_ops);
}