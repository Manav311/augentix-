#ifndef CMDPARSER_H_
#define CMDPARSER_H_

#include "mpi_index.h"
#include "mpi_osd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum CMD_ACTION_E
 * @brief Support actions for cmdparser
 */
typedef enum cmd_action {
	CMD_ACTION_ARGS = 0, /**< See args */
	CMD_ACTION_GET, /**< Get parameters */
	CMD_ACTION_SET, /**< Set parameters */
	CMD_ACTION_USR_DEF, /**< User defined action */
	CMD_ACTION_NON, /**< Do nothing */
	CMD_ACTION_NUM,
} CMD_ACTION_E;

/**
 * @struct CMD_DATA_S
 * @brief Store the attributes
 */
typedef struct cmd_data {
	CMD_ACTION_E action;
	union {
		MPI_DEV dev_idx;
		MPI_PATH path_idx;
		MPI_CHN chn_idx;
		MPI_WIN win_idx;
		MPI_ECHN echn_idx;
		OSD_HANDLE osd_handle;
	};
	void *data;
} CMD_DATA_S;

/**
 * @brief Generate wrapping function
 * @param[in] f    function name
 */
#define CMD_WRAPPER(f) CMD_##f
#define GET(f) CMD_WRAPPER(get##f) /**< CMD_get##f */
#define SET(f) CMD_WRAPPER(set##f) /**< CMD_set##f */
#define USR_DEF(f) CMD_WRAPPER(usr_def##f) /**< CMD_usr1##f */
#define SIZE(f) sizeof(f) /**< Size of related attribute */
#define PARSE(f) PARSE_##f
#define ARGS(f) ARGS_##f
#define SHOW(f) SHOW_##f
#define HELP(f) HELP_##f

/**
 * @brief Helper macro to define a CMD_S structure
 * @param[in] name    long option
 * @param[in] attr    attribute for that command, used for memory allocation
 * @param[in] f       function name, used for searching GET, SET, USR_DEF, HELP, ARGS, SHOW and PARSE
 */
#define MAKE_CMD(name, attr, f)                                                                      \
	{                                                                                            \
		(name), (0), sizeof(attr), GET(f), SET(f), NULL, HELP(f), ARGS(f), SHOW(f), PARSE(f) \
	}

/**
 * @see MAKE_CMD
 */
#define MAKE_CMD_WITH_USR_DEF(name, attr, f)                                                               \
	{                                                                                                  \
		(name), (0), sizeof(attr), GET(f), SET(f), USR_DEF(f), HELP(f), ARGS(f), SHOW(f), PARSE(f) \
	}

/**
 * @struct CMD_S
 */
typedef struct cmd {
	char *name; /**< Long option / user visible name */
	int val; /**< Short option */
	int size; /**< Size of related attribute */
	INT32 (*get)(CMD_DATA_S *opt); /**< Getting parameters from MPP system */
	INT32 (*set)(const CMD_DATA_S *opt); /**< Setting parameters from MPP system */
	INT32 (*usr_def)(CMD_DATA_S *opt); /**< User defined action */
	void (*help)(const char *str); /**< Show help description */
	void (*args)(void); /**< Show argument description */
	void (*show)(const CMD_DATA_S *opt); /**< Show retrieved parameters */
	int (*parse)(int argc, char **argv, CMD_DATA_S *opt); /**< Parse cmd arguments */
} CMD_S;

/**
 * @return Number of registered commands.
 * @retval larger than 0    success
 * @retval -EINVAL          repeated value for target cmd
 * @retval -EOVERFLOW       not enough memory for registration array.
 */
int CMD_register(CMD_S *reg_cmd);

/**
 * @return The execution result
 * @retval 0          success
 * @retval -EINVAL    invalid arguments
 * @retval -ENOMEM    cannot allocate memory for operations
 */
int CMD_execute(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* !CMDPARSER_H_ */