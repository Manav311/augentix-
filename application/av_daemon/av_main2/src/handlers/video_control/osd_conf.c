#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mpi_osd.h"

#include "agtx_osd.h"
#include "cm_osd_conf.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(Osd)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_OSD_CONF);
	parse_osd_conf(&g_conf.osd, ret_obj);
	json_object_put(ret_obj);
	avmain2_log_info("osd showWeekDay:%d.", g_conf.osd.showWeekDay);

	return 0;
}

int WRITE_DB(Osd)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

bool isOsdOverNumber(AGTX_OSD_CONF_S *osd, AGTX_OSD_PM_CONF_S *osd_pm)
{
	uint8_t osd_cnt = 0;

	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			if (g_conf.strm.video_strm[i].strm_en != 1) {
				osd_cnt = 0;
				break;
			}

			if (osd->strm[i].region[j].enabled == 1) {
				osd_cnt++;
			}

			if (osd_pm->conf[i].param[j].enabled == 1) {
				osd_cnt++;
			}

			if (osd_cnt > MPI_OSD_MAX_BIND_CHANNEL) {
				avmain2_log_err("in chn[%d]osd +osd pm num over max: %d", i,
				                MPI_OSD_MAX_BIND_CHANNEL);
				return true;
			}
		}

		/* each enc re-cnt 4 canvas */
		osd_cnt = 0;
	}
	return false;
}

bool isTimestampEnabled(AGTX_OSD_CONF_S *osd)
{
	bool isEnabled = false;

	for (int i = 0; i < MAX_AGTX_OSD_CONF_S_STRM_SIZE; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			if (osd->strm[i].region[j].type == AGTX_OSD_TYPE_INFO) {
				isEnabled = true;
				return isEnabled;
			}
		}
	}

	return isEnabled;
}

int APPLY(Osd)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	int ret = 0;
	int ptr = 0;
	AGTX_OSD_CONF_S *osd = (AGTX_OSD_CONF_S *)data;
	if (len != sizeof(AGTX_OSD_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_OSD_CONF_S));
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.osd, osd, sizeof(AGTX_OSD_CONF_S));
	AGTX_OSD_CONF_S *osd_old = (AGTX_OSD_CONF_S *)&g_old_conf_tmp;

	if (isOsdOverNumber(osd, &g_conf.osd_pm) == true) {
		recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
		return -EOVERFLOW;
	}

	if (isTimestampEnabled(osd) != isTimestampEnabled(osd_old)) {
		ptr = ((int)node) + (ENC * sizeof(Node));
		ret = NODES_execRestart((Node *)ptr);
		if (ret != 0) {
			avmain2_log_info("copy ENC to g_conf, and save old to tmp");
			recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
			return ret;
		} else {
			recoverTmptoZero();
			return 0;
		}
	}

	/* if has restart, don't need to set */
	for (int i = 0; i < MAX_AGTX_OSD_CONF_S_STRM_SIZE; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			if (osd->strm[i].region[j].start_x > 100 || osd->strm[i].region[j].start_x < 0 ||
			    osd->strm[i].region[j].start_y > 100 || osd->strm[i].region[j].start_y < 0) {
				avmain2_log_err("Invalid input coordinate > 100 || < 0 %d, %d",
				                osd->strm[i].region[j].start_x, osd->strm[i].region[j].start_y);
				recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
				return -EINVAL;
			}

			if (osd->strm[i].region[j].enabled != osd_old->strm[i].region[j].enabled ||
			    osd->strm[i].region[j].type != osd_old->strm[i].region[j].type ||
			    strcmp((const char *)osd->strm[i].region[j].type_spec,
			           (const char *)osd_old->strm[i].region[j].type_spec) != 0) {
				avmain2_log_debug("node_enc restart case");
				ptr = ((int)node) + (ENC * sizeof(Node));
				ret = NODES_execRestart((Node *)ptr);
				if (ret != 0) {
					avmain2_log_info("copy ENC to g_conf, and save old to tmp");
					recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
					return ret;
				} else {
					recoverTmptoZero();
					return 0;
				}
			}
		}
	}

	for (int i = 0; i < MAX_AGTX_OSD_CONF_S_STRM_SIZE; i++) {
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			if (osd->showWeekDay != osd_old->showWeekDay) {
				ptr = ((int)node) + (ENC * sizeof(Node));
				ret = NODES_execSet(((Node *)ptr), OSD_SHOW_WEEK_DAY, osd);
				if (ret != 0) {
					recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
					return ret;
				} else {
					recoverTmptoZero();
					return 0;
				}
			}
			if (osd->strm[i].region[j].start_x != osd_old->strm[i].region[j].start_x ||
			    osd->strm[i].region[j].start_y != osd_old->strm[i].region[j].start_y) {
				ptr = ((int)node) + (ENC * sizeof(Node));
				ret = NODES_execSet(((Node *)ptr), OSD_SRC, osd);
				if (ret != 0) {
					recoverOldConfFromTmp(&g_conf.osd, sizeof(AGTX_OSD_CONF_S));
					return ret;
				} else {
					recoverTmptoZero();
					return 0;
				}
			}
		}
	}

	recoverTmptoZero();
	return 0;
}

static JsonConfHandler osd_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_OSD_CONF, Osd);

__attribute__((constructor)) void registerOsd(void)
{
	HANDLERS_registerHandlers(&osd_ops);
}
