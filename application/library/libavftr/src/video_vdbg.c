#include "video_vdbg.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "mpi_dev.h"
#include "mpi_dip_alg.h"

#include "libdebug.h"
#include "avftr_log.h"

#include "avftr_common.h"
#include "avftr.h"
#include "video_od.h"
#include "avftr_md.h"
#include "avftr_fld.h"
#include "avftr_ld.h"
#include "avftr_td.h"
#include "avftr_ef.h"
#include "video_rms.h"
#include "avftr_aroi.h"
//#include "video_pd.h"

#define abs(x) ((x) > 0) ? (x) : -(x)

#define VDBG_FMT(fmt) "[%s:%d]" fmt

//#define VDBG_DEB(fmt, args...) printf( VDBG_FMT(fmt), __func__, __LINE__, ##args)
#define VDBG_DEB(fmt, args...)

//#define VIDEO_VDBG_DEBUG_
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static inline int clip(int x, int a, int b)
{
	x = (x < a) ? a : x;
	x = (x > b) ? b : a;
	return x;
}

int VIDEO_FTR_getVdbgStat(VIDEO_VDBG_CTX_S *vftr_vdbg_ctx)
{
	return vftr_vdbg_ctx->en;
}

int VIDEO_FTR_enableVdbg(void)
{
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;
	vftr_vdbg_ctx->en = 1;
	return 0;
}

int VIDEO_FTR_disableVdbg(void)
{
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;

	vftr_vdbg_ctx->ctx = 0;
	vftr_vdbg_ctx->en = 0;
#ifdef VIDEO_DEBUG_MODULE
	//avftr_log_err("pre disable debug");
	int ret = DEBUG_disable();
	if (ret) {
		avftr_log_err("Cannot disable debug module");
	}
#endif /* !VIDEO_DEBUG_MODULE */
	return 0;
}

int VIDEO_FTR_getVdbg(UINT32 *enable_flag)
{
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;
	*enable_flag = vftr_vdbg_ctx->ctx;
	return 0;
}

int VIDEO_FTR_setVdbg(UINT32 enable_flag)
{
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;
#ifdef VIDEO_DEBUG_MODULE
	int ret = 0;
	//avftr_log_err("pre en debug %d", vftr_vdbg_ctx->en);
	if (vftr_vdbg_ctx->en) {
		if (enable_flag) {
			ret = DEBUG_enable();
			if (ret) {
				avftr_log_err("Cannot enable debug module");
			}
		} else {
			ret = DEBUG_disable();
			if (ret) {
				avftr_log_err("Cannot disable debug module");
			}
		}
		VDBG_DEB("exit\n");
	}

#endif /* !VIDEO_DEBUG_MODULE */
	vftr_vdbg_ctx->ctx = enable_flag;
	return 0;
}

#ifdef AVFTR_VDBG

typedef enum {
	VIDEO_VDBG_CTX_OD = 0,
	VIDEO_VDBG_CTX_TD,
	VIDEO_VDBG_CTX_MD,
	VIDEO_VDBG_CTX_EF,
	VIDEO_VDBG_CTX_FLD,
	VIDEO_VDBG_CTX_LD,
	VIDEO_VDBG_CTX_RMS,
	VIDEO_VDBG_CTX_AROI,
	//VIDEO_VDBG_CTX_EXPO,
	//VIDEO_VDBG_CTX_DEBUG,
	VIDEO_VDBG_CTX_VDBG,
	VIDEO_VDBG_CTX_TYPE_NUM,
} VIDEO_VDBG_CTX_TYPE_E;

int vdbg_get_support_num[VIDEO_VDBG_CTX_TYPE_NUM] = {
	VIDEO_OD_MAX_SUPPORT_NUM, AVFTR_TD_MAX_SUPPORT_NUM,  AVFTR_MD_MAX_SUPPORT_NUM,   AVFTR_EF_MAX_SUPPORT_NUM,
	AVFTR_LD_MAX_SUPPORT_NUM, VIDEO_RMS_MAX_SUPPORT_NUM, AVFTR_AROI_MAX_SUPPORT_NUM, VIDEO_VDBG_MAX_SUPPORT_NUM,
};

