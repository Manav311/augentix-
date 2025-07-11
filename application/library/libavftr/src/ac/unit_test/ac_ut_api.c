#ifdef UNIT_TEST

#include <string.h> //for size_t/memcmp
#include <unistd.h> // for sleep
#include <stdlib.h> // System

#include "ut.h"
#include "ac.h"
#include "ac_codec.h"
#include "ac_ut_api.h"
#include "ac_ut_common.h"

#include "mpi_errno.h"
#include "mpi_common.h"
#include "mpi_types.h"
#include "mpi_sys.h"
#include <fcntl.h>

extern AcInfo g_ac_ctx;

static char *RAW_FILE = "../feature/libac/src/unit_test/files/air8k.raw";
static char *BIT_FILE[7] = { "../feature/libac/src/unit_test/files/air8k.s16le",
	                     "../feature/libac/src/unit_test/files/air8k.pcma",
	                     "../feature/libac/src/unit_test/files/air8k.pcmu",
	                     "../feature/libac/src/unit_test/files/air8k.g726_16_be",
	                     "../feature/libac/src/unit_test/files/air8k.g726_16_le",
	                     "../feature/libac/src/unit_test/files/air8k.g726_32_be",
	                     "../feature/libac/src/unit_test/files/air8k.g726_32_le" };
static char *REC_FILE[7] = { "../feature/libac/src/unit_test/files/air8k.s16le.raw",
	                     "../feature/libac/src/unit_test/files/air8k.pcma.raw",
	                     "../feature/libac/src/unit_test/files/air8k.pcmu.raw",
	                     "../feature/libac/src/unit_test/files/air8k.g726_16_be.raw",
	                     "../feature/libac/src/unit_test/files/air8k.g726_16_le.raw",
	                     "../feature/libac/src/unit_test/files/air8k.g726_32_be.raw",
	                     "../feature/libac/src/unit_test/files/air8k.g726_32_le.raw" };

char g_msg[256];
#define MAX_CASE_NUM 25
static UINT8 case_id = 0;
static AC_PARAM_S _check_param[MAX_CASE_NUM];
static int _check_res[MAX_CASE_NUM];
static AC_PARAM_S _set_param[MAX_CASE_NUM];
static int _set_res[MAX_CASE_NUM];
static AC_PARAM_S _get_param[MAX_CASE_NUM];
static int _get_res[MAX_CASE_NUM];

static void UT_utils_setAcParamDefault(AC_PARAM_S *param)
{
	param->codec = AUDIO_CODEC_TYPE_PCM16LE;

	return;
}

static void UT_assertMsgAcParam(CuTest *tc, AC_PARAM_S *actual, AC_PARAM_S *expected, char *msg)
{
	if (memcmp(actual, expected, sizeof(AC_PARAM_S))) {
		sprintf(msg, "Failed to match AC_PARAM_S: expected={%d}, actual={%d}\n", expected->codec,
		        actual->codec);
		UT_fail(tc, msg);
	}
}

/* ***********************************************************************************
 * Test Function: checkAcParam
 * Test scenario: Check param value.
 * ***********************************************************************************/
static void UT_checkAcParam_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);

	int i = case_id++;
	INT32 ret = -1;
	ret = UT_checkAcParam(&_check_param[i]);
	UT_assertMsg(tc, ret == _check_res[i], g_msg, "Check value failed: expected=%d, actual=%d\n", _check_res[i],
	             ret);

	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_setParam/AC_getParam
  * ***********************************************************************************/
static void UT_AC_setParam_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);

	int ret = 0;
	AC_PARAM_S _get;
	MPI_DEV idx = MPI_VIDEO_DEV(0);
	int i = case_id++;
	AC_PARAM_S ac_param;

	UT_utils_setAcParamDefault(&ac_param);
	AC_setParam(MPI_VIDEO_DEV(0), &ac_param);
	if (i == 0) {
		ret = AC_setParam(idx, NULL);
		UT_assertMsg(tc, ret == _set_res[i], g_msg, "Set value failed: expected=%d, actual=%d\n", _set_res[i],
		             ret);

		ret = AC_getParam(idx, NULL);
		UT_assertMsg(tc, ret == _get_res[i], g_msg, "Get value failed: expected=%d, actual=%d\n", _get_res[i],
		             ret);
	} else {
		ret = AC_setParam(idx, &_set_param[i]);
		UT_assertMsg(tc, ret == _set_res[i], g_msg, "Set value failed: expected=%d, actual=%d\n", _set_res[i],
		             ret);

		ret = AC_getParam(idx, &_get);
		UT_assertMsg(tc, ret == _get_res[i], g_msg, "Get value failed: expected=%d, actual=%d\n", _get_res[i],
		             ret);

		UT_assertMsgAcParam(tc, &_get, &_get_param[i], g_msg);
	}

	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_decode
 * ***********************************************************************************/
