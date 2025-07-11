#ifndef TZ_UPD_H_
#define TZ_UPD_H_

int enableTzUpdate(void);
int disableTzUpdate(void);
int getDst(void);
int setTimeinfo(int, char *);

#endif /* TZ_UPD_H_ */