static inline UINT32 findIvaCtxValue(void *ctx, VIDEO_VDBG_CTX_TYPE_E type, int i)
{
	switch (type) {
	case VIDEO_VDBG_CTX_AROI:
		//AVFTR_AROI_CTX_S *aroi_ctx = (AVFTR_AROI_CTX_S *)ctx;
		//return aroi_ctx[i].idx.value;
		return ((AVFTR_AROI_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_OD:
		//VIDEO_OD_CTX_S *od_ctx = (VIDEO_OD_CTX_S *)ctx;
		//return od_ctx[i].idx.value;
		return ((VIDEO_OD_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_MD:
		//VIDEO_MD_CTX_S *md_ctx = (VIDEO_MD_CTX_S *)ctx;
		//return md_ctx[i].idx.value;
		return ((AVFTR_MD_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_TD:
		//VIDEO_TD_CTX_S *td_ctx = (VIDEO_TD_CTX_S *)ctx;
		//return td_ctx[i].idx.value;
		return ((AVFTR_TD_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_EF:
		//VIDEO_EF_CTX_S *od_ctx = (VIDEO_EF_CTX_S *)ctx;
		//return ef_ctx[i].idx.value;
		return ((AVFTR_EF_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_FLD:
		//VIDEO_FLD_CTX_S *od_ctx = (VIDEO_FLD_CTX_S *)ctx;
		//return ld_ctx[i].idx.value;
		return ((AVFTR_FLD_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_LD:
		//AVFTR_LD_CTX_S *od_ctx = (AVFTR_LD_CTX_S *)ctx;
		//return ld_ctx[i].idx.value;
		return ((AVFTR_LD_CTX_S *)ctx)[i].idx.value;
	case VIDEO_VDBG_CTX_RMS:
		//VIDEO_RMD_CTX_S *od_ctx = (VIDEO_RMS_CTX_S *)ctx;
		//return RMS_ctx[i].idx.value;
		return ((VIDEO_RMS_CTX_S *)ctx)[i].idx.value;
	default:
		assert(0);
		return -1;
	}
}

static int findIvaCtx(MPI_WIN idx, void *ctx, VIDEO_VDBG_CTX_TYPE_E type)
{
	int i = 0;
	int find_idx = -1;
	int max_num = vdbg_get_support_num[type];

	for (i = 0; i < max_num; i++) {
		if (find_idx == -1 && findIvaCtxValue(ctx, type, i) == idx.value) {
			find_idx = i;
			break;
		}
	}
	return find_idx;
}

static void rescaleIvaRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *rect)
{
	rect->sx -= src_rect->x;
	rect->sy -= src_rect->y;
	rect->ex -= src_rect->x;
	rect->ey -= src_rect->y;

	rect->sx = (rect->sx * src_roi->width + (src_rect->width >> 1)) / src_rect->width
	           + src_roi->x - dst_roi->x;
	rect->sx = (rect->sx * dst_rect->width + (dst_roi->width >> 1)) / dst_roi->width;
	rect->ex = (rect->ex * src_roi->width + (src_rect->width >> 1)) / src_rect->width
	           + src_roi->x - dst_roi->x;
	rect->ex = (rect->ex * dst_rect->width + (dst_roi->width >> 1)) / dst_roi->width;

	rect->sy = (rect->sy * src_roi->height + (src_rect->height >> 1)) / src_rect->height
	           + src_roi->y - dst_roi->y;
	rect->sy = (rect->sy * dst_rect->height + (dst_roi->height >> 1)) / dst_roi->height;
	rect->ey = (rect->ey * src_roi->height + (src_rect->height >> 1)) / src_rect->height
	           + src_roi->y - dst_roi->y;
	rect->ey = (rect->ey * dst_rect->height + (dst_roi->height >> 1)) / dst_roi->height;

	rect->sx += dst_rect->x;
	rect->sy += dst_rect->y;
	rect->ex += dst_rect->x;
	rect->ey += dst_rect->y;
}

static void rescaleMdRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                         const MPI_RECT_S *dst_roi, VFTR_MD_STATUS_S *dst_stat)
{
	int i;
	for (i = 0; i < dst_stat->region_num; i++) {
		rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, &dst_stat->attr[i].pts);
	}
}

static void rescaleOdRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, MPI_IVA_OBJ_LIST_S *list)
{
	int i;
	for (i = 0; i < list->obj_num; i++) {
		rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, &list->obj[i].rect);
	}
}

static void rescaleEfRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *pts)
{
	rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, pts);
}

static void rescaleAroiRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                           const MPI_RECT_S *dst_roi, VFTR_AROI_STATUS_S *aroi_stat)
{
	rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, &aroi_stat->roi);
}

static void rescaleLdRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                         const MPI_RECT_S *dst_roi, AVFTR_RECT_POINT_S *roi)
{
	rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, roi);
}

static void rescaleRmsRes(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *roi)
{
	rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, roi);
}

static inline int getMdMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                           const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const VFTR_MD_STATUS_S *stat,
                           char *str)
{
	int offset = 0;
	int i = 0;
	VFTR_MD_STATUS_S dst_stat;
	memcpy(&dst_stat, stat, sizeof(VFTR_MD_STATUS_S));

	if (src_idx.value != dst_idx.value) {
		rescaleMdRes(src_rect, dst_rect, src_roi, dst_roi, &dst_stat);
	}
	for (i = 0; i < dst_stat.region_num; i++) {
		int g, r;
		if (dst_stat.stat[i].alarm == 0) {
			g = 255;
			r = 0;
		} else {
			g = 0;
			r = 255;
		}
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<RECT RECT=\"%d %d %d %d\" SZ=\"%d\" LNCR=\"0 %d %d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":%d,\"lncr\":[0,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
		                  dst_stat.attr[i].pts.sx, dst_stat.attr[i].pts.sy, dst_stat.attr[i].pts.ex,
		                  dst_stat.attr[i].pts.ey, 3 + dst_stat.stat[i].alarm * 2, g, r);
	}
	return offset;
}

