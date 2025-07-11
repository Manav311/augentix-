#ifndef EAIF_TEST_H_
#define EAIF_TEST_H_

#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>

const float EPSILON = 1.0e-04;
const double EPSILOND = 1.0e-04;

struct TestSuit;

using TestFun = int (*)(TestSuit *suit);

struct TestSuit {
	std::vector<TestFun> vec_func;
	std::vector<std::string> vec_str;
	std::vector<int> vec_result;
	std::string marker;
	int fail_cnt = 0;
	int success_cnt = 0;

	void run(void)
	{
		for (auto &fun : vec_func) {
			if (fun(this) == 0) {
				marker += ".";
				success_cnt += 1;
				vec_result.push_back(1);
			} else {
				marker += "x";
				fail_cnt += 1;
				vec_result.push_back(0);
			}
		}
	}

	void report(void)
	{
		std::cout << "\n  ======= Testing Report : " << __FILE__ << " ======\n\n";
		std::cout << "\tTest on number of functions is " << vec_func.size() << ".\n";
		std::cout << "\tFail on number of functions is " << fail_cnt << ".\n";
		std::cout << "\tSucc on number of functions is " << success_cnt << ".\n\n";
		std::cout << marker << "\n\n";

		int j = 0;
		for (size_t i = 0; i < vec_func.size(); i++)
			if (!vec_result[i])
				std::cout << "#" << std::to_string(i) << " " << vec_str[j++] << "\n";
		std::cout << "\n";
	}
};

#define str_cond(cond) \
	("[FAIL] " + std::string(__func__) + ":" + std::to_string(__LINE__) + " fail cond: (" + #cond + ")")

#define testAssert(cond)                                         \
	{                                                        \
		if (!(cond)) {                                   \
			suit->vec_str.push_back(str_cond(cond)); \
			return -1;                               \
		} else {                                         \
		}                                                \
	}

#define testLog(fmt, args...) printf("[LOG] [%s:%d] " fmt, __func__, __LINE__, ##args)

template <typename T, typename F> bool fsame(T a, F b)
{
	return fabs(a - (F)b) < EPSILOND;
}

#endif /* EAIF_TEST_H_ */
