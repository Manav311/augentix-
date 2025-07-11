#include "parse_sys.h"

#include <stdio.h>
#include <string.h>

#include "mpi_sys.h"

#include "parse_utils.h"

#define INVALID_VB_POOL_IDX (-1)
#define MIN_VB_POOL_IDX (0)
#define MAX_VB_POOL_IDX (MPI_MAX_PUB_POOL - 1)

static int parse_vb_conf_param(char *tok, MPI_VB_CONF_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "max_pool_cnt")) {
		get_value((void *)&p->max_pool_cnt, TYPE_INT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_vb_pool_param(char *tok, MPI_VB_CONF_S *p)
{
	static MPI_VB_POOL_CONF_S *pool = NULL;

	int hit = 1;

	if (!strcmp(tok, "pool_idx")) {
		int i;

		get_value((void *)&i, TYPE_INT32);

		if (i >= MIN_VB_POOL_IDX && i <= MAX_VB_POOL_IDX) {
			pool = &p->pub_pool[i];
		} else {
			fprintf(stderr, "Invalid pool index.\n");
		}
	} else if (!strcmp(tok, "block_size")) {
		get_value((void *)&pool->blk_size, TYPE_UINT32);
	} else if (!strcmp(tok, "block_cnt")) {
		get_value((void *)&pool->blk_cnt, TYPE_UINT32);
	} else if (!strcmp(tok, "pool_name")) {
		char *val = strtok(NULL, " =\n");
		strcpy((char *)pool->name, val);
	} else {
		hit = 0;
	}

	return hit;
}

int parse_sys_param(char *tok, SAMPLE_CONF_S *conf)
{
	int hit = 0;

	CONF_SYS_PARAM_S *p = &conf->sys;

	/* Parse VB conf parameter */
	hit = parse_vb_conf_param(tok, &p->vb_conf);
	if (hit) {
		goto end;
	}

	/* Parse VB pool parameter */
	hit = parse_vb_pool_param(tok, &p->vb_conf);
	if (hit) {
		goto end;
	}

end:
	return hit;
}

void init_sys_conf(CONF_SYS_PARAM_S *conf)
{
	int i;
	MPI_VB_CONF_S *vb_conf = &conf->vb_conf;
	MPI_VB_POOL_CONF_S *pool;

	vb_conf->max_pool_cnt = 0;

	for (i = 0; i < MPI_MAX_PUB_POOL; i++) {
		pool = &vb_conf->pub_pool[i];

		pool->blk_size = 0;
		pool->blk_cnt = 0;
		pool->name[0] = '\0';
	}
}