static int getEfMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                    const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const VFTR_EF_STATUS_S *src_list, char *str)
{
	int offset = 0;
	int i = 0;
	int cntn = 0, cntp = 0;

	VFTR_EF_STATUS_S list;
	list.fence_num = src_list->fence_num;

	for (i = 0; i < src_list->fence_num; i++) {
		list.attr[i].line = src_list->attr[i].line;
	}

	if (src_idx.value != dst_idx.value) {
		for (i = 0; i < src_list->fence_num; i++) {
			rescaleEfRes(src_rect, dst_rect, src_roi, dst_roi, &list.attr[i].line);
		}
	}

	for (i = 0; i < list.fence_num; i++) {
		int b = 100, g = 100, r = 100;
		cntp += src_list->stat[i].cnt[0];
		cntn += src_list->stat[i].cnt[1];
		if (src_list->stat[i].alarm == 0) {
			b = 200;
			g = 0;
			r = 0;
			//if (list->param[i].mode==0) { b=0; g=0; r=200; }
			//else if (list->param[i].mode==1) { b=0; g=200; r=200; }
			//else if (list->param[i].mode==2) { b=200; g=0; r=0; }
		} else {
			if (src_list->stat[i].alarm == 1) {
				b = 0;
				g = 200;
				r = 0;
			} else if (src_list->stat[i].alarm == 2) {
				b = 255;
				g = 255;
				r = 255;
			} else if (src_list->stat[i].alarm == 3) {
				b = 0;
				g = 0;
				r = 255;
			}
		}
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<LINE LINE=\"%d %d %d %d\" SZ=\"4\" CR=\"%d %d %d\"/>"
		                  "<TEXT TEXT=\"Id:%d Mode:%d\" LOC=\"%d %d\" SZ=\"1\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"line\":{\"line\":[%d,%d,%d,%d],\"sz\":4,\"cr\":[%d,%d,%d]}},"
		                  "{\"text\":{\"text\":\"Id:%d Mode:%d\",\"loc\":[%d,%d],\"sz\":1}},",
#endif /* IVA_FORMAT_XML */
		                  list.attr[i].line.sx, list.attr[i].line.sy, list.attr[i].line.ex,
		                  list.attr[i].line.ey, b, g, r, i, src_list->attr[i].mode,
		                  (list.attr[i].line.sx + list.attr[i].line.ex) / 2,
		                  (list.attr[i].line.sy + list.attr[i].line.ey) / 2);
	}
	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<TEXT TEXT=\"VDBG EF cntp:%d cntn:%d\" LOC=\"0 100\" SZ=\"1\" CR=\"0 0 255\"/>",
#else /* IVA_FORMAT_JSON */
	                  "{\"text\":{\"text\":\"VDBG EF cntp:%d cntn:%d\",\"loc\":[0,100],\"sz\":1,\"cr\":[0,0,255]}},",
#endif /* IVA_FORMAT_XML */
	                  cntp, cntn);
	return offset;
}

static inline int getAroiMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                             const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                             const VFTR_AROI_STATUS_S *src_aroi_stat, const MPI_RECT_POINT_S *src_aroi_tar, char *str)
{
	int offset = 0;
	VFTR_AROI_STATUS_S aroi_stat = *src_aroi_stat;
	MPI_RECT_POINT_S aroi_tar = *src_aroi_tar;

	if (src_idx.value != dst_idx.value) {
		rescaleAroiRes(src_rect, dst_rect, src_roi, dst_roi, &aroi_stat);
		rescaleIvaRes(src_rect, dst_rect, src_roi, dst_roi, &aroi_tar);
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<RECT RECT=\"%d %d %d %d\" SZ=\"2\" FLCR=\"200 10 10\" FLAP=\"0.5\"/>"
	                  "<TEXT TEXT=\"VDBG_AROI\" LOC=\"%d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"flcr\":[200,10,10],\"flap\":0.5}},"
	                  "{\"text\":{\"text\":\"VDBG_AROI\",\"loc\":[%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                  aroi_stat.roi.sx, aroi_stat.roi.sy, aroi_stat.roi.ex, aroi_stat.roi.ey, aroi_stat.roi.sx,
	                  aroi_stat.roi.sy);
	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<RECT RECT=\"%d %d %d %d\" SZ=\"2\" LNCR=\"10 10 200\"/>"
	                  "<TEXT TEXT=\"TAR_AROI\" LOC=\"%d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"lncr\":[10,10,200]}},"
	                  "{\"text\":{\"text\":\"TAR_AROI\",\"loc\":[%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                  aroi_tar.sx, aroi_tar.sy, aroi_tar.ex, aroi_tar.ey, aroi_tar.sx, aroi_tar.sy);
	return offset;
}

static inline int getFldMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                            const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const AVFTR_FLD_STATUS_S *stat,
                            char *str)
{
	int offset = 0;
	char text[100];
	int b = 0, g = 0, r = 0, sz = 1; //default black

	const MPI_IVA_OBJ_LIST_S *src_list = &stat->obj_list;
	const VFTR_FLD_STATUS_S *src_fld_stat = &stat->fld_status;

	MPI_IVA_OBJ_LIST_S list;
	list.obj_num = src_list->obj_num;
	memcpy(list.obj, src_list->obj, sizeof(MPI_IVA_OBJ_ATTR_S) * src_list->obj_num);

	if (src_idx.value != dst_idx.value) {
		rescaleOdRes(src_rect, dst_rect, src_roi, dst_roi, &list);
	}

	for (int i = 0; i < list.obj_num; i++) {
		if (src_fld_stat->obj_stat_list[i].result == VFTR_FLD_NORMAL) {
			sprintf(text, "%s", "NORMAL");
			b = 0;
			g = 255;
			r = 0;
		} else if (src_fld_stat->obj_stat_list[i].result == VFTR_FLD_FALLING) {
			sprintf(text, "%s", "FALLING");
			b = 0;
			g = 255;
			r = 255;
		} else if (src_fld_stat->obj_stat_list[i].result == VFTR_FLD_DOWN) {
			sprintf(text, "%s", "DOWN");
			b = 0;
			g = 127;
			r = 255;
		} else if (src_fld_stat->obj_stat_list[i].result == VFTR_FLD_FALLEN) {
			sprintf(text, "%s", "FALLEN");
			b = 0;
			g = 0;
			r = 255;
		} else {
			sprintf(text, "%s", "");
			b = 255;
			g = 255;
			r = 255;
		}
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<RECT RECT=\"%d %d %d %d\" SZ=\"2\" LNCR=\"%d %d %d\"/>"
		                  "<TEXT TEXT=\"%s\" LOC=\"%d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"lncr\":[%d,%d,%d]}},"
		                  "{\"text\":{\"text\":\"%s\",\"loc\":[%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
		                  list.obj[i].rect.sx, list.obj[i].rect.sy, list.obj[i].rect.ex, list.obj[i].rect.ey, b,
		                  g, r, text, list.obj[i].rect.sx, list.obj[i].rect.ey, 1, b, g, r);
	}

	if (src_fld_stat->result == VFTR_FLD_FALLEN) {
		sprintf(text, "%s", "FALLEN");
		b = 0;
		g = 0;
		r = 255;
	} else {
		sprintf(text, "%s", "NORMAL");
		b = 0;
		g = 255;
		r = 0;
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<TEXT TEXT=\"%s\" LOC=\"%d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>"
#else /* IVA_FORMAT_JSON */
	                  "{\"text\":{\"text\":\"%s\",\"loc\":[%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},"
#endif /* IVA_FORMAT_XML */
	                  ,
	                  text, 0, dst_rect->height, sz, b, g, r);
	return offset;
}

