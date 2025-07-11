#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_adv_img_pref.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(AdvImgPref)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_ADV_IMG_PREF);
	/* parse list of object from Json Object */
	parse_adv_img_pref(&g_conf.adv_img, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("adv_img_pref.image_mode:%d. night_mode:%d", g_conf.adv_img.image_mode,
	                 g_conf.adv_img.night_mode);

	return 0;
}

int WRITE_DB(AdvImgPref)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(AdvImgPref)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_ADV_IMG_PREF_S *adv_img_pref = (AGTX_ADV_IMG_PREF_S *)data;
	if (len != sizeof(AGTX_ADV_IMG_PREF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_ADV_IMG_PREF_S));
		return -EINVAL;
	}

	if (adv_img_pref->night_mode < AGTX_NIGHT_MODE_OFF || adv_img_pref->night_mode > AGTX_NIGHT_MODE_AUTOSWITCH ||
	    adv_img_pref->ir_led_mode < AGTX_IR_LED_MODE_OFF || adv_img_pref->ir_led_mode > AGTX_IR_LED_MODE_AUTO ||
	    adv_img_pref->image_mode < AGTX_IMAGE_MODE_COLOR || adv_img_pref->image_mode > AGTX_IMAGE_MODE_AUTO ||
	    adv_img_pref->icr_mode < AGTX_ICR_MODE_OFF || adv_img_pref->icr_mode > AGTX_ICR_MODE_AUTO ||
	    adv_img_pref->wdr_en < 0 || adv_img_pref->wdr_en > 1) {
		avmain2_log_err("Advanced image preference out of range");
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.adv_img, adv_img_pref, sizeof(AGTX_ADV_IMG_PREF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, ADV_IMG_PREF, adv_img_pref);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.adv_img, sizeof(AGTX_CMD_ADV_IMG_PREF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler adv_img_pref_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_ADV_IMG_PREF, AdvImgPref);

__attribute__((constructor)) void registerAdvImgPref(void)
{
	HANDLERS_registerHandlers(&adv_img_pref_ops);
}