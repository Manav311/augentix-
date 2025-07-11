#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_video_ptz_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(Ptz)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_VIDEO_PTZ_CONF);
	/* parse list of object from Json Object */
	parse_video_ptz_conf(&g_conf.ptz, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Read Ptz_conf from database and use mode: %d.", g_conf.ptz.mode);
	return 0;
}

int WRITE_DB(Ptz)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Ptz)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_VIDEO_PTZ_CONF_S *ptz = (AGTX_VIDEO_PTZ_CONF_S *)data;
	if (len != sizeof(AGTX_VIDEO_PTZ_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_VIDEO_PTZ_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.ptz, ptz, sizeof(AGTX_VIDEO_PTZ_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IVA;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, PTZ_ATTR, ptz);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.ptz, sizeof(AGTX_CMD_VIDEO_PTZ_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler ptz_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_VIDEO_PTZ_CONF, Ptz);

__attribute__((constructor)) void registerPtz(void)
{
	HANDLERS_registerHandlers(&ptz_ops);
}