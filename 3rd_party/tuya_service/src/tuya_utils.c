/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

#include "tuya_utils.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tuya_ipc_common_demo.h"

int forkIndependentProc(char *prog, char **arg_list)
{
	pid_t child;

	if ((child = fork()) < 0) {
		/* parent: check if fork failed */
		//PR_ERR("fork error");
	} else if (child == 0) {
		/* 1st level child: fork again */
		if ((child = fork()) < 0) {
			//PR_ERR("fork error");
		} else if (child > 0) {
			/* 1st level child: terminate itself to make init process the parent of 2nd level child */
			exit(0);
		} else {
			/* 2nd level child: execute program and will become child of init once 1st level child exits */
			execvp(prog, arg_list);
			//PR_ERR("execvp error");
			exit(0);
		}
	}

	/* parent: wait for 1st level child ends */
	waitpid(child, NULL, 0);

	return child;
}

int is_little_endian()
{
	int test_num = 0xff;
	unsigned char *ptr = (unsigned char *)&test_num;
	if (ptr[0] == 0xff) {
		return 1;
	}
	return 0;
}

