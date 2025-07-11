
#ifndef CCCLIENT_H
#define CCCLIENT_H

// macros for dump image
#define CC_GET_CMD_MODE (-1)
#define CC_GET_TFW_W (-11)
#define CC_GET_TFW_H (-12)
#define CC_GET_NRW_W (-13)
#define CC_GET_NRW_H (-14)
#define CC_GET_BAYER (-15)
#define CC_GET_INPUT_PATH_CNT (-16)
#define CC_GET_REG (-17)
#define CC_GET_GPIO (-18)
#define CC_GET_ADC (-19)
#define CC_GET_WIN_NUM (-20)

int ccClientSet(char *jstr);
int ccClientGet(char *jstr, int clientfd);
void closeCC(void);

#endif /* CCCLIENT_H */
