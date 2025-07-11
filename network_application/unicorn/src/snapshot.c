#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>

#include "chip_id.h"
#include "action.h"
#include "utils.h"
#include "vb_sys.h"

#ifndef ISW_PORT
#if (((IS_HC1702_SUPPORT || IS_HC1722_SUPPORT) || IS_HC1752_SUPPORT) || IS_HC1772_SUPPORT) || IS_HC1782_SUPPORT
#define ISW_PORT "is_TFW_0"
#else
#define ISW_PORT "ISW-WP0"
#endif
#endif

FileInfo g_file_info;

int updateDramInfo(const FileInfo *file_info, DramInfo *dram_info);
int getDramBufferSize(const char *port_type, const FileInfo *file_info, const DramInfo *dram_info);
void showNrwLog(int dump_size, FileInfo *file_info, DramInfo *dram_info);
int infoCheck(const FileInfo *file_info);
int sendValueToPc(int fd, int value);

int Snapshot_getSize(FileInfo *file_info, DramInfo *dram_info)
{
	int dump_size = 0;

	dump_size = (int)getDramBufferSize(file_info->port_type, file_info, dram_info);
	showNrwLog(dump_size, file_info, dram_info);

	return dump_size;
}

int getDecodeInfo(FileInfo *file_info, DramInfo *dram_info, DecodeInfo *decode_info)
{
	decode_info->col = file_info->col;
	decode_info->row = file_info->row;
	decode_info->bit = file_info->bit;
	decode_info->phase = file_info->phase;
	decode_info->y_only = dram_info->y_only;
	decode_info->msb_only = dram_info->msb_only;
	decode_info->col_addr_type = dram_info->col_addr_type;
	decode_info->bank_interleave_type = dram_info->bank_interleave_type;
	decode_info->bank_group_type = dram_info->bank_group_type;

	return ACT_SUCCESS;
}

int decodeInfoToStr(int fd, DecodeInfo *decode_info)
{
	sendValueToPc(fd, decode_info->col);
	sendValueToPc(fd, decode_info->row);
	sendValueToPc(fd, decode_info->bit);
	sendValueToPc(fd, decode_info->phase);
	sendValueToPc(fd, decode_info->y_only);
	sendValueToPc(fd, decode_info->msb_only);
	sendValueToPc(fd, decode_info->col_addr_type);
	sendValueToPc(fd, decode_info->bank_interleave_type);
	sendValueToPc(fd, decode_info->bank_group_type);

	return ACT_SUCCESS;
}

int Snapshot_run(FileInfo *file_info, int *dump_size_o, int fd)
{
	void *map_base;
	VbBlk blk;
	int dump_size;
	int ret;
	FrameMetadata f_meta;

	/* update dram info with file info */
	DramInfo dram_info = { 2, 1, 2, 2, 4, 4096, 12, 16384, 14, 0, 1 };

	/* get Dram memory start address*/
	const char *rname = "ddrwpgm";
	VB_open();
	VbRport rp = VB_createRport(rname, 1);
	ret = VB_sbind(rname, file_info->port_type);
	if (ret != 0) {
		DBG_HIGH("Error VB_sbind !! \n");
		return ACT_FAILURE;
	}
	VB_read(rp, &blk);

	VB_getVbPrvData(&blk, &f_meta);
	dram_info.col_addr_type = f_meta.col_addr_type;
	dram_info.bank_interleave_type = f_meta.bank_interleave_type;
	dram_info.bank_group_type = f_meta.bank_group_type;
	dram_info.msb_only = f_meta.msb_only;
	file_info->bit = (f_meta.msb_only == 1) ? 8 : 10;
	file_info->col = f_meta.width;
	file_info->row = f_meta.height;
	DBG_MED("[%s] col_addr_type %d, ", __func__, f_meta.col_addr_type);
	DBG_MED("bank_interleave_type %d, ", f_meta.bank_interleave_type);
	DBG_MED("bank_group_type %d, ", f_meta.bank_group_type);
	DBG_MED("msb_only %d\n", f_meta.msb_only);
	DBG_MED("[%s] width %d, ", __func__, f_meta.width);
	DBG_MED("height %d\n", f_meta.height);
	updateDramInfo(file_info, &dram_info);

	/* get dump buffer size */
	dump_size = Snapshot_getSize(file_info, &dram_info);
	if (dump_size == -1) {
		DBG_HIGH("Error dump size !! \n");
		return ACT_FAILURE;
	} else {
		*dump_size_o = dump_size;
	}

	/* Destroy ports */
	VB_unbind(rp);
	VB_destroyPort(rp);
	map_base = blk.ptr; // start address

	// send file decode_info
	DecodeInfo decode_info;
	getDecodeInfo(file_info, &dram_info, &decode_info);
	decodeInfoToStr(fd, &decode_info);

	sendValueToPc(fd, dump_size);

	// Start XMT
	while (dump_size > MAX_XMT_LEN) {
		if (send(fd, map_base, MAX_XMT_LEN, 0) == -1) {
			ERR("Failed to dump vb to PC!");
			goto error;
		}
		map_base += MAX_XMT_LEN;
		dump_size -= MAX_XMT_LEN;
	}
	if (send(fd, map_base, MAX_XMT_LEN, 0) == -1) {
		ERR("Failed to dump on last piece!");
		goto error;
	}

	VB_free(&blk);
	VB_close();

	return ACT_SUCCESS;

error:
	VB_free(&blk);
	VB_close();
	return ACT_FAILURE;
}

