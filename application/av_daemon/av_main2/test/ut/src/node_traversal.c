#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <cmocka_private.h>

#include "getopt.h"

#include "log_define.h"
#include "nodes.h"
#include "handlers.h"

int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

Node g_monk_node[NODE_NUM];

static int VB_initNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_initNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_initNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_initNodeTest(void)
{
	function_called();
	return 0;
}

static int WIN_IMAGE_PREF_initNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_initNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_initNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int WIN_IMAGE_PREF_exitNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_exitNodeTest(void)
{
	function_called();
	return 0;
}
static int ENC_exitNodeTest(void)
{
	printf("call\n");
	function_called();
	return 0;
}

static int VB_startNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_startNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_startNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_startNodeTest(void)
{
	function_called();
	return 0;
}

static int WIN_IMAGE_PREF_startNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_startNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_startNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int DEV_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int IMAGE_PREF_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int CHN_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int WIN_IMAGE_PREF_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int IVA_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int ENC_stopNodeTest(void)
{
	function_called();
	return 0;
}

static int VB_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int DEV_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int IMAGE_PREF_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int CHN_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int WIN_IMAGE_PREF_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int IVA_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}
static int ENC_setNodeTest(int cmd_id, void *data)
{
	AGTX_UNUSED(cmd_id);
	AGTX_UNUSED(data);

	function_called();
	return 0;
}

static void initMonkNodes(void)
{
	/*assign node id and func ptr*/
	int idx = 0;
	g_monk_node[idx].id = VB;
	g_monk_node[idx].init = VB_initNodeTest;
	g_monk_node[idx].exit = VB_exitNodeTest;
	g_monk_node[idx].start = VB_startNodeTest;
	g_monk_node[idx].stop = VB_stopNodeTest;
	g_monk_node[idx].set = VB_setNodeTest;
	g_monk_node[idx].parent = NULL;
	g_monk_node[idx].child[0] = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[1] = NULL;
	g_monk_node[idx].child[2] = NULL;
	idx++;

	g_monk_node[idx].id = DEV;
	g_monk_node[idx].init = DEV_initNodeTest;
	g_monk_node[idx].exit = DEV_exitNodeTest;
	g_monk_node[idx].start = DEV_startNodeTest;
	g_monk_node[idx].stop = DEV_stopNodeTest;
	g_monk_node[idx].set = DEV_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[VB]; /*VB*/
	g_monk_node[idx].child[0] = &g_monk_node[IMAGE_PREFERENCE]; /*IMAGE PREF*/
	g_monk_node[idx].child[1] = &g_monk_node[CHN]; /*CHN*/
	g_monk_node[idx].child[2] = NULL;
	idx++;

	g_monk_node[idx].id = IMAGE_PREFERENCE;
	g_monk_node[idx].init = IMAGE_PREF_initNodeTest;
	g_monk_node[idx].exit = IMAGE_PREF_exitNodeTest;
	g_monk_node[idx].start = IMAGE_PREF_startNodeTest;
	g_monk_node[idx].stop = IMAGE_PREF_stopNodeTest;
	g_monk_node[idx].set = IMAGE_PREF_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	g_monk_node[idx].child[2] = NULL;
	idx++;

	g_monk_node[idx].id = CHN;
	g_monk_node[idx].init = CHN_initNodeTest;
	g_monk_node[idx].exit = CHN_exitNodeTest;
	g_monk_node[idx].start = CHN_startNodeTest;
	g_monk_node[idx].stop = CHN_stopNodeTest;
	g_monk_node[idx].set = CHN_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[DEV]; /*DEV*/
	g_monk_node[idx].child[0] = &g_monk_node[WIN_IMAGE_PREFERENCE]; /*IVA*/
	g_monk_node[idx].child[1] = &g_monk_node[IVA]; /*IVA*/
	g_monk_node[idx].child[2] = &g_monk_node[ENC]; /*ENC*/
	idx++;

	g_monk_node[idx].id = WIN_IMAGE_PREFERENCE;
	g_monk_node[idx].init = WIN_IMAGE_PREF_initNodeTest;
	g_monk_node[idx].exit = WIN_IMAGE_PREF_exitNodeTest;
	g_monk_node[idx].start = WIN_IMAGE_PREF_startNodeTest;
	g_monk_node[idx].stop = WIN_IMAGE_PREF_stopNodeTest;
	g_monk_node[idx].set = WIN_IMAGE_PREF_setNodeTest;
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	g_monk_node[idx].child[2] = NULL;
	idx++;

	g_monk_node[idx].id = IVA;
	g_monk_node[idx].init = IVA_initNodeTest;
	g_monk_node[idx].exit = IVA_exitNodeTest;
	g_monk_node[idx].start = IVA_startNodeTest;
	g_monk_node[idx].stop = IVA_stopNodeTest;
	g_monk_node[idx].set = IVA_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[CHN]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	g_monk_node[idx].child[2] = NULL;
	idx++;

	g_monk_node[idx].id = ENC;
	g_monk_node[idx].init = ENC_initNodeTest;
	g_monk_node[idx].exit = ENC_exitNodeTest;
	g_monk_node[idx].start = ENC_startNodeTest;
	g_monk_node[idx].stop = ENC_stopNodeTest;
	g_monk_node[idx].set = ENC_setNodeTest;
	g_monk_node[idx].parent = &g_monk_node[CHN]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	g_monk_node[idx].child[2] = NULL;
	idx++;
}

