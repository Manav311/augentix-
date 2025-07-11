#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mpi_index.h"
#include "mpi_sys.h"
#include "mpi_enc.h"

void help(char *bin)
{
	printf("Usage:\n");
	printf("\t\t%s <ENC_IDX>\n", bin);
	printf("\n");
}

int main(int argc, char **argv)
{
	MPI_ECHN echn_idx;

	if ((argc < 2) || !isdigit(argv[1][0])) {
		help(argv[0]);
		return 0;
	}

	MPI_SYS_init();

	echn_idx.chn = atoi(argv[1]);
	MPI_ENC_requestIdr(echn_idx);

	MPI_SYS_exit();

	return 0;
}
