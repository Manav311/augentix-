#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_SIGN_SHP_ATTR                                                                                           \
	(1 + 2 * MPI_ISO_LUT_ENTRY_NUM + 2 + 2 + 2 * MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM +                        \
	 2 * MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM + MPI_ISO_LUT_ENTRY_NUM * 2 * MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM + \
	 MPI_ISO_LUT_ENTRY_NUM * 2 * MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM + MPI_ISO_LUT_ENTRY_NUM + 1 + 3 + 1)

typedef enum opt_val {
	SETALL = 0,
	DEV_IDX,
	CHN_IDX,
	WIN_IDX,
	MODE,
	MOTION_ADAPTIVE_EN,
	TYPE,
	STRENGTH,
	MANUAL_HPF_RATIO,
	MANUAL_TRANSFER_CURVE_X,
	MANUAL_TRANSFER_CURVE_Y,
	MANUAL_LUMA_CTRL_GAIN_X,
	MANUAL_LUMA_CTRL_GAIN_Y,
	AUTO_HPF_RATIO,
	AUTO_TRANSFER_CURVE_X,
	AUTO_TRANSFER_CURVE_Y,
	AUTO_LUMA_CTRL_GAIN_X,
	AUTO_LUMA_CTRL_GAIN_Y,
	MANUAL_SOFT_CLIP_SLOPE,
	AUTO_SOFT_CLIP_SLOPE,
	MA_WEAK_SHP_RATIO,
	MA_CONF_LOW_TH,
	MA_CONF_HIGHT_TH
} OptVal;

static struct option opts[] = { { "setall", no_argument, NULL, SETALL },
	                        { "dev_idx", required_argument, NULL, DEV_IDX },
	                        { "chn_idx", required_argument, NULL, CHN_IDX },
	                        { "win_idx", required_argument, NULL, WIN_IDX },
	                        { "mode", required_argument, NULL, MODE },
	                        { "motion_adaptive_en", required_argument, NULL, MOTION_ADAPTIVE_EN },
	                        { "shp_type", required_argument, NULL, TYPE },
	                        { "strength", required_argument, NULL, STRENGTH },
	                        { "manual_soft_clip_slope", required_argument, NULL, MANUAL_SOFT_CLIP_SLOPE },
	                        /*
	{ "manual_hpf_ratio", required_argument, NULL, MANUAL_HPF_RATIO },
	{ "ma_weak_shp_ratio", required_argument, NULL, MA_WEAK_SHP_RATIO },
	{ "ma_conf_low_th", required_argument, NULL, MA_CONF_LOW_TH },
	{ "ma_conf_high_th", required_argument, NULL, MA_CONF_HIGHT_TH }
	*/
	                        { NULL, 0, NULL, 0 } };

typedef struct winshpv2_param {
	int mode[2];
	int motion_adaptive_en[2];
	int shp_type[2];
	int strength[2];
	int manual_soft_clip_slope[2];
} WinShpV2Param;

static int calcWinShpV2(MPI_SHP_ATTR_V2_S *data, const WinShpV2Param *param);
static void setWinShpV2WrongAction(CMD_DATA_S *opt);
static int checkWinIdxRange(const char *str);

// =========================================================================== //
//  WinShpV2
// =========================================================================== //

static INT32 GET(WinShpV2)(CMD_DATA_S *opt)
{
	return MPI_getWinShpAttrV2(opt->win_idx, opt->data);
}

static INT32 SET(WinShpV2)(const CMD_DATA_S *opt)
{
	return MPI_setWinShpAttrV2(opt->win_idx, opt->data);
}

static void ARGS(WinShpV2)(void)
{
	printf("\tGet winshpv2 attributes: cmdsender --winshpv2 <dev_idx> <chn_idx> <win_idx>\n");
	printf("\n");
	printf("\tSet winshpv2 attributes: cmdsender --winshpv2 <option> <paramter>\n");
	printf("\tRequired items:\n"
	       "\t--dev_idx <parameter>\n"
	       "\t--chn_idx <parameter>\n"
	       "\t--win_idx <parameter>\n"
	       "\tOptional items:\n"
	       "\t--mode <parameter>\n"
	       "\t--motion_adaptive_en <parameter>\n"
	       "\t--shp_type <parameter>\n"
	       "\t--strength <parameter>\n"
	       "\t--manual_soft_clip_slope <parameter>'\n\n");
	printf("\t'--winshpv2 --setall dev_idx path_idx win_idx mode shp_auto_v2.sharpness[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] "
	       "shp_manual_v2.sharpness motion_adaptive_en\n");
	printf("\t'--winshpv2 0 0 0 0 255 100 60 0 0 0 0 0 0 0 0 0 0 128 0 0 0 0 0 0 0 0 0 0 0 0 0 16 32 416 800 1023 0 16 32 416\n"
	       "\t800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32\n"
	       "\t0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32'\n");
}