int Snapshot_exit(void)
{
	return ACT_SUCCESS;
}

int getInfoStr(FILE *fp, char *type, char *key, int value)
{
	int str_size = 0;
	char str[128];

	if (strcmp(type, "Marker") == 0) {
		str_size = sprintf(str, "%s\n", key);
	} else if (strcmp(type, "Parameter") == 0) {
		str_size = sprintf(str, "%s = %d\n", key, value);
	} else if (strcmp(type, "NewLine") == 0) {
		str_size = sprintf(str, "\n");
	} else {
		DBG_HIGH("ERROR there are on this type: %s\n", type);
		return ACT_FAILURE;
	}

	fwrite(str, sizeof(char), str_size, fp);
	return ACT_SUCCESS;
}

/*
 * getRawDramBufferSize - max dram buffer size
 * @file_info: file info
 * @dram_info: dram info.
 *
 * return buffer size
 *
 * getRawDramBufferSize is calculating maximum dram buffer size.
 *
 */
int getRawDramBufferSize(const FileInfo *file_info, const DramInfo *dram_info)
{
	/* algn16384(Width*Height*Bit/8) */
	int max_bram_buff;

	max_bram_buff = ((((file_info->row * file_info->col * file_info->bit) >> 3) >> dram_info->row_bit) + 1)
	                << (dram_info->row_bit);

	return max_bram_buff;
}

/*
 * getPageSize - get page size
 * @dram_info: dram info.
 *
 * return page size
 *
 * getPageSize is calculating page size.
 *
 */
int getPageSize(const DramInfo *dram_info)
{
	int page_size = 0;
	/* phase type (byte)*/
	if (dram_info->y_only == 0 && dram_info->msb_only == 0) {
		page_size = 15 << 3;
	} else if (dram_info->y_only == 0 && dram_info->msb_only == 1) {
		page_size = 12 << 3;
	} else if (dram_info->y_only == 1 && dram_info->msb_only == 0) {
		page_size = 10 << 3;
	} else if (dram_info->y_only == 1 && dram_info->msb_only == 1) {
		page_size = 8 << 3;
	}

	return page_size;
}

static inline int round_up_div(int num, int base)
{
	return (num + base - 1) / base;
}

static inline int round_up_base(int num, int base)
{
	return round_up_div(num, base) * base;
}

/*
 * getYuvDramBufferSize - max dram buffer size
 * @file_info: file info
 * @dram_info: dram info.
 *
 * return buffer size
 *
 * getYuvDramBufferSize calculates precise dram buffer size of YUV data.
 *
 */
