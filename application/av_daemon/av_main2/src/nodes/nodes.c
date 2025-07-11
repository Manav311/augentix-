#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "log_define.h"

Node g_nodes[NODE_NUM];

extern bool g_no_iva_flag;
extern bool g_no_image_preference_flag;
extern bool g_no_video_control_flag;

int NODES_initNodes(void)
{
	/*assign node id and func ptr*/
	int idx = 0;
	g_nodes[idx].id = VB;
	g_nodes[idx].init = NODE_initVb;
	g_nodes[idx].start = NODE_startVb;
	g_nodes[idx].stop = NODE_stopVb;
	g_nodes[idx].exit = NODE_exitVb;
	g_nodes[idx].set = NULL;
	idx++;

	g_nodes[idx].id = DEV;
	g_nodes[idx].init = NODE_initDev;
	g_nodes[idx].start = NODE_startDev;
	g_nodes[idx].stop = NODE_stopDev;
	g_nodes[idx].exit = NODE_exitDev;
	g_nodes[idx].set = NODE_setDev;
	idx++;

	g_nodes[idx].id = IMAGE_PREFERENCE;
	g_nodes[idx].init = NODE_initImagePreference;
	g_nodes[idx].start = NODE_startImagePreference;
	g_nodes[idx].stop = NODE_stopImagePreference;
	g_nodes[idx].exit = NODE_exitImagePreference;
	g_nodes[idx].set = NODE_setImagePreference;
	idx++;

	g_nodes[idx].id = CHN;
	g_nodes[idx].init = NODE_initChn;
	g_nodes[idx].start = NODE_startChn;
	g_nodes[idx].stop = NODE_stopChn;
	g_nodes[idx].exit = NODE_exitChn;
	g_nodes[idx].set = NODE_setChn;
	idx++;

	g_nodes[idx].id = WIN_IMAGE_PREFERENCE;
	g_nodes[idx].init = NODE_initWinImagePreference;
	g_nodes[idx].start = NODE_startWinImagePreference;
	g_nodes[idx].stop = NODE_stopWinImagePreference;
	g_nodes[idx].exit = NODE_exitWinImagePreference;
	g_nodes[idx].set = NODE_setWinImagePreference;
	idx++;

	g_nodes[idx].id = IVA;
	g_nodes[idx].init = NODE_initIva;
	g_nodes[idx].start = NODE_startIva;
	g_nodes[idx].stop = NODE_stopIva;
	g_nodes[idx].exit = NODE_exitIva;
	g_nodes[idx].set = NODE_setIva;
	idx++;

	g_nodes[idx].id = ENC;
	g_nodes[idx].init = NODE_initEnc;
	g_nodes[idx].start = NODE_startEnc;
	g_nodes[idx].stop = NODE_stopEnc;
	g_nodes[idx].exit = NODE_exitEnc;
	g_nodes[idx].set = NODE_setEnc;

	/*link each Nodes to others*/
	idx = 0;
	/*VB*/
	g_nodes[idx].parent = NULL;
	g_nodes[idx].child[0] = &g_nodes[DEV];
	g_nodes[idx].child[1] = NULL;
	g_nodes[idx].child[2] = NULL;
	idx++;

	/*DEV*/
	g_nodes[idx].parent = &g_nodes[VB];
	g_nodes[idx].child[0] = &g_nodes[IMAGE_PREFERENCE];
	g_nodes[idx].child[1] = &g_nodes[CHN];
	g_nodes[idx].child[2] = NULL;
	idx++;

	/*IMAGE_PREFERENCE*/
	g_nodes[idx].parent = &g_nodes[DEV];
	g_nodes[idx].child[0] = NULL;
	g_nodes[idx].child[1] = NULL;
	g_nodes[idx].child[2] = NULL;
	idx++;

	/*CHN*/
	g_nodes[idx].parent = &g_nodes[DEV];
	g_nodes[idx].child[0] = &g_nodes[WIN_IMAGE_PREFERENCE];
	g_nodes[idx].child[1] = &g_nodes[IVA];
	g_nodes[idx].child[2] = &g_nodes[ENC];
	idx++;

	/*WIN_IMAGE_PREFERENCE*/
	g_nodes[idx].parent = &g_nodes[CHN];
	g_nodes[idx].child[0] = NULL;
	g_nodes[idx].child[1] = NULL;
	g_nodes[idx].child[2] = NULL;
	idx++;

	/*IVA*/
	g_nodes[idx].parent = &g_nodes[CHN];
	g_nodes[idx].child[0] = NULL;
	g_nodes[idx].child[1] = NULL;
	g_nodes[idx].child[2] = NULL;
	idx++;

	/*ENC*/
	g_nodes[idx].parent = &g_nodes[CHN];
	g_nodes[idx].child[0] = NULL;
	g_nodes[idx].child[1] = NULL;
	g_nodes[idx].child[2] = NULL;

	return 0;
}