static void HELP(WinShpV2)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winshpv2 <MPI_WIN> [SHP_ATTR]'", "Set WinSHPV2 attributes");
}

static void SHOW(WinShpV2)(const CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_V2_S *attr = (MPI_SHP_ATTR_V2_S *)opt->data;
	int i;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("shp_auto_v2.sharpness[%d]=%d\n", i, attr->shp_auto_v2.sharpness[i]);
	}
	printf("shp_manual_v2.sharpness=%d\n", attr->shp_manual_v2.sharpness);
	printf("motion_adaptive_en=%hhu\n", attr->motion_adaptive_en);

	printf("shp_type=%d\n", attr->shp_type);
	printf("strength=%d\n", attr->strength);
	printf("shp_ex_manual.hpf_ratio=%d\n", attr->shp_ex_manual.hpf_ratio);
	printf("shp_ex_manual.transfer_curve_x=");
	for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.transfer_curve.x[i]);
		if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.transfer_curve_y=");
	for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.transfer_curve.y[i]);
		if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.luma_ctrl_gain_x=");
	for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.luma_ctrl_gain.x[i]);
		if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.luma_ctrl_gain_y=");
	for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.luma_ctrl_gain.y[i]);
		if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_auto.hpf_ratio=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->shp_ex_auto.hpf_ratio[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	for (int j = 0; j < MPI_ISO_LUT_ENTRY_NUM; j++) {
		printf("shp_ex_auto.transfer_curve[%d].x=", j);
		for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.transfer_curve[j].x[i]);
			if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.transfer_curve[%d].y=", j);
		for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.transfer_curve[j].y[i]);
			if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.luma_ctrl_gain[%d].x=", j);
		for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.luma_ctrl_gain[j].x[i]);
			if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.luma_ctrl_gain[%d].y=", j);
		for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.luma_ctrl_gain[j].y[i]);
			if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
	}
	printf("shp_ex_manual.soft_clip_slope=%d\n", attr->shp_ex_manual.soft_clip_slope);
	printf("shp_ex_auto.soft_clip_slope=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->shp_ex_auto.soft_clip_slope[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("ma_weak_shp_ratio=%hhu\n", attr->ma_weak_shp_ratio);
	printf("ma_conf_low_th=%hu\n", attr->ma_conf_low_th);
	printf("ma_conf_high_th=%hu\n", attr->ma_conf_high_th);
}

