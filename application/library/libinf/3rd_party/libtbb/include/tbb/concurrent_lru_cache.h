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

#ifndef __TBB_concurrent_lru_cache_H
#define __TBB_concurrent_lru_cache_H

#if !TBB_PREVIEW_CONCURRENT_LRU_CACHE
#error Set TBB_PREVIEW_CONCURRENT_LRU_CACHE to include concurrent_lru_cache.h
#endif

#include <map>
#include <list>

#include "tbb_stddef.h"
#include "atomic.h"
#include "internal/_aggregator_impl.h"

namespace tbb
{
namespace interface6
{
template <typename key_type, typename value_type, typename value_functor_type = value_type (*)(key_type)>
class concurrent_lru_cache : internal::no_assign {
    private:
	typedef concurrent_lru_cache self_type;
	typedef value_functor_type value_function_type;
	typedef std::size_t ref_counter_type;
	struct map_value_type;
	typedef std::map<key_type, map_value_type> map_storage_type;
	typedef std::list<typename map_storage_type::iterator> lru_list_type;
	struct map_value_type {
		value_type my_value;
		ref_counter_type my_ref_counter;
		typename lru_list_type::iterator my_lru_list_iterator;
		bool my_is_ready;

		map_value_type(value_type const &a_value, ref_counter_type a_ref_counter,
		               typename lru_list_type::iterator a_lru_list_iterator, bool a_is_ready)
		        : my_value(a_value)
		        , my_ref_counter(a_ref_counter)
		        , my_lru_list_iterator(a_lru_list_iterator)
		        , my_is_ready(a_is_ready)
		{
		}
	};

	class handle_object;

	struct aggregator_operation;
	typedef aggregator_operation aggregated_operation_type;
	typedef tbb::internal::aggregating_functor<self_type, aggregated_operation_type> aggregator_function_type;
	friend class tbb::internal::aggregating_functor<self_type, aggregated_operation_type>;
	typedef tbb::internal::aggregator<aggregator_function_type, aggregated_operation_type> aggregator_type;

    private:
	value_function_type my_value_function;
	std::size_t const my_number_of_lru_history_items;
	map_storage_type my_map_storage;
	lru_list_type my_lru_list;
	aggregator_type my_aggregator;

    public:
	typedef handle_object handle;

    public:
	concurrent_lru_cache(value_function_type f, std::size_t number_of_lru_history_items)
	        : my_value_function(f)
	        , my_number_of_lru_history_items(number_of_lru_history_items)
	{
		my_aggregator.initialize_handler(aggregator_function_type(this));
	}

	handle_object operator[](key_type k)
	{
		retrieve_aggregator_operation op(k);
		my_aggregator.execute(&op);
		if (op.is_new_value_needed()) {
			op.result().second.my_value = my_value_function(k);
			__TBB_store_with_release(op.result().second.my_is_ready, true);
		} else {
			tbb::internal::spin_wait_while_eq(op.result().second.my_is_ready, false);
		}
		return handle_object(*this, op.result());
	}

    private:
	void signal_end_of_usage(typename map_storage_type::reference value_ref)
	{
		signal_end_of_usage_aggregator_operation op(value_ref);
		my_aggregator.execute(&op);
	}

    private:
	struct handle_move_t : no_assign {
		concurrent_lru_cache &my_cache_ref;
		typename map_storage_type::reference my_map_record_ref;
		handle_move_t(concurrent_lru_cache &cache_ref, typename map_storage_type::reference value_ref)
		        : my_cache_ref(cache_ref)
		        , my_map_record_ref(value_ref){};
	};
	class handle_object {
		concurrent_lru_cache *my_cache_pointer;
		typename map_storage_type::reference my_map_record_ref;

	    public:
		handle_object(concurrent_lru_cache &cache_ref, typename map_storage_type::reference value_ref)
		        : my_cache_pointer(&cache_ref)
		        , my_map_record_ref(value_ref)
		{
		}
		handle_object(handle_move_t m)
		        : my_cache_pointer(&m.my_cache_ref)
		        , my_map_record_ref(m.my_map_record_ref)
		{
		}
		operator handle_move_t()
		{
			return move(*this);
		}
		value_type &value()
		{
			__TBB_ASSERT(my_cache_pointer, "get value from moved from object?");
			return my_map_record_ref.second.my_value;
		}
		~handle_object()
		{
			if (my_cache_pointer) {
				my_cache_pointer->signal_end_of_usage(my_map_record_ref);
			}
		}

	    private:
		friend handle_move_t move(handle_object &h)
		{
			return handle_object::move(h);
		}
		static handle_move_t move(handle_object &h)
		{
			__TBB_ASSERT(h.my_cache_pointer, "move from the same object twice ?");
			concurrent_lru_cache *cache_pointer = h.my_cache_pointer;
			h.my_cache_pointer = NULL;
			return handle_move_t(*cache_pointer, h.my_map_record_ref);
		}

