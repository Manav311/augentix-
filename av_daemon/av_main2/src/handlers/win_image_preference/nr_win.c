#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_dip_nr_win_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(NrWin)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_DIP_NR_WIN);
	/* parse list of object from Json Object */
	parse_dip_nr_win_conf(&g_conf.nr_win, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("nr_win nr win:%d.", g_conf.nr_win.win_nr_en);
	return 0;
}

int WRITE_DB(NrWin)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(NrWin)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_DIP_NR_WIN_CONF_S *nr_win = (AGTX_DIP_NR_WIN_CONF_S *)data;
	if (len != sizeof(AGTX_DIP_NR_WIN_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_DIP_NR_WIN_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.nr_win, nr_win, sizeof(AGTX_DIP_NR_WIN_CONF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = WIN_IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, NR_WIN, nr_win);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.nr_win, sizeof(AGTX_DIP_NR_WIN_CONF_S));
		return ret;
	}
	recoverTmptoZero();

	return 0;
}

static JsonConfHandler nr_win_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_DIP_NR_WIN, NrWin);

__attribute__((constructor)) void registerNrWin(void)
{
	HANDLERS_registerHandlers(&nr_win_ops);
}