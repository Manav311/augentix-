#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_HDR_SYNTH_ATTR (10)

static INT32 GET(HdrSynthAttr)(CMD_DATA_S *opt)
{
	return MPI_getHdrSynthAttr(opt->path_idx, opt->data);
}

static INT32 SET(HdrSynthAttr)(const CMD_DATA_S *opt)
{
	return MPI_setHdrSynthAttr(opt->path_idx, opt->data);
}

static void ARGS(HdrSynthAttr)(void)
{
	printf("\t'--hdr_synth dev_idx path_idx\n");
	printf("\t             se_weight_th_min se_weight_slope se_weight_min se_weight_max\n");
	printf("\t             le_weight_th_max le_weight_slope le_weight_min le_weight_max\n");
	printf("\t             local_fb_th frame_fb_strength'\n");
	printf("\t'--hdr_synth 0 0 240 56 1 63 3840 14 1 63 700 0'\n");
}

static void HELP(HdrSynthAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--hdr_synth <MPI_PATH> [HDR_SYNTH_ATTR]'", "Set HDR_SYNTH attributes");
}

static void SHOW(HdrSynthAttr)(const CMD_DATA_S *opt)
{
	MPI_HDR_SYNTH_ATTR_S *attr = (MPI_HDR_SYNTH_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("weight.se_weight_th_min = %d\n", attr->weight.se_weight_th_min);
	printf("weight.se_weight_slope = %d\n", attr->weight.se_weight_slope);
	printf("weight.se_weight_min = %d\n", attr->weight.se_weight_min);
	printf("weight.se_weight_max = %d\n", attr->weight.se_weight_max);
	printf("weight.le_weight_th_max = %d\n", attr->weight.le_weight_th_max);
	printf("weight.le_weight_slope = %d\n", attr->weight.le_weight_slope);
	printf("weight.le_weight_min = %d\n", attr->weight.le_weight_min);
	printf("weight.le_weight_max = %d\n", attr->weight.le_weight_max);
	printf("local_fb_th = %d\n", attr->local_fb_th);
	printf("frame_fb_strength = %d\n", attr->frame_fb_strength);
}

static int PARSE(HdrSynthAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_HDR_SYNTH_ATTR_S *data = (MPI_HDR_SYNTH_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_HDR_SYNTH_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->weight.se_weight_th_min = atoi(argv[optind]);
		optind++;

		data->weight.se_weight_slope = atoi(argv[optind]);
		optind++;

		data->weight.se_weight_min = atoi(argv[optind]);
		optind++;

		data->weight.se_weight_max = atoi(argv[optind]);
		optind++;

		data->weight.le_weight_th_max = atoi(argv[optind]);
		optind++;

		data->weight.le_weight_slope = atoi(argv[optind]);
		optind++;

		data->weight.le_weight_min = atoi(argv[optind]);
		optind++;

		data->weight.le_weight_max = atoi(argv[optind]);
		optind++;

		data->local_fb_th = atoi(argv[optind]);
		optind++;

		data->frame_fb_strength = atoi(argv[optind]);
		optind++;
	} else if (num == 2) {
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

static CMD_S hdr_synth_ops = MAKE_CMD("hdr_synth", MPI_HDR_SYNTH_ATTR_S, HdrSynthAttr);

__attribute__((constructor)) void regHdrSynthCmd(void)
{
	CMD_register(&hdr_synth_ops);
}
