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

#include "agtx_osd.h"
#include "cm_osd_pm_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(OsdPm)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_OSD_PM_CONF);
	parse_osd_pm_conf(&g_conf.osd_pm, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("osd_pm[0] enabled:%d.", g_conf.osd_pm.conf[0].param[0].enabled);

	return 0;
}

int WRITE_DB(OsdPm)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(OsdPm)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	int ret = 0;
	int ptr = 0;
	AGTX_OSD_PM_CONF_S *osd_pm = (AGTX_OSD_PM_CONF_S *)data;
	if (len != sizeof(AGTX_OSD_PM_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_OSD_PM_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.osd_pm, osd_pm, sizeof(AGTX_OSD_PM_CONF_S));
	AGTX_OSD_PM_CONF_S *osd_pm_old = (AGTX_OSD_PM_CONF_S *)&g_old_conf_tmp;

	if (isOsdOverNumber(&g_conf.osd, osd_pm) == true) {
		recoverOldConfFromTmp(&g_conf.osd_pm, sizeof(AGTX_OSD_PM_CONF_S));
		return -EOVERFLOW;
	}

	/* if has restart, don't need to set */
	for (int i = 0; i < MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; j++) {
			if (osd_pm->conf[i].param[j].start_x > 100 || osd_pm->conf[i].param[j].start_x < 0 ||
			    osd_pm->conf[i].param[j].start_y > 100 || osd_pm->conf[i].param[j].start_y < 0 ||
			    osd_pm->conf[i].param[j].end_x > 100 || osd_pm->conf[i].param[j].end_x < 0 ||
			    osd_pm->conf[i].param[j].end_y > 100 || osd_pm->conf[i].param[j].end_y < 0) {
				avmain2_log_err("Osd pm coordinates > 100 || < 0, %d, %d, %d, %d",
				                osd_pm->conf[i].param[j].start_x, osd_pm->conf[i].param[j].start_y,
				                osd_pm->conf[i].param[j].end_x, osd_pm->conf[i].param[j].end_y);
				recoverOldConfFromTmp(&g_conf.osd_pm, sizeof(AGTX_OSD_PM_CONF_S));
				return -EINVAL;
			}
			if (osd_pm->conf[i].param[j].enabled != osd_pm_old->conf[i].param[j].enabled ||
			    osd_pm->conf[i].param[j].end_x != osd_pm_old->conf[i].param[j].end_x ||
			    osd_pm->conf[i].param[j].end_y != osd_pm_old->conf[i].param[j].end_y ||
			    osd_pm->conf[i].param[j].start_x != osd_pm_old->conf[i].param[j].start_x ||
			    osd_pm->conf[i].param[j].start_y != osd_pm_old->conf[i].param[j].start_y) {
				avmain2_log_debug("node_enc osd pm restart case");
				ptr = ((int)node) + (ENC * sizeof(Node));
				ret = NODES_execRestart((Node *)ptr);
				if (ret != 0) {
					avmain2_log_info("copy ENC to g_conf, and save old to tmp");
					recoverOldConfFromTmp(&g_conf.osd_pm, sizeof(AGTX_OSD_PM_CONF_S));
					return ret;
				} else {
					recoverTmptoZero();
					return 0;
				}
			}
		}
	}

	for (int i = 0; i < MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; j++) {
			if (osd_pm->conf[i].param[j].alpha != osd_pm_old->conf[i].param[j].alpha ||
			    osd_pm->conf[i].param[j].color != osd_pm_old->conf[i].param[j].color) {
				ptr = ((int)node) + (ENC * sizeof(Node));
				ret = NODES_execSet(((Node *)ptr), OSD_PM, osd_pm);
				if (ret != 0) {
					recoverOldConfFromTmp(&g_conf.osd_pm, sizeof(AGTX_OSD_PM_CONF_S));
					return ret;
				} else {
					recoverTmptoZero();
					return 0;
				}
				break;
			}
		}
	}

	recoverTmptoZero();
	return 0;
}

static JsonConfHandler osd_pm_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_OSD_PM_CONF, OsdPm);

__attribute__((constructor)) void registerOsdPm(void)
{
	HANDLERS_registerHandlers(&osd_pm_ops);
}
