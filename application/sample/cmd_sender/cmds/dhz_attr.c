#include "cmd_util.h"
#include "cmdparser.h"

#include "mpi_dip_alg.h"

#include <errno.h>
#include <getopt.h>
#include <stdlib.h>

#define NUM_NON_AUTO_ATTR 3
#define NUM_AUTO_ATTR 2
#define NUM_DHZ_ATTR ((NUM_NON_AUTO_ATTR) + ((MPI_ISO_LUT_ENTRY_NUM) + 1) * (NUM_AUTO_ATTR)) // ==27

static INT32 GET(DhzAttr)(CMD_DATA_S *opt)
{
	return MPI_getDhzAttr(opt->path_idx, opt->data);
}

static INT32 SET(DhzAttr)(const CMD_DATA_S *opt)
{
	return MPI_setDhzAttr(opt->path_idx, opt->data);
}

static void ARGS(DhzAttr)(void)
{
	printf("\t'--dhz dev_idx path_idx mode dc_iir_weight gain_step_th dhz_manual.y_gain_max dhz_manual.c_gain_max "
	       "dhz_auto.y_gain_max[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] dhz_auto.c_gain_max[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]', "
	       "where 'MPI_ISO_LUT_ENTRY_NUM' = %d\n",
	       MPI_ISO_LUT_ENTRY_NUM);
	printf("\t'--dhz 0 0 0 32 10 512 340 "
	       "512 512 512 512 512 512 512 512 512 512 512 "
	       "340 340 340 340 340 340 340 340 340 340 340'\n");
}

static void HELP(DhzAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dhz <MPI_PATH> [DHZ_ATTR]'", "Set DHZ attributes");
}

static void SHOW(DhzAttr)(const CMD_DATA_S *opt)
{
	MPI_DHZ_ATTR_S *attr = (MPI_DHZ_ATTR_S *)opt->data;
	int32_t i;

	printf("device index: %hhu, path index: %hhu\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d (0: auto, 2: manual)\n", attr->mode);
	printf("dc_iir_weight=%hu\n", attr->dc_iir_weight);
	printf("gain_step_th=%hu\n", attr->gain_step_th);
	printf("dhz_manual.y_gain_max=%hu\n", attr->dhz_manual.y_gain_max);
	printf("dhz_manual.c_gain_max=%hu\n", attr->dhz_manual.c_gain_max);

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("dhz_auto.y_gain_max[%d]=%hu\n", i, attr->dhz_auto.y_gain_max[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("dhz_auto.c_gain_max[%d]=%hu\n", i, attr->dhz_auto.c_gain_max[i]);
	}
}

static int PARSE(DhzAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_DHZ_ATTR_S *data = (MPI_DHZ_ATTR_S *)opt->data;
	const int32_t num = argc - optind;
	int32_t i;

	if (num == (NUM_DHZ_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = (uint8_t)atoi(argv[optind]);
		optind++;
		opt->path_idx.path = (uint8_t)atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;
		data->dc_iir_weight = (uint16_t)atoi(argv[optind]);
		optind++;
		data->gain_step_th = (uint16_t)atoi(argv[optind]);
		optind++;
		data->dhz_manual.y_gain_max = (uint16_t)atoi(argv[optind]);
		optind++;
		data->dhz_manual.c_gain_max = (uint16_t)atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->dhz_auto.y_gain_max[i] = (uint16_t)atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->dhz_auto.c_gain_max[i] = (uint16_t)atoi(argv[optind]);
			optind++;
		}
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = (uint8_t)atoi(argv[optind]);
		optind++;
		opt->path_idx.path = (uint8_t)atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S dhz_ops = MAKE_CMD("dhz", MPI_DHZ_ATTR_S, DhzAttr);

__attribute__((constructor)) void regDhzCmd(void)
{
	CMD_register(&dhz_ops);
}
