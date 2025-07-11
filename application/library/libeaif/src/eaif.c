/**
* @cond
*
* code fragment skipped by Doxygen.
*/

#include "eaif.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "mpi_index.h"
#include "mpi_sys.h"
#include "mpi_base_types.h"
#include "mpi_errno.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_iva.h"

#include "app.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"
#include "vftr_dump.h"
#include "eaif_dump_define.h"

int g_eaif_verbose = 0;
int g_eaif_inf_debug = 0;

#define EAIF_SLEEP_TIME (10 * 1000)
#define EAIF_SNAPSHOT_CHN 3
#define EAIF_TH_NAME "eaif"

#define EXPAND_ST(infost)                                                                                   \
	{                                                                                                   \
		eaif_log_info("info->local_status %d (%s) [%d %d %d %d]\n", infost.obj_cnt,                 \
		              infost.obj_attr_ex[0].basic.category[0], infost.obj_attr_ex[0].basic.rect.sx, \
		              infost.obj_attr_ex[0].basic.rect.sy, infost.obj_attr_ex[0].basic.rect.ex,     \
		              infost.obj_attr_ex[0].basic.rect.ey);                                         \
	}

#define RUN_OR_EXIT(cb, obj)         \
	{                            \
		if (cb)              \
			cb(obj);     \
		else                 \
			return NULL; \
	}

#define RUN_OR_CONT(cb, obj)      \
	{                         \
		if (cb)           \
			cb(obj);  \
		else              \
			continue; \
	}

#define RUN(cb, ...)                     \
	{                                \
		if (cb)                  \
			cb(__VA_ARGS__); \
	}

#define RUN_RET(cb, obj, _ret)          \
	{                               \
		if (cb)                 \
			_ret = cb(obj); \
	}

int debugEmptyCb(void *ctx __attribute__((unused)))
{
	return 0;
}

// conflict with avftr_common
int getMpiWinSize(const MPI_WIN idx, MPI_SIZE_S *res)
{
	UINT8 dev_idx = MPI_GET_VIDEO_DEV(idx);
	UINT8 chn_idx = MPI_GET_VIDEO_CHN(idx);
	UINT8 win_idx = MPI_GET_VIDEO_WIN(idx);
	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	MPI_CHN_LAYOUT_S layout_attr;
	int ret;
	int i;

	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != 0) {
		eaif_log_err("Query CHN(%d) state failed. err: %d", chn_idx, ret);
		return ret;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		eaif_log_err("CHN(%d) is not added.", chn_idx);
		return -ENODEV;
	}

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != 0) {
		eaif_log_err("Get CHN(%d) layout attributes failed. err: %d", chn_idx, ret);
		return ret;
	}

	/* FIXME: check window state */
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			res->width = layout_attr.window[i].width;
			res->height = layout_attr.window[i].height;
			break;
		}
	}

	if (i == layout_attr.window_num) {
		eaif_log_err("WIN(%d, %d, %d) does not exist.", dev_idx, chn_idx, win_idx);
		return -EINVAL;
	}

	return 0;
}

static void genEaifDetect(const MPI_IVA_OBJ_LIST_S *ol __attribute__((unused)), const EaifStatusInternal *src,
                          EAIF_STATUS_S *dst)
{
	int i;
	//MPI_IVA_OBJ_ATTR_S *od = NULL;
	EAIF_OBJ_ATTR_S *obj = NULL;
	const EaifObjAttrEx *eobj = NULL;

	dst->obj_cnt = src->obj_cnt;
	for (i = 0; (UINT32)i < src->obj_cnt; i++) {
		obj = &dst->obj_attr[i];
		eobj = &src->obj_attr_ex[i];
		*obj = eobj->basic;
	}
}

