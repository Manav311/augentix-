#ifndef __TZ_UPDATE_H_
#define __TZ_UPDATE_H_

int enableTzUpdate(void);
int disableTzUpdate(void);
int getDst(void);
int setTimeinfo(int, char *);

#endif /* __TZ_UPDATE_H_ */
