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

/* Common part for the partitioner whitebox tests */

#include <typeinfo>

// enabling testing of preview features
#define TBB_PREVIEW_STATIC_PARTITIONER 1

#include "tbb/tbb_thread.h"
#include "tbb/enumerable_thread_specific.h"

#include "string.h"
#include "harness_assert.h"
#include "test_partitioner.h"

#if TBB_USE_DEBUG
// reducing number of simulations due to test timeout
const size_t max_simulated_threads = 256;
#else
const size_t max_simulated_threads = 640;
#endif

typedef tbb::enumerable_thread_specific<size_t> ThreadNumsType;
size_t g_threadNumInitialValue = 10;
ThreadNumsType g_threadNums(g_threadNumInitialValue);

// simulate a subset of task.h
namespace tbb
{
namespace internal
{
typedef unsigned short affinity_id;
}
class fake_task {
    public:
	typedef internal::affinity_id affinity_id;
	void set_affinity(affinity_id a)
	{
		my_affinity = a;
	}
	affinity_id affinity() const
	{
		return my_affinity;
	}
	void set_parent(fake_task *p)
	{
		my_parent = p;
	}
	fake_task *parent() const
	{
		return my_parent;
	}
	bool is_stolen_task() const
	{
		return false;
	}
	intptr_t ref_count() const
	{
		return 1;
	}
	bool is_cancelled() const
	{
		return false;
	}
	static void spawn(fake_task &)
	{
	} // for legacy in partitioner.h
	virtual fake_task *execute() = 0; // enables dynamic_cast

	fake_task()
	        : my_parent(0)
	        , my_affinity(0)
	{
	}
	virtual ~fake_task()
	{
	}

    private:
	fake_task *my_parent;
	affinity_id my_affinity;
};
}

#define __TBB_task_H
#define get_initial_auto_partitioner_divisor my_get_initial_auto_partitioner_divisor
#define affinity_partitioner_base_v3 my_affinity_partitioner_base_v3
#define task fake_task
#define __TBB_STATIC_THRESHOLD 0
#include "tbb/partitioner.h"
#undef __TBB_STATIC_THRESHOLD
#undef task
#undef affinity_partitioner_base_v3
#undef get_initial_auto_partitioner_divisor

