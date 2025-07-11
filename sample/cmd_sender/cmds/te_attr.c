#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_TE_ATTR (1 + 60 + 19 + 5 + 11 * 3 + 9 * 4 + 1 + 11 + 1 + 1 + 11 + 1 + 11 + 11 + 11 + 11 + 11 + 1 + 11)

static INT32 GET(TeAttr)(CMD_DATA_S *opt)
{
	return MPI_getTeAttr(opt->path_idx, opt->data);
}

static INT32 SET(TeAttr)(const CMD_DATA_S *opt)
{
	return MPI_setTeAttr(opt->path_idx, opt->data);
}

static void ARGS(TeAttr)(void)
{
	printf("\t'--te dev_idx path_idx mode te_normal.curve[0 ~ MPI_TE_CURVE_ENTRY_NUM-1] te_wdr.brightness te_wdr.strength \
	te_wdr.saliency te_wdr.noise_cstr[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] te_wdr.iso_weight te_wdr.dark_enhance te_wdr.iso_max \
	te_adapt.strength te_adpat.dark_enhance[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] te_adapt.te_adapt_based_type te_adapt.str_auto_en \
	te_adpat.str_auto[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] te_adapt.speed te_adpat.white_th[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] \
	te_adpat.black_th[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] te_adpat.max_str[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] \
	te_adpat.dark_protect_smooth[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] te_adpat.dark_protect_str[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]'\n");
	printf("\t'--te 0 0 0 0 16 32 64 113 172 230 280 340 460 580 600 740 880 1030 1200 1370 1540 1700 1860 2048 2176 2304 \
	2432 2560 2816 3072 3328 3584 3840 4096 4352 4608 4864 5120 5376 5632 5888 6144 6400 6656 7168 7680 8192 8704 9216 9728 \
	10240 10752 11264 11776 12288 12800 13312 13824 14336 14848 15360 15872 16384 4250 819 768 0 0 512 512 1024 1024 1024 1024 \
	1024 1024 1024 32 80 102400 0 2 0 1024 1024 1024 1024 1024 1024 1024 1024 1024 1024 1024 0 0 0 0 0 0 0 0 0 0 0 819 819 819 \
	819 819 819 819 819 819 4250 4250 4250 4250 4250 4250 4250 4250 4250 102400 768 768 768 768 768 768 768 768 768 0 0 512  512 \
	1024 1024 1024 1024 1024 1024 1024 80 80 80 80 80 80 80 80 80 32 0 2 8 8 8 8 8 8 8 8 8 8 8 8 0 0 16 16 16 16 16 16 16 16 16 16 16 \
	95 15040 15040 15040 15040 15040 15040 15040 15040 15040 15040 15040 512 512 512 512 512 512 512 512 512 512 512 \
	7 7 7 7 7 7 7 7 7 7 7 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2048 2048 2048 2048 2048 2048 2048 2048 2048 2048 2048 \n");
}

static void HELP(TeAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--te <MPI_PATH> [TE_ATTR]'", "Set TE attributes");
}

