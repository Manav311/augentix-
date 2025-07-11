#ifndef CMDUTIL_H_
#define CMDUTIL_H_

#include <stdio.h>

#include "mpi_index.h"

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

#define PRINT_DEV(d) printf("Device: %d\n", (d).dev)
#define PRINT_PATH(p) printf("Device: %d. Path: %d.\n", (p).dev, (p).path)
#define PRINT_CHN(c) printf("Device: %d. Channel: %d.\n", (c).dev, (c).chn)
#define PRINT_WIN(w) printf("Device: %d. Channel: %d. Window: %d\n", (w).dev, (w).chn, (w).win)
#define PRINT_ENC(e) printf("Encoder: %d.\n", (e).chn)

#define CMD_ERR_ID (0xE)
#define CMD_DEF_ERR(e) (CMD_ERR_ID << 8 | (e))

#define CMD_PRINT_HELP(str, argv, desc) printf("\t%s %-48s %s\n", (str), (argv), (desc))

#ifdef __cplusplus
}
#endif /* !__cplusplus */

#endif /* !CMDUTIL_H_ */