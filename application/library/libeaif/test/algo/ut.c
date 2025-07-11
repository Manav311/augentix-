#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "mpi_index.h"
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"

#include "inf_ut.h"

#include "app.h"
#include "eaif_algo.h"
#include "eaif.h"

/* test for priority */
int test_eaif_checkAppendable_case0(InfTest *test)
{
	EAIF_PARAM_S param = { .pos_stop_count_th = 3,
		               .pos_classify_period = 100,
		               .neg_classify_period = 25,
		               .obj_life_th = 30,
		               .obj_exist_classify_period = 0 };

	EaifAlgo algo = { .p = &param };

	MPI_IVA_OBJ_LIST_S dst_list;
	MPI_IVA_OBJ_LIST_S src_list;
	src_list.timestamp = 123456;
	src_list.obj_num = 5;
	src_list.obj[0] = (MPI_IVA_OBJ_ATTR_S){ .id = 1, .life = 160 };
	src_list.obj[1] = (MPI_IVA_OBJ_ATTR_S){ .id = 2, .life = 160 };
	src_list.obj[2] = (MPI_IVA_OBJ_ATTR_S){ .id = 3, .life = 50 };
	src_list.obj[3] = (MPI_IVA_OBJ_ATTR_S){ .id = 4, .life = 32 };
	src_list.obj[4] = (MPI_IVA_OBJ_ATTR_S){ .id = 5, .life = 10 };

	// 1. new object highest priority
	struct eaif_status_internal_s status;
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 1, .frame_counter = 20
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 1, .confid_counter = 2, .infer_counter = 5, .frame_counter = 30
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 2, .frame_counter = 16
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 18
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testSetMsg("New Object");
	testAssert(dst_list.obj_num == 1);
	testAssert(dst_list.timestamp == 123456);
	testAssert(dst_list.obj[0].id == 4);
	testAssert(!memcmp(&dst_list.obj[0], &src_list.obj[3], sizeof(MPI_IVA_OBJ_ATTR_S)));

	// 2. sorting by neg object largest frame_counter difference
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 1, .frame_counter = 18
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 5, .frame_counter = 35
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 2, .frame_counter = 30
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 3, .frame_counter = 32
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testSetMsg("Sort -ve Object") testAssert(dst_list.obj_num == 1);
	testAssert(dst_list.timestamp == 123456);
	testAssertEqInt(dst_list.obj[0].id, 2);
	testAssert(!memcmp(&dst_list.obj[0], &src_list.obj[1], sizeof(MPI_IVA_OBJ_ATTR_S)));

	// 3. sorting by pos object largest frame_counter difference
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 1, .frame_counter = 58
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 1, .confid_counter = 2, .infer_counter = 5, .frame_counter = 120
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 2, .frame_counter = 180
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 1, .confid_counter = 0, .infer_counter = 3, .frame_counter = 150
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testSetMsg("Sort +ve Object") testAssert(dst_list.obj_num == 1);
	testAssert(dst_list.timestamp == 123456);
	testAssertEqInt(dst_list.obj[0].id, 3);
	testAssert(!memcmp(&dst_list.obj[0], &src_list.obj[2], sizeof(MPI_IVA_OBJ_ATTR_S)));

	// 4. sorting by +ve/-ve object largest frame_counter difference
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 1, .frame_counter = 120
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 1, .confid_counter = 2, .infer_counter = 5, .frame_counter = 140
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 2, .frame_counter = 41
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 3, .frame_counter = 20
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testSetMsg("Sort +ve/-ve Object") testAssert(dst_list.obj_num == 1);
	testAssert(dst_list.timestamp == 123456);
	testAssertEqInt(dst_list.obj[0].id, 3);
	testAssert(!memcmp(&dst_list.obj[0], &src_list.obj[2], sizeof(MPI_IVA_OBJ_ATTR_S)));

	// 5. sorting by +ve/-ve object largest frame_counter difference
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 1, .frame_counter = 120
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 1, .confid_counter = 3, .infer_counter = 5, .frame_counter = 150
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 2, .frame_counter = 41
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 1, .confid_counter = 0, .infer_counter = 3, .frame_counter = 145
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testSetMsg("Sort +ve/-ve +ve win Object") testAssert(dst_list.obj_num == 1);
	testAssert(dst_list.timestamp == 123456);
	testAssertEqInt(dst_list.obj[0].id, 4);
	testAssert(!memcmp(&dst_list.obj[0], &src_list.obj[3], sizeof(MPI_IVA_OBJ_ATTR_S)));

	return 0;
}

