#define _GNU_SOURCE // getopt_long()

#include "cmdparser.h"
#include "cmd_util.h"

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_sys.h"

#define CMD_ITEM_NUM 64

/**< Include all commands here */

static CMD_S g_cmd[CMD_ITEM_NUM] = { { 0 } };
static struct option long_opts[CMD_ITEM_NUM + 1] = { { 0 } };

/**
 * @brief Copy argument setting from cmd to long_opt & short_opt
 * @param[in] c    return value of syscall getopt_long()
 * @return pointer to struct CMD_S
 * @retval NULL    not found
 */
static const CMD_S *getCmd(int c)
{
	for (int i = 0; i < CMD_ITEM_NUM; ++i) {
		if (g_cmd[i].val == c) {
			return &g_cmd[i];
		}
	}

	return NULL;
}

/**
 * @brief Utility funtion for qsort()
 */
static int compare(const void *a, const void *b)
{
	return strcmp(((CMD_S *)a)->name, ((CMD_S *)b)->name);
}

/**
 * @brief Show supported arguments
 * @param[in] str    name of executable file (argv[0])
 */
static void printHelp(char *str)
{
	CMD_S *cmd = g_cmd;

	printf("USAGE:\n");
	printf("\t%s [Option] [Parameter]\n", str);
	printf("\n");
	printf("Option: \n");

	while (cmd->args) {
		cmd->help(""); ++cmd;
	}
}

/**
 * @brief Show usage of the functions.
 */
static void printDetails(const CMD_S *cmd, const char *str)
{
	printf("USAGE:\n");
	cmd->help(str);
	printf("\n");

	printf("EXAMPLES: \n");
	cmd->args();
	printf("\n");
}

int CMD_execute(int argc, char **argv)
{
	CMD_DATA_S cdata = { 0 };
	const CMD_S *cmd;
	int ret = 0;
	int c;

	c = getopt_long(argc, argv, "h", long_opts, NULL);
	if (c == -1) {
		return -EINVAL;
	}

	// TODO: Integrate "-h / --help to commands."
	cmd = getCmd(c);
	if (cmd == NULL) {
		switch (c) {
		case 'h':
			printHelp(argv[0]);
			return 0;

		case ':':
			return -EINVAL;

		default:
			printf("Unknown option character '\\x%x'.\n", optopt);
			return -EINVAL;
		}
	}

	char buf[cmd->size];
	memset(&buf, 0, cmd->size);
	cdata.data = &buf;

	/**< Print detail usage if optional argument is not given */
	if (optind == argc) {
		printDetails(cmd, argv[0]);
		return 0;
	}

	/**< Parsing arguments */
	if ((ret = cmd->parse(argc, argv, &cdata))) {
		if (ret == -EINVAL) {
			printf("Invalid arguments. Please follow the format:\n");
			printf("\n");
			printDetails(cmd, argv[0]);
			return ret;
		} else if (ret < 0) {
			return ret;
		}
	}

	/**< Get (set) attributes from (to) MPP system */
	switch (cdata.action) {
	case CMD_ACTION_ARGS:
		cmd->args();
		break;
	case CMD_ACTION_GET:
		ret = cmd->get(&cdata);
		if (ret < 0) {
			break;
		}
		cmd->show(&cdata);
		break;
	case CMD_ACTION_SET:
		ret = cmd->set(&cdata);
		if (ret < 0) {
			break;
		}
		break;
	case CMD_ACTION_USR_DEF:
		ret = cmd->usr_def(&cdata);
		if (ret < 0) {
			break;
		}
		break;
	case CMD_ACTION_NON:
		break;
	default:
		/**< Action can be GET or SET only. Otherwise this is an internal error. */
		assert(cdata.action < CMD_ACTION_NUM);
		break;
	}

	return ret;
}

int CMD_register(CMD_S *cmd)
{
	/**< Count the times of function be called */
	static int count = 0;

	if (count == CMD_ITEM_NUM) {
		return -EOVERFLOW;
	}

	/**< Register struct CMD_S */
	cmd->val = count;
	memcpy(&(g_cmd[count]), cmd, sizeof(CMD_S));

	/**< Register long options */
	long_opts[count].name = cmd->name;
	long_opts[count].has_arg = no_argument;
	long_opts[count].flag = NULL;
	long_opts[count].val = cmd->val;

	/**< Point to next option */
	return (++count);
}

int main(int argc, char **argv)
{
	int nums = 0;
	int execute_result = 0;
	int ret;

	for (CMD_S *cmd = g_cmd; cmd->name != NULL; ++nums, ++cmd);
	qsort(g_cmd, nums, sizeof(CMD_S), compare);

	if (argc == 1) {
		printHelp(argv[0]);
		return EXIT_SUCCESS;
	}

	if ((ret = MPI_SYS_init())) {
		printf("MPI_SYS_init() failed. err: %d\n", ret);
		return EXIT_FAILURE;
	}

	execute_result = CMD_execute(argc, argv);

	if ((ret = MPI_SYS_exit())) {
		printf("MPI_SYS_exit() failed. err: %d\n", ret);
		return EXIT_FAILURE;
	}

	if (execute_result) {
		return EXIT_FAILURE;
	}

	return 0;
}