static void genEaifClassify(const MPI_IVA_OBJ_LIST_S *ol, const EaifStatusInternal *src, EAIF_STATUS_S *dst)
{
	int i, j, set;
	EAIF_OBJ_ATTR_S *obj = NULL;
	const MPI_IVA_OBJ_ATTR_S *od = NULL;
	const EaifObjAttrEx *eobj = NULL;

	dst->obj_cnt = ol->obj_num;
	for (i = 0; i < ol->obj_num; i++) {
		od = &ol->obj[i];
		obj = &dst->obj_attr[i];
		set = 0;
		for (j = 0; (UINT32)j < src->obj_cnt; j++) {
			eobj = &src->obj_attr_ex[j];
			if (od->id == eobj->basic.id) {
				*obj = eobj->basic;
				set = 1;
				break;
			}
		}
		if (set == 0) {
			obj->id = od->id;
			obj->rect = od->rect;
			obj->label_num = 0;
		}
	}
}

static void genEaifRes(const EAIF_PARAM_S *p, const MPI_IVA_OBJ_LIST_S *obj_list, const EaifStatusInternal *src,
                       EAIF_STATUS_S *dst)
{
	int i;
	EAIF_OBJ_ATTR_S *obj = NULL;
	const MPI_IVA_OBJ_ATTR_S *od = NULL;
	if (src->server_reachable == EAIF_URL_REACHABLE) {
		/* TODO: Modify OD attr to support prob or other attri */
		if (p->api == EAIF_API_CLASSIFY || p->api == EAIF_API_CLASSIFY_CV ||
		    p->api == EAIF_API_HUMAN_CLASSIFY) {
			genEaifClassify(obj_list, src, dst);
		} else if (p->api == EAIF_API_DETECT || p->api == EAIF_API_FACEDET || p->api == EAIF_API_FACERECO) {
			genEaifDetect(obj_list, src, dst);
		}
	} else {
		if (p->api == EAIF_API_DETECT || p->api == EAIF_API_FACEDET) {
			dst->obj_cnt = 0;
		} else {
			for (i = 0; i < obj_list->obj_num; i++) {
				od = &obj_list->obj[i];
				obj = &dst->obj_attr[i];
				obj->id = od->id;
				obj->rect = od->rect;
				obj->label_num = 0;
			}
		}
	}

	dst->timestamp = obj_list->timestamp;
}

