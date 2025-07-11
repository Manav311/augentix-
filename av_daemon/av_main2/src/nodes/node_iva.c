/* other to include you */
#include "nodes.h"

/* standard lib */
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

/* AGTX includes */
#include "agtx_event_conf.h"
#include "avftr_bm.h"
#include "avftr_conn.h"
#include "video_ptz.h"

/* private include */
#include "core.h"
#include "log_define.h"
#ifdef CB_BASED_OD
#include "sample_od.h"
#endif

extern GlobalConf g_conf;
extern Node g_nodes[NODE_NUM];
extern int g_run_flag;
typedef struct {
	MPI_WIN idx;
	MPI_SIZE_S res;
} AROI_CB_INFO;

int g_event_daemon_md_sockfd = 0;
int g_event_daemon_td_sockfd = 0;
int g_event_daemon_onvif_sockfd = 0;
AROI_CB_INFO g_aroi_cb_info = {
	.idx = { { .dev = 0, .chn = 0, .win = 0, .dummy0 = 0 } },
	.res = { .width = 0, .height = 0 },
};

#define EAIF_FACE_DIR_NAME "/usrdata/eaif/facereco"
#define EAIF_FACE_BIN_NAME "face.bin"
#define MD_ENERGY_TH_V_REF_RATIO (15) /* FIXME: Tuning the featurelib or formula */
#define MD_ENERGY_TH_V_REF_RATIO_MAX (255)

static void hideIvaAlarm(int signum)
{
	AGTX_UNUSED(signum);

	if (NODES_execSet((Node *)&g_nodes[ENC], MD_ALARM_END, &g_conf.osd) != 0) {
		avmain2_log_err("Fail to hide encoder alarm bell.\r\n");
	}

	return;
}

/**
 * @brief Tamper detection(TD) and motion detection(MD) will trigger alarm on the screen display.
 * Set OSD bell and red warning in pecified number of seconds(3).
 * NOTICE: Cancelled the previous alarm should call alarm(0). Don't use 'sleep' and 'pause'. That's blocking view.
 * @param type TD&MD alarm/end
 */
static int showIvaAlarm(NodeEncSetType type)
{
	NodeEncSetType alarm_type = type == TD_ALARM ? TD_ALARM : MD_ALARM;

	avmain2_log_debug("IVA alarm %s\r\n", type == TD_ALARM ? "TD_ALARM" : "MD_ALARM");

	if (NODES_execSet((Node *)&g_nodes[ENC], alarm_type, &g_conf.osd) != 0) {
		avmain2_log_err("Fail to show encoder alarm bell.\r\n");
	}

	signal(SIGALRM, hideIvaAlarm);
	alarm(3);
	avmain2_log_debug("IVA alarm %s\r\n", type == TD_ALARM ? "TD_ALARM" : "MD_ALARM");

	return 0;
}

/**
 * @brief Send md(info) socket to onvif server.
 */
static void sendMdEventToOnvifServer(AGTX_SW_EVENT_TRIG_TYPE_E type)
{
	int ret = 0;
	int sockfd = -1;
	int servlen = 0;
	int alarm = 0;
	struct sockaddr_un serv_addr;

	if (g_event_daemon_onvif_sockfd <= 0) {
		/* Connect to onvif */
		bzero((char *)&serv_addr, sizeof(serv_addr));
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, ONVIF_EVENT_PATH);
		servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

		if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			avmain2_log_err("Create ONVIF sockfd failed %d(%m)\r\n", errno);
			sockfd = -1;
		}

		if (sockfd > 0) {
			fcntl(sockfd, F_SETFL, O_NONBLOCK);

			if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
				avmain2_log_debug("Connecting to ONVIF server failed %d(%m)\r\n", errno);
				close(sockfd);
				sockfd = -1;
				return;
			} else {
				g_event_daemon_onvif_sockfd = sockfd;
			}
		}

		alarm = type;
		ret = write(g_event_daemon_onvif_sockfd, &alarm, sizeof(alarm));
		if (ret < 0) {
			avmain2_log_err("write socket error %d(%m)\r\n", errno);
			return;
		}

	} else {
		alarm = type;
		ret = write(g_event_daemon_onvif_sockfd, &alarm, sizeof(alarm));
		if (ret < 0) {
			avmain2_log_err("write socket error %d(%m)\r\n", errno);
			close(g_event_daemon_onvif_sockfd);
			g_event_daemon_onvif_sockfd = -1;
			return;
		}
	}
}

/**
 * @brief Callback MD warning on the screen display.
 */
static void callbackMd(uint8_t alarm)
{
	int ret = 0;
	static time_t duration;
	static uint32_t prev_alarm = 0;
	AGTX_SW_EVENT_RULE_S rule = { 0 };

	time_t t = time(0);

	if (prev_alarm ^ alarm) { /* edge trigger */
		/* ONVIF */
		sendMdEventToOnvifServer(alarm ? AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE :
		                                 AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_NEGATIVE);
		prev_alarm = alarm;
	}

	if (time(0) < duration + 2) {
		syslog(LOG_ERR, "%s < %s + 2\n", asctime(gmtime(&t)), asctime(gmtime(&duration)));
		return;
	}

	duration = t;

	if (alarm) { /* level trigger */
		showIvaAlarm(MD_ALARM);

		rule.trigger_type = AGTX_SW_EVENT_TRIG_TYPE_IVA_MD_POSITIVE;
		if (g_event_daemon_md_sockfd > 0) {
			ret = write(g_event_daemon_md_sockfd, &rule, sizeof(rule));
			if (ret < 0) {
				syslog(LOG_ERR, "Failed to write g_event_daemon_md_sockfd: %d\n", ret);
				syslog(LOG_ERR, "write socket error %d(%m)\r\n", errno);

				/* This condition happened if eventd SW thread disabled */
				if (errno == EAGAIN) {
					avmain2_log_err("MD eagain\n");
				}
				return;
			}
		}
	}

	return;
}

/**
 * @brief Callback TD warning on the screen display.
 */
static void callbackTd()
{
	int ret = 0;
	static time_t duration;
	AGTX_SW_EVENT_RULE_S rule = { 0 };

	time_t t = time(0);

	if (time(0) < duration + 2) {
		syslog(LOG_ERR, "%s < %s + 2\n", asctime(gmtime(&t)), asctime(gmtime(&duration)));
		return;
	}
	duration = t;

	showIvaAlarm(TD_ALARM);

	rule.trigger_type = AGTX_SW_EVENT_TRIG_TYPE_IVA_TD_POSITIVE;
	if (g_event_daemon_td_sockfd > 0) {
		ret = write(g_event_daemon_td_sockfd, &rule, sizeof(rule));
		if (ret < 0) {
			syslog(LOG_ERR, "Failed to write g_event_daemon_td_sockfd: %d\n", ret);
			syslog(LOG_ERR, "write socket error %d(%m)\r\n", errno);

			/* This condition happened if eventd SW thread disabled */
			if (errno == EAGAIN) {
				avmain2_log_err("TD eagain\n");
			}

			avmain2_log_err("write socket error %d(%m)\r\n", errno);
			return;
		}
	}

	return;
}

/**
 * @brief Callback LSD warning on the console.
 */
static void callbackLsd()
{
	avmain2_log_notice("Loud sound detected!\n");
	return;
}

static void callbackAroiEmpty(MPI_WIN idx, const VFTR_AROI_STATUS_S *status, const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	AGTX_UNUSED(idx);
	AGTX_UNUSED(status);
	AGTX_UNUSED(obj_list);

	return;
}

static void callbackAroiInfo(MPI_WIN idx, const VFTR_AROI_STATUS_S *status, const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	AGTX_UNUSED(idx);
	AGTX_UNUSED(obj_list);

	const MPI_RECT_POINT_S *p = &status->roi;
	MPI_RECT_POINT_S roi_tmp = { 0 };
	MPI_SIZE_S *res = &g_aroi_cb_info.res;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
	roi_tmp.sx = MAX(p->sx * 1024 / MAX(res->width, 1), 0);
	roi_tmp.sy = MAX(p->sy * 1024 / MAX(res->height, 1), 0);
	roi_tmp.ex = MIN(MAX(p->ex * 1024 / MAX(res->width, 1), 0), 1023);
	roi_tmp.ey = MIN(MAX(p->ey * 1024 / MAX(res->height, 1), 0), 1023);
	VIDEO_FTR_updateAutoPtz(&roi_tmp);

	return;
}

/**
 * @brief connect to event demon.
 * @return The execution result.
 */
static int connectEventDaemon()
{
	int servlen = 0;
	int sockfd = -1;
	struct sockaddr_un serv_addr;

	/* Connect to event demon for TD */
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, TD_SOCKET_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		avmain2_log_err("Create TD sockfd failed %d(%m)\r\n", errno);
		sockfd = -1;
	}

	if (sockfd > 0) {
		fcntl(sockfd, F_SETFL, O_NONBLOCK);

		if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
			avmain2_log_warn("Connecting to TD server failed %d(%m)\r\n", errno);
			close(sockfd);
			sockfd = -1;
		} else {
			g_event_daemon_td_sockfd = sockfd;
		}
	}

	/* Connect to event demon for MD */
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, MD_SOCKET_PATH);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		avmain2_log_err("Create MD sockfd failed %d(%m).\r\n", errno);
		sockfd = -1;
	}

	if (sockfd > 0) {
		fcntl(sockfd, F_SETFL, O_NONBLOCK);
		if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
			avmain2_log_warn("Connecting to MD server failed %d(%m).\r\n", errno);
			close(sockfd);
			sockfd = -1;
		} else {
			g_event_daemon_md_sockfd = sockfd;
		}
	}

	return 0;
}

/**
 * @brief Set tamper detection(TD).
 */