// replace library functions to simulate concurrency
namespace tbb
{
namespace internal
{
size_t my_get_initial_auto_partitioner_divisor()
{
	const size_t X_FACTOR = 4;
	return X_FACTOR * g_threadNums.local();
}

void *__TBB_EXPORTED_FUNC NFS_Allocate(size_t n_element, size_t element_size, void *hint);
void __TBB_EXPORTED_FUNC NFS_Free(void *);

void my_affinity_partitioner_base_v3::resize(unsigned factor)
{
	// Check factor to avoid asking for number of workers while there might be no arena.
	size_t new_size = factor ? factor * g_threadNums.local() : 0;
	if (new_size != my_size) {
		if (my_array) {
			NFS_Free(my_array);
			// Following two assignments must be done here for sake of exception safety.
			my_array = NULL;
			my_size = 0;
		}
		if (new_size) {
			my_array = static_cast<affinity_id *>(NFS_Allocate(new_size, sizeof(affinity_id), NULL));
			memset(my_array, 0, sizeof(affinity_id) * new_size);
			my_size = new_size;
		}
	}
}

} //namespace internal
// simulate a subset of parallel_for
namespace interface9
{
namespace internal
{
// parallel_for algorithm that executes sequentially
template <typename Range, typename Body, typename Partitioner> class start_for : public fake_task {
	Range my_range;
	Body my_body;
	typename Partitioner::task_partition_type my_partition;
	size_t m_executedBegin, m_executedEnd;
	bool m_firstTimeRun;
	size_t m_joinedBegin, m_joinedEnd;
	test_partitioner_utils::BinaryTree *m_tree;

    public:
	start_for(const Range &range, const Body &body, Partitioner &partitioner,
	          test_partitioner_utils::BinaryTree *tree)
	        : my_range(range)
	        , my_body(body)
	        , my_partition(partitioner)
	        , m_executedBegin(0)
	        , m_executedEnd(0)
	        , m_firstTimeRun(true)
	        , m_joinedBegin(/* grows left */ range.end())
	        , m_joinedEnd(range.end())
	        , m_tree(tree)
	{
		if (m_tree) {
			m_tree->push_node(
			        test_partitioner_utils::make_node(my_range.begin(), my_range.end(), affinity()));
		}
	}
	//! Splitting constructor used to generate children.
	/** parent_ becomes left child.  Newly constructed object is right child. */
	start_for(start_for &parent_, typename Partitioner::split_type &split_obj)
	        : my_range(parent_.my_range, split_obj)
	        , my_body(parent_.my_body)
	        , my_partition(parent_.my_partition, split_obj)
	        , m_executedBegin(0)
	        , m_executedEnd(0)
	        , m_firstTimeRun(true)
	        , m_joinedBegin(/* grows left */ my_range.end())
	        , m_joinedEnd(my_range.end())
	        , m_tree(parent_.m_tree)
	{
		set_parent(parent_.parent());
		my_partition.set_affinity(*this);

		if (m_tree) {
			// collecting splitting statistics
			m_tree->push_node(
			        test_partitioner_utils::make_node(my_range.begin(), my_range.end(), affinity()));
			m_tree->push_node(test_partitioner_utils::make_node(
			        parent_.my_range.begin(), parent_.my_range.end(), parent_.affinity()));
		}
	}
	//! Construct right child from the given range as response to the demand.
	/** parent_ remains left child.  Newly constructed object is right child. */
	start_for(start_for &parent_, const Range &r, depth_t d)
	        : my_range(r)
	        , my_body(parent_.my_body)
	        , my_partition(parent_.my_partition, tbb::split())
	        , m_executedBegin(0)
	        , m_executedEnd(0)
	        , m_firstTimeRun(true)
	        , m_joinedBegin(/* grows left */ r.end())
	        , m_joinedEnd(r.end())
	        , m_tree(parent_.m_tree)
	{
		set_parent(parent_.parent());
		my_partition.set_affinity(*this);
		my_partition.align_depth(d);
	}
	fake_task *execute()
	{
		my_partition.check_being_stolen(*this);
		size_t origBegin = my_range.begin();
		size_t origEnd = my_range.end();

		my_partition.execute(*this, my_range);

		ASSERT(m_executedEnd == m_joinedBegin, "Non-continuous execution");
		m_executedEnd = m_joinedEnd;

		ASSERT(origBegin == m_executedBegin && origEnd == m_executedEnd, "Not all iterations were processed");
		return NULL;
	}
	//! Run body for range, serves as callback for partitioner
	void run_body(Range &r)
	{
		if (r.is_ensure_non_emptiness())
			ASSERT(!r.empty(), "Empty ranges are not allowed");
		my_body(r);
		if (m_firstTimeRun) {
			m_firstTimeRun = false;
			m_executedBegin = m_executedEnd = r.begin();
		}
		ASSERT(m_executedBegin <= r.begin() && m_executedEnd <= r.end(), "Non-continuous execution");
		m_executedEnd = r.end();
	}
	//! spawn right task, serves as callback for partitioner
	void offer_work(typename Partitioner::split_type &split_obj)
	{
		start_for sibling(*this, split_obj);
		sibling.execute();
		join(sibling.m_executedBegin, sibling.m_executedEnd);
	}
	//! spawn right task, serves as callback for partitioner
	void offer_work(const Range &r, depth_t d = 0)
	{
		start_for sibling(*this, r, d);
		sibling.execute();
		join(sibling.m_executedBegin, sibling.m_executedEnd);
	}
	void join(size_t siblingExecutedBegin, size_t siblingExecutedEnd)
	{
		ASSERT(siblingExecutedEnd == m_joinedBegin, "?");
		m_joinedBegin = siblingExecutedBegin;
	}
};

} //namespace internal
} //namespace interfaceX
} //namespace tbb