int getYuvDramBufferSize(const FileInfo *file_info, const DramInfo *dram_info)
{
	const int macro_block_width = 16;
	const int block_width = 8;
	const int da_fifo_word = 64;

	int bank_group_num = 1 << dram_info->bank_group_type;
	int bank_interleaving_num = 1 << dram_info->bank_interleave_type;
	int blk_num_hor = round_up_base(file_info->col, macro_block_width) / block_width;
	int blk_num_ver = round_up_div(file_info->row, block_width);
	int fifo_per_block = dram_info->msb_only ? 8 : 10;
	fifo_per_block = dram_info->y_only ? fifo_per_block : fifo_per_block + fifo_per_block / 2;
	int word_per_row = blk_num_hor * fifo_per_block;
	int blk_per_group = round_up_div(blk_num_ver, bank_group_num);
	int addr_per_group = word_per_row * blk_per_group * da_fifo_word / 8;
	int dram_page_size = 1 << (dram_info->col_addr_type + 9);
	dram_page_size *= bank_interleaving_num;
	int addr_per_group_align = round_up_base(addr_per_group, dram_page_size);
	int size = addr_per_group_align * bank_group_num;
	return size;
}

int getDramBufferSize(const char *port_type, const FileInfo *file_info, const DramInfo *dram_info)
{
	int buffer_size = 0;
	int index = 0;

	if (sscanf(port_type, "ISW-WP%d", &index) == 1) {
		buffer_size = getRawDramBufferSize(file_info, dram_info);
	} else if (sscanf(port_type, "isp_NRW_%d", &index) == 1) {
		buffer_size = getYuvDramBufferSize(file_info, dram_info);
	} else {
		DBG_HIGH("Error non know port type \n");
		return ACT_FAILURE;
	}

	return buffer_size;
}

int updateDramInfo(const FileInfo *file_info, DramInfo *dram_info)
{
	/* get dram info */
	if (file_info->bit == 10) {
		dram_info->msb_only = 0;
	} else if (file_info->bit == 8) {
		dram_info->msb_only = 1;
	}

	/*define dram info*/
	dram_info->bank_interleaving_num = (1 << dram_info->bank_interleave_type); //default 2 interleaving
	dram_info->bank_group_num = (1 << dram_info->bank_group_type); //default: 4,ini_addr_*_0~3
	dram_info->bank_size =
	        512 * (1 << dram_info->col_addr_type) * dram_info->bank_interleaving_num; //default: 2048 * 2
	dram_info->bank_bit = 9 + dram_info->col_addr_type + dram_info->bank_interleave_type; //default: 12 bits
	dram_info->row_size = dram_info->bank_size * dram_info->bank_group_num; //default: 4096 * 4
	dram_info->row_bit = dram_info->bank_bit + dram_info->bank_group_type; //default: 14 bits

	return ACT_SUCCESS;
}

void showNrwLog(int dump_size, FileInfo *file_info, DramInfo *dram_info)
{
	DBG_MED("dump_size = %d\n", dump_size);
	DBG_MED("port type = %s\n", file_info->port_type);
	DBG_MED("file_info->col %d \n\
			file_info->row %d \n\
			file_info->bit %d \n\
			file_info->phase %d \n",
	        file_info->col, file_info->row, file_info->bit, file_info->phase);
	DBG_MED("dram_info->col_addr_type %d \n\
			dram_info->bank_interleave_type %d \n\
			dram_info->bank_group_type %d \n\
			dram_info->bank_interleaving_num %d \n\
			dram_info->bank_group_num %d \n\
			dram_info->bank_size %d \n\
			dram_info->bank_bit %d \n\
			dram_info->row_size %d \n\
			dram_info->row_bit %d \n\
			dram_info->y_only %d \n\
			dram_info->msb_only %d \n",
	        dram_info->col_addr_type, dram_info->bank_interleave_type, dram_info->bank_group_type,
	        dram_info->bank_interleaving_num, dram_info->bank_group_num, dram_info->bank_size, dram_info->bank_bit,
	        dram_info->row_size, dram_info->row_bit, dram_info->y_only, dram_info->msb_only);
}

int infoCheck(const FileInfo *file_info)
{
	if (file_info->row == 0) {
		DBG_HIGH("Snap shot row = 0\n");
		return ACT_FAILURE;
	}

	if (file_info->col == 0) {
		DBG_HIGH("Snap shot col = 0\n");
		return ACT_FAILURE;
	}

	if (file_info->bit == 0) {
		DBG_HIGH("Snap shot bit = 0\n");
		return ACT_FAILURE;
	}

	return ACT_SUCCESS;
}
