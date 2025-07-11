#ifndef _DISP_PROC_H_
#define _DISP_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

int display_process_initial(void);
int display_process_deinitial(void);
int display_image_update(void *pImage, unsigned long byte_size);

#ifdef __cplusplus
}
#endif

#endif