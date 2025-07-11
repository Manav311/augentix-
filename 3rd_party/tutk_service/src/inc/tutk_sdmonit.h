#ifndef TUTK_SDRECORD_H_
#define TUTK_SDRECORD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SD_FORMAT_NTFS 0x01 /*NTFS*/
#define SD_FORMAT_VFAT 0x02 /*FAT32*/
#define SD_FORMAT_UNKOWN 0x03 /*Unkown*/

extern char sd_format;

int TUTK_sd_monit_init();
int TUTK_check_sdard_usage();
int TUTK_check_sdard_format(void);
int TUTK_check_sdard_mount(void);
int TUTK_check_sdcard_filesystem();
int TUTK_get_sdcard_status();
void *thread_sd_format(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* TUTK_SDRECORD_H_ */