/* Test for appendable for positive object exists */
int test_eaif_checkAppendable_case1(InfTest *test)
{
	EAIF_PARAM_S param = { .pos_stop_count_th = 3,
		               .pos_classify_period = 100,
		               .neg_classify_period = 25,
		               .obj_life_th = 30,
		               .obj_exist_classify_period = 0 };

	EaifAlgo algo = { .p = &param };

	MPI_IVA_OBJ_LIST_S dst_list;
	MPI_IVA_OBJ_LIST_S src_list;
	src_list.timestamp = 123456;
	src_list.obj_num = 5;
	src_list.obj[0] = (MPI_IVA_OBJ_ATTR_S){ .id = 1, .life = 160 };
	src_list.obj[1] = (MPI_IVA_OBJ_ATTR_S){ .id = 2, .life = 160 };
	src_list.obj[2] = (MPI_IVA_OBJ_ATTR_S){ .id = 3, .life = 50 };
	src_list.obj[3] = (MPI_IVA_OBJ_ATTR_S){ .id = 4, .life = 32 };
	src_list.obj[4] = (MPI_IVA_OBJ_ATTR_S){ .id = 5, .life = 10 };

	// 1. new object highest priority
	struct eaif_status_internal_s status;
	status.obj_exist_any = 1;
	status.obj_exist_any_counter = 0;
	status.obj_cnt = 5;
	status.obj_attr_ex[0] = (EaifObjAttrEx){
		.basic.id = 1, .basic.label_num = 1, .confid_counter = 1, .infer_counter = 1, .frame_counter = 20
	};
	status.obj_attr_ex[1] = (EaifObjAttrEx){
		.basic.id = 2, .basic.label_num = 1, .confid_counter = 2, .infer_counter = 5, .frame_counter = 30
	};
	status.obj_attr_ex[2] = (EaifObjAttrEx){
		.basic.id = 3, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 2, .frame_counter = 16
	};
	status.obj_attr_ex[3] = (EaifObjAttrEx){
		.basic.id = 4, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 18
	};
	status.obj_attr_ex[4] = (EaifObjAttrEx){
		.basic.id = 5, .basic.label_num = 0, .confid_counter = 0, .infer_counter = 0, .frame_counter = 0
	};

	int iteration = 20;
	int appendable = 0;
	dst_list.obj_num = 0;
	testSetMsg("positive object exists with no obj_exist_classify_period = 0");
	for (int i = 0; i < iteration; i++) {
		appendable = eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
		testAssert(dst_list.timestamp == 123456);
		testAssert(dst_list.obj[0].id == 4);
		testAssert(appendable == 1);
	}

	testSetMsg("positive object exists with no obj_exist_classify_period = 25");
	param.obj_exist_classify_period = 25;
	iteration = 25;
	status.obj_exist_any = 1;
	status.obj_exist_any_counter = 0;
	dst_list.obj_num = 0;
	for (int i = 0; i < iteration; i++) {
		appendable = eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
		testAssert(dst_list.timestamp == 123456);
		testAssertEqInt(dst_list.obj_num, 0);
		testAssert(appendable == 0);
	}
	appendable = eaif_checkAppendable(&src_list, &algo, &status, &dst_list);
	testAssert(appendable == 1);

	return 0;
}

int test_eaif_cpyScaledFaceStatus_case0(InfTest *test)
{
	EaifFixedPointSize scale_factor = { 1 << EAIF_FIXED_POINT_BS, 1 << EAIF_FIXED_POINT_BS };
	struct eaif_status_internal_s src, dst;
	MPI_IVA_OBJ_LIST_S ol;

	src.obj_cnt = 2;
	src.obj_attr_ex[0].basic = (EAIF_OBJ_ATTR_S){
		.id = 0, .rect = (MPI_RECT_POINT_S){ .sx = 0 + 50, .sy = 0 + 50, .ex = 400 - 50, .ey = 400 - 50 }
	};
	src.obj_attr_ex[1].basic = (EAIF_OBJ_ATTR_S){
		.id = 1, .rect = (MPI_RECT_POINT_S){ .sx = 400 + 50, .sy = 0 + 50, .ex = 800 - 50, .ey = 400 - 50 }
	};

	ol.obj_num = 2;
	ol.obj[0] =
	        (MPI_IVA_OBJ_ATTR_S){ .id = 3, .rect = (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = 400, .ey = 400 } };
	ol.obj[1] =
	        (MPI_IVA_OBJ_ATTR_S){ .id = 4, .rect = (MPI_RECT_POINT_S){ .sx = 400, .sy = 0, .ex = 800, .ey = 400 } };

	eaif_cpyScaledFaceStatus(&scale_factor, &src, &ol, &dst);
	testSetMsg("1 fixed scale factor, 2 overlap");
	testAssertEqInt(dst.obj_attr_ex[0].basic.id, 3);
	testAssertEqInt(dst.obj_attr_ex[1].basic.id, 4);

	src.obj_attr_ex[0].basic = (EAIF_OBJ_ATTR_S){
		.id = 0, .rect = (MPI_RECT_POINT_S){ .sx = 0 + 50, .sy = 0 + 50, .ex = 400 - 50, .ey = 400 - 50 }
	};
	src.obj_attr_ex[1].basic = (EAIF_OBJ_ATTR_S){
		.id = 1, .rect = (MPI_RECT_POINT_S){ .sx = 1200 + 50, .sy = 0 + 50, .ex = 1600 - 50, .ey = 400 - 50 }
	};

	eaif_cpyScaledFaceStatus(&scale_factor, &src, &ol, &dst);
	testSetMsg("1 fixed scale factor, 1 overlap");
	testAssertEqInt(dst.obj_attr_ex[0].basic.id, 3);
	testAssertEqInt(dst.obj_attr_ex[1].basic.id, 0x3fffffff);
	return 0;
}