static inline int getLdMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                           const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const AVFTR_LD_STATUS_S *res,
                           char *str)
{
	/* Alarm Color determination */
	static int vdbg_ld_cnt = 0;
	static const int vdbg_ld_cnt_max = 50;
	static int alarm = 0;
	int b = 0, g = 255, r = 0, sz = 4;
	int offset = 0;

	AVFTR_RECT_POINT_S roi = res->roi;

	if (src_idx.value != dst_idx.value) {
		rescaleLdRes(src_rect, dst_rect, src_roi, dst_roi, &roi);
	}

	if (res->status.trig_cond != VFTR_LD_LIGHT_NONE) {
		alarm = res->status.trig_cond;
		vdbg_ld_cnt = 1;
	}
	if (vdbg_ld_cnt > 0) {
		if (alarm == VFTR_LD_LIGHT_ON) {
			b = 0;
			g = 255;
			r = 255;
			sz = 8;
		} else if (alarm == VFTR_LD_LIGHT_OFF) {
			b = 255;
			g = 0;
			r = 0;
			sz = 8;
		} else {
			b = 255;
			g = 255;
			r = 255;
			sz = 8;
		} /* LD_LIGHT_BOTH */
		if (vdbg_ld_cnt == vdbg_ld_cnt_max) {
			vdbg_ld_cnt = 0;
			sz = 4;
		} else
			(vdbg_ld_cnt++);
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<TEXT TEXT=\"VDBG_LD\" LOC=\"%d %d\"/>"
	                  "<RECT RECT=\"%d %d %d %d\" SZ=\"%d\" LNFP=\"1\" LNCR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                  "{\"text\":{\"text\":\"VDBG_LD\",\"loc\":[%d,%d]}},"
	                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":%d,\"lnfp\":1,\"lncr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                  roi.sx, roi.ey, roi.sx, roi.sy, roi.ex, roi.ey, sz, b, g, r);
	return offset;
}

static int getTdMsg(const MPI_CHN_ATTR_S *rect, const int tamper_alarm, char *str)
{
	int offset = 0;
	if (tamper_alarm > 0) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<RECT RECT=\"%d %d %d %d\" SZ=\"10\" LNAP=\"0.5\" LNCR=\"0 0 255\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":10,\"lnap\":0.5,\"lncr\":[0,0,255]}},",
#endif /* IVA_FORMAT_XML */
		                  0, 0, rect->res.width - 1, rect->res.height - 1);

		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<TEXT TEXT=\"");
#else /* IVA_FORMAT_JSON */
		                  "{\"text\":{\"text\":\"");
#endif /* IVA_FORMAT_XML */
		if (tamper_alarm & (1 << VFTR_TD_ALARM_BLOCK)) {
			offset += sprintf(&str[offset], "VDBG_TD_blocking");
		}
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "\" LOC=\"%d %d\" SZ=\"1\" CR=\"0 0 200\"/>",
#else /* IVA_FORMAT_JSON */
		                  "\",\"loc\":[%d,%d],\"sz\":1,\"cr\":[0,0,200]}},",
#endif /* IVA_FORMAT_XML */
		                  10, rect->res.height - 1);
	}
	return offset;
}

static int getRmsMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                     const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const MPI_RECT_POINT_S *roi, const MPI_IVA_RMS_REG_LIST_S *reg_list, char *str)
{
	int offset = 0;
	int g = 0, r = 0;
	MPI_RECT_POINT_S copy_roi[MPI_IVA_RMS_MAX_REG_NUM];
	unsigned int i = 0;

	for (i = 0; i < reg_list->reg_cnt; i++) {
		copy_roi[i] = roi[i];
	}

	if (src_idx.value != dst_idx.value) {
		for (i = 0; i < reg_list->reg_cnt; i++) {
			rescaleRmsRes(src_rect, dst_rect, src_roi, dst_roi, &copy_roi[i]);
		}
	}

	for (i = 0; i < reg_list->reg_cnt; i++) {
		if (reg_list->reg[i].conf > 127) {
			r = 255;
			g = (255 - ((reg_list->reg[i].conf - 128) << 1));
		} else {
			r = reg_list->reg[i].conf << 1;
			g = 255;
		}

		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<RECT RECT=\"%d %d %d %d\" SZ=\"3\" LNCR=\"0 %d %d\"/>"
		                  "<TEXT TEXT=\"%d\" LOC=\"%d %d\" SZ=\"1\" CR=\"0 0 255\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":3,\"lncr\":[0,%d,%d]}},"
		                  "{\"text\":{\"text\":\"%d\",\"loc\":[%d,%d],\"sz\":1,\"cr\":[0,0,255]}},",
#endif /* IVA_FORMAT_XML */
		                  copy_roi[i].sx, copy_roi[i].sy, copy_roi[i].ex, copy_roi[i].ey, g, r, reg_list->reg[i].conf, copy_roi[i].sx,
		                  copy_roi[i].ey);
	}
	return offset;
}

