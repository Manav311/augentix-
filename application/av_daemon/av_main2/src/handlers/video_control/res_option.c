#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_res_option.h"
#include "cm_res_option.h"

#include "agtx_cmd.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(ResOption)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_RES_OPTION);
	parse_res_option(&g_conf.res_option, ret_obj);

	avmain2_log_info("res option strm[0](%d, %d). ", g_conf.res_option.strm[0].res[0].width,
	                 g_conf.res_option.strm[0].res[0].height);

	json_object_put(ret_obj);

	return 0;
}

int WRITE_DB(ResOption)(const char *path)
{
	AGTX_UNUSED(path);

	avmain2_log_err("this command can;t change after FW flashed");
	return -EINVAL;
}

int APPLY(ResOption)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_UNUSED(data);
	AGTX_UNUSED(len);
	AGTX_UNUSED(node);

	avmain2_log_err("this command can;t change after FW flashed");
	return -EINVAL;
}

static JsonConfHandler resoption_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_RES_OPTION, ResOption);

__attribute__((constructor)) void registerResOption(void)
{
	HANDLERS_registerHandlers(&resoption_ops);
}