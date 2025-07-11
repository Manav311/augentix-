#ifndef EAIF_C_TEST_H_
#define EAIF_C_TEST_H_

#ifdef __cplusplus
// cxx definition
#include "eaif_test.h"
extern "C" {
typedef struct TestSuit TestSuit;
#else
typedef struct TestSuit TestSuit;
typedef int (*TestFun)(TestSuit *);
#endif

TestSuit *TestSuit_create(void);
void TestSuit_free(TestSuit **suit);

void TestSuit_strVecAdd(TestSuit *suit, char *msg);
void TestSuit_add(TestSuit *suit, TestFun funptr);
void TestSuit_run(TestSuit *suit);
void TestSuit_report(TestSuit *suit);

#ifndef __cplusplus // c defintiion

#define testAssert(cond)                                                                           \
	{                                                                                          \
		if (!(cond)) {                                                                     \
			char msg[512];                                                             \
			sprintf(msg, "[FAIL] %s:%d fail cond: (%s)\n", __func__, __LINE__, #cond); \
			TestSuit_strVecAdd(suit, msg);                                             \
			return -1;                                                                 \
		} else {                                                                           \
		}                                                                                  \
	}

#define testLog(fmt, args...) printf("[LOG] [%s:%d] " fmt, __func__, __LINE__, ##args)

#endif /* !__cplusplus */

#ifdef __cplusplus
}
#endif

#endif /* EAIF_TEST_H_ */