static inline int getOdMsg(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                           const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const VIDEO_FTR_OBJ_LIST_S *vftr_list, char *str)
{
	int offset = 0;
	int i = 0;
	const MPI_IVA_OBJ_LIST_S *src_list = &vftr_list->basic_list;
	const VIDEO_FTR_OBJ_ATTR_S *attr = vftr_list->obj_attr;

	MPI_IVA_OBJ_LIST_S list;
	list.obj_num = src_list->obj_num;
	memcpy(list.obj, src_list->obj, sizeof(MPI_IVA_OBJ_ATTR_S) * src_list->obj_num);

	if (src_idx.value != dst_idx.value) {
		rescaleOdRes(src_rect, dst_rect, src_roi, dst_roi, &list);
	}

	for (i = 0; i < list.obj_num; i++) {
		int b = (i < 5) ? (i * 50) : 255;
		int r = (i < 5) ? 0 : (i * 50);
		int g = 255 / (i + 1);
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<RECT RECT=\"%d %d %d %d\" SZ=\"2\" LNCR=\"%d %d %d\"/>"
		                  "<TEXT TEXT=\"%d %s\" SZ=\"1\" LOC=\"%d %d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"lncr\":[%d,%d,%d]}},"
		                  "{\"text\":{\"text\":\"%d %s\",\"sz\":1,\"loc\":[%d,%d]}},",
#endif /* IVA_FORMAT_XML */
		                  list.obj[i].rect.sx, list.obj[i].rect.sy, list.obj[i].rect.ex, list.obj[i].rect.ey, b,
		                  g, r, i, attr[i].cat, list.obj[i].rect.sx, list.obj[i].rect.sy);

		int sx = 0, ex = 0, sy = 0, ey = 0;
		int mvx = list.obj[i].mv.x;
		int mvy = list.obj[i].mv.y;
		if (mvx != 0) {
			if (mvx > 0) {
				sx = list.obj[i].rect.ex;
				ex = sx;
				sy = list.obj[i].rect.sy;
				ey = list.obj[i].rect.ey;
			} else {
				sx = list.obj[i].rect.sx;
				ex = sx;
				sy = list.obj[i].rect.sy;
				ey = list.obj[i].rect.ey;
			}
			mvx = abs(mvx);
			offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
			                  "<LINE LINE=\"%d %d %d %d\" SZ=\"%d\" LNCR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
			                  "{\"line\":{\"line\":[%d,%d,%d,%d],\"sz\":%d,\"lncr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
			                  sx, sy, ex, ey, clip((2 + mvx / 10), 2, 10), b, g, r);
		}
		if (mvy != 0) {
			if (mvy > 0) {
				sx = list.obj[i].rect.sx;
				ex = list.obj[i].rect.ex;
				sy = list.obj[i].rect.ey;
				ey = list.obj[i].rect.ey;
			} else {
				sx = list.obj[i].rect.sx;
				ex = list.obj[i].rect.ex;
				sy = list.obj[i].rect.sy;
				ey = list.obj[i].rect.sy;
			}
			mvy = abs(mvy);
			offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
			                  "<LINE LINE=\"%d %d %d %d\" SZ=\"%d\" LNCR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
			                  "{\"line\":{\"line\":[%d,%d,%d,%d],\"sz\":%d,\"lncr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
			                  sx, sy, ex, ey, clip((2 + mvy / 10), 2, 10), b, g, r);
		}
	}
	return offset;
}

static int getExpoMsg(const MPI_CHN_ATTR_S *chn_attr, const MPI_EXPOSURE_INFO_S *info, char *str)
{
	/* Alarm Color determination */
	int offset = 0;
	int x = 50;
	int y = chn_attr->res.height - 500;
	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<TEXT TEXT=\"inttime:%d\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"sensor_gain:%d\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"isp_gain:%d\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"sys_gain:%d\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"iso:%d\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"fps:%.2f\" LOC=\"%d %d\" SZ=\"1\"/>"
	                  "<TEXT TEXT=\"luma_avg:%d\" LOC=\"%d %d\" SZ=\"1\"/>",
#else /* IVA_FORMAT_JSON */
	                  "{\"text\":{\"text\":\"inttime:%d\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"sensor_gain:%d\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"isp_gain:%d\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"sys_gain:%d\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"iso:%d\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"fps:%.2f\",\"loc\":[%d,%d],\"sz\":1}},"
	                  "{\"text\":{\"text\":\"luma_avg:%d\",\"loc\":[%d,%d],\"sz\":1}},",
#endif /* IVA_FORMAT_XML */
	                  info->inttime, x, y, info->sensor_gain, x, y + 50, info->isp_gain, x, y + 100, info->sys_gain,
	                  x, y + 150, info->iso, x, y + 200, info->fps, x, y + 250, info->luma_avg, x, y + 300);
	return offset;
}

