#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_gpio_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Gpio)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_GPIO_CONF);
	/* parse list of object from Json Object */
	parse_gpio_conf(&g_conf.gpio, ret_obj);
	json_object_put(ret_obj);

	return 0;
}

int WRITE_DB(Gpio)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Gpio)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_UNUSED(data);
	AGTX_UNUSED(len);
	AGTX_UNUSED(node);

	return 0;
}

static JsonConfHandler gpio_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_GPIO_CONF, Gpio);

__attribute__((constructor)) void registerGpio(void)
{
	HANDLERS_registerHandlers(&gpio_ops);
}
