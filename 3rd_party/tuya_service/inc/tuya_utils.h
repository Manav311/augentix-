#ifndef TUYA_UTILS_H_
#define TUYA_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

int forkIndependentProc(char *prog, char **arg_list);
int is_little_endian();

#ifdef __cplusplus
}
#endif

#endif /* TUYA_UTILS_H_ */
