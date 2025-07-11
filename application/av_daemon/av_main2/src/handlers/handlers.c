#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sqllib.h"
#include "log_define.h"

#define HANDLERS_MAX_NUM (64)

static JsonConfHandler g_handlers[HANDLERS_MAX_NUM] = { { 0 } };
AgtxConf g_old_conf_tmp;
extern Node g_nodes[NODE_NUM];

static int validate_json_string_db(struct json_object **json_obj, char *str, int strlen)
{
	int ret = 0;
	struct json_object *obj = NULL;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;

	/* Parse the buf */
	if (strlen > 0) {
		obj = json_tokener_parse_ex(tok, str, strlen);
	} else {
		*json_obj = NULL;
		ret = -1;
		goto end;
	}

	jerr = json_tokener_get_error(tok);

	if (jerr == json_tokener_success) {
		*json_obj = obj;
	} else {
		avmain2_log_err("JSON Parsing errorer %s", json_tokener_error_desc(jerr));

		*json_obj = NULL;
		ret = -1;
	}
end:
	json_tokener_free(tok);

	return ret;
}

struct json_object *get_db_record_obj(const char *db_path, const int id)
{
	int ret = 0;
	char *rec = NULL;
	struct json_object *obj = NULL;

	rec = get_sqldata_by_id_str_path(db_path, "json_tbl", "jstr", id);
	if (!rec) {
		avmain2_log_err("Record (id 0x%08X) is not in DB %s", id, db_path);
		goto end;
	}

	ret = validate_json_string_db(&obj, rec, strlen(rec));
	if (ret < 0) {
		avmain2_log_err("Record (id 0x%08X) is not a valid JSON string, ret: %d", id, ret);
		/* obj will be NULL pointer */
	}

	free(rec);
end:
	return obj;
}

void saveOldConftoTmp(void *dst, void *data, int len)
{
	memcpy(&g_old_conf_tmp, dst, len);
	memcpy(dst, data, len);
}

void recoverOldConfFromTmp(void *dst, int len)
{
	memcpy(dst, &g_old_conf_tmp, len);
	memset(&g_old_conf_tmp, 0, sizeof(g_old_conf_tmp));
}

void recoverTmptoZero(void)
{
	memset(&g_old_conf_tmp, 0, sizeof(g_old_conf_tmp));
}

int HANDLERS_registerHandlers(JsonConfHandler *handler)
{
	/**< Count the times of function be called, as the handler's idx */
	static uint8_t idx = 0;
	if (idx == HANDLERS_MAX_NUM) {
		return -EOVERFLOW;
	}

	/*register to handlers list*/
	handler->id = idx;
	memcpy(&g_handlers[idx], handler, sizeof(JsonConfHandler));

	/**< Point to next option */
	return (++idx);
}

int HANDLERS_allReadDb(const char *path)
{
	for (int i = 0; i < HANDLERS_MAX_NUM; i++) {
		if (g_handlers[i].read_db != NULL) {
			g_handlers[i].read_db(path);
		}
	}

	return 0;
}

int HANDLERS_apply(int cmd_id /*atgx_cmd_id*/, int cmd_len, AGTX_CDATA data /*AGTX struct*/, void *node_ptr)
{
	int ret = 0;
	avmain2_log_debug("recv: cmd id[%d], len:%d", cmd_id, cmd_len);
	for (int i = 0; i < HANDLERS_MAX_NUM; i++) {
		avmain2_log_debug("cmd[%d].id: %d, cmd:%d", i, g_handlers[i].id, g_handlers[i].cmd);
		if (g_handlers[i].cmd == cmd_id) {
			ret = g_handlers[i].apply(data, cmd_len, node_ptr);
			if (ret != 0) {
				/*ERR*/
				avmain2_log_debug("cmd[%d].id: %d, cmd:%d", i, g_handlers[i].id, g_handlers[i].cmd);
				/*Copy global tmp back to g_conf*/
				return -EINVAL;
			}
			break;
		}
	}

	/*reply ccserver/http server success and  write db */
	return 0;
}

#ifndef USE_CCSERVER
int HANDLERS_write(int cmd_id /*atgx_cmd_id*/, void *data /*AGTX struct*/, const char *path)
{
	return 0;
}
/*phase-2 only*/
int HANDLERS_get(int cmd_id /*atgx_cmd_id*/, void *data /*AGTX struct*/, const char *path)
{
	return 0;
}
#endif
