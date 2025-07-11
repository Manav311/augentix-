#ifndef UTILS_H
#define UTILS_H

#define MAX_ACTION_LEN 4
#define MAX_FILE_SIZE_BYTES 10
#define JSON_STR_LEN 32768
#define MAX_DATA_SIZE_BYTES 256
#define DELIMITER '@'

#include "unicorn_debug.h"

//char g_dut_mode[16] = "None";

unsigned int get_command_id(const char *setting);
char *unicorn_json_add_key_int(char *buffer, char *dKey, int val, int strLen);
char *unicorn_json_add_key_string(char *buffer, char *dKey, char *val, int strLen);
void unicorn_json_delete_key(char *buffer, char *dKey, int strLen);
int unicorn_json_get_int(char *buffer, char *dKey, int strLen);
char *unicorn_json_get_string(char *buffer, char *dKey, int strLen);
char *unicorn_json_get_object(char *buffer, char *dKey, int strLen);
int unicorn_json_get_int_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen);
int unicorn_json_get_array_length(char *buffer, char *arrKey, int strLen);
char *unicorn_json_get_object_from_array(char *buffer, char *arrKey, int arrIdx, int strLen);
int unicorn_json_validation(char *buffer, int strLen);
void removeSpaces(char *src);
char *extractSingleMessage(char *src, int nbytes);
int parseFirstNum(char *src);
long parseFileSize(char *src);
void truncateMessage(char *src, const char sentinel, int nbytes);
int findSpase(char *src);
int executeSystemCommand(const char *params);
int captureStdoutStr(const char *cmd, char *stdout);
int accessUbootReg(const char *key, const char *reg, char *buf, const char *type);
int getCalibFilePath(char *type, char *file_path);
int writeJsonDataToFile(char *file, char *data);
int parseStringArray(char *string, char **str_array, int array_len, int single_size);
int parseIntArray(char *string, int *int_array);
//int getModeFromDut(void);
int isExecutableMode(char *executable_mode_list);
int folderCtrl(char *jStr);
int collectData(char *curDir, char *jStr);
int getDipExtend(char *params, char **jStr, char **buf);
int setDipExtend(char *jStr, int socketfd);

typedef int (*MPI_READ_S)(int idx, void *data);
typedef int (*MPI_WRITE_S)(int idx, const void *data);
int getMpiFunc(MPI_READ_S *read_func, MPI_WRITE_S *write_func, int cmd_id);

#endif
