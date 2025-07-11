#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_EXPOSURE_INFO (9)

static INT32 GET(ExposureAttr)(CMD_DATA_S *opt)
{
	return MPI_queryExposureInfo(opt->path_idx, opt->data);
}

static INT32 SET(ExposureAttr)(const CMD_DATA_S *opt __attribute__((unused)))
{
	return -ENOSYS;
}

static void ARGS(ExposureAttr)(void)
{
	printf("\t'--exposure dev_idx path_idx'\n");
	printf("\t'--exposure 0 0'\n");
}

static void HELP(ExposureAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--exposure <MPI_PATH>'", "Query internal exposure status information");
}

static void SHOW(ExposureAttr)(const CMD_DATA_S *opt)
{
	MPI_EXPOSURE_INFO_S *attr = (MPI_EXPOSURE_INFO_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("          inttime = %d\n", attr->inttime);
	printf("      sensor_gain = %d\n", attr->sensor_gain);
	printf("         sys_gain = %d\n", attr->sys_gain);
	printf("         isp_gain = %d\n", attr->isp_gain);
	printf("              iso = %d\n", attr->iso);
	printf("flicker_free_conf = %d\n", attr->flicker_free_conf);
	printf("      frame_delay = %d\n", attr->frame_delay);
	printf("              fps = %f\n", attr->fps);
	printf("            ratio = %d\n", attr->ratio);
	printf("         luma_avg = %d\n", attr->luma_avg);
}

static int PARSE(ExposureAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S exposure_ops = MAKE_CMD("exposure", MPI_EXPOSURE_INFO_S, ExposureAttr);

__attribute__((constructor)) void regExposureCmd(void)
{
	CMD_register(&exposure_ops);
}