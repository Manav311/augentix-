#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_iva.h"
#include "agtx_video.h"
#include "cm_video_ldc_conf.h"
#include "app_view_api.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Ldc)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_LDC_CONF);
	parse_ldc_conf(&g_conf.ldc, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("ldc enable:%d. ratio:%d", g_conf.ldc.enable, g_conf.ldc.ratio);

	return 0;
}

int WRITE_DB(Ldc)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Ldc)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_LDC_CONF_S *ldc = (AGTX_LDC_CONF_S *)data;
	if (len != sizeof(AGTX_LDC_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_LDC_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.ldc, ldc, sizeof(AGTX_LDC_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	/*change attr only need to set chn*/
	id = CHN;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, LDC_ATTR, ldc);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.ldc, sizeof(AGTX_CMD_LDC_CONF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler ldc_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_LDC_CONF, Ldc);

__attribute__((constructor)) void registerLdc(void)
{
	HANDLERS_registerHandlers(&ldc_ops);
}
