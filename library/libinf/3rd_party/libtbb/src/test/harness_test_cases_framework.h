/*
    Copyright 2005-2016 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks. Threading Building Blocks is free software;
    you can redistribute it and/or modify it under the terms of the GNU General Public License
    version 2  as  published  by  the  Free Software Foundation.  Threading Building Blocks is
    distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See  the GNU General Public License for more details.   You should have received a copy of
    the  GNU General Public License along with Threading Building Blocks; if not, write to the
    Free Software Foundation, Inc.,  51 Franklin St,  Fifth Floor,  Boston,  MA 02110-1301 USA

    As a special exception,  you may use this file  as part of a free software library without
    restriction.  Specifically,  if other files instantiate templates  or use macros or inline
    functions from this file, or you compile this file and link it with other files to produce
    an executable,  this file does not by itself cause the resulting executable to be covered
    by the GNU General Public License. This exception does not however invalidate any other
    reasons why the executable file might be covered by the GNU General Public License.
*/

#ifndef tbb_harness_test_cases_framework_H
#define tbb_harness_test_cases_framework_H

#if defined(_MSC_VER)
#define _SCL_SECURE_NO_WARNINGS
#endif

#undef DO_ITT_NOTIFY

#include "harness.h"
//#include "harness_report.h"
#include "harness_assert.h"
//#include "harness_allocator.h"
#include "tbb/tbb_stddef.h"

#include <cstdlib>

#include <vector>
#include <algorithm>

#include <string>
#include <sstream>
#include <iostream>

namespace test_framework
{
template <typename test_class> void run_test()
{
	test_class()();
}

struct assertion_failure : std::exception {
	const char *my_filename;
	int my_line;
	const char *my_expression;
	const char *my_comment;
	assertion_failure(const char *filename, int line, const char *expression, const char *comment)
	        : my_filename(filename)
	        , my_line(line)
	        , my_expression(expression)
	        , my_comment(comment)
	{
	}
	virtual const char *what() const throw()
	{
		return "test assertion failed";
	}
};
void throw_assertion_failure()
{
	throw assertion_failure("", 0, "", "");
}
void throw_assertion_failure(const char *filename, int line, const char *expression, const char *comment)
{
	throw assertion_failure(filename, line, expression, comment);
}
class test_suite {
	typedef void (*run_test_function_pointer_type)();
	typedef std::pair<std::string, run_test_function_pointer_type> tc_record_pair;
	std::vector<tc_record_pair> test_cases;

    public:
	template <class test_class> void register_test_case(std::string const &name, test_class *)
	{
		test_cases.push_back(tc_record_pair(name, &run_test<test_class>));
	}
	std::string operator()(bool silent = false)
	{
		std::stringstream str;
		size_t failed = 0;
		for (size_t i = 0; i < test_cases.size(); ++i) {
			try {
				(test_cases[i].second)();
			} catch (std::exception &e) {
				failed++;
				str << "test case \"" << test_cases[i].first << "\" failed with exception. what():\""
				    << e.what() << "\"" << std::endl;
			}
		}
		if (!silent) {
			str << test_cases.size() << " test cases are run; " << failed << " failed" << std::endl;
		}
		return str.str();
	}
};
test_suite &get_suite_ref()
{
	static test_suite ts;
	return ts;
}
void run_all_and_print_results(test_suite &ts, std::ostream &o, bool silent = false)
{
	o << ts(silent);
}
}
using test_framework::get_suite_ref;
#define TEST_CASE_WITH_FIXTURE(TC_NAME, FIXTURE_NAME)                                                               \
	struct TC_NAME;                                                                                             \
	struct TC_NAME : FIXTURE_NAME {                                                                             \
		/* explicitly implemented default constructor  \
              is need here to please gcc 4.3.2*/                                                    \
		TC_NAME()                                                                                           \
		{                                                                                                   \
		}                                                                                                   \
		void operator()();                                                                                  \
	};                                                                                                          \
	bool TC_NAME##_registerd = (get_suite_ref().register_test_case(#TC_NAME, static_cast<TC_NAME *>(0)), true); \
	void TC_NAME::operator()()

