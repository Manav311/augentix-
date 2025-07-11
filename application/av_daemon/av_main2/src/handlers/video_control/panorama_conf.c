#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>
#include <fcntl.h>

#include "agtx_iva.h"
#include "agtx_video.h"
#include "cm_panorama_conf.h"
#include "app_view_api.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Panorama)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_PANORAMA_CONF);
	parse_panorama_conf(&g_conf.panorama, ret_obj);

	avmain2_log_info("panorama enable:%d. curvature:%d", g_conf.panorama.enable, g_conf.panorama.curvature);

	return 0;
}

int WRITE_DB(Panorama)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Panorama)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_PANORAMA_CONF_S *panorama = (AGTX_PANORAMA_CONF_S *)data;
	if (len != sizeof(AGTX_PANORAMA_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_PANORAMA_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.panorama, panorama, sizeof(AGTX_PANORAMA_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	/* change attr only need to set chn */
	id = CHN;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, PANORAMA_ATTR, panorama);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.panorama, sizeof(AGTX_PANORAMA_CONF_S));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler panorama_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_PANORAMA_CONF, Panorama);

__attribute__((constructor)) void registerPanorama(void)
{
	HANDLERS_registerHandlers(&panorama_ops);
}
