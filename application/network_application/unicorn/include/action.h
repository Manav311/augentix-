#ifndef ACTION_H
#define ACTION_H

#include <stdbool.h>

#include "mpi_base_types.h"
#include "mpi_limits.h"

#define RESP_BYTE (1)
#define BUFSIZE (32768)
#define MAX_XMT_LEN (8192)
#define TIMEOUT (5)
#define MAX_ROI_OBS_NUM (4)

#define ACT_SUCCESS (0) /**< Definition of success in ACT. */
#define ACT_FAILURE (-1) /**< Definition of failure in ACT. */
#define ACT_TIMEOUT (-2) /**< Definition of timeout in ACT. */
#define ACT_NULL (-3) /**< Definition of NULL in ACT. */

/*
 * struct FileInfo - file infomation
 * @col: 	raw file column.
 * @row:  	raw file row.
 * @bit:	raw file bit number.
 * @phase:  bayer phase.
 */
typedef struct {
	int col;
	int row;
	int bit;
	int phase;
	char *port_type;
} FileInfo;

/*
 * struct DramInfo - Dram infomation
 * @col_addr_type:			page size of DRAM.(0:512, 1:1024, 2:2048, 3:4096)(byte)
 * @bank_interleave_type:	bank interleave.(0:0, 1:2, 2:4, 3:8)
 * @bank_group_type:		bank group.(0:0, 1:0~1, 2:0~3, 3:0~7)
 * @bank_interleaving_num:  bank interleave number
 * @bank_group_num:			bank group number
 * @bank_size:				single bank size
 * @bank_bit:				bit of single bank
 * @row_size:				single row size
 * @row_bit:				bit of single row
 * @y_only:					Y only
 * @msb_only:				msb only
 */
typedef struct {
	int col_addr_type;
	int bank_interleave_type;
	int bank_group_type;
	int bank_interleaving_num;
	int bank_group_num;
	int bank_size;
	int bank_bit;
	int row_size;
	int row_bit;
	int y_only;
	int msb_only;
} DramInfo;

/*
 * struct FileInfo - file infomation
 * @col: 	raw file column.
 * @row:  	raw file row.
 * @bit:	raw file bit number.
 * @y_only:	 Y only
 * @msb_only: msb only
 */
typedef struct {
	int col;
	int row;
	int bit;
	int phase;
	int y_only;
	int msb_only;
	int col_addr_type;
	int bank_interleave_type;
	int bank_group_type;
} DecodeInfo;

typedef struct frame_metadata {
	UINT16 width;
	UINT16 height;
	UINT8 msb_only;
	UINT8 y_only;
	UINT8 col_addr_type;
	UINT8 bank_interleave_type;
	UINT8 bank_group_type;
	UINT32 frame_num;
	UINT32 fps_x1001;
	unsigned long timestamp; /* timestamp of frame */
	unsigned long win_timestamp[MPI_MAX_VIDEO_WIN_NUM]; /* timestamp of windows */
	unsigned long jiffies; /* jiffies of frame */
	unsigned long win_jiffies[MPI_MAX_VIDEO_WIN_NUM]; /* jiffies of windows */
	UINT8 obs_num; /* num of ROI of obs */
	MPI_RECT_POINT_S roi_obs[MAX_ROI_OBS_NUM]; /* ROI of obs */
} FrameMetadata;

bool fileExists(const char *filePath);
unsigned long int getFileSize(const char *filePath);

// If you cannot be sure whether the file is plain texts with CRLF or not,
// you can always set "convert_crlf" to true and let dos2unix help you detect.
int recvFile(int sockfd, char *fPath, long fSize, bool convert_crlf);
int recvBinaryData(int sockfd, long fileSize, int16_t *recvData);
int sendFile(int sockfd, char *params);
int sendData(int sockfd, char *data, int size);

/* snapshot */
int Snapshot_run(FileInfo *file_info, int *dump_size, int fd);

#endif