static void UT_AC_decode_Case_Null(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int ret = 0;
	char gt_bitstream;
	int size_of_raw = 0;

	ret = AC_decode(NULL, 0, NULL, &size_of_raw);
	UT_assert(tc, "Fail to test NULL bitstream pointer as decode input.\n", ret == MPI_ERR_NULL_POINTER);

	ret = AC_decode(&gt_bitstream, 0, NULL, NULL);
	UT_assert(tc, "Fail to test NULL size_of_raw pointer as decode input.\n", ret == MPI_ERR_NULL_POINTER);

	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_decode
 * ***********************************************************************************/
static void UT_AC_decode_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int codec = case_id++;
	int ret = 0;
	int buffer_size = 256;
	int fd_bit = open(BIT_FILE[codec], O_RDONLY);
	int fd_rec = open(REC_FILE[codec], O_RDONLY);
	char gt_bitstream[buffer_size];
	char *gt_rec;
	char *rec_buffer = NULL;
	int size_of_bit = 0, size_of_raw = 0;
	AC_PARAM_S ac_param = { .codec = 1 + codec };

	AC_setParam(MPI_VIDEO_DEV(0), &ac_param);
	while ((size_of_bit = read(fd_bit, gt_bitstream, buffer_size)) > 0) {
		ret = AC_decode(gt_bitstream, size_of_bit, &rec_buffer, &size_of_raw);
		UT_assert(tc, "Fail to decode audio.\n", ret == MPI_SUCCESS);
		gt_rec = malloc(size_of_raw);
		if (read(fd_rec, gt_rec, size_of_raw) > 0) {
			if (memcmp(rec_buffer, gt_rec, size_of_raw)) {
				UT_fail(tc, "Fail to match ground truth decoded raw.\n");
			}
		} else {
			UT_fail(tc, "Ground truth decoded raw end.\n");
			break;
		}

		free(gt_rec);
		if (rec_buffer != gt_bitstream) {
			free(rec_buffer);
		}
	}

	close(fd_bit);
	close(fd_rec);
	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_encode
 * ***********************************************************************************/
static void UT_AC_encode_Case_Null(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int ret = 0;
	char gt_raw;
	int size_of_bit = 0;

	ret = AC_encode(NULL, 0, NULL, &size_of_bit);
	UT_assert(tc, "Fail to test NULL bitstream pointer as encode input.\n", ret == MPI_ERR_NULL_POINTER);

	ret = AC_encode(&gt_raw, 0, NULL, NULL);
	UT_assert(tc, "Fail to test NULL size_of_bit pointer as encode input.\n", ret == MPI_ERR_NULL_POINTER);

	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_encode
 * ***********************************************************************************/
static void UT_AC_encode_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int codec = case_id++;
	int ret = 0;
	int buffer_size = 256;
	int fd_bit = open(BIT_FILE[codec], O_RDONLY);
	int fd_raw = open(RAW_FILE, O_RDONLY);
	char *gt_bitstream;
	char gt_raw[buffer_size];
	char *bit_buffer = NULL;
	int size_of_bit = 0, size_of_raw = 0;
	AC_PARAM_S ac_param = { .codec = 1 + codec };

	AC_setParam(MPI_VIDEO_DEV(0), &ac_param);
	while ((size_of_raw = read(fd_raw, gt_raw, buffer_size)) > 0) {
		ret = AC_encode(gt_raw, size_of_raw, &bit_buffer, &size_of_bit);
		UT_assert(tc, "Fail to encode audio.\n", ret == MPI_SUCCESS);
		gt_bitstream = malloc(size_of_bit);
		if (read(fd_bit, gt_bitstream, size_of_bit) > 0) {
			if (memcmp(bit_buffer, gt_bitstream, size_of_bit)) {
				UT_fail(tc, "Fail to match ground truth encoded bitstream.\n");
			}
		} else {
			UT_fail(tc, "Ground truth encoded bitstream end.\n");
		}

		if (bit_buffer != gt_raw) {
			free(bit_buffer);
		}
		free(gt_bitstream);
	}

	close(fd_bit);
	close(fd_raw);
	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_encode/AC_decode
 * ***********************************************************************************/
static void UT_AC_encode_decode_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int codec = case_id++;
	int ret = 0;
	int buffer_size = 256;
	int fd_raw = open(RAW_FILE, O_RDONLY);
	int fd_rec = open(REC_FILE[codec], O_RDONLY);
	char gt_raw[buffer_size];
	char gt_rec[buffer_size];
	char *bit_buffer = NULL;
	char *raw_buffer = NULL;
	int size_of_bit = 0, size_of_raw = 0;
	AC_PARAM_S ac_param = { .codec = 1 + codec };

	AC_setParam(MPI_VIDEO_DEV(0), &ac_param);
	while ((size_of_raw = read(fd_raw, gt_raw, buffer_size)) > 0) {
		ret = AC_encode(gt_raw, size_of_raw, &bit_buffer, &size_of_bit);
		UT_assert(tc, "Fail to encode audio.\n", ret == MPI_SUCCESS);
		ret = AC_decode(bit_buffer, size_of_bit, &raw_buffer, &size_of_raw);
		UT_assert(tc, "Fail to decode audio.\n", ret == MPI_SUCCESS);
		if (read(fd_rec, gt_rec, size_of_raw) > 0) {
			if (memcmp(raw_buffer, gt_rec, size_of_raw)) {
				UT_fail(tc, "Fail to get decoded raw from encode-decode.\n");
			}
		} else {
			UT_fail(tc, "Ground truth decoded raw end.\n");
		}

		if (bit_buffer != gt_raw) {
			free(bit_buffer);
		}
		if (raw_buffer != bit_buffer) {
			free(raw_buffer);
		}
	}

	close(fd_raw);
	close(fd_rec);
	AC_UT_INFO("%s end\n", __func__);
}

/* ***********************************************************************************
 * Test Function: AC_decode/AC_encode
 * ***********************************************************************************/
static void UT_AC_decode_encode_Case(CuTest *tc)
{
	AC_UT_INFO("%s start\n", __func__);
	int codec = case_id++;
	int ret = 0;
	int buffer_size = 256;
	int fd_bit = open(BIT_FILE[codec], O_RDONLY);
	int fd_bit_2 = open(BIT_FILE[codec], O_RDONLY);
	char gt_bitstream[buffer_size];
	char gt_bitstream_2[buffer_size];
	char *bit_buffer = NULL;
	char *raw_buffer = NULL;
	int size_of_bit = 0, size_of_raw = 0;
	AC_PARAM_S ac_param = { .codec = 1 + codec };

	AC_setParam(MPI_VIDEO_DEV(0), &ac_param);
	while ((size_of_bit = read(fd_bit, gt_bitstream, buffer_size)) > 0) {
		ret = AC_decode(gt_bitstream, size_of_bit, &raw_buffer, &size_of_raw);
		UT_assert(tc, "Fail to decode audio.\n", ret == MPI_SUCCESS);
		ret = AC_encode(raw_buffer, size_of_raw, &bit_buffer, &size_of_bit);
		UT_assert(tc, "Fail to encode audio.\n", ret == MPI_SUCCESS);
		if (read(fd_bit_2, gt_bitstream_2, size_of_bit) > 0) {
			if (memcmp(bit_buffer, gt_bitstream_2, size_of_bit)) {
				for (int i = 0; i < size_of_bit; i++) {
					printf("%02x", bit_buffer[i]);
				}
				printf("\n");
				for (int i = 0; i < size_of_bit; i++) {
					printf("%02x", gt_bitstream_2[i]);
				}
				printf("\n");
				UT_fail(tc, "Fail to get reencoded bitstream from decode-encode.\n");
			}
		} else {
			UT_fail(tc, "Ground truth reencoded bitstream end.\n");
		}

		if (raw_buffer != gt_bitstream) {
			free(raw_buffer);
		}
		if (bit_buffer != raw_buffer) {
			free(bit_buffer);
		}
	}

	close(fd_bit);
	AC_UT_INFO("%s end\n", __func__);
}

CuSuite *UT_getSuite_AC_checkParam(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	_check_param[case_id] = g_ac_ctx.param; //default case
	_check_res[case_id++] = MPI_SUCCESS;

	for (int i = AUDIO_CODEC_TYPE_NONE; i <= AUDIO_CODEC_TYPE_NUM; i++) {
		_check_param[case_id] = g_ac_ctx.param;
		_check_param[case_id].codec = i;
		_check_res[case_id++] = (i > AUDIO_CODEC_TYPE_NONE && i < AUDIO_CODEC_TYPE_NUM) ? MPI_SUCCESS : -1;
	}

	for (int i = 0; i < case_id; i++) {
		sprintf(case_name, "UT_checkAcParam_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_checkAcParam_Case);
	}
	case_id = 0;
	return suite;
}

CuSuite *UT_getSuite_AC_setParam(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	_set_param[case_id] = g_ac_ctx.param; //NULL pointer case
	_set_res[case_id] = MPI_ERR_NULL_POINTER;
	_get_param[case_id] = g_ac_ctx.param;
	_get_res[case_id++] = MPI_ERR_NULL_POINTER;

	_set_param[case_id] = g_ac_ctx.param; //default case
	_set_res[case_id] = MPI_SUCCESS;
	_get_param[case_id] = g_ac_ctx.param;
	_get_res[case_id++] = MPI_SUCCESS;

	for (int i = AUDIO_CODEC_TYPE_NONE; i <= AUDIO_CODEC_TYPE_NUM; i++) {
		_set_param[case_id] = g_ac_ctx.param;
		_set_param[case_id].codec = i;
		_set_res[case_id] = (i > AUDIO_CODEC_TYPE_NONE && i < AUDIO_CODEC_TYPE_NUM) ? MPI_SUCCESS :
		                                                                              MPI_ERR_INVALID_PARAM;
		_get_param[case_id] = (_set_res[case_id] == MPI_SUCCESS) ? _set_param[case_id] : g_ac_ctx.param;
		_get_res[case_id++] = MPI_SUCCESS;
	}

	for (int i = 0; i < case_id; i++) {
		sprintf(case_name, "UT_AC_setParam_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_AC_setParam_Case);
	}
	case_id = 0;
	return suite;
}

CuSuite *UT_getSuite_AC_codec_Null(void)
{
	CuSuite *suite = UT_createSuite();

	UT_addTestInSuite(suite, UT_AC_decode_Case_Null);
	UT_addTestInSuite(suite, UT_AC_encode_Case_Null);

	return suite;
}

CuSuite *UT_getSuite_AC_decode(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	for (int i = AUDIO_CODEC_TYPE_NONE + 1; i < AUDIO_CODEC_TYPE_NUM; i++) {
		sprintf(case_name, "UT_AC_decode_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_AC_decode_Case);
	}
	case_id = 0;
	return suite;
}

CuSuite *UT_getSuite_AC_encode(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	for (int i = AUDIO_CODEC_TYPE_NONE + 1; i < AUDIO_CODEC_TYPE_NUM; i++) {
		sprintf(case_name, "UT_AC_encode_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_AC_encode_Case);
	}
	case_id = 0;
	return suite;
}

CuSuite *UT_getSuite_AC_encode_decode(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	for (int i = AUDIO_CODEC_TYPE_NONE + 1; i < AUDIO_CODEC_TYPE_NUM; i++) {
		sprintf(case_name, "UT_AC_encode_decode_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_AC_encode_decode_Case);
	}
	case_id = 0;
	return suite;
}

CuSuite *UT_getSuite_AC_decode_encode(void)
{
	CuSuite *suite = UT_createSuite();
	char case_name[256];
	case_id = 0;

	for (int i = AUDIO_CODEC_TYPE_NONE + 1; i < AUDIO_CODEC_TYPE_NUM; i++) {
		sprintf(case_name, "UT_AC_decode_encode_Case%d", i);
		UT_addTestWithNameInSuite(suite, case_name, UT_AC_decode_encode_Case);
	}
	case_id = 0;
	return suite;
}

void UT_runAcApiSuite(void)
{
	UT_runModuleSuite(UT_getSuite_AC_checkParam());
	UT_runModuleSuite(UT_getSuite_AC_setParam());
	UT_runModuleSuite(UT_getSuite_AC_codec_Null());
	UT_runModuleSuite(UT_getSuite_AC_decode());
	UT_runModuleSuite(UT_getSuite_AC_encode());
	UT_runModuleSuite(UT_getSuite_AC_encode_decode());
	//UT_runModuleSuite(UT_getSuite_AC_decode_encode());
}

#endif /* UNIT_TEST */