static int PARSE(WinShpV2)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_V2_S *data = (MPI_SHP_ATTR_V2_S *)opt->data;
	int num = argc - optind;
	int ret;
	int i, j;
	int calcFlag = 0;

	WinShpV2Param param = { .mode = { 0, 0 },
		                .motion_adaptive_en = { 0, 0 },
		                .shp_type = { 0, 0 },
		                .strength = { 0, 0 },
		                .manual_soft_clip_slope = { 0, 0 } };

	/* Get attributes */
	if (num == 3) {
		int showFlag = 0;
		int tmp = optind;
		opt->win_idx.dev = atoi(argv[tmp]);
		showFlag += checkWinIdxRange(argv[tmp]);
		tmp++;
		opt->win_idx.chn = atoi(argv[tmp]);
		showFlag += checkWinIdxRange(argv[tmp]);
		tmp++;
		opt->win_idx.win = atoi(argv[tmp]);
		showFlag += checkWinIdxRange(argv[tmp]);
		tmp++;
		if (showFlag == 3) {
			opt->action = CMD_ACTION_GET;
			return 0;
		}
	}

	/* Parse the options and the parameters*/
	opt->action = CMD_ACTION_SET;
	while ((ret = getopt_long(argc, argv, ":", opts, NULL)) != -1) {
		switch (ret) {
		case DEV_IDX:
			if (optarg != NULL) {
				opt->win_idx.dev = atoi(optarg);
				calcFlag++;
			}
			break;
		case CHN_IDX:
			if (optarg != NULL) {
				opt->win_idx.chn = atoi(optarg);
				calcFlag++;
			}
			break;
		case WIN_IDX:
			if (optarg != NULL) {
				opt->win_idx.win = atoi(optarg);
				calcFlag++;
			}
			break;
		case MODE:
			if (optarg != NULL) {
				param.mode[0] = 1;
				param.mode[1] = atoi(optarg);
			}
			break;
		case MOTION_ADAPTIVE_EN:
			if (optarg != NULL) {
				param.motion_adaptive_en[0] = 1;
				param.motion_adaptive_en[1] = atoi(optarg);
			}
			break;
		case TYPE:
			if (optarg != NULL) {
				param.shp_type[0] = 1;
				param.shp_type[1] = atoi(optarg);
			}
			break;
		case STRENGTH:
			if (optarg != NULL) {
				param.strength[0] = 1;
				param.strength[1] = atoi(optarg);
			}
			break;
		case MANUAL_SOFT_CLIP_SLOPE:
			if (optarg != NULL) {
				param.manual_soft_clip_slope[0] = 1;
				param.manual_soft_clip_slope[1] = atoi(optarg);
			}
			break;
		case SETALL:
			if ((argc - optind) != (NUM_SIGN_SHP_ATTR + 3)) {
				break;
			}
			opt->win_idx.dev = atoi(argv[optind]);
			optind++;
			opt->win_idx.chn = atoi(argv[optind]);
			optind++;
			opt->win_idx.win = atoi(argv[optind]);
			optind++;

			data->mode = atoi(argv[optind]);
			optind++;

			for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
				data->shp_auto_v2.sharpness[i] = atoi(argv[optind]);
				optind++;
			}

			data->shp_manual_v2.sharpness = atoi(argv[optind]);
			optind++;
			data->motion_adaptive_en = atoi(argv[optind]);
			optind++;
			data->shp_type = atoi(argv[optind]);
			optind++;
			data->strength = atoi(argv[optind]);
			optind++;
			data->shp_ex_manual.hpf_ratio = atoi(argv[optind]);
			optind++;
			for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
				data->shp_ex_manual.transfer_curve.x[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
				data->shp_ex_manual.transfer_curve.y[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
				data->shp_ex_manual.luma_ctrl_gain.x[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
				data->shp_ex_manual.luma_ctrl_gain.y[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
				data->shp_ex_auto.hpf_ratio[i] = atoi(argv[optind]);
				optind++;
			}
			for (j = 0; j < MPI_ISO_LUT_ENTRY_NUM; j++) {
				for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
					data->shp_ex_auto.transfer_curve[j].x[i] = atoi(argv[optind]);
					optind++;
				}
				for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
					data->shp_ex_auto.transfer_curve[j].y[i] = atoi(argv[optind]);
					optind++;
				}
				for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
					data->shp_ex_auto.luma_ctrl_gain[j].x[i] = atoi(argv[optind]);
					optind++;
				}
				for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
					data->shp_ex_auto.luma_ctrl_gain[j].y[i] = atoi(argv[optind]);
					optind++;
				}
			}
			data->shp_ex_manual.soft_clip_slope = atoi(argv[optind]);
			optind++;
			for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
				data->shp_ex_auto.soft_clip_slope[i] = atoi(argv[optind]);
				optind++;
			}
			data->ma_weak_shp_ratio = atoi(argv[optind]);
			optind++;
			data->ma_conf_low_th = atoi(argv[optind]);
			optind++;
			data->ma_conf_high_th = atoi(argv[optind]);
			optind++;
			return 0;
		case '?':
		default:
			setWinShpV2WrongAction(opt);
			return EINVAL;
			break;
		}
	}

	if (calcFlag == 3) {
		/* Only enter window index, show the attributes*/
		if (num == 3 || num == 6) {
			opt->action = CMD_ACTION_GET;
			return 0;
		}
		/* Set the attriutes by input values */
		MPI_getWinShpAttrV2(opt->win_idx, data);
		calcWinShpV2(data, &param);
	} else {
		setWinShpV2WrongAction(opt);
		return EINVAL;
	}
	return 0;
}

static int calcWinShpV2(MPI_SHP_ATTR_V2_S *data, const WinShpV2Param *param)
{
	data->mode = param->mode[0] ? (uint32_t)param->mode[1] : data->mode;
	data->motion_adaptive_en = param->motion_adaptive_en[0] ? param->motion_adaptive_en[1] :
	                                                          data->motion_adaptive_en;
	data->shp_type = param->shp_type[0] ? (uint32_t)param->shp_type[1] : data->shp_type;
	data->strength = param->strength[0] ? param->strength[1] : data->strength;
	data->shp_ex_manual.soft_clip_slope = param->manual_soft_clip_slope[0] ? param->manual_soft_clip_slope[1] :
	                                                                         data->shp_ex_manual.soft_clip_slope;
	return 0;
}

static void setWinShpV2WrongAction(CMD_DATA_S *opt)
{
	printf("Wrong arguments setting!  Please enter 'cmdsender --winshpv2 <option> <paramter> ");
	printf("and check the agrument list.\n");
	opt->action = CMD_ACTION_NON;
}

static int checkWinIdxRange(const char *str)
{
	if (strlen(str) == 1 && isdigit(str[0])) {
		return 1;
	}

	return 0;
}

static CMD_S winshpv2_ops = MAKE_CMD("winshpv2", MPI_SHP_ATTR_V2_S, WinShpV2);

__attribute__((constructor)) void regWinShpV2Cmd(void)
{
	CMD_register(&winshpv2_ops);
}