/*
void eaif_updateFrObjAttr(const MPI_IVA_OBJ_LIST_S *ol, const EaifAlgo *algo, EaifStatusInternal *status);
*/
int test_eaif_updateFrObjAttr_case0(InfTest *test)
{
	MPI_IVA_OBJ_LIST_S ol = {};
	EAIF_PARAM_S p = { .obj_life_th = 28 };
	EaifAlgo algo = { .p = &p };
	EaifStatusInternal status = {};
	EaifInfo info = {};

	int id = 0;
	ol.timestamp = 1234;
	ol.obj_num = 3;

	ol.obj[id].id = 1;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 0, 0, 100, 100 };
	ol.obj[id++].life = 160;

	ol.obj[id].id = 2;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 50, 50, 100, 100 };
	ol.obj[id++].life = 30;

	ol.obj[id].id = 4;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 75, 75, 100, 100 };
	ol.obj[id++].life = 24;

	id = 0;
	status.obj_cnt = 4;
	status.obj_attr_ex[id].basic.id = 2;
	status.obj_attr_ex[id++].frame_counter = 34;
	status.obj_attr_ex[id++].basic.id = 3;
	status.obj_attr_ex[id].basic.id = 4;
	status.obj_attr_ex[id++].frame_counter = 23;
	status.obj_attr_ex[id++].basic.id = 5;
	eaif_updateFrObjAttr(&ol, &algo, &info, &status);

	testAssertEqInt(status.obj_cnt, 3);
	testAssertEqInt(status.timestamp, ol.timestamp);
	testAssertEqInt(status.obj_attr_ex[0].basic.id, 1); // new obj
	testAssertEqInt(status.obj_attr_ex[1].basic.id, 2);
	testAssertEqInt(status.obj_attr_ex[1].frame_counter, 35); // update as life >= th
	testAssertEqInt(status.obj_attr_ex[2].basic.id, 4);
	testAssertEqInt(status.obj_attr_ex[2].frame_counter, 23); // no update as life < th
	return 0;
}

/*
int eaif_checkFrAppendable(const MPI_IVA_OBJ_LIST_S *src, const EaifAlgo *algo,
	                       struct eaif_status_internal_s *status, EaifInfo *info);
*/
int test_eaif_checkFrAppendable_case0(InfTest *test)
{
	EAIF_PARAM_S p = {
		.obj_life_th = 28,
		.snapshot_width = 1280,
		.snapshot_height = 720,
		.obj_exist_classify_period = 1,
		.pos_stop_count_th = 99,
		.pos_classify_period = 300,
		.neg_classify_period = 15,
	};
	EAIF_ALGO_STATUS_S ctx = { .param = p,
		                   .info = (EaifInfo){
		                           .src_resoln = { 1920, 1080 },
		                           .inf_fr_counter = 25,
		                           .inf_fr_stage = 1,
		                   } };
	MPI_IVA_OBJ_LIST_S ol;

	faceRecoInappInitAlgo(&ctx);
	EaifAlgo *algo = &ctx.algo;
	EaifStatusInternal *status = &ctx.status;
	EaifInfo *info = &ctx.info;

	int id = 0;
	ol.timestamp = 1234;
	ol.obj_num = 3;

	ol.obj[id].id = 1;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 0, 0, 100, 100 };
	ol.obj[id++].life = 160;

	ol.obj[id].id = 2;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 50, 50, 100, 100 };
	ol.obj[id++].life = 30;

	ol.obj[id].id = 4;
	ol.obj[id].rect = (MPI_RECT_POINT_S){ 75, 75, 100, 100 };
	ol.obj[id++].life = 24;

	id = 0;
	status->obj_cnt = 3;
	status->obj_attr_ex[id].basic.id = 1;
	status->obj_attr_ex[id++].frame_counter = 0;
	status->obj_attr_ex[id].basic.id = 2;
	status->obj_attr_ex[id].stage = 1;
	status->obj_attr_ex[id++].frame_counter = 25;
	status->obj_attr_ex[id].basic.id = 4;
	status->obj_attr_ex[id++].frame_counter = 23;
	eaif_updateFrObjAttr(&ol, algo, info, status);

	int appendable = eaif_checkFrAppendable(&ol, algo, status, info);

	testAssertEqInt(appendable, 1);
	testAssertEqInt(info->obj_list.obj[0].id, 2);
	testAssertEqInt(memcmp(&info->obj_list.obj[0].rect, &ol.obj[1].rect, sizeof(MPI_RECT_POINT_S)), 0);

	return 0;
}

int main()
{
	REGISTER_TEST(test_eaif_checkAppendable_case0);
	REGISTER_TEST(test_eaif_checkAppendable_case1);
	REGISTER_TEST(test_eaif_cpyScaledFaceStatus_case0);
	REGISTER_TEST(test_eaif_updateFrObjAttr_case0);
	REGISTER_TEST(test_eaif_checkFrAppendable_case0);
	TEST_RUN();

	return 0;
}