static void SHOW(TeAttr)(const CMD_DATA_S *opt)
{
	MPI_TE_ATTR_S *attr = (MPI_TE_ATTR_S *)opt->data;
	int i;

	printf("mode=%d (0: NORMAL, 1: WDR, 2: WDR_AUTO, 3:ADAPT)\n", attr->mode);

	for (i = 0; i < MPI_TE_CURVE_ENTRY_NUM; ++i) {
		printf("te_normal.curve[%d]=%d\n", i, attr->te_normal.curve[i]);
	}

	printf("te_wdr.brightness=%d\n", attr->te_wdr.brightness);
	printf("te_wdr.strength=%d\n", attr->te_wdr.strength);
	printf("te_wdr.saliency=%d\n", attr->te_wdr.saliency);

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("te_wdr.noise_cstr[%d]=%d\n", i, attr->te_wdr.noise_cstr[i]);
	}

	printf("te_wdr.iso_weight=%d\n", attr->te_wdr.iso_weight);
	printf("te_wdr.dark_enhance=%d\n", attr->te_wdr.dark_enhance);
	printf("te_wdr.iso_max=%d\n", attr->te_wdr.iso_max);
	printf("te_wdr.interval=%d\n", attr->te_wdr.interval);
	printf("te_wdr.precision=%d\n", attr->te_wdr.precision);

	printf("wdr_auto.dri_type = %d (0: GAMMA_HIST_CV, 1: LINEAR_HIST_CV)\n", attr->te_wdr_auto.dri_type);

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.dri_gain[%d] = %d\n", i, attr->te_wdr_auto.dri_gain[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.dri_offset[%d] = %d\n", i, attr->te_wdr_auto.dri_offset[i]);
	}
	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.strength[%d] = %d\n", i, attr->te_wdr_auto.strength[i]);
	}
	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.brightness[%d] = %d\n", i, attr->te_wdr_auto.brightness[i]);
	}
	printf("wdr_auto.iso_max = %d\n", attr->te_wdr_auto.iso_max);
	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.saliency[%d] = %d\n", i, attr->te_wdr_auto.saliency[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.noise_cstr[%d] = %d\n", i, attr->te_wdr_auto.noise_cstr[i]);
	}

	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		printf("wdr_auto.dark_enhance[%d] = %d\n", i, attr->te_wdr_auto.dark_enhance[i]);
	}

	printf("wdr_auto.iso_weight = %d\n", attr->te_wdr_auto.iso_weight);
	printf("wdr_auto.interval = %d\n", attr->te_wdr_auto.interval);
	printf("wdr_auto.precision = %d\n", attr->te_wdr_auto.precision);

	printf("te_adapt.strength = %d\n", attr->te_adapt.strength);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.dark_enhance[%d] = %d\n", i, attr->te_adapt.dark_enhance[i]);
	}
	printf("te_adapt.te_adapt_based_type= %d (0: TE_ADAPT_NL_BASED, 1: TE_ADAPT_INTTIME_BASED , 2: TE_ADAPT_EV_BASED ,3: TE_ADAPT_BASED_TYPE_RSV)\n",
	       attr->te_adapt.te_adapt_based_type);
	printf("te_adapt.str_auto_en= %d\n", attr->te_adapt.str_auto_en);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.str_auto[%d] = %d\n", i, attr->te_adapt.str_auto[i]);
	}
	printf("te_adapt.speed= %d\n", attr->te_adapt.speed);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.white_th[%d] = %d\n", i, attr->te_adapt.white_th[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.black_th[%d] = %d\n", i, attr->te_adapt.black_th[i]);
	}
	printf("te_adapt.max_str_prec_sel= %d\n", attr->te_adapt.max_str_prec_sel);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.max_str[%d] = %d\n", i, attr->te_adapt.max_str[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.dark_protect_smooth[%d] = %d\n", i, attr->te_adapt.dark_protect_smooth[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.dark_protect_str[%d] = %d\n", i, attr->te_adapt.dark_protect_str[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("te_adapt.dark_enhance_th[%d] = %d\n", i, attr->te_adapt.dark_enhance_th[i]);
	}
}

static int PARSE(TeAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_TE_ATTR_S *data = (MPI_TE_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_TE_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_TE_CURVE_ENTRY_NUM; ++i) {
			data->te_normal.curve[i] = atoi(argv[optind]);
			optind++;
		}

		data->te_wdr.brightness = atoi(argv[optind]);
		optind++;
		data->te_wdr.strength = atoi(argv[optind]);
		optind++;
		data->te_wdr.saliency = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_wdr.noise_cstr[i] = atoi(argv[optind]);
			optind++;
		}

		data->te_wdr.iso_weight = atoi(argv[optind]);
		optind++;
		data->te_wdr.dark_enhance = atoi(argv[optind]);
		optind++;
		data->te_wdr.iso_max = atoi(argv[optind]);
		optind++;
		data->te_wdr.interval = atoi(argv[optind]);
		optind++;
		data->te_wdr.precision = atoi(argv[optind]);
		optind++;

		data->te_wdr_auto.dri_type = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.dri_gain[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.dri_offset[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.strength[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.brightness[i] = atoi(argv[optind]);
			optind++;
		}

		data->te_wdr_auto.iso_max = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.saliency[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.noise_cstr[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; ++i) {
			data->te_wdr_auto.dark_enhance[i] = atoi(argv[optind]);
			optind++;
		}

		data->te_wdr_auto.iso_weight = atoi(argv[optind]);
		optind++;
		data->te_wdr_auto.interval = atoi(argv[optind]);
		optind++;
		data->te_wdr_auto.precision = atoi(argv[optind]);
		optind++;

		data->te_adapt.strength = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.dark_enhance[i] = atoi(argv[optind]);
			optind++;
		}
		data->te_adapt.te_adapt_based_type = atoi(argv[optind]);
		optind++;
		data->te_adapt.str_auto_en = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.str_auto[i] = atoi(argv[optind]);
			optind++;
		}
		data->te_adapt.speed = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.white_th[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.black_th[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.max_str[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.dark_protect_smooth[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.dark_protect_str[i] = atoi(argv[optind]);
			optind++;
		}
		data->te_adapt.max_str_prec_sel = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->te_adapt.dark_enhance_th[i] = atoi(argv[optind]);
			optind++;
		}

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

static CMD_S te_ops = MAKE_CMD("te", MPI_TE_ATTR_S, TeAttr);

__attribute__((constructor)) void regTeCmd(void)
{
	CMD_register(&te_ops);
}
