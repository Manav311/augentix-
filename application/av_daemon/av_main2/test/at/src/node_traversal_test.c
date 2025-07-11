#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for write

#include "getopt.h"

#include "log_define.h"
#include "nodes.h"
#include "handlers.h"

extern Node g_nodes[NODE_NUM];
Node g_monk_node[NODE_NUM];
int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

#define AVMAIN2_DEBUG

int initNodeTest(void)
{
	return 0;
}

int exitNodeTest(void)
{
	return 0;
}

int startNodeTest(void)
{
	return 0;
}

int stopNodeTest(void)
{
	return 0;
}

static int __attribute__((unused)) setNodeTest(void)
{
	return 0;
}

static void initNodes(void)
{
	/*assign node id and func ptr*/
	int idx = 0;
	g_monk_node[idx].id = VB;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = NULL;
	g_monk_node[idx].child[0] = &g_monk_node[1]; /*DEV*/
	g_monk_node[idx].child[1] = NULL; /*IMAGE_PREFERENCE*/
	idx++;

	g_monk_node[idx].id = DEV;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[0]; /*VB*/
	g_monk_node[idx].child[0] = &g_monk_node[2]; /*IMAGE PREF*/
	g_monk_node[idx].child[1] = &g_monk_node[3]; /*CHN*/
	idx++;

	g_monk_node[idx].id = IMAGE_PREFERENCE;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[0]; /*DEV*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = CHN;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[1]; /*DEV*/
	g_monk_node[idx].child[0] = &g_monk_node[4]; /*IVA*/
	g_monk_node[idx].child[1] = &g_monk_node[5]; /*ENC*/
	idx++;

	g_monk_node[idx].id = IVA;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[3]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;

	g_monk_node[idx].id = ENC;
	g_monk_node[idx].init = initNodeTest;
	g_monk_node[idx].exit = exitNodeTest;
	g_monk_node[idx].start = startNodeTest;
	g_monk_node[idx].stop = stopNodeTest;
	g_monk_node[idx].parent = &g_monk_node[3]; /*CHN*/
	g_monk_node[idx].child[0] = NULL;
	g_monk_node[idx].child[1] = NULL;
	idx++;
}

void help()
{
	printf("[usage]\r\n");
	printf("-r --real use real (not monk) nodes to test traversal\r\n");
	printf("-h --help\r\n");
	printf("-v --no_video\n");
	printf("-f --no_iva\n");
	printf("-m --no_image_pref\n");
}

int main(int argc, char **argv)
{
	avmain2_log_info("system start");

	g_run_flag = 0;
	int c = 0;
	bool use_real_nodes = false;
	const char *optstring = "hrfmv";
	struct option opts[] = { { "help", 0, NULL, 'h' },
		                 { "real", 0, NULL, 'r' },
		                 { "no_iva", 0, NULL, 'f' },
		                 { "no_image_pref", 0, NULL, 'm' },
		                 { "no_video", 0, NULL, 'v' } };
	while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'r':
			use_real_nodes = true;
			break;
		case 'f':
			g_no_iva_flag = true;
			break;
		case 'm':
			g_no_image_preference_flag = true;
			break;
		case 'v':
			g_no_video_control_flag = true;
			break;
		default:
			help();
			exit(1);
		}
	}

	avmain2_log_info("Flag [no iva, no img pref, no video]:(%d, %d, %d)\n", g_no_iva_flag,
	                 g_no_image_preference_flag, g_no_video_control_flag);

	if (g_no_iva_flag && g_no_image_preference_flag && g_no_video_control_flag) {
		help();
		exit(1);
	}

	if (use_real_nodes == false) {
		initNodes();
		for (int i = 0; i < NODE_NUM; i++) {
			avmain2_log_info("enter/leave now testing: %d...", i);
			NODES_enterNodespreOrderTraversal(&g_monk_node[i]);
			NODES_leaveNodespreOrderTraversal(&g_monk_node[i]);
		}

		sleep(1);

		for (int i = 0; i < NODE_NUM; i++) {
			avmain2_log_info("restart now testing: %d...", i);
			NODES_execRestart(&g_monk_node[i]);
		}
		avmain2_log_info("monk test end: ...");
		return 0;
	}

	NODES_initNodes();
	HANDLERS_allReadDb(TMP_ACTIVE_DB);

	NODES_enterNodespreOrderTraversal(&g_nodes[VB]);
	for (int i = 0; i < NODE_NUM; i++) {
		avmain2_log_info("enter/leave now testing: %d...", i);
		NODES_leaveNodespreOrderTraversal(&g_nodes[i]);
		NODES_enterNodespreOrderTraversal(&g_nodes[i]);
	}
	NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);

	sleep(3);

	system("cat /dev/vbs");

	sleep(3);

	NODES_enterNodespreOrderTraversal(&g_nodes[VB]);
	for (int i = 0; i < NODE_NUM; i++) {
		avmain2_log_info("restart now testing: %d...", i);
		NODES_execRestart(&g_nodes[i]);
	}
	NODES_leaveNodespreOrderTraversal(&g_nodes[VB]);

	sleep(3);

	system("cat /dev/vbs");

	sleep(3);

	avmain2_log_info("real test end: ...");

	return 0;
}