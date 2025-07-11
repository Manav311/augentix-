#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cm_img_pref.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(ImgPref)(const char *path)
{
	struct json_object *ret_obj = NULL;

	ret_obj = get_db_record_obj(path, AGTX_CMD_IMG_PREF);
	/* parse list of object from Json Object */
	parse_img_pref(&g_conf.img, ret_obj);
	json_object_put(ret_obj);

	avmain2_log_info("img_pref anti_flicker:%d.", g_conf.img.anti_flicker);
	return 0;
}

int WRITE_DB(ImgPref)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

int APPLY(ImgPref)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	AGTX_IMG_PREF_S *img_pref = (AGTX_IMG_PREF_S *)data;
	if (len != sizeof(AGTX_IMG_PREF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_IMG_PREF_S));
		return -EINVAL;
	}

	if (img_pref->anti_flicker < AGTX_ANTI_FLICKER_50HZ || img_pref->anti_flicker > AGTX_ANTI_FLICKER_NUM) {
		avmain2_log_err("Image preference(anti_flicker) out of range: %d", img_pref->anti_flicker);
		return -EINVAL;
	}

	saveOldConftoTmp(&g_conf.img, img_pref, sizeof(AGTX_IMG_PREF_S));

	NodeId id = NONE;
	int ptr = 0;
	int ret = 0;

	id = IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, IMG_PREF, img_pref);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.img, sizeof(AGTX_CMD_IMG_PREF));
		return ret;
	}

	/*Also modify each window*/
	id = WIN_IMAGE_PREFERENCE;
	ptr = ((int)node) + (id * sizeof(Node));
	ret = NODES_execSet((Node *)ptr, IMG_PREF_WIN, img_pref);
	if (ret != 0) {
		recoverOldConfFromTmp(&g_conf.img, sizeof(AGTX_CMD_IMG_PREF));
		return ret;
	}

	recoverTmptoZero();

	return 0;
}

static JsonConfHandler img_pref_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_IMG_PREF, ImgPref);

__attribute__((constructor)) void registerImgPref(void)
{
	HANDLERS_registerHandlers(&img_pref_ops);
}