void *runEaif(void *inputs)
{
	/* 1. init param */
	/* 2. test url example get method */
	eaif_log_entry();

	//EaifCtx *ctx = EAIF_GET_CTX();
	EAIF_ALGO_STATUS_S *algo_obj = (EAIF_ALGO_STATUS_S *)inputs;
	EaifInfo *info = &algo_obj->info;
	int ret = 0;
	int req_ret = 0;
	info->req_sta.avail = 1;

	RUN_OR_EXIT(algo_obj->probe, algo_obj);

	RUN_OR_EXIT(algo_obj->init_buffer, algo_obj);

	/* 3. start loop */
	while (1) {
		ret = 0;

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		if (algo_obj->running == 0) {
			info->req_sta.val = 0;
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			break;
		}

		if (info->req_sta.append) {
			info->req_sta.avail = 0;

			pthread_mutex_lock(&algo_obj->lock);

			RUN(algo_obj->set_local_info, algo_obj);

			pthread_mutex_unlock(&algo_obj->lock);
			/* Do not send if no append request */

			RUN(algo_obj->set_payload, algo_obj);

			//eaif_log_debug("od: time:%s %s", info->payload.time_payload, info->payload.meta_payload);
			if ((info->mode == EAIF_MODE_REMOTE && info->payload.remote->data.size == 0) ||
			    (info->mode == EAIF_MODE_INAPP && info->payload.inapp->size == 0)) {
				info->req_sta.append = 0;
				sleep(EAIF_TRY_SLEEP_TIME);
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
				pthread_testcancel();
				continue;
			}

			// lock for state request state writer
			pthread_mutex_lock(&algo_obj->lock);
			info->req_sta.append = 0;
			info->req_sta.wait_res = 1;
			pthread_mutex_unlock(&algo_obj->lock);

			pthread_mutex_lock(&algo_obj->inf_lock);

			RUN_RET(algo_obj->send_request, algo_obj, ret);

			pthread_mutex_unlock(&algo_obj->inf_lock);

			info->agtx_resp.success = 0;

			pthread_mutex_lock(&algo_obj->lock);
			info->req_sta.wait_res = 0;
			if (!ret) {
				RUN_RET(algo_obj->decode_result, algo_obj, req_ret);

				//EXPAND_ST(info->local_status);
				RUN(algo_obj->debug, algo_obj);

				// lock for state request state writer
				info->req_sta.wait_res = 0;
				if (req_ret || info->agtx_resp.success == 0) {
					info->req_sta.respond = 0;
					eaif_log_debug("Decode failed: resp msg: %s", info->resp.data);
				} else {
					info->req_sta.respond = 1;
				}
				info->req_sta.recv = 0;
			}

			pthread_mutex_unlock(&algo_obj->lock);

			if (ret || info->agtx_resp.success == 0) {
				eaif_log_warn("Request failed from %s!", info->local_url);
				eaif_log_debug("resp: %s", info->resp.data);
				info->local_status.obj_cnt = 0;
				sleep(EAIF_TRY_SLEEP_TIME);

				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			}

			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

			pthread_mutex_lock(&algo_obj->lock);
			// Update the inference result only if not reset
			if (!info->req_sta.reset) {
				RUN(algo_obj->merge_result, algo_obj);
			} else {
				/* Do nothing */
			}
			info->req_sta.reset = 0;
			pthread_mutex_unlock(&algo_obj->lock);

			eaif_log_debug("resp: %s id:%d name:\"%s\" obj_cnt:%d->%d", info->resp.data,
			               info->local_status.obj_attr_ex[0].basic.id,
			               info->local_status.obj_attr_ex[0].basic.category[0], info->local_status.obj_cnt,
			               status->obj_cnt);
			info->req_sta.avail = 1;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		pthread_testcancel();
		usleep(EAIF_RUN_USLEEP);
	}
	eaif_log_exit();
	/* 4. if obj_list is appending to send request => send */
	return 0;
}

/**
 * @brief Activate EAIF running thread
 * @param[in]  instance        EAIF_INSTANCE_S pointer
 * @return The execution result.
 * @retval 0                           success.
 * @retval -EFAULT                     input pointer is null.
 * @retval -EINVAL                     unknown fail to create running thread.
 * @see EAIF_deactivate()
 */
int EAIF_activate(EAIF_INSTANCE_S *instance)
{
	eaif_log_entry();

	int ret = 0;

	if (!instance) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	MPI_WIN idx = algo_obj->idx;

	EaifInfo *info = &algo_obj->info;
	pthread_mutex_lock(&algo_obj->lock);
	if (algo_obj->running) {
		eaif_log_warn("EAIF is already running!");
		goto unlock;
	}

	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != 0) {
		eaif_log_warn("Query CHN(%d, %d) channel state failed. err: %d", idx.dev, idx.chn, ret);
		goto unlock;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		eaif_log_warn("CHN(%d, %d) is not added", idx.dev, idx.chn);
		goto unlock;
	}

	MPI_CHN_ATTR_S chn_attr;
	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != 0) {
		eaif_log_warn("Get CHN(%d, %d) layout attributes failed.", idx.dev, idx.chn);
		goto unlock;
	}

	char *verbose = getenv("EAIF_VERBOSE");
	if (verbose) {
		g_eaif_verbose = atoi(verbose);
		eaif_log_info("Environment variable EAIF_VERBOSE is detected: %d!", g_eaif_verbose);
	}

	char *capture = getenv("INF_CAP_PREFIX");
	if (capture) {
		g_eaif_inf_debug = 1;
		eaif_log_info("Environment variable INF_CAP_PREFIX is detected: %s!", capture);
	}

	algo_obj->info.mode = eaif_utilsDetermineMode(algo_obj->param.url);
	algo_obj->info.algo = &algo_obj->algo;

	if (algo_obj->info.mode == EAIF_MODE_INAPP) {
		strncpy(algo_obj->model.path, algo_obj->param.api_models[algo_obj->param.api], EAIF_MODEL_LEN);
	}

	initAppCb(algo_obj);

	getMpiWinSize(idx, &info->src_resoln);

	RUN(algo_obj->init_algo, algo_obj);

	RUN(algo_obj->init_module, algo_obj);

	algo_obj->running = 1;
	info->req_sta.val = 0;
	info->prev_obj_list.obj_num = 0;

	if ((ret = pthread_create(&algo_obj->tid_eaif, NULL, runEaif, (void *)algo_obj)) != 0) {
		eaif_log_warn("Cannot create EAIF thread for window 0x%x. err: %d", idx.value, ret);
		ret = -EINVAL;
		goto error;
	}
	char *name = EAIF_TH_NAME; // max. length is 16
	if ((ret = pthread_setname_np(algo_obj->tid_eaif, name)) != 0) {
		eaif_log_warn("Set EAIF thread name failed. err: %d", ret);
		goto error;
	}
	pthread_mutex_unlock(&algo_obj->lock);

	return 0;

