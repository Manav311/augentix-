#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_audio_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Audio)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_AUDIO_CONF);
	parse_audio_conf(&g_conf.audio, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("Audio enable:%d. codec:%d", g_conf.audio.enabled, g_conf.audio.codec);

	return 0;
}

int WRITE_DB(Audio)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(Audio)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_UNUSED(node);

	AGTX_AUDIO_CONF_S *audio = (AGTX_AUDIO_CONF_S *)data;

	if (len != sizeof(AGTX_AUDIO_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_AUDIO_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.audio, audio, sizeof(AGTX_AUDIO_CONF_S));
	AGTX_AUDIO_CONF_S *audio_old = (AGTX_AUDIO_CONF_S *)&g_old_conf_tmp;

	if (audio_old->enabled != audio->enabled) {
		avmain2_log_info("Audio %s", audio->enabled == 1 ? "Enable" : "Disable");
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler audio_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_AUDIO_CONF, Audio);

__attribute__((constructor)) void registerAudio(void)
{
	HANDLERS_registerHandlers(&audio_ops);
}
