#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_video.h"
#include "cm_video_dev_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int READ_DB(VideoDev)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_VIDEO_DEV_CONF);
	parse_video_dev_conf(&g_conf.dev, ret_obj);

	avmain2_log_info("dev video_dev_idx:%d, hdr_mode:%d, stitch_en:%d, eis_en:%d, input_fps:%d, input path cnt:%d",
	                 g_conf.dev.video_dev_idx, g_conf.dev.hdr_mode, g_conf.dev.stitch_en, g_conf.dev.eis_en,
	                 g_conf.dev.input_fps, g_conf.dev.input_path_cnt);

	json_object_put(ret_obj);

	return 0;
}

int WRITE_DB(VideoDev)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(VideoDev)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_DEV_CONF_S *dev = (AGTX_DEV_CONF_S *)data;
	if (len != sizeof(AGTX_DEV_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_DEV_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.dev, dev, sizeof(AGTX_DEV_CONF_S));

	NodeId id = DEV;
	int ptr = ((int)node) + (id * sizeof(Node));

	int ret = 0;
	ret = NODES_execSet((Node *)ptr, EIS_STRENGTH, dev);

	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.dev, sizeof(AGTX_DEV_CONF_S));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler video_dev_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_VIDEO_DEV_CONF, VideoDev);

__attribute__((constructor)) void registerVideoDev(void)
{
	HANDLERS_registerHandlers(&video_dev_ops);
}