static int setTd(MPI_WIN win_idx, const AGTX_IVA_TD_CONF_S *td)
{
	int sen_min_map = 0;
	int sen_max_map = 0;
	int temp = 0;
	AVFTR_TD_PARAM_S avftr_td = { { 0 } };

	/* enable TD */
	if (td->enabled == 1) {
		if (0 != AVFTR_TD_regCallback(win_idx, callbackTd)) {
			avmain2_log_err("Fail to regist TD callback.\r\n");
			return -EACCES;
		}

		if (0 != AVFTR_TD_enable(win_idx)) {
			avmain2_log_err("Fail to enable TD window: %x.\r\n", win_idx.value);
			return -EACCES;
		}

		if (0 != AVFTR_TD_regMpiInfo(win_idx)) {
			avmain2_log_err("Fail to regist TD mpi info.\r\n");
			return -EACCES;
		}

		if (0 != AVFTR_TD_getParam(win_idx, &avftr_td)) {
			avmain2_log_err("Fail to set TD parameter on window: %x.\r\n", win_idx.value);
			return -EACCES;
		}

		avftr_td.td_param.en_block_det = td->en_block_det;
		avftr_td.td_param.en_redirect_det = td->en_redirect_det;

		avftr_td.td_param.endurance = td->endurance;
		/*Mapping to useful sensitivity range*/
		sen_min_map = 16;
		sen_max_map = 216;
		temp = ((td->sensitivity * (sen_max_map - sen_min_map) / 100) + sen_min_map);
		avftr_td.td_param.sensitivity = (temp > VFTR_TD_SENSITIVITY_MAX) ? VFTR_TD_SENSITIVITY_MAX : temp;

		avftr_td.td_param.redirect_sensitivity = td->redirect_sensitivity * 255 / 100;
		avftr_td.td_param.redirect_global_change = td->redirect_global_change * 255 / 100;
		avftr_td.td_param.redirect_trigger_delay = td->redirect_trigger_delay;

		if (0 != AVFTR_TD_setParam(win_idx, &avftr_td)) {
			avmain2_log_err("Fail to set TD parameter on window %x.\r\n", win_idx.value);
			return -EACCES;
		}

		avmain2_log_debug("Set tamper detection(TD) on window %x success.\r\n", win_idx.value);
	} else if (td->enabled == 0) {
		if (AVFTR_TD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable tamper detection(TD).\r\n");
			return -EINVAL;
		}
		if (AVFTR_TD_releaseMpiInfo(win_idx) < 0) {
			avmain2_log_err("Fail to release TD mpi info.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disalbe to set tamper detection(TD) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set shaking object detection(SHD).
 */
static int setShd(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_SHD_CONF_S *shd)
{
	int i;
	AVFTR_SHD_PARAM_S avftr_shd = { 0 };
	AVFTR_SHD_LONGTERM_LIST_S lt_list = { 0 };

	/* enable SHD */
	if (shd->enabled == 1) {
		if (AVFTR_SHD_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable shaking object detection(SHD).\r\n");
			return -EINVAL;
		}

		if (AVFTR_SHD_getParam(win_idx, &avftr_shd) != 0) {
			avmain2_log_err("Fail to get shaking object detection(SHD).\r\n");
			return -EINVAL;
		}

		avftr_shd.en = shd->enabled;
		avftr_shd.sensitivity = shd->sensitivity;
		avftr_shd.quality = shd->quality;
		avftr_shd.obj_life_th = shd->obj_life_th;
		avftr_shd.longterm_life_th = shd->longterm_life_th;
		avftr_shd.instance_duration = shd->instance_duration;
		avftr_shd.shaking_update_duration = shd->shaking_update_duration;
		avftr_shd.longterm_dec_period = shd->longterm_dec_period;

		if (AVFTR_SHD_setParam(win_idx, &avftr_shd) != 0) {
			avmain2_log_err("Fail to set shaking object detection(SHD).\r\n");
			return -EINVAL;
		}

		if (AVFTR_SHD_getUsrList(win_idx, &lt_list) != 0) {
			avmain2_log_err("Fail to get user list(LT).\r\n");
			return -EINVAL;
		}

		lt_list.num = shd->longterm_num;
		for (i = 0; i < lt_list.num; i++) {
			lt_list.item[i].rgn.sx = shd->longterm_list[i].start_x * window->width / 100;
			lt_list.item[i].rgn.sy = shd->longterm_list[i].start_y * window->height / 100;
			lt_list.item[i].rgn.ex = shd->longterm_list[i].end_x * window->width / 100;
			lt_list.item[i].rgn.ey = shd->longterm_list[i].end_y * window->height / 100;
		}

		if (AVFTR_SHD_setUsrList(win_idx, &lt_list) != 0) {
			avmain2_log_err("Fail to set user list(LT).\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Set shaking object detection(SHD) on window %x success.\r\n", win_idx.value);
	} else if (shd->enabled == 0) {
		if (AVFTR_SHD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable shaking object detection(SHD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disalbe to set shaking object detection(SHD) on window %x.\r\n", win_idx.value);
	}
	/*event daemon maybe restart in runtime, so don't force to connect it*/
	if (connectEventDaemon() != 0) {
		avmain2_log_warn("failed to connect event daemon\n");
	}

	return 0;
}

/**
 * @brief Set regional motion sensor(RMS).
 */
static int setRms(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_RMS_CONF_S *rms)
{
	AGTX_UNUSED(window);

	VIDEO_FTR_RMS_PARAM_S vftr_rms = { 0 };

	/* enable RMS */
	if (rms->enabled == 1) {
		if (VIDEO_FTR_enableRms(win_idx) != 0) {
			avmain2_log_err("Fail to enable regional motion sensor(RMS).\r\n");
			return -EINVAL;
		}

		if (VIDEO_FTR_getRmsParam(win_idx, &vftr_rms) != 0) {
			avmain2_log_err("Fail to get regional motion sensor(RMS).\r\n");
			return -EINVAL;
		}

		vftr_rms.sen = rms->sensitivity * MPI_IVA_RMS_MAX_SEN / 100;
		vftr_rms.split_x = rms->split_x;
		vftr_rms.split_y = rms->split_y;

		if (VIDEO_FTR_setRmsParam(win_idx, &vftr_rms) != 0) {
			avmain2_log_err("Fail to set regional motion sensor(RMS).\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Set regional motion sensor(RMS) on window %x success.\r\n", win_idx.value);
	} else if (rms->enabled == 0) {
		if (VIDEO_FTR_disableRms(win_idx) != 0) {
			avmain2_log_err("Fail to disable regional motion sensor(RMS).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disalbe to set regional motion sensor(RMS) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set Pan-tile-zoom(PTZ).
 */
static int setPtz(MPI_DEV dev_idx, MPI_RECT_S *window, const AGTX_VIDEO_PTZ_CONF_S *ptz)
{
	AGTX_UNUSED(window);

	int i;
	VIDEO_FTR_PTZ_PARAM_S vftr_ptz = { 0 };

	if (ptz->enabled == 1) {
		if (VIDEO_FTR_enablePtz() != 0) {
			avmain2_log_err("Fail to enable Pan-tile-zoom(PTZ).\r\n");
			return -EINVAL;
		}

		if (VIDEO_FTR_getPtzParam(&vftr_ptz) != 0) {
			avmain2_log_err("Fail to get Pan-tile-zoom(PTZ).\r\n");
			return -EINVAL;
		}

		for (i = 0; i < ptz->subwindow_disp.win_num; i++) {
			int ptz_win = ptz->subwindow_disp.win[i].win_idx;
			int ptz_chn = ptz->subwindow_disp.win[i].chn_idx;
			if ((!g_conf.layout.layout_en || g_conf.layout.layout_num <= ptz_win)) {
				avmain2_log_err("Cannot enable ptz with WIN:%x , CHN:%x not enabled\n", ptz_win,
				                ptz_chn);
				return -EINVAL;
			}
		}

		vftr_ptz.win_num = ptz->subwindow_disp.win_num;
		for (i = 0; i < ptz->subwindow_disp.win_num; i++) {
			vftr_ptz.win_id[i] = MPI_VIDEO_WIN(dev_idx.dev, ptz->subwindow_disp.win[i].chn_idx,
			                                   ptz->subwindow_disp.win[i].win_idx);
		}

		vftr_ptz.win_size_limit.min = ptz->win_size_limit_min;
		vftr_ptz.win_size_limit.max = ptz->win_size_limit_max;
		vftr_ptz.mv.x = ptz->win_speed_x;
		vftr_ptz.mv.y = ptz->win_speed_y;
		vftr_ptz.zoom_v.x = ptz->zoom_speed_width;
		vftr_ptz.zoom_v.y = ptz->zoom_speed_height;
		vftr_ptz.mv_pos.x = ptz->win_pos_x;
		vftr_ptz.mv_pos.y = ptz->win_pos_y;
		vftr_ptz.speed.x = ptz->speed_x;
		vftr_ptz.speed.y = ptz->speed_y;
		vftr_ptz.zoom_lvl = ptz->zoom_level;
		vftr_ptz.zoom_change = ptz->zoom_change;

		switch (ptz->mode) {
		case AGTX_VIDEO_PTZ_MODE_AUTO:
			if (AVFTR_AROI_regCallback(g_aroi_cb_info.idx, callbackAroiInfo) != 0) {
				avmain2_log_err("Fail to regist callback.\r\n");
				return -EINVAL;
			}
			vftr_ptz.mode = VIDEO_FTR_PTZ_MODE_AUTO;
			break;
		case AGTX_VIDEO_PTZ_MODE_MANUAL:
			if (AVFTR_AROI_regCallback(g_aroi_cb_info.idx, callbackAroiEmpty) != 0) {
				avmain2_log_err("Fail to regist callback.\r\n");
				return -EINVAL;
			}
			vftr_ptz.mode = VIDEO_FTR_PTZ_MODE_MANUAL;
			break;
		case AGTX_VIDEO_PTZ_MODE_SCAN:
			if (AVFTR_AROI_regCallback(g_aroi_cb_info.idx, callbackAroiEmpty) != 0) {
				avmain2_log_err("Fail to regist callback.\r\n");
				return -EINVAL;
			}
			vftr_ptz.mode = VIDEO_FTR_PTZ_MODE_SCAN;
			vftr_ptz.roi_bd.width = ptz->roi_width;
			vftr_ptz.roi_bd.height = ptz->roi_height;
			break;
		default:
			if (AVFTR_AROI_regCallback(g_aroi_cb_info.idx, callbackAroiEmpty) != 0) {
				avmain2_log_err("Fail to regist callback.\r\n");
				return -EINVAL;
			}
			vftr_ptz.mode = VIDEO_FTR_PTZ_MODE_MANUAL;
			break;
		}

		if (VIDEO_FTR_setPtzParam(&vftr_ptz) < 0) {
			avmain2_log_err("Fail to set ptz.\r\n");
			return -EINVAL;
		} else {
			// do nothing
		}

		avmain2_log_debug("Set Pan-tile-zoom(PTZ) for dev %x success.\r\n", dev_idx.value);
	} else if (ptz->enabled == 0) {
		if (VIDEO_FTR_disablePtz() != 0) {
			avmain2_log_err("Fail to disable Pan-tile-zoom(PTZ).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set Pan-tile-zoom(PTZ) on dev_index %x.\r\n", dev_idx.value);
	}

	return 0;
}

/**
 * @brief Set pet feeding monitor (PFM).
 */
static int setPfm(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_PFM_CONF_S *pfm)
{
	const int fps = g_conf.strm.video_strm->output_fps;
	int sen_min_map = 0;
	int sen_max_map = 0;
	int temp = 0;
	int i;
	int start_x, start_y, end_x, end_y;
	AVFTR_PFM_PARAM_S vftr_pfm = {
		.pfm_param = {
			.sensitivity = 0,
			.endurance = 0,
			.roi = { .sx = 0, .sy = 0, .ex = 0, .ey = 0 },
			.window = { .x = 0, .y = 0, .width = 0, .height = 0 },
		},
		.schedule = { 0 },
	};
	VFTR_PFM_PARAM_S *pfm_param = &vftr_pfm.pfm_param;
	AVFTR_PFM_SCHEDULE_S *schedule = &vftr_pfm.schedule;
	MPI_CHN chn = MPI_VIDEO_CHN(0, 0);
	MPI_CHN_LAYOUT_S layout_attr = { 0 };

	if (pfm->enabled == 1) {
		if (AVFTR_PFM_getParam(win_idx, &vftr_pfm) != 0) {
			avmain2_log_err("Fail to get PFM parameters.\r\n");
			return -EINVAL;
		}

		if (MPI_DEV_getChnLayout(chn, &layout_attr) != MPI_SUCCESS) {
			return -EINVAL;
		}

		for (i = 0; i < layout_attr.window_num; i++) {
			if (layout_attr.win_id[i].value == win_idx.value) {
				break;
			}
		}

		if (i == layout_attr.window_num) {
			return -EINVAL;
		}

		// parameters mapping from AGTX schema to AVFTR schema
		sen_min_map = 16;
		sen_max_map = 216;
		temp = ((pfm->sensitivity * (sen_max_map - sen_min_map) / 100) + sen_min_map);
		pfm_param->sensitivity = (temp > VFTR_PFM_SENSITIVITY_MAX) ? VFTR_PFM_SENSITIVITY_MAX : temp;

		pfm_param->endurance = pfm->endurance * fps / 200; // adapt endurance unit to second

		start_x = MIN(pfm->roi.start_x, 99);
		start_y = MIN(pfm->roi.start_y, 99);
		end_x = MAX(pfm->roi.end_x, 1);
		end_y = MAX(pfm->roi.end_y, 1);
		pfm_param->roi.sx = roundingDownAlign16(window->width * start_x / 100);
		pfm_param->roi.sy = roundingDownAlign16(window->height * start_y / 100);
		pfm_param->roi.ex = roundingDownAlign16(window->width * end_x / 100) - 1;
		pfm_param->roi.ey = roundingDownAlign16(window->height * end_y / 100) - 1;

		pfm_param->window.width = window->width;
		pfm_param->window.height = window->height;

		schedule->time_num = pfm->time_number;
		schedule->regisBg_feed_interval = pfm->regis_to_feeding_interval;
		for (i = 0; i < schedule->time_num; i++) {
			schedule->times[i] = pfm->schedule[i];
		}

		if (AVFTR_PFM_setParam(win_idx, &vftr_pfm) != 0) {
			avmain2_log_err("Fail to set pet feeding monitor param.\r\n");
			return -EACCES;
		}

		if (AVFTR_PFM_regMpiInfo(win_idx) != 0) {
			avmain2_log_err("Fail to register PFM mpi info.\r\n");
			return -EACCES;
		}

		if (AVFTR_PFM_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable PFM.\r\n");
			return -EACCES;
		}

		avmain2_log_debug("Set PFM on window %x success.\r\n", win_idx.value);
	} else {
		if (AVFTR_PFM_disable(win_idx)) {
			avmain2_log_err("Fail to disable pet feeding monitor(PFM).\r\n");
			return -EINVAL;
		}
		if (AVFTR_PFM_releaseMpiInfo(win_idx)) {
			avmain2_log_err("Fail to release PFM mpi info.\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Disable PFM on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set pedestrian detection(PD).
 */
static int setPd(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_PD_CONF_S *pd)
{
	int length = 0;
	VFTR_OSC_PARAM_S vftr_pd = {
		.min_sz = { .width = 0, .height = 0 },
		.max_sz = { .width = 0, .height = 0 },
		.min_aspect_ratio = 0,
		.max_aspect_ratio = 0,
		.obj_life_th = 0,
	};

	/* enable OD */
	if (pd->enabled == 1) {
		if (AVFTR_PD_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable PD.\r\n");
			return -EINVAL;
		}

		if (AVFTR_PD_getParam(win_idx, &vftr_pd) != 0) {
			avmain2_log_err("Fail to get PD.\r\n");
			return -EINVAL;
		}

		length = (window->width > window->height) ? window->height : window->width;

		vftr_pd.max_aspect_ratio =
		        ((pd->max_aspect_ratio_w << VFTR_OSC_AR_FRACTIONAL_BIT) + (pd->max_aspect_ratio_h >> 1)) /
		        pd->max_aspect_ratio_h;
		vftr_pd.min_aspect_ratio =
		        ((pd->min_aspect_ratio_w << VFTR_OSC_AR_FRACTIONAL_BIT) + (pd->min_aspect_ratio_h >> 1)) /
		        pd->min_aspect_ratio_h;
		vftr_pd.min_sz.width = pd->min_size * length / 100;
		vftr_pd.min_sz.height = pd->min_size * length / 100;
		vftr_pd.max_sz.width = pd->max_size * length / 100;
		vftr_pd.max_sz.height = pd->max_size * length / 100;
		vftr_pd.obj_life_th = pd->obj_life_th;

		if (AVFTR_PD_setParam(win_idx, &vftr_pd) != 0) {
			avmain2_log_err("Fail to set PD.\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Set pedestrian detection(PD) on window %x success.\r\n", win_idx.value);
	} else if (pd->enabled == 0) {
		if (AVFTR_PD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable pedestrian detection(PD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set pedestrian detection(PD) on window %x.\r\n", win_idx.value);
	}
	return 0;
}

/**
 * @brief Set IVA object detection(OD).
 */
static int setOd(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_OD_CONF_S *od)
{
	AGTX_UNUSED(window);
	UINT8 fps_ptr = g_conf.strm.video_strm->output_fps; /* NOTICE:Don't change value*/
	VIDEO_FTR_OD_PARAM_S vftr_od = { 0 };

	/* enable OD */
	if (od->enabled == 1) {
		if (VIDEO_FTR_enableOd(win_idx) != 0) {
			avmain2_log_err("Fail to enable OD.\r\n");
			return -EINVAL;
		}

		if (VIDEO_FTR_getOdParam(win_idx, &vftr_od) != 0) {
			avmain2_log_err("Fail to get OD.\r\n");
			return -EINVAL;
		}

#define VFTR_OD_MAX_DETECTION_SEC 3
#define VFTR_OD_MIN_DETECTION_SEC 0
#define VFTR_OD_DETECTION_SECOND 0.9 //Recommend Default
#define VFTR_OD_DETECTION_PERCENT 70 //Recommend Default

#define VFTR_OD_MAX_TRACK_REFINE_SEC 2
#define VFTR_OD_MIN_TRACK_REFINE_SEC 0
#define VFTR_OD_TRACK_REFINE_SECOND 0.267 //Recommend Default
#define VFTR_OD_TRACK_REFINE_PERCENT 86 //Recommend Default

		//1-VFTR_OD_TRACK_REFINE_SECOND/(VFTR_OD_MAX_TRACK_REFINE_SEC-VFTR_OD_MIN_TRACK_REFINE_SEC)
		int od_qual_t =
		        MPI_IVA_OD_MAX_QUA + 1 -
		        (fps_ptr * (100 - od->od_qual) * (VFTR_OD_MAX_DETECTION_SEC - VFTR_OD_MIN_DETECTION_SEC)) / 100;

		if (od_qual_t < MPI_IVA_OD_MIN_QUA) {
			od_qual_t = MPI_IVA_OD_MIN_QUA;
		} else if (od_qual_t > MPI_IVA_OD_MAX_QUA) {
			od_qual_t = MPI_IVA_OD_MAX_QUA;
		}

		int od_track_refine_t = MPI_IVA_OD_MAX_TRACK_REFINE + 1 -
		                        (fps_ptr * (100 - od->od_track_refine) *
		                         (VFTR_OD_MAX_TRACK_REFINE_SEC - VFTR_OD_MIN_TRACK_REFINE_SEC)) /
		                                100;

		if (od_track_refine_t < MPI_IVA_OD_MIN_TRACK_REFINE) {
			od_track_refine_t = MPI_IVA_OD_MIN_TRACK_REFINE;
		} else if (od_track_refine_t > MPI_IVA_OD_MAX_TRACK_REFINE) {
			od_track_refine_t = MPI_IVA_OD_MAX_TRACK_REFINE;
		}

		vftr_od.od_param.od_qual = od_qual_t;
		vftr_od.od_param.od_track_refine = od_track_refine_t;
		vftr_od.od_param.od_size_th = od->od_size_th * MPI_IVA_OD_MAX_OBJ_SIZE / 100;
		vftr_od.od_param.od_sen = od->od_sen * MPI_IVA_OD_MAX_SEN / 100;
		vftr_od.od_param.en_stop_det = od->en_stop_det;
		vftr_od.od_param.en_gmv_det = od->en_gmv_det;
		// Add 5 new OD parameters
		vftr_od.od_param.od_conf_th = od->od_conf_th;
		vftr_od.od_param.od_iou_th = od->od_iou_th;
		vftr_od.od_param.od_snapshot_w = od->od_snapshot_w;
		vftr_od.od_param.od_snapshot_h = od->od_snapshot_h;
		vftr_od.od_param.od_snapshot_type = od->od_snapshot_type;

		vftr_od.od_motor_param.en_motor = od->en_motor;
		vftr_od.en_crop_outside_obj = od->en_crop_outside_obj;

		if (VIDEO_FTR_setOdParam(win_idx, &vftr_od) != 0) {
			avmain2_log_err("Fail to set object detection(OD).\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Set object detection(OD) on window %x success.\r\n", win_idx.value);
	} else if (od->enabled == 0) {
		if (VIDEO_FTR_disableOd(win_idx) != 0) {
			avmain2_log_err("Fail to disable object detection(OD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set object detection(OD) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set motion detection(MD).
 */
static int setMd(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_MD_CONF_S *md)
{
	AVFTR_MD_PARAM_S avftr_md = { 0 };
	int ret;

	/* enable MD */
	if (md->enabled == 1) {
		if (AVFTR_MD_regCallback(win_idx, callbackMd) != 0) {
			avmain2_log_err("Fail to regist MD callback.\r\n");
			return -EINVAL;
		}

		if (AVFTR_MD_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable MD.\r\n");
			return -EINVAL;
		}

		if (AVFTR_MD_getParam(win_idx, &avftr_md) != 0) {
			avmain2_log_err("Fail to get MD param.\r\n");
			return -EINVAL;
		}

		/* AVFTR_VIDEO_JIF_HZ (unit: 10ms) */
		avftr_md.duration = md->alarm_buffer;
		avftr_md.en_skip_shake = md->en_skip_shake;
		avftr_md.en_skip_pd = md->en_skip_pd;
		avftr_md.md_param.region_num = md->rgn_cnt;
		if (md->rgn_cnt > VFTR_MD_MAX_REG_NUM) {
			avmain2_log_err(" MD region num %d > max num: %d.\r\n", md->rgn_cnt, VFTR_MD_MAX_REG_NUM);
			return -EINVAL;
		}
		/* Add new MD region */
		for (int i = 0; i < md->rgn_cnt; i++) {
			/* Convert from percent to actually point */
			avftr_md.md_param.attr[i].pts.sx = (window->width * md->rgn_list[i].sx / 100);
			avftr_md.md_param.attr[i].pts.sy = (window->height * md->rgn_list[i].sy / 100);
			avftr_md.md_param.attr[i].pts.ex = (window->width * md->rgn_list[i].ex / 100);
			avftr_md.md_param.attr[i].pts.ey = (window->height * md->rgn_list[i].ey / 100);
			avftr_md.md_param.attr[i].thr_v_obj_max = md->rgn_list[i].max_spd;
			avftr_md.md_param.attr[i].thr_v_obj_min = md->rgn_list[i].min_spd;
			avftr_md.md_param.attr[i].obj_life_th = md->rgn_list[i].obj_life_th;
			avftr_md.md_param.attr[i].det_method = (md->rgn_list[i].det == AGTX_IVA_MD_DET_NORMAL) ?
			                                               VFTR_MD_DET_NORMAL :
			                                               VFTR_MD_DET_SUBTRACT;
			avftr_md.md_param.attr[i].md_mode = (md->rgn_list[i].mode == AGTX_IVA_MD_MODE_AREA) ?
			                                            VFTR_MD_MOVING_AREA :
			                                            (md->rgn_list[i].mode == AGTX_IVA_MD_MODE_ENERGY) ?
			                                            VFTR_MD_MOVING_ENERGY :
			                                            VFTR_MD_MOVING_AREA;
			if (md->rgn_list[i].mode == AGTX_IVA_MD_MODE_AREA) {
				avftr_md.md_param.attr[i].thr_v_reg =
				        ((window->width * window->height / 100) * (100 - md->rgn_list[i].sens)) / 1000;
			} else {
				avftr_md.md_param.attr[i].thr_v_reg =
				        (((((window->width * window->height) / 100) * md->rgn_list[i].max_spd) /
				          MD_ENERGY_TH_V_REF_RATIO) *
				         (100 - md->rgn_list[i].sens)) /
				        1000;
			}
		}

		ret = AVFTR_MD_setParam(win_idx, &avftr_md);
		if (ret != 0) {
			avmain2_log_err("Fail to set MD param. err: %d\r\n", ret);
			return -EINVAL;
		}

		avmain2_log_debug("Set motion detection(MD) on window %x success.\r\n", win_idx.value);
	} else if (md->enabled == 0) {
		if (AVFTR_MD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable motion detection(MD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set motion detection(MD) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set loud sound detection(LSD).
 */
static int setLsd(MPI_DEV dev_idx, const AGTX_IAA_LSD_CONF_S *lsd)
{
	AFTR_SD_PARAM_S aftr_lsd = { 0 };

	/* enable SD */
	if (lsd->enabled == 1) {
		if (AVFTR_SD_regCallback(dev_idx, callbackLsd) != 0) {
			avmain2_log_err("Fail to regist LSD callback.\r\n");
			return -EACCES;
		}

		if (AVFTR_SD_enable(dev_idx) != 0) {
			avmain2_log_err("Fail to enable SD.\r\n");
			return -EINVAL;
		}

		if (AVFTR_SD_getParam(dev_idx, &aftr_lsd) != 0) {
			avmain2_log_err("Fail to get SD param.\r\n");
			return -EINVAL;
		}

		aftr_lsd.duration = lsd->duration;
		aftr_lsd.suppression = lsd->suppression;
		aftr_lsd.volume = lsd->volume;

		if (AVFTR_SD_setParam(dev_idx, &aftr_lsd) != 0) {
			avmain2_log_err("Fail to set SD param.\r\n");
			return -EINVAL;
		}

		avmain2_log_debug("Set loud sound detection(LSD).\r\n");
	} else if (lsd->enabled == 0) {
		if (AVFTR_SD_disable(dev_idx) != 0) {
			avmain2_log_err("Fail to disable loud sound detection(LSD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set loud sound detection(LSD) on dev_idx %x.\r\n", dev_idx.value);
	}

	return 0;
}

/**
 * @brief Set light detection(LD).
 */
static int setLd(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_LD_CONF_S *ld)
{
	VFTR_LD_PARAM_S vftr_ld = { 0 };

	/* enable LD */
	if (ld->enabled == 1) {
		if (AVFTR_LD_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable light detection(LD).\r\n");
			return -EINVAL;
		}

		MPI_RECT_S rect;
		if (getRoi(win_idx, &rect) != 0) {
			avmain2_log_err("Fail to get region of interest(ROI).\r\n");
			return -EINVAL;
		}
		MPI_RECT_POINT_S roi = {
			.sx = rect.x, .sy = rect.y, .ex = (rect.x + rect.width - 1), .ey = (rect.y + rect.height - 1)
		};

		if (AVFTR_LD_regMpiInfo(win_idx, &roi) != 0) {
			avmain2_log_err("Fail to regist LD info.\r\n");
			return -EINVAL;
		}

		if (AVFTR_LD_getParam(win_idx, &vftr_ld) != 0) {
			avmain2_log_err("Fail to get LD parameter.\r\n");
			return -EINVAL;
		}

		/* sen_th             : min=12 mid=62 max=112 */
		/* alarm_latency      : fixed=5               */
		/* alarm_latency_cycle: min=1  mid=3  max=5   */
		/* det_period         : min=2  mid=6  max=10  */

		vftr_ld.sen_th = 112 - ld->sensitivity;

		vftr_ld.alarm_supr = 7 - ((ld->sensitivity + 10) / 20);
		vftr_ld.alarm_latency = 5;
		vftr_ld.alarm_latency_cycle = ((ld->sensitivity + 12) / 25) + 1;
		vftr_ld.det_period = (10 - (ld->sensitivity * 8 / 100));
		vftr_ld.trig_cond = ld->trigger_cond;

		vftr_ld.roi.sx = ld->det_region.start_x * (window->width - 1) / 100;
		vftr_ld.roi.sy = ld->det_region.start_y * (window->height - 1) / 100;
		vftr_ld.roi.ex = ld->det_region.end_x * (window->width - 1) / 100;
		vftr_ld.roi.ey = ld->det_region.end_y * (window->height - 1) / 100;

		if (AVFTR_LD_setParam(win_idx, &vftr_ld) != 0) {
			avmain2_log_err("Fail to set LD parameter.\r\n");
			return -EINVAL;
		}

		if (AVFTR_LD_regMpiInfo(win_idx, &vftr_ld.roi) != 0) {
			avmain2_log_err("Fail to regist LD info.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Set light detection(LD) on window %x success.\r\n", win_idx.value);
	} else if (ld->enabled == 0) {
		if (AVFTR_LD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable light detection(LD).\r\n");
			return -EINVAL;
		}
		if (AVFTR_LD_releaseMpiInfo(win_idx) < 0) {
			avmain2_log_err("Fail to release LD mpi info.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set light detection(LD) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set fall detection(FLD).
 */
static int setFld(MPI_WIN win_idx, const AGTX_IVA_FLD_CONF_S *fld)
{
	AVFTR_FLD_PARAM_S avftr_fld = { { 0 } };
	/* enable fld */
	if (fld->enabled == 1) {
		if (AVFTR_FLD_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable fall detection(FLD).\r\n");
			return -EINVAL;
		}

		if (AVFTR_FLD_getParam(win_idx, &avftr_fld) != 0) {
			avmain2_log_err("Fail to get DK parameter.\r\n");
			return -EINVAL;
		}

		/*Avoid invalid value*/
		VFTR_FLD_PARAM_S *vftr_fld = &avftr_fld.fld_param;
		vftr_fld->obj_life_th = MAX(fld->obj_life_th, 0);
		vftr_fld->obj_falling_mv_th = MAX(fld->obj_falling_mv_th, 0);
		vftr_fld->obj_stop_mv_th = MAX(fld->obj_stop_mv_th, 0);
		vftr_fld->falling_period_th = MAX(fld->falling_period_th, 10);
		vftr_fld->down_period_th = MAX(fld->down_period_th, 10);
		vftr_fld->fallen_period_th = MAX(fld->fallen_period_th, 10);
		vftr_fld->demo_level = MAX(fld->demo_level, 0);

		vftr_fld->obj_high_ratio_th = (MAX(fld->obj_high_ratio_th, 0) << VFTR_FLD_FRACTION) / 100;

		if (AVFTR_FLD_setParam(win_idx, &avftr_fld) != 0) {
			avmain2_log_err("Fail to set FLD parameter.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Set fall detection(FLD) on window %x success.\r\n", win_idx.value);
	} else if (fld->enabled == 0) {
		if (AVFTR_FLD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable fall detection(FLD).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set fall detection(FLD) on win_idx %x.\r\n", win_idx.value);
	}
	return 0;
}

/**
 * @brief Set electric fence(EF).
 */
static int setEf(MPI_WIN win_idx, MPI_RECT_S *window, AGTX_IVA_EF_CONF_S *ef)
{
	int i = 0;
	AVFTR_EF_VL_ATTR_S ef_line = { 0 };
	VFTR_EF_PARAM_S vftr_ef = { 0 };

	/* enable ef */
	if (ef->enabled == 1) {
		if (AVFTR_EF_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable electric fence(EF).\r\n");
			return -EINVAL;
		}
		/* get old ef line */
		if (AVFTR_EF_getParam(win_idx, &vftr_ef) != 0) {
			avmain2_log_err("Fail to enable electric fence(EF).\r\n");
			return -EINVAL;
		}

		/* Clean old ef line */
		for (i = 0; i < vftr_ef.fence_num; i++) {
			if (AVFTR_EF_rmVl(win_idx, vftr_ef.attr[i].id) != 0) {
				avmain2_log_err("Fail to remove electric fence(EF) id.\r\n");
				return -EINVAL;
			}
		}

		/* new ef line */
		for (i = 0; i < ef->line_cnt; i++) {
			/* Convert from percent to actually point */
			ef_line.id = ef->line_list[i].id;
			ef_line.line.sx =
			        MIN(MAX((window->width - 1) * ef->line_list[i].start_x / 100, 0), window->width - 1);
			ef_line.line.sy =
			        MIN(MAX((window->height - 1) * ef->line_list[i].start_y / 100, 0), window->height - 1);
			ef_line.line.ex =
			        MIN(MAX((window->width - 1) * ef->line_list[i].end_x / 100, 0), window->width - 1);
			ef_line.line.ey =
			        MIN(MAX((window->height - 1) * ef->line_list[i].end_y / 100, 0), window->height - 1);
			ef_line.obj_size_min.width = window->width * ef->line_list[i].obj_min_w / 100;
			ef_line.obj_size_min.height = window->height * ef->line_list[i].obj_min_h / 100;
			ef_line.obj_size_max.width = window->width * ef->line_list[i].obj_max_w / 100;
			ef_line.obj_size_max.height = window->height * ef->line_list[i].obj_max_h / 100;
			ef_line.obj_area = ef->line_list[i].obj_area * window->width * window->height / 100;
			ef_line.obj_v_th = ef->line_list[i].obj_v_th * VFTR_EF_MAX_THR_V_OBJ / 100;
			ef_line.obj_life_th = ef->line_list[i].obj_life_th;
			switch (ef->line_list[i].mode) {
			case AGTX_IVA_EF_MODE_DIR_NONE:
				ef_line.mode = VFTR_EF_DIR_NONE;
				break;
			case AGTX_IVA_EF_MODE_DIR_POS:
				ef_line.mode = VFTR_EF_DIR_POS;
				break;
			case AGTX_IVA_EF_MODE_DIR_NEG:
				ef_line.mode = VFTR_EF_DIR_NEG;
				break;
			case AGTX_IVA_EF_MODE_DIR_BOTH:
				ef_line.mode = VFTR_EF_DIR_BOTH;
				break;
			default:
				ef_line.mode = VFTR_EF_DIR_NONE;
				printf("Unkown IVA EF mode\n");
				break;
			}
			if (AVFTR_EF_addVl(win_idx, &ef_line) != 0) {
				avmain2_log_err("Fail to set electric fence(EF).\r\n");
				return -EINVAL;
			}
			ef->line_list[i].id = ef_line.id;
		}

		avmain2_log_debug("Set electric fence(EF) on window %x success.\r\n", win_idx.value);

		if (AVFTR_EF_checkParam(win_idx) != 0) {
			avmain2_log_err("Fail to check electric fence(EF) parameter.\r\n");
			return -EINVAL;
		}

	} else if (ef->enabled == 0) {
		if (AVFTR_EF_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable electric fence(EF).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set electric fence(EF) on window %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set edge AI assisted feature(EAIF).
 */
static int setEaif(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_EAIF_CONF_S *eaif)
{
	AGTX_UNUSED(window);

	AVFTR_EAIF_PARAM_S avftr_eaif = { 0 };
	const char *avmain_eaif_face_file_dir = EAIF_FACE_DIR_NAME;
	const char *avmain_eaif_face_face_db = EAIF_FACE_BIN_NAME;
	int tar_dev = (eaif->target_idx) & 0xff;
	int tar_chn = (eaif->target_idx >> 8) & 0xff;
	int tar_win = (eaif->target_idx >> 16) & 0xff;

	/* enable eaif */
	if (eaif->enabled == 1) {
		if (AVFTR_EAIF_getParam(win_idx, &avftr_eaif) != 0) {
			avmain2_log_err("Fail to get edge AI assisted feature(EAIF) parameter.\r\n");
			return -EINVAL;
		}
		avftr_eaif.target_idx = MPI_VIDEO_WIN(tar_dev, tar_chn, tar_win);
		avftr_eaif.api = eaif->api;
		avftr_eaif.data_fmt = eaif->data_fmt;
		avftr_eaif.obj_life_th = eaif->obj_life_th;
		avftr_eaif.detection_period = eaif->detection_period;
		avftr_eaif.identification_period = eaif->identification_period;
		avftr_eaif.min_size = eaif->min_size;
		avftr_eaif.inf_with_obj_list = eaif->inf_with_obj_list;
		avftr_eaif.pos_stop_count_th = eaif->pos_stop_count_th;
		avftr_eaif.pos_classify_period = eaif->pos_classify_period;
		avftr_eaif.neg_classify_period = eaif->neg_classify_period;
		avftr_eaif.obj_exist_classify_period = eaif->obj_exist_classify_period;
		avftr_eaif.snapshot_width = MAX(eaif->snapshot_width, 0);
		avftr_eaif.snapshot_height = MAX(eaif->snapshot_height, 0);
		avftr_eaif.inf_utils.cmd = eaif->inf_cmd;
		avftr_eaif.inf_utils.roi.sx = eaif->facereco_roi_sx;
		avftr_eaif.inf_utils.roi.sy = eaif->facereco_roi_sy;
		avftr_eaif.inf_utils.roi.ex = eaif->facereco_roi_ex;
		avftr_eaif.inf_utils.roi.ey = eaif->facereco_roi_ey;
		strncpy(avftr_eaif.inf_utils.dir, (const char *)avmain_eaif_face_file_dir, EAIF_URL_CHAR_LEN);
		strncpy(avftr_eaif.inf_utils.face_db, (const char *)avmain_eaif_face_face_db, EAIF_CHAR_LEN);
		strncpy(avftr_eaif.inf_utils.face_name, (const char *)eaif->face_name, EAIF_CHAR_LEN);
		strncpy(avftr_eaif.url, (const char *)eaif->url, EAIF_URL_CHAR_LEN);
		strncpy(avftr_eaif.classify_model, (const char *)eaif->classify_model, EAIF_MODEL_LEN);
		strncpy(avftr_eaif.classify_cv_model, (const char *)eaif->classify_cv_model, EAIF_MODEL_LEN);
		strncpy(avftr_eaif.detect_model, (const char *)eaif->detect_model, EAIF_MODEL_LEN);
		strncpy(avftr_eaif.face_reco_model, (const char *)eaif->face_reco_model, EAIF_MODEL_LEN);
		strncpy(avftr_eaif.face_detect_model, (const char *)eaif->face_detect_model, EAIF_MODEL_LEN);
		strncpy(avftr_eaif.human_classify_model, (const char *)eaif->human_classify_model, EAIF_MODEL_LEN);

		if (AVFTR_EAIF_setParam(win_idx, &avftr_eaif) != 0) {
			avmain2_log_err("Fail to set edge AI assisted feature(EAIF) parameter.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Set edge AI assisted feature(EAIF) on window %x success.\r\n", win_idx.value);

		if (AVFTR_EAIF_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable edge AI assisted feature(EAIF).\r\n");
			return -EINVAL;
		}

	} else if (eaif->enabled == 0) {
		if (AVFTR_EAIF_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable edge AI assisted feature(EAIF).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set edge AI assisted feature(EAIF) on window %x.\r\n", win_idx.value);
	}
	return 0;
}

/**
 * @brief Set door keeper(DK).
 */
static int setDk(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_DK_CONF_S *dk)
{
	int start_x, start_y, end_x, end_y;
	AVFTR_DK_PARAM_S avftr_dk = { { 0 } };

	/* enable dk */
	if (dk->enabled == 1) {
		if (AVFTR_DK_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable DK.\r\n");
			return -EINVAL;
		}

		if (AVFTR_DK_getParam(win_idx, &avftr_dk) != 0) {
			avmain2_log_err("Fail to get DK parameter.\r\n");
			return -EINVAL;
		}

		/* Avoid invalid value */
		VFTR_DK_PARAM_S *vftr_dk = &avftr_dk.dk_param;
		vftr_dk->obj_life_th = MAX(dk->obj_life_th, 0);
		vftr_dk->loiter_period_th = MAX(dk->loiter_period_th, 10);

		vftr_dk->overlap_ratio_th = (MAX(dk->overlap_ratio_th, 0) << VFTR_DK_OVERLAP_FRACTION) / 100;

		start_x = MIN(dk->roi.start_x, 99);
		start_y = MIN(dk->roi.start_y, 99);
		end_x = MAX(dk->roi.end_x, 1);
		end_y = MAX(dk->roi.end_y, 1);
		vftr_dk->roi_pts.sx = (window->width * start_x / 100);
		vftr_dk->roi_pts.sy = (window->height * start_y / 100);
		vftr_dk->roi_pts.ex = (window->width * end_x / 100);
		vftr_dk->roi_pts.ey = (window->height * end_y / 100);

		if (AVFTR_DK_setParam(win_idx, &avftr_dk) != 0) {
			avmain2_log_err("Fail to set DK parameter.\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Set door keeper(DK) on window %x success.\r\n", win_idx.value);
	} else if (dk->enabled == 0) {
		if (AVFTR_DK_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable door keeper(DK).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set door keeper(DK) on win_idx %x.\r\n", win_idx.value);
	}

	return 0;
}

/**
 * @brief Set baby monitor(BM).
 */
static int setBm(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_BM_CONF_S *bm)
{
	AVFTR_BM_PARAM_S avftr_bm = { { 0 } };
	VFTR_FGD_PARAM_S *fgd_param = &avftr_bm.fgd_param;
	MPI_CHN chn = MPI_VIDEO_CHN(0, 0);
	MPI_CHN_LAYOUT_S layout_attr = { 0 };
	MPI_WIN_ATTR_S win_attr = { { 0 }, .fps = 0 };
	int ret;
	int i;

	if (bm->enabled == 1) {
		if ((ret = AVFTR_BM_getParam(win_idx, &avftr_bm))) {
			return -EINVAL;
		}

		ret = MPI_DEV_getChnLayout(chn, &layout_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Failed to get chn(%d, %d) layout. err: %d\r\n", chn.dev, chn.chn, ret);
			return -EINVAL;
		}

		ret = MPI_DEV_getWindowAttr(win_idx, &win_attr);
		if (ret != 0) {
			avmain2_log_err("Failed to get win(%d, %d, %d) attribute. err: %d\r\n", win_idx.dev,
			                win_idx.chn, win_idx.win, ret);
			return ret;
		}

		for (i = 0; i < layout_attr.window_num; i++) {
			if (layout_attr.win_id[i].value == win_idx.value) {
				break;
			}
		}

		if (i == layout_attr.window_num) {
			return -EINVAL;
		}

		// parameters mapping from AGTX schema to AVFTR schema
		fgd_param->sensitivity = (bm->sensitivity * 255) / 100;	        // Map [0, 100] to [0, 255]
		fgd_param->boundary_thickness = bm->boundary_thickness;
		fgd_param->quality = (bm->quality * 255) / 100;                 // Map [0, 100] to [0, 255]
		fgd_param->obj_life_th = 16;
		fgd_param->time_buffer = bm->time_buffer * (int)win_attr.fps;   // Convert to num-of-frames
		fgd_param->roi = (MPI_RECT_POINT_S){
			.sx = roundingDownAlign16((bm->roi.start_x * window->width) / 100),
			.sy = roundingDownAlign16((bm->roi.start_y * window->height) / 100),
			.ex = roundingDownAlign16((bm->roi.end_x * window->width) / 100) - 1,
			.ey = roundingDownAlign16((bm->roi.end_y * window->height) / 100) - 1,
		};
		fgd_param->event_type = VFTR_FGD_OBJECT_MONITOR;
		fgd_param->suppression = bm->suppression * (int)win_attr.fps;   // Convert unit to num-of-frames

		if ((ret = AVFTR_BM_setParam(win_idx, &avftr_bm))) {
			return -EACCES;
		}

		if ((ret = AVFTR_BM_enable(win_idx))) {
			return -EACCES;
		}
	} else {
		if (AVFTR_BM_disable(win_idx)) {
			return -EINVAL;
		}

		if (AVFTR_BM_releaseMpiInfo(win_idx)) {
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Set automatic region of interest (AROI).
 */
static int setAroi(MPI_WIN win_idx, MPI_RECT_S *window, const AGTX_IVA_AROI_CONF_S *aroi)
{
	int aspect_ratio = 0;
	UINT16 ar_limit = ~0;
	UINT8 i;
	AVFTR_AROI_PARAM_S avftr_aroi = { 0 };

	/* enable aroi */
	if (aroi->enabled == 1) {
		if (AVFTR_AROI_enable(win_idx) != 0) {
			avmain2_log_err("Fail to enable automatic region of interest(AROI).\r\n");
			return -EINVAL;
		}

		if (AVFTR_AROI_getParam(win_idx, &avftr_aroi) != 0) {
			avmain2_log_err("Fail to get automatic region of interest(AROI) param.\r\n");
			return -EINVAL;
		}
		if (aroi->aspect_ratio_height == 0) {
			aspect_ratio = 0;
		} else {
			aspect_ratio =
			        (aroi->aspect_ratio_width << VFTR_AROI_AR_FRACTIONAL_BIT) / aroi->aspect_ratio_height;
			aspect_ratio = (aspect_ratio > ar_limit) ? ar_limit : aspect_ratio;
		}

		MPI_CHN chn = MPI_VIDEO_CHN(0, 0);
		MPI_CHN_LAYOUT_S layout_attr = { 0 };
		MPI_DEV_getChnLayout(chn, &layout_attr);

		for (i = 0; i < layout_attr.window_num; i++) {
			if (layout_attr.win_id[i].value == win_idx.value) {
				break;
			}
		}
		if (i == layout_attr.window_num) {
			return -EINVAL;
		}
		avftr_aroi.en_skip_shake = aroi->en_skip_shake;
		/*TODO: not full layout*/
		window->width = layout_attr.window[i].width;
		window->height = layout_attr.window[i].height;
		avftr_aroi.aroi_param.obj_life_th = aroi->obj_life_th;
		avftr_aroi.aroi_param.aspect_ratio = (UINT16)aspect_ratio;
		avftr_aroi.aroi_param.min_roi.width = (aroi->min_roi_width * window->width + 50) / 100;
		avftr_aroi.aroi_param.min_roi.height = (aroi->min_roi_height * window->height + 50) / 100;
		avftr_aroi.aroi_param.max_roi.width = (aroi->max_roi_width * window->width + 50) / 100;
		avftr_aroi.aroi_param.max_roi.height = (aroi->max_roi_height * window->height + 50) / 100;

		/* Track detla  min=0, mid=50, max=200  */
		/* Return Delta min=4, mid=8,  max=50  */
		/* Wait time    min=2, mid=29, max=100 */
		/* update rate  min=16, mid=16, max=64 */

		if (aroi->track_speed < 50) {
			avftr_aroi.aroi_param.max_track_delta_x = aroi->track_speed;
			avftr_aroi.aroi_param.max_track_delta_y = aroi->track_speed;
			avftr_aroi.aroi_param.update_ratio = 16;
		} else {
			avftr_aroi.aroi_param.max_track_delta_x = aroi->track_speed * 3 - 100;
			avftr_aroi.aroi_param.max_track_delta_y = avftr_aroi.aroi_param.max_track_delta_x;
			avftr_aroi.aroi_param.update_ratio = (aroi->track_speed * 48) / 50 - 32;
		}

		if (aroi->return_speed > 50) {
			avftr_aroi.aroi_param.max_return_delta_x = aroi->return_speed / 2;
			avftr_aroi.aroi_param.wait_time = 2 + ((((100 - aroi->return_speed) * 54) + 50) / 100);
		} else {
			avftr_aroi.aroi_param.max_return_delta_x = 4 + ((aroi->return_speed - 4 + 13) / 25);
			avftr_aroi.aroi_param.wait_time = 29 + ((((100 - (aroi->return_speed * 2)) * 71) + 50) / 100);
		}
		avftr_aroi.aroi_param.max_return_delta_y = avftr_aroi.aroi_param.max_return_delta_x;

		if (AVFTR_AROI_setParam(win_idx, &avftr_aroi) != 0) {
			avmain2_log_err("Fail to set automatic region of interest(AROI) param.\r\n");
			return -EINVAL;
		}

		g_aroi_cb_info.idx.value = win_idx.value;
		g_aroi_cb_info.res = (MPI_SIZE_S){ .width = window->width, .height = window->height };

		avmain2_log_debug("Set AROI on window %x success.\r\n", win_idx.value);
	} else if (aroi->enabled == 0) {
		if (AVFTR_AROI_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable automatic region of interest(AROI).\r\n");
			return -EINVAL;
		}
		avmain2_log_debug("Disable to set AROI on win_idx %x.\r\n", win_idx.value);
	}

	return 0;
}

static inline void toMpiLayoutWindow(const MPI_RECT_S *pos, MPI_SIZE_S *chn_res, MPI_RECT_S *lyt_res)
{
	lyt_res->x = (((pos->x * (chn_res->width - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	lyt_res->y = (((pos->y * (chn_res->height - 1) + 512) >> 10) + 16) & 0xFFFFFFE0;
	lyt_res->width = MIN((((pos->width * (chn_res->width - 1) + 512) >> 10) + 9) & 0xFFFFFFF0, chn_res->width);

	/* Handle boundary condition */
	if (pos->y + pos->height == 1024) {
		lyt_res->height = chn_res->height - lyt_res->y;
	} else {
		lyt_res->height = (((pos->height * (chn_res->height - 1) + 512) >> 10) + 16) & 0xFFFFFFE0;
	}
}

/**
 * @brief Regist iaa module instance.
 */
static int register_iaa(MPI_DEV dev_idx)
{
	if (0 != AVFTR_SD_addInstance(dev_idx)) {
		avmain2_log_err("Fail to regist sound detection(SD).\r\n");
		return -EACCES;
	}

	return 0;
}

/**
 * @brief deregister iaa module instance.
 */
static int deregister_iaa(MPI_DEV dev_idx)
{
	/* LSD */
	if (0 != AVFTR_SD_deleteInstance(dev_idx)) {
		avmain2_log_err("Fail to delete LSD instance.\r\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Regist iva module instance.
 */
static int register_iva(MPI_WIN win_idx)
{
	if (0 != AVFTR_AROI_addInstance(win_idx)) {
		avmain2_log_err("Fail to add AROI instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_BM_addInstance(win_idx)) {
		avmain2_log_err("Fail to add BM Instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_DK_addInstance(win_idx)) {
		avmain2_log_err("Fail to add DK instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_EAIF_addInstance(win_idx)) {
		avmain2_log_err("Fail to add EAIF instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_EF_addInstance(win_idx)) {
		avmain2_log_err("Fail to add EF instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_FLD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add FLD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_LD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add LD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_MD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add MD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_PD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add PD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_SHD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add SHD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_TD_addInstance(win_idx)) {
		avmain2_log_err("Fail to add TD instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_PFM_addInstance(win_idx)) {
		avmain2_log_err("Fail to add PFM instance.\r\n");
		return -EACCES;
	}

	return 0;
}

/**
 * @brief deregister iva module instance.
 */
static int deregister_iva(MPI_WIN win_idx)
{
	if (0 != AVFTR_AROI_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete AROI instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_BM_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete BM Instance.\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_DK_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete DK instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_EAIF_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete EAIF instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_EF_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete EF instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_FLD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete FLD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_LD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete FLD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_MD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete MD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_PD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete PD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_SHD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete SHD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_TD_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete TD instance\r\n");
		return -EACCES;
	}

	if (0 != AVFTR_PFM_deleteInstance(win_idx)) {
		avmain2_log_err("Fail to delete PFM instance\r\n");
		return -EACCES;
	}

	return 0;
}

int NODE_initIva(void)
{
	if (AVFTR_initServer() < 0) {
		avmain2_log_err("Fail to Initialize AVFTR server.\r\n");
		return -EACCES;
	}
	/* event daemon maybe restart in runtime, so don't force to connect it */
	if (connectEventDaemon() != 0) {
		avmain2_log_err("Fail to connect event daemon.\r\n");
	}

	avmain2_log_debug("Init IVA\r\n");
	return 0;
}

/**
 * @brief Start iva should follow this flow.(register instance-> enable -> set parameter -> run iva/iaa)
 * TIP: This NODE_startIva statement can use global variable.
 */
int NODE_startIva(void)
{
	/* Recent IVA only support one channel, one window */
	MPI_DEV dev_idx = MPI_VIDEO_DEV(0);
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);

	MPI_RECT_S pos = { 0 };
	MPI_RECT_S window = { 0 };
	MPI_SIZE_S res = { 0 };
	pos.x = g_conf.layout.video_layout[0].window_array[0].pos_x;
	pos.y = g_conf.layout.video_layout[0].window_array[0].pos_y;
	pos.width = g_conf.layout.video_layout[0].window_array[0].pos_width;
	pos.height = g_conf.layout.video_layout[0].window_array[0].pos_height;
	res.width = g_conf.strm.video_strm[0].width;
	res.height = g_conf.strm.video_strm[0].height;

	toMpiLayoutWindow(&pos, &res, &window /* output window exactly rect */);

#ifdef CB_BASED_OD
	avmain2_log_info("node_iva.c: g_conf.od.version = %d", g_conf.od.version);
	if (g_conf.od.version == 1) {
		/** Optional feature: Register OD CallBack Function

		 * Register the custom AI algorithm for Object Detection.
		 * The default MV-based OD will be executed if not registered.
		 */
		MPI_IVA_OD_CALLBACK_S cb;
		ML_CB_CTX_S *cb_ctx = calloc(1, sizeof(ML_CB_CTX_S));

		SAMPLE_OD_registerCallback(MPI_VIDEO_WIN(0, 0, 0), &cb);

		MPI_IVA_regOdCallback(MPI_VIDEO_WIN(0, 0, 0), &cb, cb_ctx);
	}
#endif /*< CB_BASED_OD */

	/* Register iva instance */
	if (0 != register_iva(win_idx)) {
		avmain2_log_err("Fail to regist iva module.\r\n");
		return -EACCES;
	}
	/* Register iaa instance */
	if (0 != register_iaa(dev_idx)) {
		avmain2_log_err("Fail to regist iaa module.\r\n");
		return -EACCES;
	}

	if (0 != setAroi(win_idx, &window, &g_conf.aroi)) {
		avmain2_log_err("Fail to set AROI.\r\n");
		return -EACCES;
	}

	if (0 != setBm(win_idx, &window, &g_conf.bm)) {
		avmain2_log_err("Fail to set BM.\r\n");
		return -EACCES;
	}

	if (0 != setDk(win_idx, &window, &g_conf.dk)) {
		avmain2_log_err("Fail to set door keeper(DK).\r\n");
		return -EACCES;
	}

	if (0 != setEaif(win_idx, &window, &g_conf.eaif)) {
		avmain2_log_err("Fail to set edge AI assisted feature(EAIF).\r\n");
		return -EACCES;
	}

	if (0 != setEf(win_idx, &window, &g_conf.ef)) {
		avmain2_log_err("Fail to set eletric fence(EF).\r\n");
		return -EACCES;
	}

	if (0 != setFld(win_idx, &g_conf.fld)) {
		avmain2_log_err("Fail to set fall detection(FLD).\r\n");
		return -EACCES;
	}

	if (0 != setLd(win_idx, &window, &g_conf.ld)) {
		avmain2_log_err("Fail to set light detection(LD).\r\n");
		return -EACCES;
	}

	if (0 != setLsd(dev_idx, &g_conf.lsd)) {
		avmain2_log_err("Fail to set loud sound detection(LSD).\r\n");
		return -EACCES;
	}

	if (0 != setMd(win_idx, &window, &g_conf.md)) {
		avmain2_log_err("Fail to set motion detection(MD).\r\n");
		return -EACCES;
	}

	if (0 != setOd(win_idx, &window, &g_conf.od)) {
		avmain2_log_err("Fail to set object detection(OD).\r\n");
		return -EACCES;
	}

	if (0 != setPd(win_idx, &window, &g_conf.pd)) {
		avmain2_log_err("Fail to set pedestrian detection(PD).\r\n");
		return -EACCES;
	}

	if (0 != setPfm(win_idx, &window, &g_conf.pfm)) {
		avmain2_log_err("Fail to set pet feeding monitor(PFM).\r\n");
		return -EACCES;
	}

	if (0 != setPtz(dev_idx, &window, &g_conf.ptz)) {
		avmain2_log_err("Fail to set Set Pan-tile-zoom(PTZ).\r\n");
		return -EACCES;
	}

	if (0 != setRms(win_idx, &window, &g_conf.rms)) {
		avmain2_log_err("Fail to set regional motion sensor(RMS).\r\n");
		return -EACCES;
	}

	if (0 != setShd(win_idx, &window, &g_conf.shd)) {
		avmain2_log_err("Fail to set shaking object detection(SHD).\r\n");
		return -EACCES;
	}

	if (0 != setTd(win_idx, &g_conf.td)) {
		avmain2_log_err("Fail to set tamper detection(TD).\r\n");
		return -EACCES;
	}

	/* Run iva and iaa processing. */
	if (AVFTR_runIaa(dev_idx) < 0) {
		avmain2_log_err("Fail to create AFTR_runIaa thread for DEV:%x.\r\n", dev_idx.value);
		return -EINVAL;
	}

	if (AVFTR_runIva(win_idx) < 0) {
		avmain2_log_err("Fail to create VFTR_runIva thread for WIN:%x.\r\n", win_idx.value);
		return -EINVAL;
	}

	avmain2_log_debug("Run IVA!!\r\n");

	return 0;
}

int NODE_stopIva(void)
{
	/* Recent IVA only support one channel, one window */
	const MPI_DEV dev_idx = MPI_VIDEO_DEV(0);
	const MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);

	if (g_conf.aroi.enabled == 1) {
		avmain2_log_debug("Disable automatic region of interest(AROI)\r\n");
		if (AVFTR_AROI_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable (AROI).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.bm.enabled == 1) {
		avmain2_log_debug("Disable baby monitor(BM)\r\n");
		if (AVFTR_BM_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable baby monitor(BM).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.dk.enabled == 1) {
		avmain2_log_debug("Disable door keeper(DK)\r\n");
		if (AVFTR_DK_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable door keeper(DK).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.eaif.enabled == 1) {
		avmain2_log_debug("Disable edge AI assisted feature(EAIF)\r\n");
		if (AVFTR_EAIF_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disabledoor (EAIF).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.ef.enabled == 1) {
		avmain2_log_debug("Disable electric fence(EF)\r\n");
		for (int i = g_conf.ef.line_cnt; i; i--) {
			if (AVFTR_EF_rmVl(win_idx, i) != 0) {
				avmain2_log_err("Fail to remove EF line[%d]\r\n", i);
				return -EINVAL;
			}
		}

		if (AVFTR_EF_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable electric fence(EF).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.fld.enabled == 1) {
		avmain2_log_debug("Disable fall detection(FLD).\r\n");
		if (AVFTR_FLD_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disabledoor fall detection(FLD).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.ld.enabled == 1) {
		avmain2_log_debug("Disable light detection(LD)\r\n");
		if (AVFTR_LD_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable LD\r\n");
			return -EINVAL;
		}

		if (AVFTR_LD_releaseMpiInfo(win_idx) < 0) {
			avmain2_log_err("Fail to release LD mpi info.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.lsd.enabled == 1) {
		if (AVFTR_SD_disable(dev_idx) < 0) {
			avmain2_log_err("Fail to disable LSD.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.md.enabled == 1) {
		avmain2_log_debug("Disable motion detection(MD)\r\n");
		if (AVFTR_MD_disable(win_idx) != 0) {
			avmain2_log_err("Fail to disable motion detection(MD).\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.od.enabled == 1) {
		if (VIDEO_FTR_disableOd(win_idx) < 0) {
			avmain2_log_err("Fail to disable OD.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.pd.enabled == 1) {
		if (AVFTR_PD_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable PD.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.pfm.enabled == 1) {
		avmain2_log_debug("Disable pet feeding monitor(PFM)\r\n");
		if (AVFTR_PFM_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable PFM.\r\n");
			return -EINVAL;
		}

		if (AVFTR_PFM_releaseMpiInfo(win_idx) < 0) {
			avmain2_log_err("Fail to release PFM mpi info.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.ptz.enabled == 1) {
		if (VIDEO_FTR_disablePtz() < 0) {
			avmain2_log_err("Fail to disable PTZ.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.rms.enabled == 1) {
		if (VIDEO_FTR_disableRms(win_idx) < 0) {
			avmain2_log_err("Fail to disable RMS.\r\n");
			return -EINVAL;
		}
	}

	if (g_conf.td.enabled == 1) {
		avmain2_log_debug("Disable tamper detection(TD)\r\n");
		if (AVFTR_TD_disable(win_idx) < 0) {
			avmain2_log_err("Fail to disable TD\r\n");
			return -EINVAL;
		}

		if (AVFTR_TD_releaseMpiInfo(win_idx) < 0) {
			avmain2_log_err("Fail to release TD mpi info.\r\n");
			return -EINVAL;
		}
	}

	if (0 != deregister_iva(win_idx)) {
		avmain2_log_err("Fail to deregister iva module.\r\n");
		return -EACCES;
	}

	if (0 != deregister_iaa(dev_idx)) {
		avmain2_log_err("Fail to deregister iaa module.\r\n");
		return -EACCES;
	}

	if (AVFTR_exitIva(win_idx) < 0) {
		avmain2_log_err("Cannot exit VFTR_runIva thread for WIN:%x\r\n", win_idx.value);
		return -EINVAL;
	}

	if (AVFTR_exitIaa(dev_idx) < 0) {
		avmain2_log_err("Cannot exit AFTR_runIaa thread for DEV:%x\r\n", dev_idx.value);
		return -EINVAL;
	}

	return 0;
}

int NODE_exitIva(void)
{
	avmain2_log_debug("Exit IVA\r\n");
	if (AVFTR_exitServer() < 0) {
		avmain2_log_err("Exit AVFTR server failed\r\n");
		return -EACCES;
	}
	return 0;
}

int NODE_setIva(int cmd_id, void *data)
{
	/* Recent IVA only support one channel, one window */
	MPI_DEV dev_idx = MPI_VIDEO_DEV(0);
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);

	MPI_RECT_S pos = { 0 };
	MPI_RECT_S window = { 0 };
	MPI_SIZE_S res = { 0 };
	pos.x = g_conf.layout.video_layout[0].window_array[0].pos_x;
	pos.y = g_conf.layout.video_layout[0].window_array[0].pos_y;
	pos.width = g_conf.layout.video_layout[0].window_array[0].pos_width;
	pos.height = g_conf.layout.video_layout[0].window_array[0].pos_height;
	res.width = g_conf.strm.video_strm[0].width;
	res.height = g_conf.strm.video_strm[0].height;

	toMpiLayoutWindow(&pos, &res, &window /* output window exactly rect */);

	switch (cmd_id) {
	case AROI_ATTR:
		if (0 != setAroi(win_idx, &window, (AGTX_IVA_AROI_CONF_S *)data)) {
			avmain2_log_err("Fail to set AROI.\r\n");
			return -EINVAL;
		}
		break;
	case BM_ATTR:
		if (0 != setBm(win_idx, &window, (AGTX_IVA_BM_CONF_S *)data)) {
			avmain2_log_err("Fail to set baby monitor(BM).\r\n");
			return -EINVAL;
		}
		break;
	case DK_ATTR:
		if (0 != setDk(win_idx, &window, (AGTX_IVA_DK_CONF_S *)data)) {
			avmain2_log_err("Fail to set door keeper(DK).\r\n");
			return -EINVAL;
		}
		break;
	case EAIF_ATTR:
		if (0 != setEaif(win_idx, &window, (AGTX_IVA_EAIF_CONF_S *)data)) {
			avmain2_log_err("Fail to set edge AI assisted feature(EAIF).\r\n");
			return -EINVAL;
		}
		break;
	case EF_ATTR:
		if (0 != setEf(win_idx, &window, (AGTX_IVA_EF_CONF_S *)data)) {
			avmain2_log_err("Fail to set electric fence(EF).\r\n");
			return -EINVAL;
		}
		break;
	case FLD_ATTR:
		if (0 != setFld(win_idx, (AGTX_IVA_FLD_CONF_S *)data)) {
			avmain2_log_err("Fail to set fall detection(FLD).\r\n");
			return -EINVAL;
		}
		break;
	case LD_ATTR:
		if (0 != setLd(win_idx, &window, (AGTX_IVA_LD_CONF_S *)data)) {
			avmain2_log_err("Fail to set light detection(LD).\r\n");
			return -EINVAL;
		}
		break;
	case LSD_ATTR:
		if (0 != setLsd(dev_idx, (AGTX_IAA_LSD_CONF_S *)data)) {
			avmain2_log_err("Fail to set loud sound detection(LSD).\r\n");
			return -EINVAL;
		}
		break;
	case MD_ATTR:
		if (0 != setMd(win_idx, &window, (AGTX_IVA_MD_CONF_S *)data)) {
			avmain2_log_err("Fail to set motion detection(MD).\r\n");
			return -EINVAL;
		}
		break;
	case OD_ATTR:
		if (0 != setOd(win_idx, &window, (AGTX_IVA_OD_CONF_S *)data)) {
			avmain2_log_err("Fail to set object detection(OD).\r\n");
			return -EINVAL;
		}
		break;
	case PD_ATTR:
		if (0 != setPd(win_idx, &window, (AGTX_IVA_PD_CONF_S *)data)) {
			avmain2_log_err("Fail to set pedestrian detection(PD).\r\n");
			return -EINVAL;
		}
		break;
	case PFM_ATTR:
		if (0 != setPfm(win_idx, &window, (AGTX_IVA_PFM_CONF_S *)data)) {
			avmain2_log_err("Fail to set pet feeding monitor(PFM).\r\n");
			return -EINVAL;
		}
		break;
	case PTZ_ATTR:

		if (0 != setPtz(dev_idx, &window, (AGTX_VIDEO_PTZ_CONF_S *)data)) {
			avmain2_log_err("Fail to set Set Pan-tile-zoom(PTZ).\r\n");
			return -EINVAL;
		}
		break;
	case RMS_ATTR:
		if (0 != setRms(win_idx, &window, (AGTX_IVA_RMS_CONF_S *)data)) {
			avmain2_log_err("Fail to set regional motion sensor(RMS).\r\n");
			return -EINVAL;
		}
		break;
	case SHD_ATTR:
		if (0 != setShd(win_idx, &window, (AGTX_IVA_SHD_CONF_S *)data)) {
			avmain2_log_err("Fail to set shaking object detection(SHD).\r\n");
			return -EINVAL;
		}
		break;
	case TD_ATTR:
		if (0 != setTd(win_idx, (AGTX_IVA_TD_CONF_S *)data)) {
			avmain2_log_err("Fail to set tamper detection(TD).\r\n");
			return -EINVAL;
		}
		break;
	case VDBG_ATTR:
		avmain2_log_debug("case VDBG_ATTR\r\n");
		break;
	default:
		break;
	}

	return 0;
}