namespace whitebox_simulation
{
using namespace tbb::interface9::internal;
template <typename Range, typename Body, typename Partitioner>
void parallel_for(const Range &range, const Body &body, Partitioner &partitioner,
                  test_partitioner_utils::BinaryTree *tree = NULL)
{
	if (!range.empty()) {
		flag_task parent;
		start_for<Range, Body, Partitioner> start(range, body, partitioner, tree);
		start.set_parent(&parent);
		start.execute();
	}
}

} //namespace whitebox_simulation

template <typename Range, typename Body, typename Partitioner>
void test_case(Range &range, const Body &body, Partitioner &partitioner,
               test_partitioner_utils::BinaryTree *tree = NULL)
{
	whitebox_simulation::parallel_for(range, body, partitioner, tree);
}

// Functions generate size for range objects used in tests
template <typename T> size_t default_range_size_generator(T *factor, unsigned index, size_t thread_num)
{
	return size_t(factor[index] * thread_num);
}

size_t shifted_left_range_size_generator(size_t *factor, unsigned index, size_t thread_num)
{
	return factor[index] * thread_num - 1;
}

size_t shifted_right_range_size_generator(size_t *factor, unsigned index, size_t thread_num)
{
	return factor[index] * thread_num + 1;
}

size_t max_range_size_generator(size_t *, unsigned, size_t)
{
	return size_t(-1);
}

size_t simple_size_generator(size_t *, unsigned index, size_t)
{
	return index;
}

namespace uniform_iterations_distribution
{
/*
 * Test checks uniform distribution of range's iterations among all tasks just after
 * work distribution phase has been completed and just before work balancing phase has been started
 */

using namespace test_partitioner_utils;

class ParallelTestBody {
    public:
	struct use_case_settings_t;

	typedef void (*CheckerFuncType)(const char *, size_t, const use_case_settings_t *, const RangeStatisticData &);

	struct use_case_settings_t {
		size_t thread_num; // number of threads used during current use case
		unsigned factors_array_len; // size of 'factors' array
		size_t range_begin; // beginning of range iterations
		bool provide_feedback; // 'true' if range should give feedback
		bool ensure_non_empty_size; // don't allow empty size ranges

		size_t above_threads_size_tolerance; // allowed value for number of created ranges
		        // when initial size of the range was greater or
		        // equal to number of threads

		size_t below_threads_size_tolerance; // allowed value for number of created ranges
		        // when initial size of the range was less than
		        // number of threads

		size_t between_min_max_ranges_tolerance; // allowed value for difference of iterations
		        // between bigger and lesser ranges

		CheckerFuncType checker; // checker function for a particular test case
	};

	ParallelTestBody(size_t parallel_group_thread_starting_index)
	        : m_parallel_group_thread_starting_index(parallel_group_thread_starting_index)
	{
	}

	void operator()(size_t) const
	{
		ASSERT(false, "Empty ParallelTestBody called");
	}

