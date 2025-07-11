#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_DIP_ATTR (17)

static INT32 GET(DipAttr)(CMD_DATA_S *opt)
{
	return MPI_getDipAttr(opt->path_idx, opt->data);
}

static INT32 SET(DipAttr)(const CMD_DATA_S *opt)
{
	return MPI_setDipAttr(opt->path_idx, opt->data);
}

static void ARGS(DipAttr)(void)
{
	printf("\t'--dip dev_idx path_idx is_dip_en is_ae_en is_iso_en is_awb_en is_csm_en is_te_en is_pta_en is_nr_en "
	       "is_shp_en is_gamma_en is_dpc_en is_dms_en is_me_en is_enh_en is_coring_en is_fcs_en is_dhz_en'\n");
	printf("\t'--dip 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'\n");
}

static void HELP(DipAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dip <MPI_PATH> [DIP_ATTR]", "Set DIP attributes");
}

static void SHOW(DipAttr)(const CMD_DATA_S *opt)
{
	MPI_DIP_ATTR_S *attr = opt->data;

	printf("device index: %hhu, path index: %hhu\n", opt->path_idx.dev, opt->path_idx.path);
	printf("dip=%hhu, ae=%hhu, iso=%hhu, awb=%hhu, nr=%hhu, te=%hhu, pta=%hhu, csm=%hhu, shp=%hhu, gamma=%hhu, "
	       "dpc=%hhu, dms=%hhu, me=%hhu, enh=%hhu, coring=%hhu, fcs=%hhu, dhz=%hhu\n",
	       attr->is_dip_en, attr->is_ae_en, attr->is_iso_en, attr->is_awb_en, attr->is_nr_en, attr->is_te_en,
	       attr->is_pta_en, attr->is_csm_en, attr->is_shp_en, attr->is_gamma_en, attr->is_dpc_en, attr->is_dms_en,
	       attr->is_me_en, attr->is_enh_en, attr->is_coring_en, attr->is_fcs_en, attr->is_dhz_en);
}

static int PARSE(DipAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num == (NUM_DIP_ATTR + 2)) {
		MPI_DIP_ATTR_S *attr = (MPI_DIP_ATTR_S *)(opt->data);

		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		attr->is_dip_en = atoi(argv[optind]);
		optind++;
		attr->is_ae_en = atoi(argv[optind]);
		optind++;
		attr->is_iso_en = atoi(argv[optind]);
		optind++;
		attr->is_awb_en = atoi(argv[optind]);
		optind++;
		attr->is_nr_en = atoi(argv[optind]);
		optind++;
		attr->is_te_en = atoi(argv[optind]);
		optind++;
		attr->is_pta_en = atoi(argv[optind]);
		optind++;
		attr->is_csm_en = atoi(argv[optind]);
		optind++;
		attr->is_shp_en = atoi(argv[optind]);
		optind++;
		attr->is_gamma_en = atoi(argv[optind]);
		optind++;
		attr->is_dpc_en = atoi(argv[optind]);
		optind++;
		attr->is_dms_en = atoi(argv[optind]);
		optind++;
		attr->is_me_en = atoi(argv[optind]);
		optind++;
		attr->is_enh_en = atoi(argv[optind]);
		optind++;
		attr->is_coring_en = atoi(argv[optind]);
		optind++;
		attr->is_fcs_en = atoi(argv[optind]);
		optind++;
		attr->is_dhz_en = atoi(argv[optind]);
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

static CMD_S dip_ops = MAKE_CMD("dip", MPI_DIP_ATTR_S, DipAttr);

__attribute__((constructor)) void regDipCmd(void)
{
	CMD_register(&dip_ops);
}