/* Test case that fails as leak_memory() leaks a dynamically allocated block. */
static void readDBMemLeakTest(void **state)
{
	(void)state; /* unused */

	HANDLERS_allReadDb(TMP_ACTIVE_DB);
}

static void initExitOrderTest(void **state)
{
	(void)state;
	expect_function_call(VB_initNodeTest);
	expect_function_call(DEV_initNodeTest);
	expect_function_call(IMAGE_PREF_initNodeTest);
	expect_function_call(CHN_initNodeTest);
	expect_function_call(WIN_IMAGE_PREF_initNodeTest);
	expect_function_call(IVA_initNodeTest);
	expect_function_call(ENC_initNodeTest);

	expect_function_call(VB_startNodeTest);
	expect_function_call(DEV_startNodeTest);
	expect_function_call(IMAGE_PREF_startNodeTest);
	expect_function_call(CHN_startNodeTest);
	expect_function_call(WIN_IMAGE_PREF_startNodeTest);
	expect_function_call(IVA_startNodeTest);
	expect_function_call(ENC_startNodeTest);

	expect_function_call(ENC_stopNodeTest);
	expect_function_call(ENC_exitNodeTest);
	expect_function_call(IVA_stopNodeTest);
	expect_function_call(IVA_exitNodeTest);
	expect_function_call(WIN_IMAGE_PREF_stopNodeTest);
	expect_function_call(WIN_IMAGE_PREF_exitNodeTest);
	expect_function_call(CHN_stopNodeTest);
	expect_function_call(CHN_exitNodeTest);
	expect_function_call(IMAGE_PREF_stopNodeTest);
	expect_function_call(IMAGE_PREF_exitNodeTest);
	expect_function_call(DEV_stopNodeTest);
	expect_function_call(DEV_exitNodeTest);
	expect_function_call(VB_stopNodeTest);
	expect_function_call(VB_exitNodeTest);

	NODES_enterNodespreOrderTraversal(&g_monk_node[VB]);
	NODES_startNodespreOrderTraversal(&g_monk_node[VB]);
	NODES_leaveNodespreOrderTraversal(&g_monk_node[VB]);
}