static bool isSkipCase(Node *current)
{
	bool isSkip = false;
	if (current->id == IVA && g_no_iva_flag) {
		avmain2_log_notice("skip IVA");
		isSkip = true;
	}

	if (current->id == IMAGE_PREFERENCE && g_no_image_preference_flag) {
		avmain2_log_notice("skip  IMAGE_PREFERENCE");
		isSkip = true;
	}

	if ((current->id == VB || current->id == DEV || current->id == CHN || current->id == ENC) &&
	    g_no_video_control_flag) {
		avmain2_log_notice("skip VIDEO CONTROL");
		isSkip = true;
	}

	return isSkip;
}

int NODES_enterNodespreOrderTraversal(Node *current)
{
	int ret = 0;
	if (current == NULL) {
		return 0;
	}

	avmain2_log_info("NODE id: %d", current->id);

	/*e.g if skip IVA, use main.c flag to skip .start() func of node_IVA at here */
	if (current->init != NULL && isSkipCase(current) == false) {
		if (current->init() != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("init ptr == NULL");
	}

	NODES_enterNodespreOrderTraversal(current->child[0]);
	NODES_enterNodespreOrderTraversal(current->child[1]);
	NODES_enterNodespreOrderTraversal(current->child[2]);

	return 0;
}

int NODES_startNodespreOrderTraversal(Node *current)
{
	int ret = 0;
	if (current == NULL) {
		return 0;
	}

	avmain2_log_info("NODE id: %d", current->id);

	if (current->start != NULL && isSkipCase(current) == false) {
		if (current->start() != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("start ptr == NULL");
	}
	NODES_startNodespreOrderTraversal(current->child[0]);
	NODES_startNodespreOrderTraversal(current->child[1]);
	NODES_startNodespreOrderTraversal(current->child[2]);

	return 0;
}

/*
method ref:
https://www.includehelp.com/data-structure-tutorial/reverse-postorder-traversal-in-binary-tree-using-recursion-in-c-cpp.aspx
*/
int NODES_leaveNodespreOrderTraversal(Node *current)
{
	int ret = 0;
	if (current == NULL) {
		return 0;
	}

	NODES_leaveNodespreOrderTraversal(((Node *)current)->child[2]);
	NODES_leaveNodespreOrderTraversal(((Node *)current)->child[1]);
	NODES_leaveNodespreOrderTraversal(((Node *)current)->child[0]);

	avmain2_log_info("NODE id: %d", current->id);

	if (current->stop != NULL && isSkipCase(current) == false) {
		ret = current->stop();
		if (ret != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("stop ptr == NULL");
	}

	if (current->exit != NULL && isSkipCase(current) == false) {
		ret = current->exit();
		if (ret != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("exit ptr == NULL");
	}

	return 0;
}

static int execStart(Node *current)
{
	if (current == NULL) {
		return 0;
	}

	int ret = 0;

	if (current->start != NULL && isSkipCase(current) == false) {
		ret = current->start();
		avmain2_log_debug("id %d start!", current->id);
		if (ret != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("start ptr == NULL");
		return -EACCES;
	}

	execStart(current->child[0]);
	execStart(current->child[1]);
	execStart(current->child[2]);

	return 0;
}

static int execStop(Node *current)
{
	if (current == NULL) {
		return 0;
	}

	execStop(((Node *)current)->child[2]);
	execStop(((Node *)current)->child[1]);
	execStop(((Node *)current)->child[0]);

	int ret = 0;

	if (current->stop != NULL && isSkipCase(current) == false) {
		avmain2_log_debug("id %d stop!", current->id);
		ret = current->stop();
		if (ret != 0) {
			return ret;
		}
	} else {
		avmain2_log_err("stop ptr == NULL");
		return -EACCES;
	}

	return 0;
}

int NODES_execRestart(Node *current)
{
	if (current == NULL) {
		avmain2_log_err("node is NULL ptr");
		return -EACCES;
	}

	int ret = 0;

	ret = execStop(current);
	if (ret != 0) {
		avmain2_log_err("failed to exec stop, id:%d", current->id);
		return ret;
	}
	ret = execStart(current);
	if (ret != 0) {
		avmain2_log_err("failed to exec start, id:%d", current->id);
		return ret;
	}

	return 0;
}

int NODES_execSet(Node *node, int agtx_cmd_id, void *data)
{
	avmain2_log_debug("NODE id: %d", node->id);

	if (isSkipCase(node)) {
		avmain2_log_info("skip this node");
		return 0;
	}

	if (((node == NULL) || (node->set == NULL)) || (data == NULL)) {
		avmain2_log_err("ptr is null");
		return -ENOENT;
	}

	int ret = 0;
	ret = node->set(agtx_cmd_id, data);
	if (ret != 0) {
		avmain2_log_err("failed to exec set");
		return -EINVAL;
	}
	return 0;
}
