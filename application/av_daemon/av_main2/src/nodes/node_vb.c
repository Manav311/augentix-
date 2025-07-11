#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mpi_sys.h"

#include "agtx_video.h"
#include "agtx_cmd.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;

int NODE_initVb(void)
{
	INT32 ret = 0;

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Initialize system failed. ret: %d", ret);
		return -ENXIO;
	}

	return 0;
}

#define MACRO_BLOCK_WIDTH (16)
#define MACRO_BLOCK_HEIGHT (16)
#define BLOCK_WIDTH (8)
#define BLOCK_HEIGHT (8)
#define MV_PER_WORD (2)
/* HC1783/1703 = 64, HC1782/1772/1752/1702 = 64 */
#define DA_FIFO_WORD (64)
/* DRAM_PAGE_SIZE , DRAM_BANK_NUM must same to product DRAM, product_defconfig */
#define DRAM_BANK_NUM (8)
#define DRAM_PAGE_SIZE (2048)
#define round_up_base(value, base) (((value) + (base - 1)) / (base)) * (base)

static uint32_t calcTmvBufferSize(const uint32_t width, const uint32_t height)
{
	uint32_t blk_num_hor = round_up_base(width, MACRO_BLOCK_WIDTH) / BLOCK_WIDTH;
	uint32_t blk_num_ver = round_up_base(height, MACRO_BLOCK_HEIGHT) / BLOCK_HEIGHT;
	uint32_t word_per_row = round_up_base(blk_num_hor, MV_PER_WORD) / MV_PER_WORD;
	uint32_t addr_per_group = word_per_row * blk_num_ver * (DA_FIFO_WORD / 8);
	uint32_t size = round_up_base(addr_per_group, DRAM_PAGE_SIZE * DRAM_BANK_NUM);

	return size;
}

static inline void toMpiLayoutWindow(const MPI_RECT_S *pos, MPI_SIZE_S *chn_res, MPI_RECT_S *lyt_res)
{
#define MIN(a, b) ((a) < (b) ? (a) : (b))
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

INT32 NODE_startVb(void) /*this return use MPI or errno?*/
{
	INT32 ret;
	INT32 pool_cnt = 0;

	INT32 size = 0;

	MPI_VB_CONF_S vb_conf;
	char pool_name[24];
	memset(&vb_conf, 0, sizeof(MPI_VB_CONF_S));
	vb_conf.max_pool_cnt = MAX_VB_POOL_NUM;

	if (g_conf.dev.input_path_cnt == MAX_VIDEO_INPUT + 1) {
		/*2 sensor case ?*/
		/*f49763 has 2 sensor input path*/
		if ((g_conf.dev.input_path[0].width != g_conf.dev.input_path[1].width) ||
		    (g_conf.dev.input_path[0].height != g_conf.dev.input_path[1].height)) {
			avmain2_log_err("Resolution for two sensors are not the same.");
			return -EINVAL;
		}
	}

	MPI_RECT_S pos = { 0 };
	MPI_RECT_S window = { 0 };
	MPI_SIZE_S res = { 0 };

	/* get max window size of max chn 0 resolution in product option*/
	pos.x = 0; /*min start of pos.x*/
	pos.y = 0; /*min start of pos.y*/
	pos.width = 1024; /*max of pox width*/
	pos.height = 1024; /*max of pox width*/
	res.width = g_conf.res_option.strm[0].res[0].width;
	res.height = g_conf.res_option.strm[0].res[0].height;
	avmain2_log_debug("parse (%d, %d)", res.width, res.height);
	toMpiLayoutWindow(&pos, &res, &window /*output window exactly rect*/);

	size = calcTmvBufferSize(window.width, window.height);
	size += 16; /* TMV offset for meta data */
#ifdef CB_BASED_OD
	vb_conf.pub_pool[pool_cnt].blk_cnt = 4;
#else
	vb_conf.pub_pool[pool_cnt].blk_cnt = 1;
#endif
	avmain2_log_info("calc VB size: %d, win 0,0 (%d, %d), blk: %d", size, window.width, window.height,
	                 vb_conf.pub_pool[pool_cnt].blk_cnt);

	vb_conf.pub_pool[pool_cnt].blk_size = size;
	sprintf(pool_name, "isp_TMV_%d", pool_cnt);
	strcpy((char *)vb_conf.pub_pool[pool_cnt].name, pool_name);

	ret = MPI_VB_setConf(&vb_conf);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Configure video buffer failed. ret: %d", ret);
		return -ENXIO;
	}

	ret = MPI_VB_init();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Initialize video buffer failed. ret: %d", ret);
		return -ENXIO;
	}

	return 0;
}

int NODE_stopVb(void)
{
	INT32 ret = 0;
	ret = MPI_VB_exit();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Exit video buffer failed. ret: %d", ret);
		return -ENXIO;
	}

	return 0;
}

int NODE_exitVb(void)
{
	INT32 ret = 0;

	ret = MPI_SYS_exit();
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Exit system failed. ret: %d", ret);
		return -ENXIO;
	}

	return 0;
}

int NODE_setVb(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	return 0;
}