static int getDebugMsg(const DEBUG_ITEM_LIST_S *list, char *data)
{
	int offset = 0;
	int i;
	for (i = 0; i < list->num; i++) {
		const DEBUG_ITEM_S *item = &list->item[i];
		if (offset >= DEBUG_BUFFER_LIM)
			break;
		switch (item->type) {
		case DEBUG_LINE:
			offset += sprintf(&data[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
			                  "<LINE LINE=\"%d %d %d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
			                  "{\"line\":{\"line\":[%d,%d,%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
			                  item->line.pts.sx, item->line.pts.sy, item->line.pts.ex, item->line.pts.ey,
			                  item->line.width, item->line.color[0], item->line.color[1],
			                  item->line.color[2]);
			break;
		case DEBUG_RECT:
			offset += sprintf(&data[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
			                  "<RECT RECT=\"%d %d %d %d\" SZ=\"%d\" LNCR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
			                  "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":%d,\"lncr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
			                  item->rect.pts.sx, item->rect.pts.sy, item->rect.pts.ex, item->rect.pts.ey,
			                  item->rect.line_width, item->rect.line_color[0], item->rect.line_color[1],
			                  item->rect.line_color[2]);
			break;
		case DEBUG_TEXT:
			offset +=
			        sprintf(&data[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
			                "<TEXT TEXT=\"%s\" LOC=\"%d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
			                "{\"text\":{\"text\":\"%s\",\"loc\":[%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
			                &item->text.str[0], (INT32)item->text.pt.x, (INT32)item->text.pt.y,
			                item->text.size, item->text.color[0], item->text.color[1], item->text.color[2]);
			break;
		case DEBUG_STR:
			offset += sprintf(&data[offset], "%s", item->str.str);
			break;
		default:
			break;
		}
	}
	return offset;
}

static int VIDEO_VDBG_transMdRes(AVFTR_MD_CTX_S *vftr_md_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                 const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                 const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_md_ctx, VIDEO_VDBG_CTX_MD);
	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_md_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_MD)) {
		VFTR_MD_STATUS_S *stat = &vftr_md_ctx[enable_idx].md_res[buf_idx];
		return getMdMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, stat, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transEfRes(AVFTR_EF_CTX_S *vftr_ef_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                 const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                 const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_ef_ctx, VIDEO_VDBG_CTX_EF);
	if (enable_idx < 0) {
		return 0;
	}
	if (vftr_ef_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_EF)) {
		VFTR_EF_STATUS_S *list = &vftr_ef_ctx[enable_idx].ef_res[buf_idx];
		return getEfMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, list, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transAroiRes(AVFTR_AROI_CTX_S *vftr_aroi_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                   const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                   const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_aroi_ctx, VIDEO_VDBG_CTX_AROI);
	if (enable_idx < 0)
		return 0;
	if (vftr_aroi_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_AROI)) {
		VFTR_AROI_STATUS_S *aroi_res = &vftr_aroi_ctx[enable_idx].aroi_res[buf_idx];
		MPI_RECT_POINT_S *aroi_tar = &vftr_aroi_ctx[enable_idx].tar[buf_idx];
		return getAroiMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, aroi_res, aroi_tar, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transFldRes(AVFTR_FLD_CTX_S *vftr_fld_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                  const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                  const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_fld_ctx, VIDEO_VDBG_CTX_FLD);
	if (enable_idx < 0)
		return 0;
	if (vftr_fld_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_FLD)) {
		AVFTR_FLD_STATUS_S *stat = &vftr_fld_ctx[enable_idx].fld_res[buf_idx];
		return getFldMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, stat, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transLdRes(AVFTR_LD_CTX_S *vftr_ld_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                 const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                 const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_ld_ctx, VIDEO_VDBG_CTX_LD);
	if (enable_idx < 0)
		return 0;
	if (vftr_ld_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_LD)) {
		AVFTR_LD_STATUS_S *res = &vftr_ld_ctx[enable_idx].ld_res[buf_idx];
		return getLdMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, res, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transTdRes(AVFTR_TD_CTX_S *vftr_td_ctx, MPI_WIN src_idx, MPI_WIN dst_idx,
                                 const MPI_RECT_S *src_rect __attribute__((unused)),
                                 const MPI_RECT_S *dst_rect __attribute__((unused)),
                                 const MPI_RECT_S *src_roi __attribute__((unused)),
                                 const MPI_RECT_S *dst_roi __attribute__((unused)), char *str, int buf_idx,
                                 int vdbg_ctx)
{
	int ret = 0;
	int enable_idx = findIvaCtx(src_idx, vftr_td_ctx, VIDEO_VDBG_CTX_TD);
	if (enable_idx < 0)
		return 0;
	if (vftr_td_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_TD)) {
		VFTR_TD_STATUS_S *td_result = &vftr_td_ctx[enable_idx].td_res[buf_idx];
		MPI_CHN chn_id = MPI_VIDEO_CHN(dst_idx.dev, dst_idx.chn);
		MPI_CHN_ATTR_S chn_attr;
		ret = MPI_DEV_getChnAttr(chn_id, &chn_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get channel attr on chn %u.", chn_id.chn);
			return 0;
		}
		return getTdMsg(&chn_attr, td_result->alarm, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transRmsRes(VIDEO_RMS_CTX_S *vftr_rms_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                                  const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_rms_ctx, VIDEO_VDBG_CTX_RMS);
	if (enable_idx < 0)
		return 0;
	if (vftr_rms_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_RMS)) {
		MPI_IVA_RMS_REG_LIST_S *reg_list = &vftr_rms_ctx[enable_idx].reg_list[buf_idx];
		MPI_RECT_POINT_S *roi = vftr_rms_ctx[enable_idx].roi;
		return getRmsMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, roi, reg_list, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transOdRes(VIDEO_OD_CTX_S *vftr_od_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                                 const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx, int vdbg_ctx)
{
	int enable_idx = findIvaCtx(src_idx, vftr_od_ctx, VIDEO_VDBG_CTX_OD);
	if (enable_idx < 0)
		return 0;
	if (vftr_od_ctx[enable_idx].en && (vdbg_ctx & VIDEO_VDBG_OD)) {
		VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx[enable_idx].ol[buf_idx];
		return getOdMsg(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, obj_list, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_transExpoRes(VIDEO_VDBG_CTX_S *vftr_vdbg_ctx, MPI_WIN src_idx __attribute__((unused)),
                                   MPI_WIN dst_idx, const MPI_RECT_S *src_rect __attribute__((unused)),
                                   const MPI_RECT_S *dst_rect __attribute__((unused)),
                                   const MPI_RECT_S *src_roi __attribute__((unused)),
                                   const MPI_RECT_S *dst_roi __attribute__((unused)), char *str,
                                   int buf_idx __attribute__((unused)), int vdbg_ctx)
{
	if (vftr_vdbg_ctx->en && (vdbg_ctx & VIDEO_VDBG_EXPO)) {
		int ret = 0;
		MPI_EXPOSURE_INFO_S info;
		MPI_PATH path_idx = MPI_INPUT_PATH(dst_idx.dev, 0);
		ret = MPI_queryExposureInfo(path_idx, &info);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get exposure info on dev%d. err: %d", dst_idx.dev, ret);
			return 0;
		}
		MPI_CHN chn_id = MPI_VIDEO_CHN(dst_idx.dev, dst_idx.chn);
		MPI_CHN_ATTR_S chn_attr;
		ret = MPI_DEV_getChnAttr(chn_id, &chn_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get channel attr on chn%u. err: %d", chn_id.chn, ret);
			return 0;
		}
		return getExpoMsg(&chn_attr, &info, str);
	} else {
		return 0;
	}
}

static int VIDEO_VDBG_getDebugRes(int buf_idx)
{
#ifdef VIDEO_DEBUG_MODULE
	int sz = 0;
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;
	if (vftr_vdbg_ctx->ctx & VIDEO_VDBG_DEBUG) {
		char *data = &vftr_vdbg_ctx->data[buf_idx][0];
#ifdef VIDEO_VDBG_DEBUG_
		UINT32 state;
		DEBUG_getComponentState(DEBUG_DIP, DEBUG_DIP_AE, &state);
		printf("[VDBG] IVA AE state: %d\n", state);
#endif /* !VIDEO_VDBG_DEBUG_ */
		DEBUG_ITEM_LIST_S list;
		DEBUG_getList(&list);
		DEBUG_rmLockAll();
		sz = getDebugMsg(&list, data);
		vftr_vdbg_ctx->data_len[buf_idx] = sz;
	}
	VDBG_DEB("exit\n");
	return sz;
#else /* VIDEO_DEBUG_MODULE */
	return 0;
#endif /* !VIDEO_DEBUG_MODULE */
}

static int VIDEO_VDBG_transDebugRes(VIDEO_VDBG_CTX_S *vftr_vdbg_ctx, char *data, int buf_idx, int vdbg_ctx)
{
#ifdef VIDEO_DEBUG_MODULE
	int sz = 0;
	VDBG_DEB("enter\n");
	if (vdbg_ctx & VIDEO_VDBG_DEBUG) {
		char *src_data = &vftr_vdbg_ctx->data[buf_idx][0];
		sz = sprintf(data, "%s", src_data);
	}
	VDBG_DEB("exit\n");
	return sz;
#else /* VIDEO_DEBUG_MODULE */
	return 0;
#endif /* !VIDEO_DEBUG_MODULE */
}

INT32 writeLine(char *data, DEBUG_ITEM_S *item)
{
	INT32 buffer_size = 0;
	DEBUG_LINE_S *line = &item->line;
	buffer_size = sprintf(data,
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                      "<LINE LINE=\"%d %d %d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                      "{\"line\":{\"line\":[%d,%d,%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                      line->pts.sx, line->pts.sy, line->pts.ex, line->pts.ey, line->width, line->color[0],
	                      line->color[1], line->color[2]);
	return buffer_size;
}

INT32 writeRect(char *data, DEBUG_ITEM_S *item)
{
	INT32 buffer_size = 0;
	DEBUG_RECT_S *rect = &item->rect;
	buffer_size = sprintf(data,
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                      "<RECT RECT=\"%d %d %d %d\" SZ=\"%d\" LNCR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                      "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":%d,\"lncr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                      rect->pts.sx, rect->pts.sy, rect->pts.ex, rect->pts.ey, rect->line_width,
	                      rect->line_color[0], rect->line_color[1], rect->line_color[2]);
	return buffer_size;
}

INT32 writeText(char *data, DEBUG_ITEM_S *item)
{
	INT32 buffer_size = 0;
	DEBUG_TEXT_S *text = &item->text;
	buffer_size =
	        sprintf(data,
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                "<TEXT TEXT=\"%s\" LOC=\"%d %d\" SZ=\"%d\" CR=\"%d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
	                "{\"text\":{\"text\":\"%s\",\"loc\":[%d,%d],\"sz\":%d,\"cr\":[%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                text->str, text->pt.x, text->pt.y, text->size, text->color[0], text->color[1], text->color[2]);
	return buffer_size;
}

INT32 writeStr(char *data, DEBUG_ITEM_S *item)
{
	INT32 buffer_size = 0;
	DEBUG_STR_S *str = &item->str;
	buffer_size = sprintf(data, "%s", str->str);
	return buffer_size;
}

#endif /* !AVFTR_VDBG */

/**
 * @brief Init and register debug module for vdbg
 * @param[in] none
 * @see AVFTR_VDBG_debugEnable()
 * @retval execution result
 */
int AVFTR_VDBG_debugInit(void)
{
#ifdef AVFTR_VDBG
#ifdef VIDEO_DEBUG_MODULE
	INT32 ret = 0;
#ifdef VIDEO_DEBUG_MODULE
	ret |= DEBUG_registerWriteFunc(DEBUG_LINE, writeLine);
	ret |= DEBUG_registerWriteFunc(DEBUG_RECT, writeRect);
	ret |= DEBUG_registerWriteFunc(DEBUG_TEXT, writeText);
	ret |= DEBUG_registerWriteFunc(DEBUG_STR, writeStr);

	ret |= DEBUG_register(DEBUG_AVMAIN, DEBUG_AVMAIN_AROI);
	ret |= DEBUG_register(DEBUG_MPP_IVA, DEBUG_MPP_IVA_OD);
	ret |= DEBUG_register(DEBUG_DIP, DEBUG_DIP_AE);
	if (ret) {
		avftr_log_err("Cannot init vdbg debug\n");
	}
#endif /* !VIDEO_DEBUG_MODULE */
	return ret;
#endif /* !VIDEO_DEBUG_MODULE */
#else
	return 0;
#endif /* !AVFTR_VDBG */
}

/**
 * @brief Enable debug module for vdbg
 * @param[in]  none
 * @see VIDEO_FTR_getVdbgRes()
 * @retval execution result
 */
int AVFTR_VDBG_debugEnable(void)
{
#ifdef VIDEO_DEBUG_MODULE
	int ret = DEBUG_enable();
	return ret;
#else
	return 0;
#endif /* !AVFTR_VDBG */
}

int AVFTR_VDBG_debugExit(void)
{
#ifdef VIDEO_DEBUG_MODULE
	int ret;
	ret = DEBUG_disable();
	if (ret) {
		avftr_log_err("Cannot disable vdbg debug");
	}
	return ret;
#else
	return 0;
#endif /* !AVFTR_VDBG */
}

/**
 * @brief Get results of video vdbg results.
 * @param[in]  vdbg_ctx        VDBG context info.
 * @param[in]  src_idx         source video window idx.
 * @param[in]  dst_idx         destination video window idx
 * @param[in]  src_rect        source MPI_RECT_S
 * @param[in]  dst_rect        destination MPI_RECT_S
 * @param[in]  str             string buffer
 * @param[in]  buf_idx         ring buffer idx
 * @param[out] dataoffset      written data offset
 * @see VIDEO_FTR_getVdbgRes()
 * @retval length of MSG format
 */
int VIDEO_FTR_transVdbgRes(void *vftr_ctx_ptr, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                           const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx)
{
/* TODO support Feature window mode */
#ifdef AVFTR_VDBG
	AVFTR_VIDEO_CTX_S *vftr_ctx = (AVFTR_VIDEO_CTX_S *)vftr_ctx_ptr;
	VIDEO_VDBG_CTX_S *vdbg_ctx = &vftr_ctx->vdbg_ctx;
	if (vdbg_ctx->en) {
		char *data = str; //(char *)&vftr_vdbg_ctx->data[buf_idx][0];
		int dataoffset = 0;
		dataoffset += sprintf(&data[dataoffset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                      "<VDBG>");
#else /* IVA_FORMAT_JSON */
		                      "\"vdbg\":[ ");
#endif /* IVA_FORMAT_XML */
		dataoffset +=
		        VIDEO_VDBG_transMdRes(vftr_ctx->md_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset +=
		        VIDEO_VDBG_transEfRes(vftr_ctx->ef_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset += VIDEO_VDBG_transAroiRes(vftr_ctx->aroi_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset],
		                                      buf_idx, vdbg_ctx->ctx);
		dataoffset += VIDEO_VDBG_transFldRes(vftr_ctx->fld_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi,
		                                     dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset +=
		        VIDEO_VDBG_transLdRes(vftr_ctx->ld_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset +=
		        VIDEO_VDBG_transTdRes(vftr_ctx->td_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset += VIDEO_VDBG_transRmsRes(vftr_ctx->rms_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset],
		                                     buf_idx, vdbg_ctx->ctx);
		/* See VIDEO_FTR_getPdRes */
		//dataoffset += VIDEO_FTR_getPdRes(dev_idx, chn_idx, &obj_list, (char*)&data[dataoffset]);
		dataoffset +=
		        VIDEO_VDBG_transOdRes(vftr_ctx->od_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
		dataoffset += VIDEO_VDBG_transExpoRes(vdbg_ctx, src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, (char *)&data[dataoffset],
		                                      buf_idx, vdbg_ctx->ctx);
#ifdef VIDEO_DEBUG_MODULE
		dataoffset += VIDEO_VDBG_transDebugRes(vdbg_ctx, (char *)&data[dataoffset], buf_idx, vdbg_ctx->ctx);
#endif /* !VIDEO_DEBUG_MODULE */

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		dataoffset += sprintf(&data[dataoffset], "</VDBG>");
#else /* IVA_FORMAT_JSON */
		dataoffset += (sprintf(&data[dataoffset - 1], "],") - 1);
#endif /* IVA_FORMAT_XML */
		return dataoffset;
	}
#endif /* !AVFTR_VDBG */
	return 0;
}

/**
 * @brief Get predefined VDBG format for Multiplayer.
 * @param[in] idx       MPI_WIN idx
 * @param[in] buf_idx   ring buffer idx
 * @see VIDEO_FTR_transVdbgRes()
 * @retval execution result
 */
int VIDEO_FTR_getVdbgRes(MPI_WIN idx __attribute__((unused)), int buf_idx)
{
	VIDEO_VDBG_CTX_S *vftr_vdbg_ctx = &vftr_res_shm->vdbg_ctx;

	if (vftr_vdbg_ctx->en) {
		VIDEO_VDBG_getDebugRes(buf_idx);
		// Add enable check?
		return 0;
	}
	return 0;
}