error:
	RUN(algo_obj->exit_module, algo_obj);
	delAppCb(algo_obj);

unlock:
	pthread_mutex_unlock(&algo_obj->lock);
	eaif_log_exit();
	return ret;
}

/**
 * @brief Dectivate EAIF running thread
 * @param[in]  instance        EAIF_INSTANCE_S pointer
 * @return The execution result.
 * @retval 0                           success.
 * @retval -ENODEV                     EAIF thread is not runnning.
 * @retval -EFAULT                     Fail to join thread/ input pointer is null.
 * @see EAIF_activate()
 */
int EAIF_deactivate(EAIF_INSTANCE_S *instance)
{
	eaif_log_entry();

	if (!instance) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	EaifInfo *info = &algo_obj->info;
	MPI_WIN idx = algo_obj->idx;
	VOID *res;
	int ret = 0;

	if (algo_obj->running == 0) {
		eaif_log_warn("EAIF is not running at win:0x%x!", idx.value);
		return -ENODEV;
	}

	ret = pthread_cancel(algo_obj->tid_eaif);
	if (ret != 0) {
		eaif_log_err("Cancel thread to run eaif failed. %s", strerror(ret));
		return -EFAULT;
	}

	ret = pthread_join(algo_obj->tid_eaif, &res);
	if (ret != 0) {
		eaif_log_err("Join thread to run eaif failed. %s", strerror(ret));
		return -EFAULT;
	}

	if (res == PTHREAD_CANCELED) {
		eaif_log_debug("Edge AI assisted feature thread was canceled.");
	} else {
		eaif_log_err("Edge AI assisted feature wasn't canceled (shouldn't happen!).");
		return -EFAULT;
	}

	RUN(algo_obj->exit_module, algo_obj);

	pthread_mutex_lock(&algo_obj->lock);
	algo_obj->running = 0;
	info->req_sta.val = 0;

	RUN(algo_obj->release_buffer, algo_obj);

	pthread_mutex_unlock(&algo_obj->lock);

	delAppCb(algo_obj);

	eaif_log_exit();

	return ret;
}

/**
 * @brief Reset EAIF internal status.
 * @details Clear EAIF internal tracking table and set corresponding request state.
 *          Instance also drops the current inference result if it's inferencing.
 * @param[out]  instance       EAIF_INSTANCE_S pointer.
 * @return The execution result.
 * @retval 0				   success.
 * @retval -EFAULT			   input pointer is null.
*/
int EAIF_reset(EAIF_INSTANCE_S *instance)
{
	if (!instance) {
		eaif_log_err("Pointer to the input of EAIF_reset() should not be NULL.");
		return -EFAULT;
	}

	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	MPI_IVA_OBJ_LIST_S obj_list = { 0 };
	EaifInfo *info = &algo_obj->info;
	EaifStatusInternal *status = &algo_obj->status;
	MPI_WIN idx = algo_obj->idx;

	pthread_mutex_lock(&algo_obj->lock);
	if (algo_obj->running == 0) {
		eaif_log_warn("Eaif on window 0x%x is not running!", idx.value);
		pthread_mutex_unlock(&algo_obj->lock);
		return 0;
	}

	// clear tracking record
	eaif_updateObjAttr(&obj_list, 0, &algo_obj->algo, status);
	info->req_sta.append = 0;
	if (info->req_sta.wait_res) {
		info->req_sta.reset = 1;
	}
	pthread_mutex_unlock(&algo_obj->lock);
	return 0;
}

