#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_PCA_TABLE (1)

static int readPcaTable(const char *file_name, MPI_PCA_TABLE_S *pca_lut)
{
	FILE *file;

	/* Open file for reading */
	file = fopen(file_name, "r");

	if (file == NULL) {
		fprintf(stderr, "\nError opening file\n");
		return -1;
	}

	/* read file contents till end of file */
	fread(pca_lut, sizeof(MPI_PCA_TABLE_S), 1, file);

	/* close file */
	fclose(file);

	return 0;
}

static void printPcaTable(MPI_PCA_TABLE_S *table)
{
	int i, j, k;
	printf("h[9][13][28] = {\n");
	for (i = 0; i < 9; i++) {
		printf("    {\n");
		for (j = 0; j < 13; j++) {
			printf("        { ");
			for (k = 0; k < 28; k++) {
				printf("%4d, ", table->h[i][j][k]);
			}
			printf(" },\n");
		}
		printf("    },\n");
	}
	printf("};\n");

	printf("s[9][13][28] = {\n");
	for (i = 0; i < 9; i++) {
		printf("    {\n");
		for (j = 0; j < 13; j++) {
			printf("        { ");
			for (k = 0; k < 28; k++) {
				printf("%4d, ", table->s[i][j][k]);
			}
			printf(" },\n");
		}
		printf("    },\n");
	}
	printf("};\n");

	printf("l[9][13][28] = {\n");
	for (i = 0; i < 9; i++) {
		printf("    {\n");
		for (j = 0; j < 13; j++) {
			printf("        { ");
			for (k = 0; k < 28; k++) {
				printf("%4d, ", table->l[i][j][k]);
			}
			printf(" },\n");
		}
		printf("    },\n");
	}
	printf("};\n");
}

static INT32 GET(PcaTable)(CMD_DATA_S *opt)
{
	return MPI_getPcaTable(opt->path_idx, opt->data);
}

static INT32 SET(PcaTable)(const CMD_DATA_S *opt)
{
	return MPI_setPcaTable(opt->path_idx, opt->data);
}

static INT32 USR_DEF(PcaTable)(CMD_DATA_S *opt)
{
	printPcaTable(opt->data);
	return 0;
}

static void ARGS(PcaTable)(void)
{
	printf("\t'--pca_table dev_idx path_idx lut_bin'\n");
	printf("\t'--pca_table 0 0 pca.lut'\n");
	printf("\t'--pca_table lut_bin'\n");
	printf("\t'--pca_table pca.lut'\n");
}

static void HELP(PcaTable)(const char *str)
{
	CMD_PRINT_HELP(str, "'--pca_table <MPI_PATH> [PCA_TABLE_BIN]'",
	               "Set PCA table, PCA_TABLE_BIN is encoded based on MPI_PCA_TABLE_S");
	CMD_PRINT_HELP(str, "'--pca_table [PCA_TABLE_BIN]'",
	               "Show PCA table, PCA_TABLE_BIN is encoded based on MPI_PCA_TABLE_S");
}

static void SHOW(PcaTable)(const CMD_DATA_S *opt)
{
	MPI_PCA_TABLE_S *table = (MPI_PCA_TABLE_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printPcaTable(table);
}

static int PARSE(PcaTable)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_PCA_TABLE_S *data = (MPI_PCA_TABLE_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_PCA_TABLE + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;

		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		readPcaTable(argv[optind], data);
		optind++;

	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;

		opt->path_idx.path = atoi(argv[optind]);
		optind++;

	} else if (num == NUM_PCA_TABLE) {
		opt->action = CMD_ACTION_USR_DEF;
		readPcaTable(argv[optind], data);
		optind++;

	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S pca_table_ops = MAKE_CMD_WITH_USR_DEF("pca_table", MPI_PCA_TABLE_S, PcaTable);

__attribute__((constructor)) void regPcaTableCmd(void)
{
	CMD_register(&pca_table_ops);
}