	static void uniform_distribution_checker(const char *rangeName, size_t rangeSize,
	                                         const use_case_settings_t *settings, const RangeStatisticData &stat)
	{
		// Checking that all threads were given a task
		if (rangeSize >= settings->thread_num) {
			uint64_t disparity =
			        max(stat.m_rangeNum, settings->thread_num) - min(stat.m_rangeNum, settings->thread_num);
			if (disparity > settings->above_threads_size_tolerance) {
				REPORT("ERROR: '%s (f=%d|e=%d)': |#ranges(%llu)-#threads(%llu)|=%llu > %llu=tolerance\n",
				       rangeName, int(settings->provide_feedback), int(settings->ensure_non_empty_size),
				       stat.m_rangeNum, settings->thread_num, disparity,
				       uint64_t(settings->above_threads_size_tolerance));
				ASSERT(disparity <= settings->above_threads_size_tolerance,
				       "Incorrect number of range "
				       "objects was created before work balancing phase started");
			}
		} else if (settings->ensure_non_empty_size && rangeSize != 0) {
			uint64_t disparity = max(stat.m_rangeNum, rangeSize) - min(stat.m_rangeNum, rangeSize);
			if (disparity > settings->below_threads_size_tolerance) {
				REPORT("ERROR: '%s (f=%d|e=%d)': |#ranges-range size|=%llu > %llu=tolerance\n",
				       rangeName, int(settings->provide_feedback), int(settings->ensure_non_empty_size),
				       disparity, uint64_t(settings->below_threads_size_tolerance));
				ASSERT(disparity <= settings->below_threads_size_tolerance,
				       "Incorrect number of range objects"
				       " was created before work balancing phase started");
			}
		}
		// Checking difference between min and max number of range iterations
		size_t diff = stat.m_maxRangeSize - stat.m_minRangeSize;
		if (diff > settings->between_min_max_ranges_tolerance) {
			REPORT("ERROR: '%s (f=%d|e=%d)': range size difference=%llu > %llu=tolerance\n", rangeName,
			       int(settings->provide_feedback), int(settings->ensure_non_empty_size), uint64_t(diff),
			       uint64_t(settings->between_min_max_ranges_tolerance));
			ASSERT(diff <= settings->between_min_max_ranges_tolerance,
			       "Uniform iteration distribution error");
		}
	}
	// Checker for test cases where ranges don't provide feedback during proportional split to
	// partitioner and differ from tbb::blocked_range implementation in their splitting algorithm
	static void nonuniform_distribution_checker(const char *rangeName, size_t rangeSize,
	                                            const use_case_settings_t *settings, const RangeStatisticData &stat)
	{
		if (stat.m_rangeNum > settings->thread_num) {
			REPORT("ERROR: '%s (f=%d|e=%d)': %llu=#ranges > #threads=%llu\n", rangeName,
			       int(settings->provide_feedback), int(settings->ensure_non_empty_size),
			       uint64_t(stat.m_rangeNum), uint64_t(settings->thread_num));
			ASSERT(stat.m_rangeNum <= settings->thread_num,
			       "Incorrect number of range objects was created before work balancing phase started");
		}
		// Checking difference between min and max number of range iterations
		size_t diff = stat.m_maxRangeSize - stat.m_minRangeSize;
		if (diff > rangeSize) {
			REPORT("ERROR: '%s (f=%d|e=%d)': range size difference=%llu > %llu=initial range size\n",
			       rangeName, int(settings->provide_feedback), int(settings->ensure_non_empty_size),
			       uint64_t(diff), uint64_t(rangeSize));
			ASSERT(diff <= rangeSize, "Iteration distribution error");
		}
	}

    protected:
	size_t m_parallel_group_thread_starting_index; // starting index of thread

	template <typename Range, typename Partitioner, typename T>
	void test(use_case_settings_t &settings, T factors[],
	          size_t (*rsgFunc)(T *, unsigned, size_t) = &default_range_size_generator<T>) const
	{
		for (unsigned i = 0; i < settings.factors_array_len; ++i) {
			size_t range_end = rsgFunc(factors, i, settings.thread_num);
			RangeStatisticData stat = { /*range num=*/0, /*minimal size of range=*/0,
				                    /*maximal size of range=*/0,
				                    /*minimal size of range was not rewritten yet=*/false };
			Range range = Range(settings.range_begin, range_end, &stat, settings.provide_feedback,
			                    settings.ensure_non_empty_size);
			Partitioner my_partitioner;
			test_case(range, SimpleBody(), my_partitioner, NULL);
			size_t range_size = range_end - settings.range_begin;
			const char *rangeName = typeid(range).name();
			settings.checker(rangeName, range_size, &settings, stat);
		}
	}
};

template <typename ParallelTestBody> void test()
{
	size_t hw_threads_num = tbb::tbb_thread::hardware_concurrency();
	size_t threadsToRunOn = std::min<size_t>(max_simulated_threads, hw_threads_num);

	size_t parallel_group_thread_starting_index = 1;
	while (parallel_group_thread_starting_index <= max_simulated_threads - threadsToRunOn) {
		NativeParallelFor(threadsToRunOn, ParallelTestBody(parallel_group_thread_starting_index));
		parallel_group_thread_starting_index += threadsToRunOn;
	}
	NativeParallelFor(max_simulated_threads - parallel_group_thread_starting_index,
	                  ParallelTestBody(parallel_group_thread_starting_index));
}

} /* namespace uniform_iterations_distribution */