/**
 * @brief Send request to eaif module waiting list and get most updated result.
 * @param[in]  instance        EAIF_INSTANCE_S pointer
 * @param[in]  obj_list        MPI object list
 * @param[out]  status         most updated result from inference module
 * @return The execution result.
 * @retval 0                           success.
 * @retval -EFAULT                     input pointers ars null.
 * @see EAIF_testRequestV2()
 */
int EAIF_testRequest(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status)
{
	eaif_log_entry();

	if (!instance || !obj_list || !status) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	EaifInfo *info = &algo_obj->info;
	EaifStatusInternal *pstatus = &algo_obj->status;
	MPI_WIN idx = algo_obj->idx;

	pthread_mutex_lock(&algo_obj->lock);
	if (algo_obj->running == 0) {
		eaif_log_warn("Eaif on window 0x%x is not running!", idx.value);
		pthread_mutex_unlock(&algo_obj->lock);
		return 0;
	}

	info->req_sta.append = 0;

	if (algo_obj->update)
		info->req_sta.append = algo_obj->update(algo_obj, obj_list);

	if (!info->req_sta.append)
		info->req_sta.recv = 1;

	if (status) {
		/* copy current status */
		if (info->req_sta.respond) {
			eaif_log_debug("status cnt:%d (%s) [%d %d %d %d]\n", pstatus->obj_cnt,
			               pstatus->obj_attr_ex[0].basic.category[0], pstatus->obj_attr_ex[0].basic.rect.sx,
			               pstatus->obj_attr_ex[0].basic.rect.sy, pstatus->obj_attr_ex[0].basic.rect.ex,
			               pstatus->obj_attr_ex[0].basic.rect.ey);
			eaif_cpyRespStatus(pstatus, status);
			info->req_sta.respond = 0;
			info->req_sta.recv = 1;
		} else if (info->req_sta.recv) {
			eaif_cpyRespStatus(pstatus, status);
		} else {
			/* do nothing */
		}

		if (info->req_sta.respond || info->req_sta.recv) {
			VFTR_dumpStart();
			VFTR_dumpWithJiffies(obj_list, sizeof(*obj_list),
			                     COMPOSE_EAIF_FLAG_VER_1(MPI_IVA_OBJ_LIST_S, MPI_IVA_OBJ_LIST_S),
			                     obj_list->timestamp);
			VFTR_dumpWithJiffies(status, sizeof(*status),
			                     COMPOSE_EAIF_FLAG_VER_1(EAIF_STATUS_S, EAIF_INSTANCE_S),
			                     status->timestamp);
			VFTR_dumpEnd();
		}
	}

	pthread_mutex_unlock(&algo_obj->lock);

	eaif_log_exit();

	return 0;
}

/**
 * @brief Send request to eaif module waiting list and get most updated result.
 * @param[in]  instance        EAIF_INSTANCE_S pointer
 * @param[in]  obj_list        MPI object list
 * @param[out]  status         most updated result and merge to obj list deps on params.
 * @return The execution result.
 * @retval 0                           success.
 * @retval -EFAULT                     input pointers ars null.
 * @see EAIF_testRequestV2()
 */