namespace test_framework_unit_tests
{
namespace test_helper
{
template <size_t id> struct tag {
};
template <typename tag> struct test_case {
	static bool is_run;
	void operator()()
	{
		is_run = true;
	}
};
template <typename tag> bool test_case<tag>::is_run = false;

}
using namespace test_framework;
namespace test_test_suite_ref
{
void run_all_runs_all_registered_test_cases()
{
	test_suite s;
	using test_helper::tag;
	test_helper::test_case<tag<__LINE__> > tc1;
	test_helper::test_case<tag<__LINE__> > tc2;
	s.register_test_case("tc1", &tc1);
	s.register_test_case("tc2", &tc2);
	s();
	ASSERT(tc1.is_run && tc2.is_run, "test_suite::operator() should run all the tests");
}

struct silent_switch_fixture {
	test_helper::test_case<test_helper::tag<__LINE__> > empty_test_case;
};
struct run_all_and_print_results_should_respect_silent_mode : silent_switch_fixture {
	void operator()()
	{
		using test_helper::tag;
		test_helper::test_case<tag<__LINE__> > do_nothing_tc;
		test_suite ts;
		ts.register_test_case("tc_name", &do_nothing_tc);
		bool silent = true;
		ASSERT(ts(silent).empty(), "in silent mode no message except error should be output");
	}
};
struct run_all_and_print_results_should_respect_verbose_mode : silent_switch_fixture {
	void operator()()
	{
		using test_helper::tag;
		test_helper::test_case<tag<__LINE__> > do_nothing_tc;
		test_suite ts;
		ts.register_test_case("tc_name", &do_nothing_tc);
		bool silent = true;
		ASSERT(!ts(!silent).empty(), "in verbose mode all messages should be outputed");
	}
};
}
namespace test_test_case_macro
{
test_suite &get_suite_ref()
{
	static test_suite ts;
	return ts;
}
typedef test_helper::test_case<test_helper::tag<__LINE__> > unique_test_type;
TEST_CASE_WITH_FIXTURE(test_auto_registration, unique_test_type)
{
	unique_test_type::operator()();
}
void run_test_test_case_macro()
{
	get_suite_ref()();
	ASSERT(unique_test_type::is_run, "test case macro should register the test case in suite");
}
void test_test_case_macro_does_not_create_test_case_object()
{
	ASSERT(false, "to implement");
}
}
namespace internal_assertions_failure_test_cases
{
test_suite &get_suite_ref()
{
	static test_suite ts;
	return ts;
}

//TODO: investigate compilation errors regarding tbb::set_assertion_handler
//        struct empty_fixture{};
//        TEST_CASE_WITH_FIXTURE(test_internal_assertion_does_not_stop_test_suite,empty_fixture){
//            struct handler{
//                static void _( const char* /*filename*/, int /*line*/, const char* /*expression*/, const char * /*comment*/ ){
//                }
//            };
//
//            tbb::assertion_handler_type previous  = tbb::set_assertion_handler(handler::_);
//            __TBB_ASSERT(false,"this assert should not stop the test suite run");
//            tbb::set_assertion_handler(previous );
////            ASSERT(assertion_handler::is_called,"__TBB_ASSERT should call installed assertion handler");
//        }
//        TEST_CASE_WITH_FIXTURE(test_internal_assertion_does_mark_the_test_as_failed,empty_fixture){
//            test_suite ts;
//            struct _{
////                static
//                static void assertion_handler_type( const char* /*filename*/, int /*line*/, const char* /*expression*/, const char * /*comment*/ ){
//                }
//            };
//            tbb::assertion_handler_type previous  = tbb::set_assertion_handler(_::assertion_handler_type);
//            __TBB_ASSERT(false,"this assert should not stop the test suite run");
//            tbb::set_assertion_handler(previous );
//            std::string result = ts();
//            std::size_t test_case_name_begin_pos = result.find("test case \"");
//            std::size_t failed_begin_pos = result.find("failed");
//            ASSERT(test_case_name_begin_pos!=std::string::npos && failed_begin_pos!=std::string::npos && test_case_name_begin_pos<failed_begin_pos,"internal assertion should result in test failure");
//        }

}
void run_all_test()
{
	test_test_suite_ref::run_all_runs_all_registered_test_cases();
	test_test_suite_ref::run_all_and_print_results_should_respect_silent_mode()();
	test_test_suite_ref::run_all_and_print_results_should_respect_verbose_mode()();
	test_test_case_macro::run_test_test_case_macro();
	//TODO: uncomment and implement
	//        test_test_case_macro::test_test_case_macro_does_not_create_test_case_object();
	run_all_and_print_results(internal_assertions_failure_test_cases::get_suite_ref(), std::cout, !Verbose);
}
}

int TestMain()
{
	SetHarnessErrorProcessing(test_framework::throw_assertion_failure);
	//TODO: deal with assertions during stack unwinding
	//tbb::set_assertion_handler( test_framework::throw_assertion_failure );
	{
		test_framework_unit_tests::run_all_test();
	}
	bool silent = !Verbose;
	run_all_and_print_results(test_framework::get_suite_ref(), std::cout, silent);
	return Harness::Done;
}

#endif //tbb_harness_test_cases_framework_H