	    private:
		void operator=(handle_object &);
#if __SUNPRO_CC
		// Presumably due to a compiler error, private copy constructor
		// breaks expressions like handle h = cache[key];
	    public:
#endif
		handle_object(handle_object &);
	};

    private:
	//TODO: looks like aggregator_operation is a perfect match for statically typed variant type
	struct aggregator_operation : tbb::internal::aggregated_operation<aggregator_operation> {
		enum e_op_type { op_retive, op_signal_end_of_usage };
		//TODO: try to use pointer to function apply_visitor here
		//TODO: try virtual functions and measure the difference
		e_op_type my_operation_type;
		aggregator_operation(e_op_type operation_type)
		        : my_operation_type(operation_type)
		{
		}
		void cast_and_handle(self_type &container)
		{
			if (my_operation_type == op_retive) {
				static_cast<retrieve_aggregator_operation *>(this)->handle(container);
			} else {
				static_cast<signal_end_of_usage_aggregator_operation *>(this)->handle(container);
			}
		}
	};
	struct retrieve_aggregator_operation : aggregator_operation, private internal::no_assign {
		key_type my_key;
		typename map_storage_type::pointer my_result_map_record_pointer;
		bool my_is_new_value_needed;
		retrieve_aggregator_operation(key_type key)
		        : aggregator_operation(aggregator_operation::op_retive)
		        , my_key(key)
		        , my_is_new_value_needed(false)
		{
		}
		void handle(self_type &container)
		{
			my_result_map_record_pointer = &container.retrieve_serial(my_key, my_is_new_value_needed);
		}
		typename map_storage_type::reference result()
		{
			return *my_result_map_record_pointer;
		}
		bool is_new_value_needed()
		{
			return my_is_new_value_needed;
		}
	};
	struct signal_end_of_usage_aggregator_operation : aggregator_operation, private internal::no_assign {
		typename map_storage_type::reference my_map_record_ref;
		signal_end_of_usage_aggregator_operation(typename map_storage_type::reference map_record_ref)
		        : aggregator_operation(aggregator_operation::op_signal_end_of_usage)
		        , my_map_record_ref(map_record_ref)
		{
		}
		void handle(self_type &container)
		{
			container.signal_end_of_usage_serial(my_map_record_ref);
		}
	};

    private:
	void handle_operations(aggregator_operation *op_list)
	{
		while (op_list) {
			op_list->cast_and_handle(*this);
			aggregator_operation *tmp = op_list;
			op_list = op_list->next;
			tbb::internal::itt_store_word_with_release(tmp->status, uintptr_t(1));
		}
	}

    private:
	typename map_storage_type::reference retrieve_serial(key_type k, bool &is_new_value_needed)
	{
		typename map_storage_type::iterator it = my_map_storage.find(k);
		if (it == my_map_storage.end()) {
			it = my_map_storage.insert(
			        it, std::make_pair(k, map_value_type(value_type(), 0, my_lru_list.end(), false)));
			is_new_value_needed = true;
		} else {
			typename lru_list_type::iterator list_it = it->second.my_lru_list_iterator;
			if (list_it != my_lru_list.end()) {
				__TBB_ASSERT(!it->second.my_ref_counter,
				             "item to be evicted should not have a live references");
				//item is going to be used. Therefore it is not a subject for eviction
				//so - remove it from LRU history.
				my_lru_list.erase(list_it);
				it->second.my_lru_list_iterator = my_lru_list.end();
			}
		}
		++(it->second.my_ref_counter);
		return *it;
	}

	void signal_end_of_usage_serial(typename map_storage_type::reference map_record_ref)
	{
		typename map_storage_type::iterator it = my_map_storage.find(map_record_ref.first);
		__TBB_ASSERT(it != my_map_storage.end(), "cache should not return past-end iterators to outer world");
		__TBB_ASSERT(&(*it) == &map_record_ref,
		             "dangling reference has been returned to outside world? data race ?");
		__TBB_ASSERT(my_lru_list.end() == std::find(my_lru_list.begin(), my_lru_list.end(), it),
		             "object in use should not be in list of unused objects ");
		if (!--(it->second.my_ref_counter)) {
			//it was the last reference so put it to the LRU history
			if (my_lru_list.size() >= my_number_of_lru_history_items) {
				//evict items in order to get a space
				size_t number_of_elements_to_evict =
				        1 + my_lru_list.size() - my_number_of_lru_history_items;
				for (size_t i = 0; i < number_of_elements_to_evict; ++i) {
					typename map_storage_type::iterator it_to_evict = my_lru_list.back();
					__TBB_ASSERT(!it_to_evict->second.my_ref_counter,
					             "item to be evicted should not have a live references");
					my_lru_list.pop_back();
					my_map_storage.erase(it_to_evict);
				}
			}
			my_lru_list.push_front(it);
			it->second.my_lru_list_iterator = my_lru_list.begin();
		}
	}
};
} // namespace interface6

using interface6::concurrent_lru_cache;

} // namespace tbb
#endif //__TBB_concurrent_lru_cache_H