int EAIF_testRequestV2(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status)
{
	eaif_log_entry();

	if (!instance || !obj_list) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	EaifInfo *info = &algo_obj->info;
	EaifStatusInternal *pstatus = &algo_obj->status;
	const EAIF_PARAM_S *p = &algo_obj->param;
	MPI_WIN idx = algo_obj->idx;

	pthread_mutex_lock(&algo_obj->lock);
	if (algo_obj->running == 0) {
		eaif_log_warn("Eaif on window 0x%x is not running!", idx.value);
		pthread_mutex_unlock(&algo_obj->lock);
		return 0;
	}

	info->req_sta.append = 0;
	if (algo_obj->update)
		info->req_sta.append = algo_obj->update(algo_obj, obj_list);

	if (!info->req_sta.append)
		info->req_sta.recv = 1;

	if (status) {
		/* copy current status */
		if (info->req_sta.respond) {
			genEaifRes(p, obj_list, pstatus, status);
			info->req_sta.respond = 0;
			info->req_sta.recv = 1;
		} else if (info->req_sta.recv) {
			genEaifRes(p, obj_list, pstatus, status);
		} else {
			// do nothing
		}

		if (info->req_sta.respond || info->req_sta.recv) {
			VFTR_dumpStart();
			VFTR_dumpWithJiffies(obj_list, sizeof(*obj_list),
			                     COMPOSE_EAIF_FLAG_VER_1(MPI_IVA_OBJ_LIST_S, MPI_IVA_OBJ_LIST_S),
			                     obj_list->timestamp);
			VFTR_dumpWithJiffies(status, sizeof(*status),
			                     COMPOSE_EAIF_FLAG_VER_1(EAIF_STATUS_S, EAIF_INSTANCE_S),
			                     status->timestamp);
			VFTR_dumpEnd();
		}
	}

	pthread_mutex_unlock(&algo_obj->lock);

	eaif_log_exit();

	return 0;
}

/**
 * @brief create EAIF instance.
 * @param[in]  idx         video window index.
 * @see EAIF_deleteInstance
 * @retval NULL                    No more space to register idx / malloc EAIF instance failed
 * @retval EAIF_INSTANCE_S*        success.
 */
EAIF_INSTANCE_S *EAIF_newInstance(MPI_WIN idx)
{
	eaif_log_entry();

	EAIF_INSTANCE_S *eaif_instance = (EAIF_INSTANCE_S *)malloc(sizeof(EAIF_INSTANCE_S));
	if (!eaif_instance) {
		eaif_log_err("Fail to allocate memory.");
		return NULL;
	}

	eaif_instance->algo_status = (EAIF_ALGO_STATUS_S *)calloc(1, sizeof(EAIF_ALGO_STATUS_S));
	if (!eaif_instance->algo_status) {
		free(eaif_instance);
		eaif_log_err("Fail to allocate memory.");
		return NULL;
	}

	*eaif_instance->algo_status = (EAIF_ALGO_STATUS_S) { 
		.lock = PTHREAD_MUTEX_INITIALIZER,
		.inf_lock = PTHREAD_MUTEX_INITIALIZER,
		.running = 0,
		.idx = idx,
		.param = { .target_idx = { { 0 } },
			.snapshot_width = 0,
			.snapshot_height = 0,
			.api = 0,
			.url = { "http://192.168.87.87:1234" },
			.pos_stop_count_th = 3,
			.pos_classify_period = 100,
			.neg_classify_period = 25,
			.obj_exist_classify_period = 0,
			.data_fmt = EAIF_DATA_JPEG,
			.api_models = {
				"face_reco",
				"detect",
				"classify",
				"classify_cv",
				"human_classify"
			},
		},
		.info = { 0 },
		.status = { 0 } 
	};

	eaif_log_exit();

	return eaif_instance;
}

/**
 * @brief Delete EAIF instance.
 * @param[in]  instance      EAIF instance to be deleted.
 * @see EAIF_addInstance
 * @retval 0                 success.
 * @retval -EFAULT           input variables are null
 */
int EAIF_deleteInstance(EAIF_INSTANCE_S **instance)
{
	eaif_log_entry();

	if (instance == NULL) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	if (*instance == NULL) {
		eaif_log_err("Pointer to the EAIF instance should not be NULL.");
		return -EFAULT;
	}

	if ((*instance)->algo_status == NULL) {
		eaif_log_err("Pointer to the EAIF algo instance should not be NULL.");
		return -EFAULT;
	}

	free((*instance)->algo_status);
	free(*instance);
	*instance = NULL;

	eaif_log_exit();

	return 0;
}

