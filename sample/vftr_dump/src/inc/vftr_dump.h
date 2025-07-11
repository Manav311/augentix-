#ifndef IVA_DUMP_H_
#define IVA_DUMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "vftr_dump_define.h"
#include "mpi_index.h"

#define DBG(...)

#define MAX_IVA_DUMP_NUM 8

typedef struct dump_filter_config {
	int id_num;
	UINT16 id[MAX_IVA_DUMP_NUM];
	int group_num;
	UINT16 group[MAX_IVA_DUMP_NUM];
	int no_group_num;
	UINT16 no_group[MAX_IVA_DUMP_NUM];
	int type_num;
	UINT16 type[FLAG_LENGTH_TYPE];
	int cat_num;
	UINT16 cat[FLAG_MAX_CAT_VALUE];
	pid_t tid;
} DumpFilterConfig;

typedef struct dump_config {
	int is_binary;
#ifdef CONFIG_APP_VFTR_DUMP_SUPPORT_SEI
	const char *rtsp_app;
	MPI_WIN win;
#endif
	const char *in_binary_path;
	const char *img_directory;
	double interval_in_sec;
	DumpFilterConfig filter;
} DumpConfig;

int DUMP_start(int pid, const DumpConfig *config);
void DUMP_stop(void);
void DUMP_list(void);

#ifdef __cplusplus
}
#endif

#endif
