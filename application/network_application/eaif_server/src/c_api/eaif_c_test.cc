#include "eaif_test.h"

#include "eaif_c_test.h"

TestSuit *TestSuit_create(void)
{
	return new TestSuit;
}

void TestSuit_free(TestSuit **suit)
{
	delete *suit;
	suit = nullptr;
}

void TestSuit_strVecAdd(TestSuit *suit, char *msg)
{
	suit->vec_str.push_back(std::string(msg));
}

void TestSuit_add(TestSuit *suit, TestFun funptr)
{
	suit->vec_func.push_back(funptr);
}

void TestSuit_run(TestSuit *suit)
{
	suit->run();
}

void TestSuit_report(TestSuit *suit)
{
	suit->report();
}
