#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_osd.h"

#include "cmd_util.h"

#define NUM_OSD_ATTR (1)

static INT32 GET(OsdAttr)(CMD_DATA_S *opt)
{
	MPI_OSD_RGN_ATTR_S *attr = (MPI_OSD_RGN_ATTR_S *)opt->data;

	return MPI_getOsdRgnAttr(opt->osd_handle, attr);
}

// FIXME: redesign OSD setter
static INT32 SET(OsdAttr)(const CMD_DATA_S *opt)
{
	MPI_OSD_RGN_ATTR_S *attr = (MPI_OSD_RGN_ATTR_S *)opt->data;

	return MPI_setOsdRgnAttr(opt->osd_handle, attr);
}

static void ARGS(OsdAttr)(void)
{
	printf("\t'--osd handle show[0 ~ 1]'\n");
	printf("\t'--osd 0 0'\n");
}

static void HELP(OsdAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--osd <OSD_HANDLE> [OSD_ATTR]'", "Set OSD show or hide.");
}

static void SHOW(OsdAttr)(const CMD_DATA_S *opt __attribute__((unused)))
{
}

// FIXME: redesign OSD getter and setter
static int PARSE(OsdAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_OSD_RGN_ATTR_S *attr = (MPI_OSD_RGN_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == NUM_OSD_ATTR + 1) {
		opt->action = CMD_ACTION_SET;
		opt->osd_handle = atoi(argv[optind++]);
		attr->show = atoi(argv[optind++]);
	} else if (num == 1) {
		opt->action = CMD_ACTION_GET;
		opt->osd_handle = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S osd_ops = MAKE_CMD("osd", MPI_OSD_RGN_ATTR_S, OsdAttr);

__attribute__((constructor)) void regOsdCmd(void)
{
	CMD_register(&osd_ops);
}
