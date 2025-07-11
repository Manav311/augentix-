#ifndef __EXTRACT_H__
#define __EXTRACT_H__

#ifdef __cplusplus
extern "C" {
#endif

int AVC_extract_avc(int argc, char **argv);
int AVC_getSampleNum(char *file_name, int *num);
int AVC_getFrameData(char *file_name, int index, char *ptr, int *frame_len);
int AVC_releaseExtractor();

int extract_aac(int argc, char **argv);
int AAC_getSampleNum(char *file_name, int *num);
int AAC_getFrameData(int index, char *ptr, int *frame_len);
int AAC_releaseExtractor();

#ifdef __cplusplus
}
#endif

#endif