/**
 * @brief Get parameters of EAIF.
 * @param[in]  instance   EAIF instance.
 * @param[out] param      EAIF parameters.
 * @see EAIF_setParam
 * @retval 0              success.
 * @retval -EFAULT        input variables are null
 */
int EAIF_getParam(EAIF_INSTANCE_S *instance, EAIF_PARAM_S *param)
{
	eaif_log_entry();

	if (!instance || !param) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	pthread_mutex_lock(&instance->algo_status->lock);
	memcpy(param, &instance->algo_status->param, sizeof(EAIF_PARAM_S));
	pthread_mutex_unlock(&instance->algo_status->lock);

	eaif_log_exit();
	return 0;
}

/**
 * @brief Check parameters of EAIF.
 * @param[in]  param      EAIF parameters.
 * @see EAIF_setParam
 * @retval 0              success.
 * @retval -EFAULT        input variable is = null
 * @retval -EINVAL        input variable is invalid
 * @retval -ENODEV        video index, inapp target file not found.
 */
int EAIF_checkParam(const EAIF_PARAM_S *param)
{
	if (!param) {
		return -EFAULT;
	}

	return eaif_utilsCheckParam(param);
}

/**
 * @brief set parameters of EAIF.
 * @param[in]  instance   EAIF instance.
 * @param[in]  param      EAIF parameters.
 * @see EAIF_getParam
 * @retval 0              success.
 * @retval -EFAULT        inputs variables are null
 */
int EAIF_setParam(EAIF_INSTANCE_S *instance, const EAIF_PARAM_S *param)
{
	eaif_log_entry();

	if (!instance || !param) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	/* set param */
	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	EAIF_PARAM_S *local_param = &algo_obj->param;
	EaifStatusInternal *status = &algo_obj->status;

	int restart = 0;
	int running = algo_obj->running;

	// check for url, snapshot index, snapshot size, api method
	if (strcmp(local_param->url, param->url) || (local_param->target_idx.value != param->target_idx.value) ||
	    local_param->api != param->api || local_param->snapshot_width != param->snapshot_width ||
	    local_param->snapshot_height != param->snapshot_height || local_param->data_fmt != param->data_fmt ||
	    strncmp(local_param->api_models[param->api], param->api_models[param->api], EAIF_URL_CHAR_LEN)) {
		restart = 1 && running;
	}

	if (restart)
		EAIF_deactivate(instance);

	pthread_mutex_lock(&algo_obj->lock);
	memcpy(local_param, param, sizeof(EAIF_PARAM_S));
	status->obj_cnt = 0;
	pthread_mutex_unlock(&algo_obj->lock);

	if (restart)
		EAIF_activate(instance);

	eaif_log_exit();
	return 0;
}

/**
 * @brief Apply face utils for inapp eaif.
 * @param[in]  instance   EAIF instance.
 * @param[in]  param      EAIF parameters.
 * @see EAIF_setParam
 * @retval 0              success.
 * @retval -EFAULT        inputs variables are null
 * @retval -ENODEV        input files are invalid/ not found, modules not activated
 * @retval -EINVAL        onloaded model format is not compatible for face application
 */
int EAIF_applyFaceUtils(EAIF_INSTANCE_S *instance, const EAIF_PARAM_S *param)
{
	eaif_log_entry();

	if (!instance || !param) {
		eaif_log_err("Input args should not be NULL");
		return -EFAULT;
	}

	/* set param */
	EAIF_ALGO_STATUS_S *algo_obj = instance->algo_status;
	InferenceModel *model = &algo_obj->model;

	int running = algo_obj->running;

	// check for url, snapshot index, snapshot size, api method
	if (!running || algo_obj->info.mode != EAIF_MODE_INAPP) {
		eaif_log_err("EAIF inapp mode is not running!");
		return -ENODEV;
	}

	pthread_mutex_lock(&algo_obj->inf_lock);
	int ret = eaif_faceUtils(&param->inf_utils, model);
	pthread_mutex_unlock(&algo_obj->inf_lock);

	eaif_log_exit();
	return ret;
}
