#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct inf_test;

/**@brief struct for InfTest
 **/
typedef struct inf_test {
	int size; /* unit test function Size */
	int (*tests[128])(struct inf_test *); /* unit test functors*/
	char msg[128][512]; /* unit test assertion message */
	int ret[128]; /* unit test return value */
	int index; /* unit test running index */
	void *args; /* unit test program args */
} InfTest;

InfTest g_test = {};

char g_msg[256] = {};

/**@brief set current assertion message */
#define testSetMsg(fmt, args...)                        \
	{                                               \
		int size = sprintf(g_msg, fmt, ##args); \
		g_msg[size] = 0;                        \
	}

/**@brief test assertion and store predefined assertion message */
#define testAssert(cond)                                                                                          \
	{                                                                                                         \
		if (!(cond)) {                                                                                    \
			sprintf(test->msg[test->index], "%s:L%d [%s] Assertion failed cond:\"(%s)\"\n", __func__, \
			        __LINE__, g_msg, #cond);                                                          \
			return -1;                                                                                \
		}                                                                                                 \
	}

/**@brief test assertion and store predefined assertion message with value shown*/
#define testAssertEqInt(a, b)                                                                                    \
	{                                                                                                        \
		if (!((a) == (b))) {                                                                             \
			sprintf(test->msg[test->index],                                                          \
			        "%s:L%d [%s] Assertion failed cond:\"(%s(%d) is not equal to %d)\"\n", __func__, \
			        __LINE__, g_msg, #a, a, b);                                                      \
			return -1;                                                                               \
		}                                                                                                \
	}

/**@brief test assertion and store assertion message */
#define testAssertMsg(cond, fmt, args...)                                                                         \
	{                                                                                                         \
		if (!(cond)) {                                                                                    \
			char msg[256] = {};                                                                       \
			sprintf(msg, fmt, ##args);                                                                \
			sprintf(test->msg[test->index], "%s:L%d [%s] Assertion failed cond:\"(%s)\"\n", __func__, \
			        __LINE__, msg, #cond);                                                            \
			return -1;                                                                                \
		}                                                                                                 \
	}

#define F_EPSILON (1e-5)
#define testAssertFloatEqMsg(fq, fgt, fmt, args...)                                                               \
	{                                                                                                         \
		bool a = ((fgt - F_EPSILON) < fq) && ((fgt + F_EPSILON) > fq);                                    \
		if (!(a)) {                                                                                       \
			char msg[256] = {};                                                                       \
			sprintf(msg, fmt, ##args);                                                                \
			sprintf(test->msg[test->index], "%s:L%d [%s] Assertion failed cond:\"(%.4f == %.4f)\"\n", \
			        __func__, __LINE__, msg, fq, fgt);                                                \
			return -1;                                                                                \
		}                                                                                                 \
	}

/**@brief register unit test function to struct InfTest */
#define REGISTER_TEST(test)                       \
	{                                         \
		g_test.tests[g_test.size] = test; \
		g_test.size++;                    \
	}

#ifndef __cplusplus

static inline void test_run(InfTest *test, char *file_name)
{
	int success_cnt = test->size;
	int fail_cnt = 0;
	char markers[129] = {};
	memset(markers, '.', test->size);
	int i;
	for (i = 0; i < test->size; i++) {
		test->index = i;
		g_msg[0] = 0;
		test->ret[i] = test->tests[i](test);
		if (test->ret[i]) {
			fail_cnt += 1;
			markers[i] = 'x';
		}
	}
	success_cnt -= fail_cnt;

	fprintf(stdout, "\n  ======= Testing Report : %s ======\n\n", file_name);
	fprintf(stdout, "\tTest on number of functions is %d\n", test->size);
	fprintf(stdout, "\tFail on number of functions is %d\n", fail_cnt);
	fprintf(stdout, "\tSucc on number of functions is %d\n\n", success_cnt);
	fprintf(stdout, "%s\n\n", markers);
	for (i = 0; i < test->size; i++) {
		if (test->ret[i])
			fprintf(stderr, "# %d :: %s\n", i, test->msg[i]);
	}
	fprintf(stdout, "\n");
}

#else

#include <string>

static inline void test_run(InfTest *test, std::string file_name)
{
	int success_cnt = test->size;
	int fail_cnt = 0;
	char markers[129] = {};
	memset(markers, '.', test->size);
	int i;
	for (i = 0; i < test->size; i++) {
		test->index = i;
		test->ret[i] = test->tests[i](test);
		if (test->ret[i]) {
			fail_cnt += 1;
			markers[i] = 'x';
		}
	}
	success_cnt -= fail_cnt;

	fprintf(stdout, "\n  ======= Testing Report : %s ======\n\n", file_name.c_str());
	fprintf(stdout, "\tTest on number of functions is %d\n", test->size);
	fprintf(stdout, "\tFail on number of functions is %d\n", fail_cnt);
	fprintf(stdout, "\tSucc on number of functions is %d\n\n", success_cnt);
	fprintf(stdout, "%s\n\n", markers);
	for (i = 0; i < test->size; i++) {
		if (test->ret[i])
			fprintf(stderr, "# %d :: %s\n", i, test->msg[i]);
	}
	fprintf(stdout, "\n");
}

#endif

/**@brief Explicit running unit test and show unit test report */
#define TEST_RUN()                           \
	{                                    \
		test_run(&g_test, __FILE__); \
	}
