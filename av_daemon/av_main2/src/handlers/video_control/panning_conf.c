#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_video.h"
#include "cm_panning_conf.h"
#include "app_view_api.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Panning)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_PANNING_CONF);
	parse_panning_conf(&g_conf.panning, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("panning enable:%d. radius:%d", g_conf.panning.enable, g_conf.panning.radius);

	return 0;
}

int WRITE_DB(Panning)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Panning)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_PANNING_CONF_S *panning = (AGTX_PANNING_CONF_S *)data;
	if (len != sizeof(AGTX_PANNING_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_PANNING_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.panning, panning, sizeof(AGTX_PANNING_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	/* change attr only need to set chn */
	id = CHN;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, PANNING_ATTR, panning);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.panning, sizeof(AGTX_PANNING_CONF_S));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler panning_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_PANNING_CONF, Panning);

__attribute__((constructor)) void registerPanning(void)
{
	HANDLERS_registerHandlers(&panning_ops);
}
