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

#ifndef __TBB_parallel_while
#define __TBB_parallel_while

#include "task.h"
#include <new>

namespace tbb
{
template <typename Body> class parallel_while;

//! @cond INTERNAL
namespace internal
{
template <typename Stream, typename Body> class while_task;

//! For internal use only.
/** Executes one iteration of a while.
        @ingroup algorithms */
template <typename Body> class while_iteration_task : public task {
	const Body &my_body;
	typename Body::argument_type my_value;
	/*override*/ task *execute()
	{
		my_body(my_value);
		return NULL;
	}
	while_iteration_task(const typename Body::argument_type &value, const Body &body)
	        : my_body(body)
	        , my_value(value)
	{
	}
	template <typename Body_> friend class while_group_task;
	friend class tbb::parallel_while<Body>;
};

//! For internal use only
/** Unpacks a block of iterations.
        @ingroup algorithms */
template <typename Body> class while_group_task : public task {
	static const size_t max_arg_size = 4;
	const Body &my_body;
	size_t size;
	typename Body::argument_type my_arg[max_arg_size];
	while_group_task(const Body &body)
	        : my_body(body)
	        , size(0)
	{
	}
	/*override*/ task *execute()
	{
		typedef while_iteration_task<Body> iteration_type;
		__TBB_ASSERT(size > 0, NULL);
		task_list list;
		task *t;
		size_t k = 0;
		for (;;) {
			t = new (allocate_child()) iteration_type(my_arg[k], my_body);
			if (++k == size)
				break;
			list.push_back(*t);
		}
		set_ref_count(int(k + 1));
		spawn(list);
		spawn_and_wait_for_all(*t);
		return NULL;
	}
	template <typename Stream, typename Body_> friend class while_task;
};

//! For internal use only.
/** Gets block of iterations from a stream and packages them into a while_group_task.
        @ingroup algorithms */
template <typename Stream, typename Body> class while_task : public task {
	Stream &my_stream;
	const Body &my_body;
	empty_task &my_barrier;
	/*override*/ task *execute()
	{
		typedef while_group_task<Body> block_type;
		block_type &t = *new (allocate_additional_child_of(my_barrier)) block_type(my_body);
		size_t k = 0;
		while (my_stream.pop_if_present(t.my_arg[k])) {
			if (++k == block_type::max_arg_size) {
				// There might be more iterations.
				recycle_to_reexecute();
				break;
			}
		}
		if (k == 0) {
			destroy(t);
			return NULL;
		} else {
			t.size = k;
			return &t;
		}
	}
	while_task(Stream &stream, const Body &body, empty_task &barrier)
	        : my_stream(stream)
	        , my_body(body)
	        , my_barrier(barrier)
	{
	}
	friend class tbb::parallel_while<Body>;
};

} // namespace internal
//! @endcond

//! Parallel iteration over a stream, with optional addition of more work.
/** The Body b has the requirement: \n
        "b(v)"                      \n
        "b.argument_type"           \n
    where v is an argument_type
    @ingroup algorithms */
template <typename Body> class parallel_while : internal::no_copy {
    public:
	//! Construct empty non-running parallel while.
	parallel_while()
	        : my_body(NULL)
	        , my_barrier(NULL)
	{
	}

	//! Destructor cleans up data members before returning.
	~parallel_while()
	{
		if (my_barrier) {
			my_barrier->destroy(*my_barrier);
			my_barrier = NULL;
		}
	}

	//! Type of items
	typedef typename Body::argument_type value_type;

	//! Apply body.apply to each item in the stream.
	/** A Stream s has the requirements \n
         "S::value_type"                \n
         "s.pop_if_present(value) is convertible to bool */
	template <typename Stream> void run(Stream &stream, const Body &body);

	//! Add a work item while running.
	/** Should be executed only by body.apply or a thread spawned therefrom. */
	void add(const value_type &item);

    private:
	const Body *my_body;
	empty_task *my_barrier;
};

template <typename Body> template <typename Stream> void parallel_while<Body>::run(Stream &stream, const Body &body)
{
	using namespace internal;
	empty_task &barrier = *new (task::allocate_root()) empty_task();
	my_body = &body;
	my_barrier = &barrier;
	my_barrier->set_ref_count(2);
	while_task<Stream, Body> &w = *new (my_barrier->allocate_child())
	                                      while_task<Stream, Body>(stream, body, barrier);
	my_barrier->spawn_and_wait_for_all(w);
	my_barrier->destroy(*my_barrier);
	my_barrier = NULL;
	my_body = NULL;
}

template <typename Body> void parallel_while<Body>::add(const value_type &item)
{
	__TBB_ASSERT(my_barrier, "attempt to add to parallel_while that is not running");
	typedef internal::while_iteration_task<Body> iteration_type;
	iteration_type &i = *new (task::allocate_additional_child_of(*my_barrier)) iteration_type(item, *my_body);
	task::self().spawn(i);
}

} // namespace

#endif /* __TBB_parallel_while */
