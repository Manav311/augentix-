#ifndef HANDLERS_H_
#define HANDLERS_H_

#include "agtx_cmd.h"
#include "agtx_osd.h"
#include "json.h"

#include "core.h"
#include "nodes.h"
typedef struct {
	int id; /*idx in g_handlers*/
	int cmd; /*cmd in libagtx*/
	int (*read_db)(const char * /*db path*/);
	int (*Write_db)(const char * /*db path*/);
	int (*apply)(AGTX_CDATA /*agtx input*/, int /*len*/, void * /*node should be root*/);
} JsonConfHandler;

/**
 * @brief Generate wrapping function
 * @param[in] f    function name
 */
#define HANLDER_WRAPPER(f) HANDLER_##f
#define READ_DB(f) HANLDER_WRAPPER(readDB##f) /**< HANDLER_readDB##f */
#define WRITE_DB(f) HANLDER_WRAPPER(writeDB##f) /**< HANDLER_writeDB##f */
#define APPLY(f) HANLDER_WRAPPER(apply##f) /**< HANDLER_apply##f */

/**
 * @brief Helper macro to define a JsonConfHandler structure
 * @param[in] f       function name, used for read_db, write_db, apply
 */
#define MAKE_JSON_CONF_HANDLER(cmd, f)                        \
	{                                                     \
		(0), (cmd), READ_DB(f), WRITE_DB(f), APPLY(f) \
	}

int HANDLERS_registerHandlers(JsonConfHandler *handler);
int HANDLERS_allReadDb(const char *path);
int HANDLERS_apply(int cmd_id /*atgx_cmd_id*/, int cmd_len, AGTX_CDATA data /*AGTX struct*/, void *node_ptr);
#ifdef USE_CCSERVER
#else
int HANDLERS_write(int cmd_id /*atgx_cmd_id*/, void *data /*AGTX struct*/, const char *path);
int HANDLERS_get(int cmd_id /*atgx_cmd_id*/, void *data /*AGTX struct*/, const char *path);
#endif

struct json_object *get_db_record_obj(const char *db_path, const int id);
void saveOldConftoTmp(void *dst, void *data, int len);
void recoverOldConfFromTmp(void *dst, int len);
void recoverTmptoZero(void);
bool isOsdOverNumber(AGTX_OSD_CONF_S *osd, AGTX_OSD_PM_CONF_S *osd_pm);
bool isTimestampEnabled(AGTX_OSD_CONF_S *osd);
#endif