static void VB_restartOrderTest(void **state)
{
	(void)state;
	expect_function_call(ENC_stopNodeTest);
	expect_function_call(IVA_stopNodeTest);
	expect_function_call(WIN_IMAGE_PREF_stopNodeTest);
	expect_function_call(CHN_stopNodeTest);
	expect_function_call(IMAGE_PREF_stopNodeTest);
	expect_function_call(DEV_stopNodeTest);
	expect_function_call(VB_stopNodeTest);

	expect_function_call(VB_startNodeTest);
	expect_function_call(DEV_startNodeTest);
	expect_function_call(IMAGE_PREF_startNodeTest);
	expect_function_call(CHN_startNodeTest);
	expect_function_call(WIN_IMAGE_PREF_startNodeTest);
	expect_function_call(IVA_startNodeTest);
	expect_function_call(ENC_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (VB * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void DEV_restartOrderTest(void **state)
{
	(void)state;
	expect_function_call(ENC_stopNodeTest);
	expect_function_call(IVA_stopNodeTest);
	expect_function_call(WIN_IMAGE_PREF_stopNodeTest);
	expect_function_call(CHN_stopNodeTest);
	expect_function_call(IMAGE_PREF_stopNodeTest);
	expect_function_call(DEV_stopNodeTest);

	expect_function_call(DEV_startNodeTest);
	expect_function_call(IMAGE_PREF_startNodeTest);
	expect_function_call(CHN_startNodeTest);
	expect_function_call(WIN_IMAGE_PREF_startNodeTest);
	expect_function_call(IVA_startNodeTest);
	expect_function_call(ENC_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (DEV * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void IMAGE_PREF_restartOrderTest(void **state)
{
	(void)state;

	expect_function_call(IMAGE_PREF_stopNodeTest);
	expect_function_call(IMAGE_PREF_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (IMAGE_PREFERENCE * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void CHN_restartOrderTest(void **state)
{
	(void)state;
	expect_function_call(ENC_stopNodeTest);
	expect_function_call(IVA_stopNodeTest);
	expect_function_call(WIN_IMAGE_PREF_stopNodeTest);
	expect_function_call(CHN_stopNodeTest);

	expect_function_call(CHN_startNodeTest);
	expect_function_call(WIN_IMAGE_PREF_startNodeTest);
	expect_function_call(IVA_startNodeTest);
	expect_function_call(ENC_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (CHN * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void WIN_IMAGE_PREF_restartOrderTest(void **state)
{
	(void)state;

	expect_function_call(WIN_IMAGE_PREF_stopNodeTest);
	expect_function_call(WIN_IMAGE_PREF_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (WIN_IMAGE_PREFERENCE * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void IVA_restartOrderTest(void **state)
{
	(void)state;
	expect_function_call(IVA_stopNodeTest);
	expect_function_call(IVA_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (IVA * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

static void ENC_restartOrderTest(void **state)
{
	(void)state;
	expect_function_call(ENC_stopNodeTest);
	expect_function_call(ENC_startNodeTest);

	void *node = &g_monk_node[0];
	int ptr = ((int)node) + (ENC * sizeof(Node));
	NODES_execRestart((Node *)ptr);
}

int main(void)
{
	printf("call\n");

	initMonkNodes();

	const struct CMUnitTest test_1[] = { cmocka_unit_test(readDBMemLeakTest),
		                             cmocka_unit_test(initExitOrderTest),
		                             cmocka_unit_test(VB_restartOrderTest),
		                             cmocka_unit_test(DEV_restartOrderTest),
		                             cmocka_unit_test(IMAGE_PREF_restartOrderTest),
		                             cmocka_unit_test(CHN_restartOrderTest),
		                             cmocka_unit_test(WIN_IMAGE_PREF_restartOrderTest),
		                             cmocka_unit_test(IVA_restartOrderTest),
		                             cmocka_unit_test(ENC_restartOrderTest) };

	int result = 0;
	result = cmocka_run_group_tests(test_1, NULL, NULL);

	return result;
}
