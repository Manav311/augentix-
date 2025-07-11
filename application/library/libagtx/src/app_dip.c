#include "app_dip_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "mpi_dip_alg.h"

#define CLAMP_8B(x) ((x) > 255 ? 255 : ((x) < 0 ? 0 : x))
#define PREF_TH (50)
#define MAX_INPUT_PATH MPI_MAX_INPUT_PATH_NUM

// TODO: in getXxx(), set dev_idx in json data structure with dev_idx passed by function

INT32 APP_DIP_setCal(MPI_PATH path_idx, const AGTX_DIP_CAL_CONF_S *cal_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_CAL_ATTR_S cal_attr[MAX_INPUT_PATH];
	memset(cal_attr, 0, sizeof(cal_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getCalAttr(path_idx, &cal_attr[sns_path]);

		cal_attr[sns_path].cal_en = (UINT8)cal_cfg->cal[sns_path].cal_en;
		cal_attr[sns_path].dbc_en = (UINT8)cal_cfg->cal[sns_path].dbc_en;
		cal_attr[sns_path].dcc_en = (UINT8)cal_cfg->cal[sns_path].dcc_en;
		cal_attr[sns_path].lsc_en = (UINT8)cal_cfg->cal[sns_path].lsc_en;

		/* set to mpi */
		path_idx.path = sns_path;
		ret = MPI_setCalAttr(path_idx, &cal_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getCal(MPI_PATH path_idx, AGTX_DIP_CAL_CONF_S *cal_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_CAL_ATTR_S cal_attr[MAX_INPUT_PATH];
	memset(cal_attr, 0, sizeof(cal_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getCalAttr(path_idx, &cal_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif

		/* assign to agtx interface */
		cal_cfg->cal[sns_path].cal_en = cal_attr[sns_path].cal_en;
		cal_cfg->cal[sns_path].dbc_en = cal_attr[sns_path].dbc_en;
		cal_cfg->cal[sns_path].dcc_en = cal_attr[sns_path].dcc_en;
		cal_cfg->cal[sns_path].lsc_en = cal_attr[sns_path].lsc_en;
	}
	cal_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setDbc(MPI_PATH path_idx, const AGTX_DIP_DBC_CONF_S *dbc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_DBC_ATTR_S dbc_attr[MAX_INPUT_PATH];
	memset(dbc_attr, 0, sizeof(dbc_attr));
	INT32 dbc_chn = 0;

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getDbcAttr(path_idx, &dbc_attr[sns_path]);

		dbc_attr[sns_path].mode = dbc_cfg->dbc[sns_path].mode;
		dbc_attr[sns_path].dbc_level = (UINT16)dbc_cfg->dbc[sns_path].dbc_level;
		dbc_attr[sns_path].type = dbc_cfg->dbc[sns_path].type;

		for (dbc_chn = 0; dbc_chn < MPI_DBC_CHN_NUM; dbc_chn++) {
			dbc_attr[sns_path].dbc_manual.manual.black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].manual_black_level[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[0].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_0[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[1].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_1[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[2].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_2[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[3].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_3[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[4].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_4[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[5].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_5[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[6].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_6[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[7].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_7[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[8].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_8[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[9].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_9[dbc_chn];
			dbc_attr[sns_path].dbc_auto.auto_table[10].black_level[dbc_chn] =
			        (UINT16)dbc_cfg->dbc[sns_path].auto_black_level_10[dbc_chn];
		}

		path_idx.path = sns_path;
		ret = MPI_setDbcAttr(path_idx, &dbc_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getDbc(MPI_PATH path_idx, AGTX_DIP_DBC_CONF_S *dbc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_DBC_ATTR_S dbc_attr[MAX_INPUT_PATH];
	memset(dbc_attr, 0, sizeof(dbc_attr));
	INT32 dbc_chn = 0;

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getDbcAttr(path_idx, &dbc_attr[sns_path]);

#if 0
		if (ret)
			return ret;
#endif

		dbc_cfg->dbc[sns_path].mode = dbc_attr[sns_path].mode;
		dbc_cfg->dbc[sns_path].dbc_level = dbc_attr[sns_path].dbc_level;
		dbc_cfg->dbc[sns_path].type = dbc_attr[sns_path].type;

		for (dbc_chn = 0; dbc_chn < MPI_DBC_CHN_NUM; dbc_chn++) {
			dbc_cfg->dbc[sns_path].manual_black_level[dbc_chn] =
			        dbc_attr[sns_path].dbc_manual.manual.black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_0[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[0].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_1[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[1].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_2[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[2].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_3[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[3].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_4[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[4].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_5[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[5].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_6[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[6].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_7[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[7].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_8[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[8].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_9[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[9].black_level[dbc_chn];
			dbc_cfg->dbc[sns_path].auto_black_level_10[dbc_chn] =
			        dbc_attr[sns_path].dbc_auto.auto_table[10].black_level[dbc_chn];
		}
	}

	dbc_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setDcc(MPI_PATH path_idx, const AGTX_DIP_DCC_CONF_S *dcc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_DCC_ATTR_S dcc_attr[MAX_INPUT_PATH];
	memset(dcc_attr, 0, sizeof(dcc_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getDccAttr(path_idx, &dcc_attr[sns_path]);

		dcc_attr[sns_path].mode = dcc_cfg->dcc[sns_path].mode;
		dcc_attr[sns_path].type = dcc_cfg->dcc[sns_path].type;

		for (INT32 num = 0; num < MPI_DCC_CHN_NUM; num++) {
			dcc_attr[sns_path].gain[num] = (UINT16)dcc_cfg->dcc[sns_path].gain[num];
			dcc_attr[sns_path].offset_2s[num] = dcc_cfg->dcc[sns_path].offset[num];
			dcc_attr[sns_path].dcc_manual.manual.gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].manual_gain[num];
			dcc_attr[sns_path].dcc_manual.manual.offset_2s[num] = dcc_cfg->dcc[sns_path].manual_offset[num];

			// auto dcc
			dcc_attr[sns_path].dcc_auto.auto_table[0].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_0[num];
			dcc_attr[sns_path].dcc_auto.auto_table[0].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_0[num];
			dcc_attr[sns_path].dcc_auto.auto_table[1].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_1[num];
			dcc_attr[sns_path].dcc_auto.auto_table[1].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_1[num];
			dcc_attr[sns_path].dcc_auto.auto_table[2].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_2[num];
			dcc_attr[sns_path].dcc_auto.auto_table[2].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_2[num];
			dcc_attr[sns_path].dcc_auto.auto_table[3].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_3[num];
			dcc_attr[sns_path].dcc_auto.auto_table[3].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_3[num];
			dcc_attr[sns_path].dcc_auto.auto_table[4].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_4[num];
			dcc_attr[sns_path].dcc_auto.auto_table[4].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_4[num];
			dcc_attr[sns_path].dcc_auto.auto_table[5].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_5[num];
			dcc_attr[sns_path].dcc_auto.auto_table[5].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_5[num];
			dcc_attr[sns_path].dcc_auto.auto_table[6].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_6[num];
			dcc_attr[sns_path].dcc_auto.auto_table[6].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_6[num];
			dcc_attr[sns_path].dcc_auto.auto_table[7].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_7[num];
			dcc_attr[sns_path].dcc_auto.auto_table[7].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_7[num];
			dcc_attr[sns_path].dcc_auto.auto_table[8].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_8[num];
			dcc_attr[sns_path].dcc_auto.auto_table[8].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_8[num];
			dcc_attr[sns_path].dcc_auto.auto_table[9].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_9[num];
			dcc_attr[sns_path].dcc_auto.auto_table[9].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_9[num];
			dcc_attr[sns_path].dcc_auto.auto_table[10].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_10[num];
			dcc_attr[sns_path].dcc_auto.auto_table[10].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_10[num];
			dcc_attr[sns_path].dcc_auto.auto_table[11].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_11[num];
			dcc_attr[sns_path].dcc_auto.auto_table[11].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_11[num];
			dcc_attr[sns_path].dcc_auto.auto_table[12].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_12[num];
			dcc_attr[sns_path].dcc_auto.auto_table[12].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_12[num];
			dcc_attr[sns_path].dcc_auto.auto_table[13].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_13[num];
			dcc_attr[sns_path].dcc_auto.auto_table[13].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_13[num];
			dcc_attr[sns_path].dcc_auto.auto_table[14].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_14[num];
			dcc_attr[sns_path].dcc_auto.auto_table[14].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_14[num];
			dcc_attr[sns_path].dcc_auto.auto_table[15].gain[num] =
			        (UINT16)dcc_cfg->dcc[sns_path].auto_gain_15[num];
			dcc_attr[sns_path].dcc_auto.auto_table[15].offset_2s[num] =
			        dcc_cfg->dcc[sns_path].auto_offset_15[num];
		}

		ret = MPI_setDccAttr(path_idx, &dcc_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getDcc(MPI_PATH path_idx, AGTX_DIP_DCC_CONF_S *dcc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_DCC_ATTR_S dcc_attr[MAX_INPUT_PATH];
	memset(dcc_attr, 0, sizeof(dcc_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getDccAttr(path_idx, &dcc_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif

		dcc_cfg->dcc[sns_path].mode = dcc_attr[sns_path].mode;
		dcc_cfg->dcc[sns_path].type = dcc_attr[sns_path].type;

		for (INT32 num = 0; num < MPI_DCC_CHN_NUM; num++) {
			dcc_cfg->dcc[sns_path].gain[num] = dcc_attr[sns_path].gain[num];
			dcc_cfg->dcc[sns_path].offset[num] = dcc_attr[sns_path].offset_2s[num];
			dcc_cfg->dcc[sns_path].manual_gain[num] = dcc_attr[sns_path].dcc_manual.manual.gain[num];
			dcc_cfg->dcc[sns_path].manual_offset[num] = dcc_attr[sns_path].dcc_manual.manual.offset_2s[num];

			// auto dcc
			dcc_cfg->dcc[sns_path].auto_gain_0[num] = dcc_attr[sns_path].dcc_auto.auto_table[0].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_0[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[0].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_1[num] = dcc_attr[sns_path].dcc_auto.auto_table[1].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_1[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[1].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_2[num] = dcc_attr[sns_path].dcc_auto.auto_table[2].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_2[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[2].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_3[num] = dcc_attr[sns_path].dcc_auto.auto_table[3].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_3[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[3].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_4[num] = dcc_attr[sns_path].dcc_auto.auto_table[4].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_4[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[4].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_5[num] = dcc_attr[sns_path].dcc_auto.auto_table[5].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_5[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[5].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_6[num] = dcc_attr[sns_path].dcc_auto.auto_table[6].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_6[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[6].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_7[num] = dcc_attr[sns_path].dcc_auto.auto_table[7].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_7[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[7].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_8[num] = dcc_attr[sns_path].dcc_auto.auto_table[8].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_8[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[8].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_9[num] = dcc_attr[sns_path].dcc_auto.auto_table[9].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_9[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[9].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_10[num] = dcc_attr[sns_path].dcc_auto.auto_table[10].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_10[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[10].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_11[num] = dcc_attr[sns_path].dcc_auto.auto_table[11].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_11[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[11].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_12[num] = dcc_attr[sns_path].dcc_auto.auto_table[12].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_12[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[12].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_13[num] = dcc_attr[sns_path].dcc_auto.auto_table[13].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_13[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[13].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_14[num] = dcc_attr[sns_path].dcc_auto.auto_table[14].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_14[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[14].offset_2s[num];
			dcc_cfg->dcc[sns_path].auto_gain_15[num] = dcc_attr[sns_path].dcc_auto.auto_table[15].gain[num];
			dcc_cfg->dcc[sns_path].auto_offset_15[num] =
			        dcc_attr[sns_path].dcc_auto.auto_table[15].offset_2s[num];
		}
	}

	dcc_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setLsc(MPI_PATH path_idx, const AGTX_DIP_LSC_CONF_S *lsc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_LSC_ATTR_S lsc_attr[MAX_INPUT_PATH];
	memset(lsc_attr, 0, sizeof(lsc_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getLscAttr(path_idx, &lsc_attr[sns_path]);

		lsc_attr[sns_path].origin = (INT32)lsc_cfg->lsc[sns_path].origin;
		lsc_attr[sns_path].x_trend_2s = (UINT32)lsc_cfg->lsc[sns_path].x_trend;
		lsc_attr[sns_path].y_trend_2s = (UINT32)lsc_cfg->lsc[sns_path].y_trend;
		lsc_attr[sns_path].x_curvature = (UINT32)lsc_cfg->lsc[sns_path].x_curvature;
		lsc_attr[sns_path].y_curvature = (UINT32)lsc_cfg->lsc[sns_path].y_curvature;
		lsc_attr[sns_path].tilt_2s = (INT32)lsc_cfg->lsc[sns_path].tilt;

		ret = MPI_setLscAttr(path_idx, &lsc_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getLsc(MPI_PATH path_idx, AGTX_DIP_LSC_CONF_S *lsc_cfg)
{
	INT32 ret __attribute__((unused)) = 0;
	MPI_LSC_ATTR_S lsc_attr[MAX_INPUT_PATH];
	memset(lsc_attr, 0, sizeof(lsc_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; sns_path++) {
		path_idx.path = sns_path;
		ret = MPI_getLscAttr(path_idx, &lsc_attr[sns_path]);
#if 0
		if (ret)
			return ret;
#endif

		lsc_cfg->lsc[sns_path].origin = lsc_attr[sns_path].origin;
		lsc_cfg->lsc[sns_path].x_trend = lsc_attr[sns_path].x_trend_2s;
		lsc_cfg->lsc[sns_path].y_trend = lsc_attr[sns_path].y_trend_2s;
		lsc_cfg->lsc[sns_path].x_curvature = lsc_attr[sns_path].x_curvature;
		lsc_cfg->lsc[sns_path].y_curvature = lsc_attr[sns_path].y_curvature;
		lsc_cfg->lsc[sns_path].tilt = lsc_attr[sns_path].tilt_2s;
	}

	lsc_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setRoi(MPI_PATH path_idx, const AGTX_DIP_ROI_CONF_S *roi_cfg)
{
	INT32 ret = 0;
	MPI_ROI_ATTR_S roi_attr;

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; ++sns_path) {
		path_idx.path = sns_path;
		ret = MPI_getRoiAttr(path_idx, &roi_attr);

		/* continue if get failure */
		if (ret) continue;

		roi_attr.luma_roi.sx = (INT16)roi_cfg->roi[sns_path].luma_roi_sx;
		roi_attr.luma_roi.sy = (INT16)roi_cfg->roi[sns_path].luma_roi_sy;
		roi_attr.luma_roi.ex = (INT16)roi_cfg->roi[sns_path].luma_roi_ex;
		roi_attr.luma_roi.ey = (INT16)roi_cfg->roi[sns_path].luma_roi_ey;
		roi_attr.awb_roi.sx = (INT16)roi_cfg->roi[sns_path].awb_roi_sx;
		roi_attr.awb_roi.sy = (INT16)roi_cfg->roi[sns_path].awb_roi_sy;
		roi_attr.awb_roi.ex = (INT16)roi_cfg->roi[sns_path].awb_roi_ex;
		roi_attr.awb_roi.ey = (INT16)roi_cfg->roi[sns_path].awb_roi_ey;
		roi_attr.zone_lum_avg_roi.sx = (INT16)roi_cfg->roi[sns_path].zone_lum_avg_roi_sx;
		roi_attr.zone_lum_avg_roi.sy = (INT16)roi_cfg->roi[sns_path].zone_lum_avg_roi_sy;
		roi_attr.zone_lum_avg_roi.ex = (INT16)roi_cfg->roi[sns_path].zone_lum_avg_roi_ex;
		roi_attr.zone_lum_avg_roi.ey = (INT16)roi_cfg->roi[sns_path].zone_lum_avg_roi_ey;

		path_idx.path = sns_path;
		ret = MPI_setRoiAttr(path_idx, &roi_attr);
#if 0
		if (ret)
			return ret;
#endif
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getRoi(MPI_PATH path_idx, AGTX_DIP_ROI_CONF_S *roi_cfg)
{
	INT32 ret = 0;
	MPI_ROI_ATTR_S roi_attr[MAX_INPUT_PATH];
	memset(roi_attr, 0, sizeof(roi_attr));

	for (INT32 sns_path = 0; sns_path < MAX_INPUT_PATH; ++sns_path) {
		path_idx.path = sns_path;
		ret = MPI_getRoiAttr(path_idx, &roi_attr[sns_path]);

		/* workaround if sub path is not enabled, CPS ID: 35788 */
		if (ret && (sns_path != 0)) {
			roi_attr[sns_path].luma_roi.sx = 0;
			roi_attr[sns_path].luma_roi.sy = 0;
			roi_attr[sns_path].luma_roi.ex = 1024;
			roi_attr[sns_path].luma_roi.ey = 1024;
			roi_attr[sns_path].awb_roi.sx = 0;
			roi_attr[sns_path].awb_roi.sy = 0;
			roi_attr[sns_path].awb_roi.ex = 1024;
			roi_attr[sns_path].awb_roi.ey = 1024;
			roi_attr[sns_path].zone_lum_avg_roi.sx = 0;
			roi_attr[sns_path].zone_lum_avg_roi.sy = 0;
			roi_attr[sns_path].zone_lum_avg_roi.ex = 1024;
			roi_attr[sns_path].zone_lum_avg_roi.ey = 1024;
		}

		roi_cfg->roi[sns_path].luma_roi_sx = roi_attr[sns_path].luma_roi.sx;
		roi_cfg->roi[sns_path].luma_roi_sy = roi_attr[sns_path].luma_roi.sy;
		roi_cfg->roi[sns_path].luma_roi_ex = roi_attr[sns_path].luma_roi.ex;
		roi_cfg->roi[sns_path].luma_roi_ey = roi_attr[sns_path].luma_roi.ey;
		roi_cfg->roi[sns_path].awb_roi_sx = roi_attr[sns_path].awb_roi.sx;
		roi_cfg->roi[sns_path].awb_roi_sy = roi_attr[sns_path].awb_roi.sy;
		roi_cfg->roi[sns_path].awb_roi_ex = roi_attr[sns_path].awb_roi.ex;
		roi_cfg->roi[sns_path].awb_roi_ey = roi_attr[sns_path].awb_roi.ey;
		roi_cfg->roi[sns_path].zone_lum_avg_roi_sx = roi_attr[sns_path].zone_lum_avg_roi.sx;
		roi_cfg->roi[sns_path].zone_lum_avg_roi_sy = roi_attr[sns_path].zone_lum_avg_roi.sy;
		roi_cfg->roi[sns_path].zone_lum_avg_roi_ex = roi_attr[sns_path].zone_lum_avg_roi.ex;
		roi_cfg->roi[sns_path].zone_lum_avg_roi_ey = roi_attr[sns_path].zone_lum_avg_roi.ey;
	}

	roi_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setCtrl(MPI_PATH path_idx, const AGTX_DIP_CTRL_CONF_S *ctrl_cfg)
{
	INT32 ret = 0;
	MPI_DIP_ATTR_S ctrl_attr;

	ret = MPI_getDipAttr(path_idx, &ctrl_attr);

	ctrl_attr.is_dip_en = (UINT8)ctrl_cfg->is_dip_en;
	ctrl_attr.is_ae_en = (UINT8)ctrl_cfg->is_ae_en;
	ctrl_attr.is_iso_en = (UINT8)ctrl_cfg->is_iso_en;
	ctrl_attr.is_awb_en = (UINT8)ctrl_cfg->is_awb_en;
	ctrl_attr.is_nr_en = (UINT8)ctrl_cfg->is_nr_en;
	ctrl_attr.is_te_en = (UINT8)ctrl_cfg->is_te_en;
	ctrl_attr.is_pta_en = (UINT8)ctrl_cfg->is_pta_en;
	ctrl_attr.is_csm_en = (UINT8)ctrl_cfg->is_csm_en;
	ctrl_attr.is_shp_en = (UINT8)ctrl_cfg->is_shp_en;
	ctrl_attr.is_gamma_en = (UINT8)ctrl_cfg->is_gamma_en;
	ctrl_attr.is_dpc_en = (UINT8)ctrl_cfg->is_dpc_en;
	ctrl_attr.is_dms_en = (UINT8)ctrl_cfg->is_dms_en;
	ctrl_attr.is_me_en = (UINT8)ctrl_cfg->is_me_en;
	ctrl_attr.is_enh_en = (UINT8)ctrl_cfg->is_enh_en;
	ctrl_attr.is_coring_en = (UINT8)ctrl_cfg->is_coring_en;
	ctrl_attr.is_fcs_en = (UINT8)ctrl_cfg->is_fcs_en;
	ctrl_attr.is_dhz_en = (UINT8)ctrl_cfg->is_dhz_en;

	ret = MPI_setDipAttr(path_idx, &ctrl_attr);
	if (ret)
		return ret;

	return MPI_SUCCESS;
}

INT32 APP_DIP_getCtrl(MPI_PATH path_idx, AGTX_DIP_CTRL_CONF_S *ctrl_cfg)
{
	INT32 ret = 0;
	MPI_DIP_ATTR_S ctrl_attr;

	ret = MPI_getDipAttr(path_idx, &ctrl_attr);
	if (ret)
		return ret;

	ctrl_cfg->is_dip_en = ctrl_attr.is_dip_en;
	ctrl_cfg->is_ae_en = ctrl_attr.is_ae_en;
	ctrl_cfg->is_iso_en = ctrl_attr.is_iso_en;
	ctrl_cfg->is_awb_en = ctrl_attr.is_awb_en;
	ctrl_cfg->is_nr_en = ctrl_attr.is_nr_en;
	ctrl_cfg->is_te_en = ctrl_attr.is_te_en;
	ctrl_cfg->is_pta_en = ctrl_attr.is_pta_en;
	ctrl_cfg->is_csm_en = ctrl_attr.is_csm_en;
	ctrl_cfg->is_shp_en = ctrl_attr.is_shp_en;
	ctrl_cfg->is_gamma_en = ctrl_attr.is_gamma_en;
	ctrl_cfg->is_dpc_en = ctrl_attr.is_dpc_en;
	ctrl_cfg->is_dms_en = ctrl_attr.is_dms_en;
	ctrl_cfg->is_me_en = ctrl_attr.is_me_en;
	ctrl_cfg->is_enh_en = ctrl_attr.is_enh_en;
	ctrl_cfg->is_coring_en = ctrl_attr.is_coring_en;
	ctrl_cfg->is_fcs_en = ctrl_attr.is_fcs_en;
	ctrl_cfg->is_dhz_en = ctrl_attr.is_dhz_en;

	ctrl_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setAe(MPI_PATH path_idx, const AGTX_DIP_AE_CONF_S *ae_cfg)
{
	INT32 ret = 0;
	MPI_AE_ATTR_S ae_attr;

	ret = MPI_getAeAttr(path_idx, &ae_attr);

	/* exp strategy*/
	ae_attr.strategy.mode = ae_cfg->exp_strategy;

	/* fps mode*/
	ae_attr.fps_mode = ae_cfg->fps_mode;

	ae_attr.sys_gain_range.max = (UINT32)ae_cfg->max_sys_gain;
	ae_attr.sys_gain_range.min = (UINT32)ae_cfg->min_sys_gain;
	ae_attr.sensor_gain_range.max = (UINT32)ae_cfg->max_sensor_gain;
	ae_attr.sensor_gain_range.min = (UINT32)ae_cfg->min_sensor_gain;
	ae_attr.isp_gain_range.max = (UINT32)ae_cfg->max_isp_gain;
	ae_attr.isp_gain_range.min = (UINT32)ae_cfg->min_isp_gain;
	ae_attr.frame_rate = (FLOAT)ae_cfg->frame_rate;
	ae_attr.slow_frame_rate = (FLOAT)ae_cfg->slow_frame_rate;
	ae_attr.speed = (UINT8)ae_cfg->speed;
	ae_attr.black_speed_bias = (UINT8)ae_cfg->black_speed_bias;
	ae_attr.interval = (UINT8)ae_cfg->interval;
	ae_attr.brightness = (UINT16)ae_cfg->brightness;
	ae_attr.tolerance = (UINT16)ae_cfg->tolerance;
	ae_attr.gain_thr_up = (UINT32)ae_cfg->gain_thr_up;
	ae_attr.gain_thr_down = (UINT32)ae_cfg->gain_thr_down;

	ae_attr.strategy.strength = (INT32)ae_cfg->exp_strength;

	ae_attr.roi.awb_weight = (UINT8)ae_cfg->roi_awb_weight;
	ae_attr.roi.luma_weight = (UINT8)ae_cfg->roi_luma_weight;
	ae_attr.roi.zone_lum_avg_weight = (UINT8)ae_cfg->roi_zone_lum_avg_weight;

	ae_attr.delay.black_delay_frame = (UINT16)ae_cfg->black_delay_frame;
	ae_attr.delay.white_delay_frame = (UINT16)ae_cfg->white_delay_frame;

	ae_attr.anti_flicker.enable = (BOOL)ae_cfg->anti_flicker.enable;
	ae_attr.anti_flicker.frequency = (UINT8)ae_cfg->anti_flicker.frequency;
	ae_attr.anti_flicker.luma_delta = (UINT16)ae_cfg->anti_flicker.luma_delta;

	ae_attr.manual.is_valid = (BOOL)ae_cfg->manual.enabled;
	ae_attr.manual.enable.val = (UINT32)ae_cfg->manual.flag;
	ae_attr.manual.exp_value = (UINT32)ae_cfg->manual.exp_value;
	ae_attr.manual.inttime = (UINT32)ae_cfg->manual.inttime;
	ae_attr.manual.sensor_gain = (UINT32)ae_cfg->manual.sensor_gain;
	ae_attr.manual.isp_gain = (UINT32)ae_cfg->manual.isp_gain;
	ae_attr.manual.sys_gain = (UINT32)ae_cfg->manual.sys_gain;

	for (INT32 i = 0; i < MPI_AE_ZONE_ROW; ++i) {
		for (INT32 j = 0; j < MPI_AE_ZONE_COLUMN; ++j) {
			ae_attr.zone_weight.manual_table[i][j] =
			        ae_cfg->zone_weight.manual_table[i * MPI_AE_ZONE_COLUMN + j];
		}
	}

	ae_attr.zone_weight.mode = ae_cfg->zone_weight.mode;

	ae_attr.inttime_range.max = (UINT32)ae_cfg->max_inttime;
	ae_attr.inttime_range.min = (UINT32)ae_cfg->min_inttime;

	ret = MPI_setAeAttr(path_idx, &ae_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getAe(MPI_PATH path_idx, AGTX_DIP_AE_CONF_S *ae_cfg)
{
	INT32 ret = 0;
	MPI_AE_ATTR_S ae_attr;

	ret = MPI_getAeAttr(path_idx, &ae_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* exp strategy*/
	ae_cfg->exp_strategy = ae_attr.strategy.mode;

	/* fps mode*/
	if (ae_attr.fps_mode == AE_FPS_FIXED) {
		ae_cfg->fps_mode = AGTX_FPS_MO_FIXED;
	} else if (ae_attr.fps_mode == AE_FPS_DROP) {
		ae_cfg->fps_mode = AGTX_FPS_MO_VARIABLE;
	} else {
		printf("AGTX_DIP_AE_CONF_S fps_mode is out of range\n");
		return MPI_FAILURE;
	}

	ae_cfg->max_sys_gain = ae_attr.sys_gain_range.max;
	ae_cfg->min_sys_gain = ae_attr.sys_gain_range.min;
	ae_cfg->max_sensor_gain = ae_attr.sensor_gain_range.max;
	ae_cfg->min_sensor_gain = ae_attr.sensor_gain_range.min;
	ae_cfg->max_isp_gain = ae_attr.isp_gain_range.max;
	ae_cfg->min_isp_gain = ae_attr.isp_gain_range.min;
	ae_cfg->frame_rate = ae_attr.frame_rate;
	ae_cfg->slow_frame_rate = ae_attr.slow_frame_rate;
	ae_cfg->speed = ae_attr.speed;
	ae_cfg->black_speed_bias = ae_attr.black_speed_bias;
	ae_cfg->interval = ae_attr.interval;
	ae_cfg->brightness = ae_attr.brightness;
	ae_cfg->tolerance = ae_attr.tolerance;
	ae_cfg->gain_thr_up = ae_attr.gain_thr_up;
	ae_cfg->gain_thr_down = ae_attr.gain_thr_down;

	ae_cfg->exp_strength = ae_attr.strategy.strength;

	ae_cfg->roi_awb_weight = ae_attr.roi.awb_weight;
	ae_cfg->roi_luma_weight = ae_attr.roi.luma_weight;
	ae_cfg->roi_zone_lum_avg_weight = ae_attr.roi.zone_lum_avg_weight;

	ae_cfg->black_delay_frame = ae_attr.delay.black_delay_frame;
	ae_cfg->white_delay_frame = ae_attr.delay.white_delay_frame;

	ae_cfg->anti_flicker.enable = ae_attr.anti_flicker.enable;
	ae_cfg->anti_flicker.frequency = ae_attr.anti_flicker.frequency;
	ae_cfg->anti_flicker.luma_delta = ae_attr.anti_flicker.luma_delta;

	ae_cfg->manual.enabled = ae_attr.manual.is_valid;
	ae_cfg->manual.flag = ae_attr.manual.enable.val;
	ae_cfg->manual.exp_value = ae_attr.manual.exp_value;
	ae_cfg->manual.inttime = ae_attr.manual.inttime;
	ae_cfg->manual.sensor_gain = ae_attr.manual.sensor_gain;
	ae_cfg->manual.isp_gain = ae_attr.manual.isp_gain;
	ae_cfg->manual.sys_gain = ae_attr.manual.sys_gain;

	for (INT32 i = 0; i < MPI_AE_ZONE_ROW; ++i) {
		for (INT32 j = 0; j < MPI_AE_ZONE_COLUMN; ++j) {
			ae_cfg->zone_weight.manual_table[i * MPI_AE_ZONE_COLUMN + j] =
			        ae_attr.zone_weight.manual_table[i][j];
		}
	}

	ae_cfg->zone_weight.mode = ae_attr.zone_weight.mode;

	ae_cfg->max_inttime = ae_attr.inttime_range.max;
	ae_cfg->min_inttime = ae_attr.inttime_range.min;

	ae_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setIso(MPI_PATH path_idx, const AGTX_DIP_ISO_CONF_S *iso_cfg)
{
	INT32 ret = 0;
	MPI_ISO_ATTR_S iso_attr;

	ret = MPI_getIsoAttr(path_idx, &iso_attr);

	iso_attr.mode = iso_cfg->mode;
	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		iso_attr.iso_auto.effective_iso[num] = (INT32)iso_cfg->auto_iso_table[num];
	}
	iso_attr.iso_manual.effective_iso = (INT32)iso_cfg->manual_iso;
	iso_attr.iso_type = iso_cfg->iso_type;
	iso_attr.daa.di_max = iso_cfg->daa.di_max;
	iso_attr.daa.di_rising_speed = iso_cfg->daa.di_rising_speed;
	iso_attr.daa.di_fallen_speed = iso_cfg->daa.di_fallen_speed;
	iso_attr.daa.qp_upper_th = iso_cfg->daa.qp_upper_th;
	iso_attr.daa.qp_lower_th = iso_cfg->daa.qp_lower_th;
	iso_attr.daa.enable = iso_cfg->daa.enable;

	ret = MPI_setIsoAttr(path_idx, &iso_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getIso(MPI_PATH path_idx, AGTX_DIP_ISO_CONF_S *iso_cfg)
{
	INT32 ret = 0;

	MPI_ISO_ATTR_S iso_attr;
	ret = MPI_getIsoAttr(path_idx, &iso_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	iso_cfg->mode = iso_attr.mode;
	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		iso_cfg->auto_iso_table[num] = iso_attr.iso_auto.effective_iso[num];
	}
	iso_cfg->manual_iso = iso_attr.iso_manual.effective_iso;
	iso_cfg->iso_type = iso_attr.iso_type;
	iso_cfg->daa.di_max = iso_attr.daa.di_max;
	iso_cfg->daa.di_rising_speed = iso_attr.daa.di_rising_speed;
	iso_cfg->daa.di_fallen_speed = iso_attr.daa.di_fallen_speed;
	iso_cfg->daa.qp_upper_th = iso_attr.daa.qp_upper_th;
	iso_cfg->daa.qp_lower_th = iso_attr.daa.qp_lower_th;
	iso_cfg->daa.enable = iso_attr.daa.enable;

	iso_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setAwb(MPI_PATH path_idx, const AGTX_DIP_AWB_CONF_S *awb_cfg)
{
	INT32 ret = 0;
	MPI_AWB_ATTR_S awb_attr;

	ret = MPI_getAwbAttr(path_idx, &awb_attr);

	awb_attr.speed = (UINT8)awb_cfg->speed;
	awb_attr.wht_density = (UINT8)awb_cfg->wht_density;
	awb_attr.r_extra_gain = (UINT8)awb_cfg->r_extra_gain;
	awb_attr.b_extra_gain = (UINT8)awb_cfg->b_extra_gain;
	awb_attr.g_extra_gain = (UINT8)awb_cfg->g_extra_gain;
	awb_attr.wht_weight = (UINT8)awb_cfg->wht_weight;
	awb_attr.gwd_weight = (UINT8)awb_cfg->gwd_weight;
	awb_attr.color_tolerance = (UINT8)awb_cfg->color_tolerance;
	awb_attr.max_lum_gain = (UINT8)awb_cfg->max_lum_gain;
	awb_attr.low_k = (UINT16)awb_cfg->low_k;
	awb_attr.high_k = (UINT16)awb_cfg->high_k;
	awb_attr.over_exp_th = (UINT16)awb_cfg->over_exp_th;
	awb_attr.ccm_domain = awb_cfg->ccm_domain;
	awb_attr.k_table_valid_size = (UINT8)awb_cfg->k_table_valid_size;

	for (INT32 k_num = 0; k_num < MPI_K_TABLE_ENTRY_NUM; k_num++) {
		awb_attr.k_table[k_num].k = (UINT16)awb_cfg->k_table_list[k_num].k;

		for (INT32 num = 0; num < MPI_AWB_CHN_NUM; num++) {
			awb_attr.k_table[k_num].gain[num] = (UINT16)awb_cfg->k_table_list[k_num].gain[num];
			awb_attr.delta_table[k_num].gain[num] = (INT16)awb_cfg->delta_table_list[k_num].gain[num];
		}

		for (INT32 m_num = 0; m_num < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); m_num++) {
			awb_attr.k_table[k_num].matrix[m_num] = (UINT16)awb_cfg->k_table_list[k_num].maxtrix[m_num];
		}

		awb_attr.bias_table[k_num].k = (UINT16)awb_cfg->k_table_bias_list[k_num].k;
		awb_attr.bias_table[k_num].color_tolerance_bias =
		        (UINT16)awb_cfg->k_table_bias_list[k_num].color_tolerance_bias;
		awb_attr.bias_table[k_num].wht_weight_bias = (UINT16)awb_cfg->k_table_bias_list[k_num].wht_weight_bias;
		awb_attr.bias_table[k_num].gwd_weight_bias = (UINT16)awb_cfg->k_table_bias_list[k_num].gwd_weight_bias;
		awb_attr.bias_table[k_num].r_extra_gain_bias =
		        (UINT16)awb_cfg->k_table_bias_list[k_num].r_extra_gain_bias;
		awb_attr.bias_table[k_num].g_extra_gain_bias =
		        (UINT16)awb_cfg->k_table_bias_list[k_num].g_extra_gain_bias;
		awb_attr.bias_table[k_num].b_extra_gain_bias =
		        (UINT16)awb_cfg->k_table_bias_list[k_num].b_extra_gain_bias;
	}


	ret = MPI_setAwbAttr(path_idx, &awb_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getAwb(MPI_PATH path_idx, AGTX_DIP_AWB_CONF_S *awb_cfg)
{
	INT32 ret = 0;
	MPI_AWB_ATTR_S awb_attr;

	ret = MPI_getAwbAttr(path_idx, &awb_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	awb_cfg->speed = awb_attr.speed;
	awb_cfg->wht_density = awb_attr.wht_density;
	awb_cfg->r_extra_gain = awb_attr.r_extra_gain;
	awb_cfg->b_extra_gain = awb_attr.b_extra_gain;
	awb_cfg->g_extra_gain = awb_attr.g_extra_gain;
	awb_cfg->wht_weight = awb_attr.wht_weight;
	awb_cfg->gwd_weight = awb_attr.gwd_weight;
	awb_cfg->color_tolerance = awb_attr.color_tolerance;
	awb_cfg->max_lum_gain = awb_attr.max_lum_gain;
	awb_cfg->low_k = awb_attr.low_k;
	awb_cfg->high_k = awb_attr.high_k;
	awb_cfg->over_exp_th = awb_attr.over_exp_th;
	awb_cfg->ccm_domain = awb_attr.ccm_domain;
	awb_cfg->k_table_valid_size = awb_attr.k_table_valid_size;

	for (INT32 k_num = 0; k_num < MPI_K_TABLE_ENTRY_NUM; k_num++) {
		awb_cfg->k_table_list[k_num].k = awb_attr.k_table[k_num].k;

		for (INT32 num = 0; num < MPI_AWB_CHN_NUM; num++) {
			awb_cfg->k_table_list[k_num].gain[num] = awb_attr.k_table[k_num].gain[num];
			awb_cfg->delta_table_list[k_num].gain[num] = awb_attr.delta_table[k_num].gain[num];
		}

		for (INT32 m_num = 0; m_num < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); m_num++) {
			awb_cfg->k_table_list[k_num].maxtrix[m_num] = awb_attr.k_table[k_num].matrix[m_num];
		}

		awb_cfg->k_table_bias_list[k_num].k = awb_attr.bias_table[k_num].k;
		awb_cfg->k_table_bias_list[k_num].color_tolerance_bias =
		        awb_attr.bias_table[k_num].color_tolerance_bias;
		awb_cfg->k_table_bias_list[k_num].wht_weight_bias = awb_attr.bias_table[k_num].wht_weight_bias;
		awb_cfg->k_table_bias_list[k_num].gwd_weight_bias = awb_attr.bias_table[k_num].gwd_weight_bias;
		awb_cfg->k_table_bias_list[k_num].r_extra_gain_bias = awb_attr.bias_table[k_num].r_extra_gain_bias;
		awb_cfg->k_table_bias_list[k_num].g_extra_gain_bias = awb_attr.bias_table[k_num].g_extra_gain_bias;
		awb_cfg->k_table_bias_list[k_num].b_extra_gain_bias = awb_attr.bias_table[k_num].b_extra_gain_bias;
	}

	awb_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setCsm(MPI_PATH path_idx, const AGTX_DIP_CSM_CONF_S *csm_cfg)
{
	INT32 ret = 0;
	MPI_CSM_ATTR_S csm_attr;
	ret = MPI_getCsmAttr(path_idx, &csm_attr);

	csm_attr.mode = csm_cfg->mode;
	csm_attr.bw_en = (UINT8)csm_cfg->bw_en;
	csm_attr.hue_angle = (INT16)csm_cfg->hue;
	csm_attr.csm_manual.saturation = (UINT8)csm_cfg->manual_sat;

	for (int num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		csm_attr.csm_auto.saturation[num] = (UINT8)csm_cfg->auto_sat_table[num];
	}

	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_color.coeff[i] = (INT16)csm_cfg->cst_color.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_color.offset[i] = (INT16)csm_cfg->cst_color.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_bw.coeff[i] = (INT16)csm_cfg->cst_bw.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_bw.offset[i] = (INT16)csm_cfg->cst_bw.offset[i];
	}

	csm_attr.cst_auto_en = (UINT8)csm_cfg->cst_auto_en;

	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[0].coeff[i] = (INT16)csm_cfg->cst_auto_0.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[0].offset[i] = (INT16)csm_cfg->cst_auto_0.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[1].coeff[i] = (INT16)csm_cfg->cst_auto_1.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[1].offset[i] = (INT16)csm_cfg->cst_auto_1.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[2].coeff[i] = (INT16)csm_cfg->cst_auto_2.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[2].offset[i] = (INT16)csm_cfg->cst_auto_2.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[3].coeff[i] = (INT16)csm_cfg->cst_auto_3.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[3].offset[i] = (INT16)csm_cfg->cst_auto_3.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[4].coeff[i] = (INT16)csm_cfg->cst_auto_4.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[4].offset[i] = (INT16)csm_cfg->cst_auto_4.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[5].coeff[i] = (INT16)csm_cfg->cst_auto_5.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[5].offset[i] = (INT16)csm_cfg->cst_auto_5.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[6].coeff[i] = (INT16)csm_cfg->cst_auto_6.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[6].offset[i] = (INT16)csm_cfg->cst_auto_6.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[7].coeff[i] = (INT16)csm_cfg->cst_auto_7.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[7].offset[i] = (INT16)csm_cfg->cst_auto_7.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[8].coeff[i] = (INT16)csm_cfg->cst_auto_8.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[8].offset[i] = (INT16)csm_cfg->cst_auto_8.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[9].coeff[i] = (INT16)csm_cfg->cst_auto_9.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[9].offset[i] = (INT16)csm_cfg->cst_auto_9.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_attr.cst_auto[10].coeff[i] = (INT16)csm_cfg->cst_auto_10.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_attr.cst_auto[10].offset[i] = (INT16)csm_cfg->cst_auto_10.offset[i];
	}

	ret = MPI_setCsmAttr(path_idx, &csm_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getCsm(MPI_PATH path_idx, AGTX_DIP_CSM_CONF_S *csm_cfg)
{
	INT32 ret = 0;
	MPI_CSM_ATTR_S csm_attr;

	ret = MPI_getCsmAttr(path_idx, &csm_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	csm_cfg->mode = csm_attr.mode;
	csm_cfg->bw_en = csm_attr.bw_en;
	csm_cfg->hue = csm_attr.hue_angle;
	csm_cfg->manual_sat = csm_attr.csm_manual.saturation;

	for (int num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		csm_cfg->auto_sat_table[num] = csm_attr.csm_auto.saturation[num];
	}

	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_color.coeff[i] = csm_attr.cst_color.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_color.offset[i] = csm_attr.cst_color.offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_bw.coeff[i] = csm_attr.cst_bw.coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_bw.offset[i] = csm_attr.cst_bw.offset[i];
	}

	csm_cfg->cst_auto_en = csm_attr.cst_auto_en;

	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_0.coeff[i] = csm_attr.cst_auto[0].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_0.offset[i] = csm_attr.cst_auto[0].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_1.coeff[i] = csm_attr.cst_auto[1].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_1.offset[i] = csm_attr.cst_auto[1].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_2.coeff[i] = csm_attr.cst_auto[2].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_2.offset[i] = csm_attr.cst_auto[2].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_3.coeff[i] = csm_attr.cst_auto[3].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_3.offset[i] = csm_attr.cst_auto[3].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_4.coeff[i] = csm_attr.cst_auto[4].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_4.offset[i] = csm_attr.cst_auto[4].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_5.coeff[i] = csm_attr.cst_auto[5].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_5.offset[i] = csm_attr.cst_auto[5].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_6.coeff[i] = csm_attr.cst_auto[6].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_6.offset[i] = csm_attr.cst_auto[6].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_7.coeff[i] = csm_attr.cst_auto[7].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_7.offset[i] = csm_attr.cst_auto[7].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_8.coeff[i] = csm_attr.cst_auto[8].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_8.offset[i] = csm_attr.cst_auto[8].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_9.coeff[i] = csm_attr.cst_auto[9].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_9.offset[i] = csm_attr.cst_auto[9].offset[i];
	}
	for (int i = 0; i < (MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM); i++) {
		csm_cfg->cst_auto_10.coeff[i] = csm_attr.cst_auto[10].coeff[i];
	}
	for (int i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		csm_cfg->cst_auto_10.offset[i] = csm_attr.cst_auto[10].offset[i];
	}

	csm_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setDms(MPI_PATH path_idx, const AGTX_DIP_DMS_CONF_S *dms_cfg)
{
	INT32 ret = 0;
	MPI_DMS_ATTR_S dms_attr;

	ret = MPI_getDmsAttr(path_idx, &dms_attr);
	/* check and convert to attr_s */
	dms_attr.mode = dms_cfg->mode;
	dms_attr.dms_manual.g_at_m_inter_ratio = dms_cfg->manual_g_at_m_inter_ratio;
	dms_attr.dms_manual.m_at_m_inter_ratio = dms_cfg->manual_m_at_m_inter_ratio;
	dms_attr.dms_manual.m_at_g_inter_ratio = dms_cfg->manual_m_at_g_inter_ratio;
	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		dms_attr.dms_auto.g_at_m_inter_ratio[i] = dms_cfg->auto_g_at_m_inter_ratio_list[i];
		dms_attr.dms_auto.m_at_m_inter_ratio[i] = dms_cfg->auto_m_at_m_inter_ratio_list[i];
		dms_attr.dms_auto.m_at_g_inter_ratio[i] = dms_cfg->auto_m_at_g_inter_ratio_list[i];
	}
	dms_attr.dms_ctrl_method = dms_cfg->dms_ctrl_method;

	ret = MPI_setDmsAttr(path_idx, &dms_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}
	return MPI_SUCCESS;
}

INT32 APP_DIP_getDms(MPI_PATH path_idx, AGTX_DIP_DMS_CONF_S *dms_cfg)
{
	INT32 ret = 0;
	MPI_DMS_ATTR_S dms_attr;

	ret = MPI_getDmsAttr(path_idx, &dms_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	dms_cfg->mode = dms_attr.mode;
	dms_cfg->manual_g_at_m_inter_ratio = dms_attr.dms_manual.g_at_m_inter_ratio;
	dms_cfg->manual_m_at_m_inter_ratio = dms_attr.dms_manual.m_at_m_inter_ratio;
	dms_cfg->manual_m_at_g_inter_ratio = dms_attr.dms_manual.m_at_g_inter_ratio;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		dms_cfg->auto_g_at_m_inter_ratio_list[i] = dms_attr.dms_auto.g_at_m_inter_ratio[i];
		dms_cfg->auto_m_at_m_inter_ratio_list[i] = dms_attr.dms_auto.m_at_m_inter_ratio[i];
		dms_cfg->auto_m_at_g_inter_ratio_list[i] = dms_attr.dms_auto.m_at_g_inter_ratio[i];
	}

	dms_cfg->video_dev_idx = 0;

	dms_cfg->dms_ctrl_method = dms_attr.dms_ctrl_method;
	return MPI_SUCCESS;
}

INT32 APP_DIP_setPca(MPI_PATH path_idx, const AGTX_DIP_PCA_CONF_S *pca_cfg)
{
	INT32 ret = 0;
	MPI_PCA_TABLE_S pca_table;

	ret = MPI_getPcaTable(path_idx, &pca_table);

	int i, j, k;
	for (i = 0; i < PCA_L_ENTRY_NUM; i++) {
		for (j = 0; j < PCA_S_ENTRY_NUM; j++) {
			for (k = 0; k < PCA_H_ENTRY_NUM; k++) {
				pca_table.h[i][j][k] = pca_cfg->h_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM +
				                                        j * PCA_H_ENTRY_NUM + k];
				pca_table.s[i][j][k] = pca_cfg->s_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM +
				                                        j * PCA_H_ENTRY_NUM + k];
				pca_table.l[i][j][k] = pca_cfg->l_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM +
				                                        j * PCA_H_ENTRY_NUM + k];
			}
		}
	}

	ret = MPI_setPcaTable(path_idx, &pca_table);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getPca(MPI_PATH path_idx, AGTX_DIP_PCA_CONF_S *pca_cfg)
{
	INT32 ret = 0;
	MPI_PCA_TABLE_S pca_table;

	ret = MPI_getPcaTable(path_idx, &pca_table);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	int i, j, k;
	for (i = 0; i < PCA_L_ENTRY_NUM; i++) {
		for (j = 0; j < PCA_S_ENTRY_NUM; j++) {
			for (k = 0; k < PCA_H_ENTRY_NUM; k++) {
				pca_cfg->h_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM + j * PCA_H_ENTRY_NUM + k] = pca_table.h[i][j][k];
				pca_cfg->s_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM + j * PCA_H_ENTRY_NUM + k] = pca_table.s[i][j][k];
				pca_cfg->l_table[i * PCA_H_ENTRY_NUM * PCA_S_ENTRY_NUM + j * PCA_H_ENTRY_NUM + k] = pca_table.l[i][j][k];
			}
		}
	}

	pca_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setPta(MPI_PATH path_idx, const AGTX_DIP_PTA_CONF_S *pta_cfg)
{
	INT32 ret = 0;
	MPI_PTA_ATTR_S pta_attr;

	ret = MPI_getPtaAttr(path_idx, &pta_attr);

	pta_attr.mode = pta_cfg->mode;
	pta_attr.brightness = (UINT8)pta_cfg->brightness;
	pta_attr.contrast = (UINT8)pta_cfg->contrast;
	pta_attr.break_point = (UINT8)pta_cfg->break_point;

	for (INT32 num = 0; num < MPI_PTA_CURVE_ENTRY_NUM; num++) {
		pta_attr.pta_manual.curve[num] = (UINT32)pta_cfg->curve[num];
	}

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		pta_attr.pta_auto.tone[num] = (UINT32)pta_cfg->auto_tone_table[num];
	}

	ret = MPI_setPtaAttr(path_idx, &pta_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getPta(MPI_PATH path_idx, AGTX_DIP_PTA_CONF_S *pta_cfg)
{
	INT32 ret = 0;
	MPI_PTA_ATTR_S pta_attr;
	ret = MPI_getPtaAttr(path_idx, &pta_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	pta_cfg->mode = pta_attr.mode;
	pta_cfg->brightness = pta_attr.brightness;
	pta_cfg->contrast = pta_attr.contrast;
	pta_cfg->break_point = pta_attr.break_point;

	for (INT32 num = 0; num < MPI_PTA_CURVE_ENTRY_NUM; num++) {
		pta_cfg->curve[num] = pta_attr.pta_manual.curve[num];
	}

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		pta_cfg->auto_tone_table[num] = pta_attr.pta_auto.tone[num];
	}

	pta_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setShp(MPI_PATH path_idx, const AGTX_DIP_SHP_CONF_S *shp_cfg)
{
	INT32 ret = 0;
	MPI_SHP_ATTR_V2_S shp_attr;

	ret = MPI_getShpAttrV2(path_idx, &shp_attr);

	shp_attr.mode = shp_cfg->mode;
	shp_attr.shp_manual_v2.sharpness = shp_cfg->manual_shp;
	shp_attr.motion_adaptive_en = shp_cfg->motion_adaptive_en;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_attr.shp_auto_v2.sharpness[num] = shp_cfg->auto_shp_table[num];
	}

	shp_attr.shp_type = shp_cfg->shp_type;
	shp_attr.strength = shp_cfg->strength;
	shp_attr.shp_ex_manual.hpf_ratio = shp_cfg->manual_hpf_ratio;
	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_attr.shp_ex_auto.hpf_ratio[num] = shp_cfg->auto_hpf_ratio[num];
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_TRANSFER_CURVE_SIZE; num++) {
		shp_attr.shp_ex_manual.transfer_curve.x[num] = shp_cfg->manual_shp_transfer_curve[num].x;
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_TRANSFER_CURVE_SIZE; num++) {
		shp_attr.shp_ex_manual.transfer_curve.y[num] = shp_cfg->manual_shp_transfer_curve[num].y;
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; num++) {
		shp_attr.shp_ex_manual.luma_ctrl_gain.x[num] = shp_cfg->manual_shp_luma_ctrl_gain[num].x;
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; num++) {
		shp_attr.shp_ex_manual.luma_ctrl_gain.y[num] = shp_cfg->manual_shp_luma_ctrl_gain[num].y;
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[0].x[num] = shp_cfg->auto_shp_transfer_curve_0[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[1].x[num] = shp_cfg->auto_shp_transfer_curve_1[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[2].x[num] = shp_cfg->auto_shp_transfer_curve_2[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[3].x[num] = shp_cfg->auto_shp_transfer_curve_3[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[4].x[num] = shp_cfg->auto_shp_transfer_curve_4[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[5].x[num] = shp_cfg->auto_shp_transfer_curve_5[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[6].x[num] = shp_cfg->auto_shp_transfer_curve_6[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[7].x[num] = shp_cfg->auto_shp_transfer_curve_7[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[8].x[num] = shp_cfg->auto_shp_transfer_curve_8[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[9].x[num] = shp_cfg->auto_shp_transfer_curve_9[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[10].x[num] = shp_cfg->auto_shp_transfer_curve_10[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[0].y[num] = shp_cfg->auto_shp_transfer_curve_0[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[1].y[num] = shp_cfg->auto_shp_transfer_curve_1[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[2].y[num] = shp_cfg->auto_shp_transfer_curve_2[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[3].y[num] = shp_cfg->auto_shp_transfer_curve_3[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[4].y[num] = shp_cfg->auto_shp_transfer_curve_4[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[5].y[num] = shp_cfg->auto_shp_transfer_curve_5[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[6].y[num] = shp_cfg->auto_shp_transfer_curve_6[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[7].y[num] = shp_cfg->auto_shp_transfer_curve_7[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[8].y[num] = shp_cfg->auto_shp_transfer_curve_8[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[9].y[num] = shp_cfg->auto_shp_transfer_curve_9[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; num++) {
		shp_attr.shp_ex_auto.transfer_curve[10].y[num] = shp_cfg->auto_shp_transfer_curve_10[num].y;
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[0].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_0[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[1].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_1[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[2].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_2[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[3].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_3[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[4].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_4[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[5].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_5[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[6].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_6[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[7].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_7[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[8].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_8[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[9].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_9[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[10].x[num] = shp_cfg->auto_shp_luma_ctrl_gain_10[num].x;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[0].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_0[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[1].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_1[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[2].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_2[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[3].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_3[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[4].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_4[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[5].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_5[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[6].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_6[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[7].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_7[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[8].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_8[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[9].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_9[num].y;
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; num++) {
		shp_attr.shp_ex_auto.luma_ctrl_gain[10].y[num] = shp_cfg->auto_shp_luma_ctrl_gain_10[num].y;
	}

	shp_attr.shp_ex_manual.soft_clip_slope = shp_cfg->manual_soft_clip_slope;
	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_attr.shp_ex_auto.soft_clip_slope[num] = shp_cfg->auto_soft_clip_slope[num];
	}

	shp_attr.ma_weak_shp_ratio = shp_cfg->ma_weak_shp_ratio;
	shp_attr.ma_conf_low_th = shp_cfg->ma_conf_low_th;
	shp_attr.ma_conf_high_th = shp_cfg->ma_conf_high_th;

	ret = MPI_setShpAttrV2(path_idx, &shp_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}
	return MPI_SUCCESS;
}

INT32 APP_DIP_getShp(MPI_PATH path_idx, AGTX_DIP_SHP_CONF_S *shp_cfg)
{
	INT32 ret = 0;
	MPI_SHP_ATTR_V2_S shp_attr;

	ret = MPI_getShpAttrV2(path_idx, &shp_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	shp_cfg->mode = shp_attr.mode;
	shp_cfg->manual_shp = shp_attr.shp_manual_v2.sharpness;
	shp_cfg->motion_adaptive_en = shp_attr.motion_adaptive_en;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_cfg->auto_shp_table[num] = shp_attr.shp_auto_v2.sharpness[num];
	}

	shp_cfg->shp_type = shp_attr.shp_type;
	shp_cfg->strength = shp_attr.strength;
	shp_cfg->manual_hpf_ratio = shp_attr.shp_ex_manual.hpf_ratio;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_cfg->auto_hpf_ratio[num] = shp_attr.shp_ex_auto.hpf_ratio[num];
	}

	for (INT32 num = 0; num < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; num++) {
		shp_cfg->manual_shp_transfer_curve[num].x = shp_attr.shp_ex_manual.transfer_curve.x[num];
	}
	for (INT32 num = 0; num < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; num++) {
		shp_cfg->manual_shp_transfer_curve[num].y = shp_attr.shp_ex_manual.transfer_curve.y[num];
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; num++) {
		shp_cfg->manual_shp_luma_ctrl_gain[num].x = shp_attr.shp_ex_manual.luma_ctrl_gain.x[num];
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; num++) {
		shp_cfg->manual_shp_luma_ctrl_gain[num].y = shp_attr.shp_ex_manual.luma_ctrl_gain.y[num];
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_0[num].x = shp_attr.shp_ex_auto.transfer_curve[0].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_1[num].x = shp_attr.shp_ex_auto.transfer_curve[1].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_2[num].x = shp_attr.shp_ex_auto.transfer_curve[2].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_3[num].x = shp_attr.shp_ex_auto.transfer_curve[3].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_4[num].x = shp_attr.shp_ex_auto.transfer_curve[4].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_5[num].x = shp_attr.shp_ex_auto.transfer_curve[5].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_6[num].x = shp_attr.shp_ex_auto.transfer_curve[6].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_7[num].x = shp_attr.shp_ex_auto.transfer_curve[7].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_8[num].x = shp_attr.shp_ex_auto.transfer_curve[8].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_9[num].x = shp_attr.shp_ex_auto.transfer_curve[9].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_10[num].x = shp_attr.shp_ex_auto.transfer_curve[10].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_0[num].y = shp_attr.shp_ex_auto.transfer_curve[0].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_1[num].y = shp_attr.shp_ex_auto.transfer_curve[1].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_2[num].y = shp_attr.shp_ex_auto.transfer_curve[2].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_3[num].y = shp_attr.shp_ex_auto.transfer_curve[3].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_4[num].y = shp_attr.shp_ex_auto.transfer_curve[4].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_5[num].y = shp_attr.shp_ex_auto.transfer_curve[5].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_6[num].y = shp_attr.shp_ex_auto.transfer_curve[6].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_7[num].y = shp_attr.shp_ex_auto.transfer_curve[7].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_8[num].y = shp_attr.shp_ex_auto.transfer_curve[8].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_9[num].y = shp_attr.shp_ex_auto.transfer_curve[9].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; num++) {
		shp_cfg->auto_shp_transfer_curve_10[num].y = shp_attr.shp_ex_auto.transfer_curve[10].y[num];
	}

	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_0[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[0].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_1[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[1].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_2[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[2].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_3[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[3].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_4[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[4].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_5[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[5].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_6[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[6].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_7[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[7].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_8[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[8].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_9[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[9].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_10[num].x = shp_attr.shp_ex_auto.luma_ctrl_gain[10].x[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_0[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[0].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_1[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[1].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_2[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[2].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_3[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[3].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_4[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[4].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_5[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[5].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_6[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[6].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_7[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[7].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_8[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[8].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_9[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[9].y[num];
	}
	for (INT32 num = 0; num < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; num++) {
		shp_cfg->auto_shp_luma_ctrl_gain_10[num].y = shp_attr.shp_ex_auto.luma_ctrl_gain[10].y[num];
	}

	shp_cfg->manual_soft_clip_slope = shp_attr.shp_ex_manual.soft_clip_slope;
	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		shp_cfg->auto_soft_clip_slope[num] = shp_attr.shp_ex_auto.soft_clip_slope[num];
	}

	shp_cfg->ma_weak_shp_ratio = shp_attr.ma_weak_shp_ratio;
	shp_cfg->ma_conf_low_th = shp_attr.ma_conf_low_th;
	shp_cfg->ma_conf_high_th = shp_attr.ma_conf_high_th;

	shp_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setNr(MPI_PATH path_idx, const AGTX_DIP_NR_CONF_S *nr_cfg)
{
	INT32 ret = 0;
	MPI_NR_ATTR_S nr_attr;

	ret = MPI_getNrAttr(path_idx, &nr_attr);

	nr_attr.mode = nr_cfg->mode;

	switch (nr_cfg->lut_type) {
	case AGTX_NR_LUT_TYPE_0:
		nr_attr.lut_type = NR_LUT_TYPE_0;
		break;
	case AGTX_NR_LUT_TYPE_1:
		nr_attr.lut_type = NR_LUT_TYPE_1;
		break;
	case AGTX_NR_LUT_TYPE_2:
		nr_attr.lut_type = NR_LUT_TYPE_2;
		break;
	case AGTX_NR_LUT_TYPE_3:
		nr_attr.lut_type = NR_LUT_TYPE_3;
		break;
	default:
		nr_attr.lut_type = NR_LUT_TYPE_0;
		break;
	}

	nr_attr.motion_comp = (UINT8)nr_cfg->motion_comp;
	nr_attr.trail_suppress = (UINT8)nr_cfg->trail_suppress;
	nr_attr.ghost_remove = (UINT8)nr_cfg->ghost_remove;
	nr_attr.ma_y_strength = (UINT8)nr_cfg->ma_y_strength;
	nr_attr.mc_y_strength = (UINT8)nr_cfg->mc_y_strength;
	nr_attr.ma_c_strength = (UINT8)nr_cfg->ma_c_strength;
	nr_attr.ratio_3d = (UINT8)nr_cfg->ratio_3d;
	nr_attr.mc_y_level_offset = (INT16)nr_cfg->mc_y_level_offset;
	nr_attr.me_frame_fallback_en = (UINT8)nr_cfg->me_frame_fallback_en;
	nr_attr.fss_ratio_min = (UINT16)nr_cfg->fss_ratio_min;
	nr_attr.fss_ratio_max = (UINT16)nr_cfg->fss_ratio_max;

	nr_attr.nr_manual.y_level_3d = (UINT8)nr_cfg->manual_y_level_3d;
	nr_attr.nr_manual.c_level_3d = (UINT8)nr_cfg->manual_c_level_3d;
	nr_attr.nr_manual.y_level_2d = (UINT8)nr_cfg->manual_y_level_2d;
	nr_attr.nr_manual.c_level_2d = (UINT8)nr_cfg->manual_c_level_2d;
	nr_attr.nr_manual.fss_y_level_3d = (UINT8)nr_cfg->manual_fss_y_level_3d;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		nr_attr.nr_auto.y_level_3d[num] = (UINT8)nr_cfg->auto_y_level_3d_list[num];
		nr_attr.nr_auto.c_level_3d[num] = (UINT8)nr_cfg->auto_c_level_3d_list[num];
		nr_attr.nr_auto.y_level_2d[num] = (UINT8)nr_cfg->auto_y_level_2d_list[num];
		nr_attr.nr_auto.c_level_2d[num] = (UINT8)nr_cfg->auto_c_level_2d_list[num];
		nr_attr.nr_auto.fss_y_level_3d[num] = (UINT8)nr_cfg->auto_fss_y_level_3d_list[num];
	}
	ret = MPI_setNrAttr(path_idx, &nr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getNr(MPI_PATH path_idx, AGTX_DIP_NR_CONF_S *nr_cfg)
{
	INT32 ret = 0;
	MPI_NR_ATTR_S nr_attr;

	ret = MPI_getNrAttr(path_idx, &nr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	nr_cfg->mode = nr_attr.mode;

	switch (nr_attr.lut_type) {
	case NR_LUT_TYPE_0:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_0;
		break;
	case NR_LUT_TYPE_1:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_1;
		break;
	case NR_LUT_TYPE_2:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_2;
		break;
	case NR_LUT_TYPE_3:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_3;
		break;
	default:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_0;
		break;
	}

	nr_cfg->motion_comp = nr_attr.motion_comp;
	nr_cfg->trail_suppress = nr_attr.trail_suppress;
	nr_cfg->ghost_remove = nr_attr.ghost_remove;
	nr_cfg->ma_y_strength = nr_attr.ma_y_strength;
	nr_cfg->mc_y_strength = nr_attr.mc_y_strength;
	nr_cfg->ma_c_strength = nr_attr.ma_c_strength;
	nr_cfg->ratio_3d = nr_attr.ratio_3d;
	nr_cfg->mc_y_level_offset = nr_attr.mc_y_level_offset;
	nr_cfg->me_frame_fallback_en = nr_attr.me_frame_fallback_en;
	nr_cfg->fss_ratio_min = nr_attr.fss_ratio_min;
	nr_cfg->fss_ratio_max = nr_attr.fss_ratio_max;

	nr_cfg->manual_y_level_3d = nr_attr.nr_manual.y_level_3d;
	nr_cfg->manual_c_level_3d = nr_attr.nr_manual.c_level_3d;
	nr_cfg->manual_y_level_2d = nr_attr.nr_manual.y_level_2d;
	nr_cfg->manual_c_level_2d = nr_attr.nr_manual.c_level_2d;
	nr_cfg->manual_fss_y_level_3d = nr_attr.nr_manual.fss_y_level_3d;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		nr_cfg->auto_y_level_3d_list[num] = nr_attr.nr_auto.y_level_3d[num];
		nr_cfg->auto_c_level_3d_list[num] = nr_attr.nr_auto.c_level_3d[num];
		nr_cfg->auto_y_level_2d_list[num] = nr_attr.nr_auto.y_level_2d[num];
		nr_cfg->auto_c_level_2d_list[num] = nr_attr.nr_auto.c_level_2d[num];
		nr_cfg->auto_fss_y_level_3d_list[num] = nr_attr.nr_auto.fss_y_level_3d[num];
	}

	nr_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setWinShp(MPI_WIN win_idx, const AGTX_SHP_WINDOW_PARAM_S *shp_cfg)
{
	INT32 ret = 0;
	MPI_SHP_ATTR_V2_S shp_attr;

	ret = MPI_getWinShpAttrV2(win_idx, &shp_attr);

	shp_attr.strength = shp_cfg->strength;

	ret = MPI_setWinShpAttrV2(win_idx, &shp_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getWinShp(MPI_WIN win_idx, AGTX_SHP_WINDOW_PARAM_S *shp_cfg)
{
	INT32 ret = 0;
	MPI_SHP_ATTR_V2_S shp_attr;
	ret = MPI_getWinShpAttrV2(win_idx, &shp_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	shp_cfg->strength = shp_attr.strength;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setWinNr(MPI_WIN win_idx, const AGTX_NR_WINDOW_PARAM_S *nr_cfg)
{
	INT32 ret = 0;
	MPI_NR_ATTR_S nr_attr;

	ret = MPI_getWinNrAttr(win_idx, &nr_attr);

	nr_attr.mode = nr_cfg->mode;

	switch (nr_cfg->lut_type) {
	case AGTX_NR_LUT_TYPE_0:
		nr_attr.lut_type = NR_LUT_TYPE_0;
		break;
	case AGTX_NR_LUT_TYPE_1:
		nr_attr.lut_type = NR_LUT_TYPE_1;
		break;
	case AGTX_NR_LUT_TYPE_2:
		nr_attr.lut_type = NR_LUT_TYPE_2;
		break;
	case AGTX_NR_LUT_TYPE_3:
		nr_attr.lut_type = NR_LUT_TYPE_3;
		break;
	default:
		nr_attr.lut_type = NR_LUT_TYPE_0;
		break;
	}

	nr_attr.motion_comp = (UINT8)nr_cfg->motion_comp;
	nr_attr.trail_suppress = (UINT8)nr_cfg->trail_suppress;
	nr_attr.ghost_remove = (UINT8)nr_cfg->ghost_remove;
	nr_attr.ma_y_strength = (UINT8)nr_cfg->ma_y_strength;
	nr_attr.mc_y_strength = (UINT8)nr_cfg->mc_y_strength;
	nr_attr.ma_c_strength = (UINT8)nr_cfg->ma_c_strength;
	nr_attr.ratio_3d = (UINT8)nr_cfg->ratio_3d;
	nr_attr.mc_y_level_offset = (INT16)nr_cfg->mc_y_level_offset;
	nr_attr.me_frame_fallback_en = (UINT8)nr_cfg->me_frame_fallback_en;
	nr_attr.fss_ratio_min = (UINT16)nr_cfg->fss_ratio_min;
	nr_attr.fss_ratio_max = (UINT16)nr_cfg->fss_ratio_max;

	nr_attr.nr_manual.y_level_3d = (UINT8)nr_cfg->manual_y_level_3d;
	nr_attr.nr_manual.c_level_3d = (UINT8)nr_cfg->manual_c_level_3d;
	nr_attr.nr_manual.y_level_2d = (UINT8)nr_cfg->manual_y_level_2d;
	nr_attr.nr_manual.c_level_2d = (UINT8)nr_cfg->manual_c_level_2d;
	nr_attr.nr_manual.fss_y_level_3d = (UINT8)nr_cfg->manual_fss_y_level_3d;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		nr_attr.nr_auto.y_level_3d[num] = (UINT8)nr_cfg->auto_y_level_3d_list[num];
		nr_attr.nr_auto.c_level_3d[num] = (UINT8)nr_cfg->auto_c_level_3d_list[num];
		nr_attr.nr_auto.y_level_2d[num] = (UINT8)nr_cfg->auto_y_level_2d_list[num];
		nr_attr.nr_auto.c_level_2d[num] = (UINT8)nr_cfg->auto_c_level_2d_list[num];
		nr_attr.nr_auto.fss_y_level_3d[num] = (UINT8)nr_cfg->auto_fss_y_level_3d_list[num];
	}
	ret = MPI_setWinNrAttr(win_idx, &nr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getWinNr(MPI_WIN win_idx, AGTX_NR_WINDOW_PARAM_S *nr_cfg)
{
	INT32 ret = 0;
	MPI_NR_ATTR_S nr_attr;

	ret = MPI_getWinNrAttr(win_idx, &nr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	nr_cfg->mode = nr_attr.mode;

	switch (nr_attr.lut_type) {
	case NR_LUT_TYPE_0:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_0;
		break;
	case NR_LUT_TYPE_1:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_1;
		break;
	case NR_LUT_TYPE_2:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_2;
		break;
	case NR_LUT_TYPE_3:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_3;
		break;
	default:
		nr_cfg->lut_type = AGTX_NR_LUT_TYPE_0;
		break;
	}

	nr_cfg->motion_comp = nr_attr.motion_comp;
	nr_cfg->trail_suppress = nr_attr.trail_suppress;
	nr_cfg->ghost_remove = nr_attr.ghost_remove;
	nr_cfg->ma_y_strength = nr_attr.ma_y_strength;
	nr_cfg->mc_y_strength = nr_attr.mc_y_strength;
	nr_cfg->ma_c_strength = nr_attr.ma_c_strength;
	nr_cfg->ratio_3d = nr_attr.ratio_3d;
	nr_cfg->mc_y_level_offset = nr_attr.mc_y_level_offset;
	nr_cfg->me_frame_fallback_en = nr_attr.me_frame_fallback_en;
	nr_cfg->fss_ratio_min = nr_attr.fss_ratio_min;
	nr_cfg->fss_ratio_max = nr_attr.fss_ratio_max;

	nr_cfg->manual_y_level_3d = nr_attr.nr_manual.y_level_3d;
	nr_cfg->manual_c_level_3d = nr_attr.nr_manual.c_level_3d;
	nr_cfg->manual_y_level_2d = nr_attr.nr_manual.y_level_2d;
	nr_cfg->manual_c_level_2d = nr_attr.nr_manual.c_level_2d;
	nr_cfg->manual_fss_y_level_3d = nr_attr.nr_manual.fss_y_level_3d;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		nr_cfg->auto_y_level_3d_list[num] = nr_attr.nr_auto.y_level_3d[num];
		nr_cfg->auto_c_level_3d_list[num] = nr_attr.nr_auto.c_level_3d[num];
		nr_cfg->auto_y_level_2d_list[num] = nr_attr.nr_auto.y_level_2d[num];
		nr_cfg->auto_c_level_2d_list[num] = nr_attr.nr_auto.c_level_2d[num];
		nr_cfg->auto_fss_y_level_3d_list[num] = nr_attr.nr_auto.fss_y_level_3d[num];
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_setTe(MPI_PATH path_idx, const AGTX_DIP_TE_CONF_S *te_cfg)
{
	INT32 ret = 0;
	MPI_TE_ATTR_S te_attr;
	INT32 i = 0;

	ret = MPI_getTeAttr(path_idx, &te_attr);

	te_attr.mode = te_cfg->mode;

	for (INT32 num = 0; num < MPI_TE_CURVE_ENTRY_NUM; num++) {
		te_attr.te_normal.curve[num] = (UINT32)te_cfg->normal_ctl[num];
	}

	te_attr.te_wdr.brightness = (UINT16)te_cfg->wdr_ctl.brightness;
	te_attr.te_wdr.strength = (UINT16)te_cfg->wdr_ctl.strength;
	te_attr.te_wdr.saliency = (UINT16)te_cfg->wdr_ctl.saliency;
	te_attr.te_wdr.iso_weight = (UINT8)te_cfg->wdr_ctl.iso_weight;
	te_attr.te_wdr.dark_enhance = (UINT8)te_cfg->wdr_ctl.dark_enhance;
	te_attr.te_wdr.iso_max = (UINT32)te_cfg->wdr_ctl.iso_max;
	te_attr.te_wdr.interval = (UINT8)te_cfg->wdr_ctl.interval;
	te_attr.te_wdr.precision = (UINT8)te_cfg->wdr_ctl.precision;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		te_attr.te_wdr.noise_cstr[num] = (UINT16)te_cfg->wdr_ctl.noise_cstr[num];
	}

	te_attr.te_wdr_auto.dri_type = te_cfg->wdr_auto_ctl.dri_type;

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_wdr_auto.dri_gain[i] = (INT32)te_cfg->wdr_auto_ctl.dri_gain[i];
		te_attr.te_wdr_auto.dri_offset[i] = (INT32)te_cfg->wdr_auto_ctl.dri_offset[i];
		te_attr.te_wdr_auto.noise_cstr[i] = (UINT16)te_cfg->wdr_auto_ctl.noise_cstr[i];
	}

	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		te_attr.te_wdr_auto.strength[i] = (UINT16)te_cfg->wdr_auto_ctl.strength[i];
		te_attr.te_wdr_auto.brightness[i] = (UINT16)te_cfg->wdr_auto_ctl.brightness[i];
		te_attr.te_wdr_auto.saliency[i] = (UINT16)te_cfg->wdr_auto_ctl.saliency[i];
		te_attr.te_wdr_auto.dark_enhance[i] = (UINT8)te_cfg->wdr_auto_ctl.dark_enhance[i];
	}

	te_attr.te_wdr_auto.iso_max = (UINT32)te_cfg->wdr_auto_ctl.iso_max;
	te_attr.te_wdr_auto.iso_weight = (UINT8)te_cfg->wdr_auto_ctl.iso_weight;
	te_attr.te_wdr_auto.interval = (UINT8)te_cfg->wdr_auto_ctl.interval;
	te_attr.te_wdr_auto.precision = (UINT8)te_cfg->wdr_auto_ctl.precision;

	te_attr.te_adapt.strength = (INT32)te_cfg->adapt_ctl.strength;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.dark_enhance[i] = (INT32)te_cfg->adapt_ctl.dark_enhance[i];
	}
	te_attr.te_adapt.te_adapt_based_type = te_cfg->adapt_ctl.te_adapt_based_type;
	te_attr.te_adapt.str_auto_en = (INT32)te_cfg->adapt_ctl.str_auto_en;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.str_auto[i] = (INT32)te_cfg->adapt_ctl.str_auto[i];
	}
	te_attr.te_adapt.speed = (INT32)te_cfg->adapt_ctl.speed;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.white_th[i] = (INT32)te_cfg->adapt_ctl.white_th[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.black_th[i] = (INT32)te_cfg->adapt_ctl.black_th[i];
	}
	te_attr.te_adapt.max_str_prec_sel = (INT32)te_cfg->adapt_ctl.max_str_prec_sel;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.max_str[i] = (INT32)te_cfg->adapt_ctl.max_str[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.dark_protect_smooth[i] = (INT32)te_cfg->adapt_ctl.dark_protect_smooth[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.dark_protect_str[i] = (INT32)te_cfg->adapt_ctl.dark_protect_str[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_attr.te_adapt.dark_enhance_th[i] = (INT32)te_cfg->adapt_ctl.dark_enhance_th[i];
	}

	ret = MPI_setTeAttr(path_idx, &te_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getTe(MPI_PATH path_idx, AGTX_DIP_TE_CONF_S *te_cfg)
{
	INT32 ret = 0;
	MPI_TE_ATTR_S te_attr;
	INT32 i = 0;

	ret = MPI_getTeAttr(path_idx, &te_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	te_cfg->mode = te_attr.mode;

	for (INT32 num = 0; num < MPI_TE_CURVE_ENTRY_NUM; num++) {
		te_cfg->normal_ctl[num] = te_attr.te_normal.curve[num];
	}

	te_cfg->wdr_ctl.brightness = te_attr.te_wdr.brightness;
	te_cfg->wdr_ctl.strength = te_attr.te_wdr.strength;
	te_cfg->wdr_ctl.saliency = te_attr.te_wdr.saliency;
	te_cfg->wdr_ctl.iso_weight = te_attr.te_wdr.iso_weight;
	te_cfg->wdr_ctl.dark_enhance = te_attr.te_wdr.dark_enhance;
	te_cfg->wdr_ctl.iso_max = te_attr.te_wdr.iso_max;
	te_cfg->wdr_ctl.interval = te_attr.te_wdr.interval;
	te_cfg->wdr_ctl.precision = te_attr.te_wdr.precision;

	for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
		te_cfg->wdr_ctl.noise_cstr[num] = te_attr.te_wdr.noise_cstr[num];
	}

	te_cfg->wdr_auto_ctl.dri_type = te_attr.te_wdr_auto.dri_type;

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->wdr_auto_ctl.dri_gain[i] = te_attr.te_wdr_auto.dri_gain[i];
		te_cfg->wdr_auto_ctl.dri_offset[i] = te_attr.te_wdr_auto.dri_offset[i];
		te_cfg->wdr_auto_ctl.noise_cstr[i] = te_attr.te_wdr_auto.noise_cstr[i];
	}

	for (i = 0; i < MPI_DRI_LUT_ENTRY_NUM; i++) {
		te_cfg->wdr_auto_ctl.strength[i] = te_attr.te_wdr_auto.strength[i];
		te_cfg->wdr_auto_ctl.brightness[i] = te_attr.te_wdr_auto.brightness[i];
		te_cfg->wdr_auto_ctl.saliency[i] = te_attr.te_wdr_auto.saliency[i];
		te_cfg->wdr_auto_ctl.dark_enhance[i] = te_attr.te_wdr_auto.dark_enhance[i];
	}

	te_cfg->wdr_auto_ctl.iso_max = te_attr.te_wdr_auto.iso_max;
	te_cfg->wdr_auto_ctl.iso_weight = te_attr.te_wdr_auto.iso_weight;
	te_cfg->wdr_auto_ctl.interval = te_attr.te_wdr_auto.interval;
	te_cfg->wdr_auto_ctl.precision = te_attr.te_wdr_auto.precision;

	te_cfg->adapt_ctl.strength = te_attr.te_adapt.strength;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.dark_enhance[i] = te_attr.te_adapt.dark_enhance[i];
	}
	te_cfg->adapt_ctl.te_adapt_based_type = te_attr.te_adapt.te_adapt_based_type;
	te_cfg->adapt_ctl.str_auto_en = te_attr.te_adapt.str_auto_en;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.str_auto[i] = te_attr.te_adapt.str_auto[i];
	}
	te_cfg->adapt_ctl.speed = te_attr.te_adapt.speed;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.white_th[i] = te_attr.te_adapt.white_th[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.black_th[i] = te_attr.te_adapt.black_th[i];
	}
	te_cfg->adapt_ctl.max_str_prec_sel = te_attr.te_adapt.max_str_prec_sel;
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.max_str[i] = te_attr.te_adapt.max_str[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.dark_protect_smooth[i] = te_attr.te_adapt.dark_protect_smooth[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.dark_protect_str[i] = te_attr.te_adapt.dark_protect_str[i];
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		te_cfg->adapt_ctl.dark_enhance_th[i] = te_attr.te_adapt.dark_enhance_th[i];
	}
	te_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setGamma(MPI_PATH path_idx, const AGTX_DIP_GAMMA_CONF_S *gamma_cfg)
{
	INT32 ret = 0;
	MPI_GAMMA_ATTR_S gamma_attr;

	ret = MPI_getGammaAttr(path_idx, &gamma_attr);

	gamma_attr.mode = gamma_cfg->gamma;

	for (int i = 0; i < MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE; i++) {
		gamma_attr.gma_manual.curve[i] = gamma_cfg->gamma_manual[i];
	}

	ret = MPI_setGammaAttr(path_idx, &gamma_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getGamma(MPI_PATH path_idx, AGTX_DIP_GAMMA_CONF_S *gamma_cfg)
{
	INT32 ret = 0;
	MPI_GAMMA_ATTR_S gamma_attr;

	ret = MPI_getGammaAttr(path_idx, &gamma_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	gamma_cfg->gamma = gamma_attr.mode;

	for (int i = 0; i < MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE; i++) {
		gamma_cfg->gamma_manual[i] = gamma_attr.gma_manual.curve[i];
	}

	gamma_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setEnh(MPI_PATH path_idx, const AGTX_DIP_ENH_CONF_S *enh_cfg)
{
	INT32 ret = 0;
	MPI_ENH_ATTR_S enh_attr;

	ret = MPI_getEnhAttr(path_idx, &enh_attr);

	/* check and convert to attr_s */
	enh_attr.mode = enh_cfg->mode;
	enh_attr.enh_manual.y_txr_strength = enh_cfg->manual_y_txr_strength;
	enh_attr.enh_manual.y_txr_edge = enh_cfg->manual_y_txr_edge;
	enh_attr.enh_manual.y_txr_detail = enh_cfg->manual_y_txr_detail;
	enh_attr.enh_manual.y_zone_strength = enh_cfg->manual_y_zone_strength;
	enh_attr.enh_manual.y_zone_edge = enh_cfg->manual_y_zone_edge;
	enh_attr.enh_manual.y_zone_detail = enh_cfg->manual_y_zone_detail;
	enh_attr.enh_manual.y_zone_radius = enh_cfg->manual_y_zone_radius;
	enh_attr.enh_manual.y_zone_weight = enh_cfg->manual_y_zone_weight;
	enh_attr.enh_manual.c_strength = enh_cfg->manual_c_strength;
	enh_attr.enh_manual.c_radius = enh_cfg->manual_c_radius;
	enh_attr.enh_manual.c_edge = enh_cfg->manual_c_edge;
	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		enh_attr.enh_auto.y_txr_strength[i] = enh_cfg->auto_y_txr_strength_list[i];
		enh_attr.enh_auto.y_txr_edge[i] = enh_cfg->auto_y_txr_edge_list[i];
		enh_attr.enh_auto.y_txr_detail[i] = enh_cfg->auto_y_txr_detail_list[i];
		enh_attr.enh_auto.y_zone_strength[i] = enh_cfg->auto_y_zone_strength_list[i];
		enh_attr.enh_auto.y_zone_edge[i] = enh_cfg->auto_y_zone_edge_list[i];
		enh_attr.enh_auto.y_zone_detail[i] = enh_cfg->auto_y_zone_detail_list[i];
		enh_attr.enh_auto.y_zone_radius[i] = enh_cfg->auto_y_zone_radius_list[i];
		enh_attr.enh_auto.y_zone_weight[i] = enh_cfg->auto_y_zone_weight_list[i];
		enh_attr.enh_auto.c_strength[i] = enh_cfg->auto_c_strength_list[i];
		enh_attr.enh_auto.c_radius[i] = enh_cfg->auto_c_radius_list[i];
		enh_attr.enh_auto.c_edge[i] = enh_cfg->auto_c_edge_list[i];
	}

	ret = MPI_setEnhAttr(path_idx, &enh_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getEnh(MPI_PATH path_idx, AGTX_DIP_ENH_CONF_S *enh_cfg)
{
	INT32 ret = 0;
	MPI_ENH_ATTR_S enh_attr;

	ret = MPI_getEnhAttr(path_idx, &enh_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	enh_cfg->mode = enh_attr.mode;
	enh_cfg->manual_y_txr_strength = enh_attr.enh_manual.y_txr_strength;
	enh_cfg->manual_y_txr_edge = enh_attr.enh_manual.y_txr_edge;
	enh_cfg->manual_y_txr_detail = enh_attr.enh_manual.y_txr_detail;
	enh_cfg->manual_y_zone_strength = enh_attr.enh_manual.y_zone_strength;
	enh_cfg->manual_y_zone_edge = enh_attr.enh_manual.y_zone_edge;
	enh_cfg->manual_y_zone_detail = enh_attr.enh_manual.y_zone_detail;
	enh_cfg->manual_y_zone_radius = enh_attr.enh_manual.y_zone_radius;
	enh_cfg->manual_y_zone_weight = enh_attr.enh_manual.y_zone_weight;
	enh_cfg->manual_c_strength = enh_attr.enh_manual.c_strength;
	enh_cfg->manual_c_radius = enh_attr.enh_manual.c_radius;
	enh_cfg->manual_c_edge = enh_attr.enh_manual.c_edge;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		enh_cfg->auto_y_txr_strength_list[i] = enh_attr.enh_auto.y_txr_strength[i];
		enh_cfg->auto_y_txr_edge_list[i] = enh_attr.enh_auto.y_txr_edge[i];
		enh_cfg->auto_y_txr_detail_list[i] = enh_attr.enh_auto.y_txr_detail[i];
		enh_cfg->auto_y_zone_strength_list[i] = enh_attr.enh_auto.y_zone_strength[i];
		enh_cfg->auto_y_zone_edge_list[i] = enh_attr.enh_auto.y_zone_edge[i];
		enh_cfg->auto_y_zone_detail_list[i] = enh_attr.enh_auto.y_zone_detail[i];
		enh_cfg->auto_y_zone_radius_list[i] = enh_attr.enh_auto.y_zone_radius[i];
		enh_cfg->auto_y_zone_weight_list[i] = enh_attr.enh_auto.y_zone_weight[i];
		enh_cfg->auto_c_strength_list[i] = enh_attr.enh_auto.c_strength[i];
		enh_cfg->auto_c_radius_list[i] = enh_attr.enh_auto.c_radius[i];
		enh_cfg->auto_c_edge_list[i] = enh_attr.enh_auto.c_edge[i];
	}

	enh_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setCoring(MPI_PATH path_idx, const AGTX_DIP_CORING_CONF_S *coring_cfg)
{
	INT32 ret = 0;
	MPI_CORING_ATTR_S coring_attr;

	ret = MPI_getCoringAttr(path_idx, &coring_attr);

	/* check and convert to attr_s */
	coring_attr.mode = coring_cfg->mode;
	coring_attr.coring_slope = coring_cfg->coring_slope;
	coring_attr.coring_manual.abs_th = coring_cfg->manual_abs_th;
	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		coring_attr.coring_auto.abs_th[i] = coring_cfg->auto_abs_th_list[i];
	}

	ret = MPI_setCoringAttr(path_idx, &coring_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getCoring(MPI_PATH path_idx, AGTX_DIP_CORING_CONF_S *coring_cfg)
{
	INT32 ret = 0;
	MPI_CORING_ATTR_S coring_attr;

	ret = MPI_getCoringAttr(path_idx, &coring_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	coring_cfg->mode = coring_attr.mode;
	coring_cfg->coring_slope = coring_attr.coring_slope;
	coring_cfg->manual_abs_th = coring_attr.coring_manual.abs_th;
	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		coring_cfg->auto_abs_th_list[i] = coring_attr.coring_auto.abs_th[i];
	}

	coring_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setFcs(MPI_PATH path_idx, const AGTX_DIP_FCS_CONF_S *fcs_cfg)
{
	INT32 ret = 0;
	MPI_FCS_ATTR_S fcs_attr;

	ret = MPI_getFcsAttr(path_idx, &fcs_attr);

	/* check and convert to attr_s */
	fcs_attr.mode = fcs_cfg->mode;
	fcs_attr.fcs_manual.strength = fcs_cfg->manual_strength;
	fcs_attr.fcs_manual.threshold = fcs_cfg->manual_threshold;
	fcs_attr.fcs_manual.offset = fcs_cfg->manual_offset;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		fcs_attr.fcs_auto.strength[i] = fcs_cfg->auto_strength_list[i];
		fcs_attr.fcs_auto.threshold[i] = fcs_cfg->auto_threshold_list[i];
		fcs_attr.fcs_auto.offset[i] = fcs_cfg->auto_offset_list[i];
	}

	ret = MPI_setFcsAttr(path_idx, &fcs_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getFcs(MPI_PATH path_idx, AGTX_DIP_FCS_CONF_S *fcs_cfg)
{
	INT32 ret = 0;
	MPI_FCS_ATTR_S fcs_attr;

	ret = MPI_getFcsAttr(path_idx, &fcs_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	fcs_cfg->mode = fcs_attr.mode;
	fcs_cfg->manual_strength = fcs_attr.fcs_manual.strength;
	fcs_cfg->manual_threshold = fcs_attr.fcs_manual.threshold;
	fcs_cfg->manual_offset = fcs_attr.fcs_manual.offset;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		fcs_cfg->auto_strength_list[i] = fcs_attr.fcs_auto.strength[i];
		fcs_cfg->auto_threshold_list[i] = fcs_attr.fcs_auto.threshold[i];
		fcs_cfg->auto_offset_list[i] = fcs_attr.fcs_auto.offset[i];
	}

	fcs_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setDhz(MPI_PATH path_idx, const AGTX_DIP_DHZ_CONF_S *dhz_cfg)
{
	INT32 ret = 0;
	MPI_DHZ_ATTR_S dhz_attr;

	ret = MPI_getDhzAttr(path_idx, &dhz_attr);

	/* check and convert to attr_s */
	dhz_attr.mode = dhz_cfg->mode;
	dhz_attr.dc_iir_weight = (UINT16)dhz_cfg->dc_iir_weight;
	dhz_attr.gain_step_th = (UINT16)dhz_cfg->gain_step_th;
	dhz_attr.dhz_manual.y_gain_max = (UINT16)dhz_cfg->manual_y_gain_max;
	dhz_attr.dhz_manual.c_gain_max = (UINT16)dhz_cfg->manual_c_gain_max;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		dhz_attr.dhz_auto.y_gain_max[i] = (UINT16)dhz_cfg->auto_y_gain_max_list[i];
		dhz_attr.dhz_auto.c_gain_max[i] = (UINT16)dhz_cfg->auto_c_gain_max_list[i];
	}

	ret = MPI_setDhzAttr(path_idx, &dhz_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getDhz(MPI_PATH path_idx, AGTX_DIP_DHZ_CONF_S *dhz_cfg)
{
	INT32 ret = 0;
	MPI_DHZ_ATTR_S dhz_attr;

	ret = MPI_getDhzAttr(path_idx, &dhz_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to cfg */
	dhz_cfg->mode = dhz_attr.mode;
	dhz_cfg->dc_iir_weight = dhz_attr.dc_iir_weight;
	dhz_cfg->gain_step_th = dhz_attr.gain_step_th;
	dhz_cfg->manual_y_gain_max = dhz_attr.dhz_manual.y_gain_max;
	dhz_cfg->manual_c_gain_max = dhz_attr.dhz_manual.c_gain_max;

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		dhz_cfg->auto_y_gain_max_list[i] = dhz_attr.dhz_auto.y_gain_max[i];
		dhz_cfg->auto_c_gain_max_list[i] = dhz_attr.dhz_auto.c_gain_max[i];
	}

	dhz_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setHdrSynth(MPI_PATH path_idx, const AGTX_DIP_HDR_SYNTH_CONF_S *hdr_synth_cfg)
{
	INT32 ret = 0;
	MPI_HDR_SYNTH_ATTR_S hdr_synth_attr;

	ret = MPI_getHdrSynthAttr(path_idx, &hdr_synth_attr);

	/* check and convert to attr_s */
	hdr_synth_attr.local_fb_th = hdr_synth_cfg->local_fb_th;
	hdr_synth_attr.weight.se_weight_th_min = hdr_synth_cfg->weight_se_weight_th_min;
	hdr_synth_attr.weight.se_weight_slope = hdr_synth_cfg->weight_se_weight_slope;
	hdr_synth_attr.weight.se_weight_min = hdr_synth_cfg->weight_se_weight_min;
	hdr_synth_attr.weight.se_weight_max = hdr_synth_cfg->weight_se_weight_max;
	hdr_synth_attr.weight.le_weight_th_max = hdr_synth_cfg->weight_le_weight_th_max;
	hdr_synth_attr.weight.le_weight_slope = hdr_synth_cfg->weight_le_weight_slope;
	hdr_synth_attr.weight.le_weight_min = hdr_synth_cfg->weight_le_weight_min;
	hdr_synth_attr.weight.le_weight_max = hdr_synth_cfg->weight_le_weight_max;
	hdr_synth_attr.frame_fb_strength = hdr_synth_cfg->frame_fb_strength;

	ret = MPI_setHdrSynthAttr(path_idx, &hdr_synth_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getHdrSynth(MPI_PATH path_idx, AGTX_DIP_HDR_SYNTH_CONF_S *hdr_synth_cfg)
{
	INT32 ret = 0;
	MPI_HDR_SYNTH_ATTR_S hdr_synth_attr;

	ret = MPI_getHdrSynthAttr(path_idx, &hdr_synth_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	hdr_synth_cfg->local_fb_th = hdr_synth_attr.local_fb_th;
	hdr_synth_cfg->weight_se_weight_th_min = hdr_synth_attr.weight.se_weight_th_min;
	hdr_synth_cfg->weight_se_weight_slope = hdr_synth_attr.weight.se_weight_slope;
	hdr_synth_cfg->weight_se_weight_min = hdr_synth_attr.weight.se_weight_min;
	hdr_synth_cfg->weight_se_weight_max = hdr_synth_attr.weight.se_weight_max;
	hdr_synth_cfg->weight_le_weight_th_max = hdr_synth_attr.weight.le_weight_th_max;
	hdr_synth_cfg->weight_le_weight_slope = hdr_synth_attr.weight.le_weight_slope;
	hdr_synth_cfg->weight_le_weight_min = hdr_synth_attr.weight.le_weight_min;
	hdr_synth_cfg->weight_le_weight_max = hdr_synth_attr.weight.le_weight_max;
	hdr_synth_cfg->frame_fb_strength = hdr_synth_attr.frame_fb_strength;

	hdr_synth_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_setStat(MPI_PATH path_idx, const AGTX_DIP_STAT_CONF_S *stat_cfg)
{
	INT32 ret = 0;
	MPI_STAT_CFG_S stat_attr;

	ret = MPI_getStatisticsConfig(path_idx, &stat_attr);

	/* check and convert to attr_s */
	stat_attr.wb.lum_max = stat_cfg->wb.lum_max;
	stat_attr.wb.lum_min = stat_cfg->wb.lum_min;
	stat_attr.wb.lum_slope = stat_cfg->wb.lum_slope;

	for (INT32 i = 0; i < MPI_WB_RB_POINT_NUM; i++) {
		stat_attr.wb.rb_point_x[i] = stat_cfg->wb.rb_point_x[i];
		stat_attr.wb.rb_point_y[i] = stat_cfg->wb.rb_point_y[i];
	}

	for (INT32 i = 0; i < MPI_WB_RB_POINT_NUM - 1; i++) {
		stat_attr.wb.rb_rgn_th[i] = stat_cfg->wb.rb_rgn_th[i];
		stat_attr.wb.rb_rgn_slope[i] = stat_cfg->wb.rb_rgn_slope[i];
	}

	stat_attr.wb.gwd_auto_lum_thd_enable = stat_cfg->wb.gwd_auto_lum_thd_enable;
	stat_attr.wb.gwd_auto_lum_thd_param.lum_max_degree = stat_cfg->wb.gwd_auto_lum_max_degree;
	stat_attr.wb.gwd_auto_lum_thd_param.indoor_ev_thd = stat_cfg->wb.gwd_auto_indoor_ev_thd;
	stat_attr.wb.gwd_auto_lum_thd_param.outdoor_ev_thd = stat_cfg->wb.gwd_auto_outdoor_ev_thd;
	stat_attr.wb.gwd_auto_lum_thd_param.indoor_lum_range = stat_cfg->wb.gwd_auto_indoor_lum_range;
	stat_attr.wb.gwd_auto_lum_thd_param.outdoor_lum_range = stat_cfg->wb.gwd_auto_outdoor_lum_range;
	stat_attr.wb.gwd_auto_lum_thd_param.min_lum_bnd = stat_cfg->wb.gwd_auto_min_lum_bnd;

	ret = MPI_setStatisticsConfig(path_idx, &stat_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_DIP_getStat(MPI_PATH path_idx, AGTX_DIP_STAT_CONF_S *stat_cfg)
{
	INT32 ret = 0;
	MPI_STAT_CFG_S stat_attr;

	ret = MPI_getStatisticsConfig(path_idx, &stat_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	stat_cfg->wb.lum_max = stat_attr.wb.lum_max;
	stat_cfg->wb.lum_min = stat_attr.wb.lum_min;
	stat_cfg->wb.lum_slope = stat_attr.wb.lum_slope;

	for (INT32 i = 0; i < MPI_WB_RB_POINT_NUM; i++) {
		stat_cfg->wb.rb_point_x[i] = stat_attr.wb.rb_point_x[i];
		stat_cfg->wb.rb_point_y[i] = stat_attr.wb.rb_point_y[i];
	}

	for (INT32 i = 0; i < MPI_WB_RB_POINT_NUM - 1; i++) {
		stat_cfg->wb.rb_rgn_th[i] = stat_attr.wb.rb_rgn_th[i];
		stat_cfg->wb.rb_rgn_slope[i] = stat_attr.wb.rb_rgn_slope[i];
	}

	stat_cfg->wb.gwd_auto_lum_thd_enable = stat_attr.wb.gwd_auto_lum_thd_enable;
	stat_cfg->wb.gwd_auto_lum_max_degree = stat_attr.wb.gwd_auto_lum_thd_param.lum_max_degree;
	stat_cfg->wb.gwd_auto_indoor_ev_thd = stat_attr.wb.gwd_auto_lum_thd_param.indoor_ev_thd;
	stat_cfg->wb.gwd_auto_outdoor_ev_thd = stat_attr.wb.gwd_auto_lum_thd_param.outdoor_ev_thd;
	stat_cfg->wb.gwd_auto_indoor_lum_range = stat_attr.wb.gwd_auto_lum_thd_param.indoor_lum_range;
	stat_cfg->wb.gwd_auto_outdoor_lum_range = stat_attr.wb.gwd_auto_lum_thd_param.outdoor_lum_range;
	stat_cfg->wb.gwd_auto_min_lum_bnd = stat_attr.wb.gwd_auto_lum_thd_param.min_lum_bnd;

	stat_cfg->video_dev_idx = 0;

	return MPI_SUCCESS;
}

INT32 APP_DIP_getExposureInfo(MPI_PATH path_idx, AGTX_DIP_EXPOSURE_INFO_S *exp_info)
{
	INT32 ret = 0;
	MPI_EXPOSURE_INFO_S mpi_exp_info;

	ret = MPI_queryExposureInfo(path_idx, &mpi_exp_info);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	/* check and convert to attr_s */
	exp_info->inttime = mpi_exp_info.inttime;
	exp_info->sensor_gain = mpi_exp_info.sensor_gain;
	exp_info->isp_gain = mpi_exp_info.isp_gain;
	exp_info->sys_gain = mpi_exp_info.sys_gain;
	exp_info->iso = mpi_exp_info.iso;
	exp_info->frame_delay = mpi_exp_info.frame_delay;
	exp_info->flicker_free_conf = mpi_exp_info.flicker_free_conf;
	exp_info->fps = mpi_exp_info.fps;
	exp_info->ratio = mpi_exp_info.ratio;
	exp_info->luma_avg = mpi_exp_info.luma_avg;
	exp_info->video_dev_idx = 0;

	return MPI_SUCCESS;
}
INT32 APP_DIP_getWhiteBalanceInfo(MPI_PATH path_idx, AGTX_DIP_WHITE_BALANCE_INFO_S *wb_info)
{
	INT32 ret = 0;
	MPI_WHITE_BALANCE_INFO_S mpi_wb_info;

	ret = MPI_queryWhiteBalanceInfo(path_idx, &mpi_wb_info);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	for (int i = 0; i < MPI_AWB_CHN_NUM; ++i) {
		wb_info->gain0[i] = mpi_wb_info.gain0[i];
		wb_info->gain1[i] = mpi_wb_info.gain1[i];
	}

	for (int i = 0; i < MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM; ++i) {
		wb_info->matrix[i] = mpi_wb_info.matrix[i];
	}

	wb_info->color_temp = mpi_wb_info.color_temp;

	return MPI_SUCCESS;
}

INT32 APP_DIP_getTeInfo(MPI_PATH path_idx, AGTX_DIP_TE_INFO_S *te_info)
{
	INT32 ret = 0;
	MPI_TE_INFO_S mpi_te_info;
	ret = MPI_queryTeInfo(path_idx, &mpi_te_info);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}
	/* check and convert to attr_s */
	te_info->tm_enable = mpi_te_info.tm_enable;
	for (int i = 0; i < MAX_AGTX_DIP_TE_INFO_S_TM_CURVE_SIZE; i++) {
		te_info->tm_curve[i] = mpi_te_info.tm_curve[i];
	}
	return MPI_SUCCESS;
}
