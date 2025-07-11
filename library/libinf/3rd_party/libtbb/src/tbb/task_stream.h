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

#ifndef _TBB_task_stream_H
#define _TBB_task_stream_H

#include "tbb/tbb_stddef.h"
#include <deque>
#include <climits>
#include "tbb/atomic.h" // for __TBB_Atomic*
#include "tbb/spin_mutex.h"
#include "tbb/tbb_allocator.h"
#include "scheduler_common.h"
#include "tbb_misc.h" // for FastRandom

namespace tbb
{
namespace internal
{
//! Essentially, this is just a pair of a queue and a mutex to protect the queue.
/** The reason std::pair is not used is that the code would look less clean
    if field names were replaced with 'first' and 'second'. **/
template <typename T, typename mutex_t> struct queue_and_mutex {
	typedef std::deque<T, tbb_allocator<T> > queue_base_t;

	queue_base_t my_queue;
	mutex_t my_mutex;

	queue_and_mutex()
	        : my_queue()
	        , my_mutex()
	{
	}
	~queue_and_mutex()
	{
	}
};

typedef uintptr_t population_t;
const population_t one = 1;

inline void set_one_bit(population_t &dest, int pos)
{
	__TBB_ASSERT(pos >= 0, NULL);
	__TBB_ASSERT(pos < int(sizeof(population_t) * CHAR_BIT), NULL);
	__TBB_AtomicOR(&dest, one << pos);
}

inline void clear_one_bit(population_t &dest, int pos)
{
	__TBB_ASSERT(pos >= 0, NULL);
	__TBB_ASSERT(pos < int(sizeof(population_t) * CHAR_BIT), NULL);
	__TBB_AtomicAND(&dest, ~(one << pos));
}

inline bool is_bit_set(population_t val, int pos)
{
	__TBB_ASSERT(pos >= 0, NULL);
	__TBB_ASSERT(pos < int(sizeof(population_t) * CHAR_BIT), NULL);
	return (val & (one << pos)) != 0;
}

//! The container for "fairness-oriented" aka "enqueued" tasks.
template <int Levels> class task_stream : no_copy {
	typedef queue_and_mutex<task *, spin_mutex> lane_t;
	population_t population[Levels];
	padded<lane_t> *lanes[Levels];
	unsigned N;

    public:
	task_stream()
	        : N()
	{
		for (int level = 0; level < Levels; level++) {
			population[level] = 0;
			lanes[level] = NULL;
		}
	}

	void initialize(unsigned n_lanes)
	{
		const unsigned max_lanes = sizeof(population_t) * CHAR_BIT;

		N = n_lanes >= max_lanes ? max_lanes : n_lanes > 2 ? 1 << (__TBB_Log2(n_lanes - 1) + 1) : 2;
		__TBB_ASSERT(N == max_lanes || N >= n_lanes && ((N - 1) & N) == 0, "number of lanes miscalculated");
		__TBB_ASSERT(N <= sizeof(population_t) * CHAR_BIT, NULL);
		for (int level = 0; level < Levels; level++) {
			lanes[level] = new padded<lane_t>[N];
			__TBB_ASSERT(!population[level], NULL);
		}
	}

	~task_stream()
	{
		for (int level = 0; level < Levels; level++)
			if (lanes[level])
				delete[] lanes[level];
	}

	//! Push a task into a lane.
	void push(task *source, int level, FastRandom &random)
	{
		// Lane selection is random. Each thread should keep a separate seed value.
		unsigned idx;
		for (;;) {
			idx = random.get() & (N - 1);
			spin_mutex::scoped_lock lock;
			if (lock.try_acquire(lanes[level][idx].my_mutex)) {
				lanes[level][idx].my_queue.push_back(source);
				set_one_bit(population[level], idx); //TODO: avoid atomic op if the bit is already set
				break;
			}
		}
	}

	//! Try finding and popping a task.
	task *pop(int level, unsigned &last_used_lane)
	{
		task *result = NULL;
		// Lane selection is round-robin. Each thread should keep its last used lane.
		unsigned idx = (last_used_lane + 1) & (N - 1);
		for (; population[level]; idx = (idx + 1) & (N - 1)) {
			if (is_bit_set(population[level], idx)) {
				lane_t &lane = lanes[level][idx];
				spin_mutex::scoped_lock lock;
				if (lock.try_acquire(lane.my_mutex) && !lane.my_queue.empty()) {
					result = lane.my_queue.front();
					lane.my_queue.pop_front();
					if (lane.my_queue.empty())
						clear_one_bit(population[level], idx);
					break;
				}
			}
		}
		last_used_lane = idx;
		return result;
	}

	//! Checks existence of a task.
	bool empty(int level)
	{
		return !population[level];
	}

	//! Destroys all remaining tasks in every lane. Returns the number of destroyed tasks.
	/** Tasks are not executed, because it would potentially create more tasks at a late stage.
        The scheduler is really expected to execute all tasks before task_stream destruction. */
	intptr_t drain()
	{
		intptr_t result = 0;
		for (int level = 0; level < Levels; level++)
			for (unsigned i = 0; i < N; ++i) {
				lane_t &lane = lanes[level][i];
				spin_mutex::scoped_lock lock(lane.my_mutex);
				for (lane_t::queue_base_t::iterator it = lane.my_queue.begin();
				     it != lane.my_queue.end(); ++it, ++result) {
					__TBB_ASSERT(is_bit_set(population[level], i), NULL);
					task *t = *it;
					tbb::task::destroy(*t);
				}
				lane.my_queue.clear();
				clear_one_bit(population[level], i);
			}
		return result;
	}
}; // task_stream

} // namespace internal
} // namespace tbb

#endif /* _TBB_task_stream_H */
