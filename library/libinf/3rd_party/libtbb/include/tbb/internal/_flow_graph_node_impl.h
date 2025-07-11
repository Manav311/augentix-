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

#ifndef __TBB__flow_graph_node_impl_H
#define __TBB__flow_graph_node_impl_H

#ifndef __TBB_flow_graph_H
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#include "_flow_graph_item_buffer_impl.h"

//! @cond INTERNAL
namespace internal
{
using tbb::internal::aggregated_operation;
using tbb::internal::aggregating_functor;
using tbb::internal::aggregator;

template <typename T, typename A> class function_input_queue : public item_buffer<T, A> {
    public:
	bool pop(T &t)
	{
		return this->pop_front(t);
	}

	bool push(T &t)
	{
		return this->push_back(t);
	}
};

//! Input and scheduling for a function node that takes a type Input as input
//  The only up-ref is apply_body_impl, which should implement the function
//  call and any handling of the result.
template <typename Input, typename A, typename ImplType>
class function_input_base : public receiver<Input>, tbb::internal::no_assign {
	enum op_stat { WAIT = 0, SUCCEEDED, FAILED };
	enum op_type {
		reg_pred,
		rem_pred,
		app_body,
		try_fwd,
		tryput_bypass,
		app_body_bypass
#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
		,
		add_blt_pred,
		del_blt_pred,
		blt_pred_cnt,
		blt_pred_cpy // create vector copies of preds and succs
#endif
	};
	typedef function_input_base<Input, A, ImplType> class_type;

    public:
	//! The input type of this receiver
	typedef Input input_type;
	typedef typename receiver<input_type>::predecessor_type predecessor_type;
	typedef predecessor_cache<input_type, null_mutex> predecessor_cache_type;
	typedef function_input_queue<input_type, A> input_queue_type;
	typedef typename A::template rebind<input_queue_type>::other queue_allocator_type;

#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
	typedef typename predecessor_cache_type::built_predecessors_type built_predecessors_type;
	typedef typename receiver<input_type>::predecessor_list_type predecessor_list_type;
#endif

	//! Constructor for function_input_base
	function_input_base(graph &g, size_t max_concurrency, input_queue_type *q = NULL)
	        : my_graph(g)
	        , my_max_concurrency(max_concurrency)
	        , my_concurrency(0)
	        , my_queue(q)
	        , forwarder_busy(false)
	{
		my_predecessors.set_owner(this);
		my_aggregator.initialize_handler(handler_type(this));
	}

	//! Copy constructor
	function_input_base(const function_input_base &src, input_queue_type *q = NULL)
	        : receiver<Input>()
	        , tbb::internal::no_assign()
	        , my_graph(src.my_graph)
	        , my_max_concurrency(src.my_max_concurrency)
	        , my_concurrency(0)
	        , my_queue(q)
	        , forwarder_busy(false)
	{
		my_predecessors.set_owner(this);
		my_aggregator.initialize_handler(handler_type(this));
	}

	//! Destructor
	// The queue is allocated by the constructor for {multi}function_node.
	// TODO: pass the graph_buffer_policy to the base so it can allocate the queue instead.
	// This would be an interface-breaking change.
	virtual ~function_input_base()
	{
		if (my_queue)
			delete my_queue;
	}

	//! Put to the node, returning a task if available
	virtual task *try_put_task(const input_type &t)
	{
		if (my_max_concurrency == 0) {
			return create_body_task(t);
		} else {
			operation_type op_data(t, tryput_bypass);
			my_aggregator.execute(&op_data);
			if (op_data.status == SUCCEEDED) {
				return op_data.bypass_t;
			}
			return NULL;
		}
	}

	//! Adds src to the list of cached predecessors.
	/* override */ bool register_predecessor(predecessor_type &src)
	{
		operation_type op_data(reg_pred);
		op_data.r = &src;
		my_aggregator.execute(&op_data);
		return true;
	}

	//! Removes src from the list of cached predecessors.
	/* override */ bool remove_predecessor(predecessor_type &src)
	{
		operation_type op_data(rem_pred);
		op_data.r = &src;
		my_aggregator.execute(&op_data);
		return true;
	}

#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
	//! Adds to list of predecessors added by make_edge
	/*override*/ void internal_add_built_predecessor(predecessor_type &src)
	{
		operation_type op_data(add_blt_pred);
		op_data.r = &src;
		my_aggregator.execute(&op_data);
	}

	//! removes from to list of predecessors (used by remove_edge)
	/*override*/ void internal_delete_built_predecessor(predecessor_type &src)
	{
		operation_type op_data(del_blt_pred);
		op_data.r = &src;
		my_aggregator.execute(&op_data);
	}

	/*override*/ size_t predecessor_count()
	{
		operation_type op_data(blt_pred_cnt);
		my_aggregator.execute(&op_data);
		return op_data.cnt_val;
	}

	/*override*/ void copy_predecessors(predecessor_list_type &v)
	{
		operation_type op_data(blt_pred_cpy);
		op_data.predv = &v;
		my_aggregator.execute(&op_data);
	}

	/*override*/ built_predecessors_type &built_predecessors()
	{
		return my_predecessors.built_predecessors();
	}
#endif /* TBB_PREVIEW_FLOW_GRAPH_FEATURES */

    protected:
	void reset_function_input_base(reset_flags f)
	{
		my_concurrency = 0;
		if (my_queue) {
			my_queue->reset();
		}
		reset_receiver(f);
		forwarder_busy = false;
	}

	graph &my_graph;
	const size_t my_max_concurrency;
	size_t my_concurrency;
	input_queue_type *my_queue;
	predecessor_cache<input_type, null_mutex> my_predecessors;

	/*override*/ void reset_receiver(reset_flags f)
	{
		if (f & rf_clear_edges)
			my_predecessors.clear();
		else
			my_predecessors.reset();
		__TBB_ASSERT(!(f & rf_clear_edges) || my_predecessors.empty(), "function_input_base reset failed");
	}

    private:
	friend class apply_body_task_bypass<class_type, input_type>;
	friend class forward_task_bypass<class_type>;

	class operation_type : public aggregated_operation<operation_type> {
	    public:
		char type;
		union {
			input_type *elem;
			predecessor_type *r;
#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
			size_t cnt_val;
			predecessor_list_type *predv;
#endif /* TBB_PREVIEW_FLOW_GRAPH_FEATURES */
		};
		tbb::task *bypass_t;
		operation_type(const input_type &e, op_type t)
		        : type(char(t))
		        , elem(const_cast<input_type *>(&e))
		{
		}
		operation_type(op_type t)
		        : type(char(t))
		        , r(NULL)
		{
		}
	};

	bool forwarder_busy;
	typedef internal::aggregating_functor<class_type, operation_type> handler_type;
	friend class internal::aggregating_functor<class_type, operation_type>;
	aggregator<handler_type, operation_type> my_aggregator;

	void handle_operations(operation_type *op_list)
	{
		operation_type *tmp;
		while (op_list) {
			tmp = op_list;
			op_list = op_list->next;
			switch (tmp->type) {
			case reg_pred:
				my_predecessors.add(*(tmp->r));
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				if (!forwarder_busy) {
					forwarder_busy = true;
					spawn_forward_task();
				}
				break;
			case rem_pred:
				my_predecessors.remove(*(tmp->r));
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				break;
			case app_body:
				__TBB_ASSERT(my_max_concurrency != 0, NULL);
				--my_concurrency;
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				if (my_concurrency < my_max_concurrency) {
					input_type i;
					bool item_was_retrieved = false;
					if (my_queue)
						item_was_retrieved = my_queue->pop(i);
					else
						item_was_retrieved = my_predecessors.get_item(i);
					if (item_was_retrieved) {
						++my_concurrency;
						spawn_body_task(i);
					}
				}
				break;
			case app_body_bypass: {
				task *new_task = NULL;
				__TBB_ASSERT(my_max_concurrency != 0, NULL);
				--my_concurrency;
				if (my_concurrency < my_max_concurrency) {
					input_type i;
					bool item_was_retrieved = false;
					if (my_queue)
						item_was_retrieved = my_queue->pop(i);
					else
						item_was_retrieved = my_predecessors.get_item(i);
					if (item_was_retrieved) {
						++my_concurrency;
						new_task = create_body_task(i);
					}
				}
				tmp->bypass_t = new_task;
				__TBB_store_with_release(tmp->status, SUCCEEDED);
			} break;
			case tryput_bypass:
				internal_try_put_task(tmp);
				break;
			case try_fwd:
				internal_forward(tmp);
				break;
#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
			case add_blt_pred: {
				my_predecessors.internal_add_built_predecessor(*(tmp->r));
				__TBB_store_with_release(tmp->status, SUCCEEDED);
			} break;
			case del_blt_pred:
				my_predecessors.internal_delete_built_predecessor(*(tmp->r));
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				break;
			case blt_pred_cnt:
				tmp->cnt_val = my_predecessors.predecessor_count();
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				break;
			case blt_pred_cpy:
				my_predecessors.copy_predecessors(*(tmp->predv));
				__TBB_store_with_release(tmp->status, SUCCEEDED);
				break;
#endif /* TBB_PREVIEW_FLOW_GRAPH_FEATURES */
			}
		}
	}

	//! Put to the node, but return the task instead of enqueueing it
	void internal_try_put_task(operation_type *op)
	{
		__TBB_ASSERT(my_max_concurrency != 0, NULL);
		if (my_concurrency < my_max_concurrency) {
			++my_concurrency;
			task *new_task = create_body_task(*(op->elem));
			op->bypass_t = new_task;
			__TBB_store_with_release(op->status, SUCCEEDED);
		} else if (my_queue && my_queue->push(*(op->elem))) {
			op->bypass_t = SUCCESSFULLY_ENQUEUED;
			__TBB_store_with_release(op->status, SUCCEEDED);
		} else {
			op->bypass_t = NULL;
			__TBB_store_with_release(op->status, FAILED);
		}
	}

	//! Tries to spawn bodies if available and if concurrency allows
	void internal_forward(operation_type *op)
	{
		op->bypass_t = NULL;
		if (my_concurrency < my_max_concurrency || !my_max_concurrency) {
			input_type i;
			bool item_was_retrieved = false;
			if (my_queue)
				item_was_retrieved = my_queue->pop(i);
			else
				item_was_retrieved = my_predecessors.get_item(i);
			if (item_was_retrieved) {
				++my_concurrency;
				op->bypass_t = create_body_task(i);
				__TBB_store_with_release(op->status, SUCCEEDED);
				return;
			}
		}
		__TBB_store_with_release(op->status, FAILED);
		forwarder_busy = false;
	}

	//! Applies the body to the provided input
	//  then decides if more work is available
	task *apply_body_bypass(input_type &i)
	{
		task *new_task = static_cast<ImplType *>(this)->apply_body_impl_bypass(i);
		if (my_max_concurrency != 0) {
			operation_type op_data(
			        app_body_bypass); // tries to pop an item or get_item, enqueues another apply_body
			my_aggregator.execute(&op_data);
			tbb::task *ttask = op_data.bypass_t;
			new_task = combine_tasks(new_task, ttask);
		}
		return new_task;
	}

	//! allocates a task to apply a body
	inline task *create_body_task(const input_type &input)
	{
		return (my_graph.is_active()) ? new (task::allocate_additional_child_of(*(my_graph.root_task())))
		                                        apply_body_task_bypass<class_type, input_type>(*this, input) :
		                                NULL;
	}

	//! Spawns a task that applies a body
	inline void spawn_body_task(const input_type &input)
	{
		task *tp = create_body_task(input);
		// tp == NULL => g.reset(), which shouldn't occur in concurrent context
		if (tp) {
			FLOW_SPAWN(*tp);
		}
	}

	//! This is executed by an enqueued task, the "forwarder"
	task *forward_task()
	{
		operation_type op_data(try_fwd);
		task *rval = NULL;
		do {
			op_data.status = WAIT;
			my_aggregator.execute(&op_data);
			if (op_data.status == SUCCEEDED) {
				tbb::task *ttask = op_data.bypass_t;
				rval = combine_tasks(rval, ttask);
			}
		} while (op_data.status == SUCCEEDED);
		return rval;
	}

	inline task *create_forward_task()
	{
		return (my_graph.is_active()) ? new (task::allocate_additional_child_of(*(my_graph.root_task())))
		                                        forward_task_bypass<class_type>(*this) :
		                                NULL;
	}

	//! Spawns a task that calls forward()
	inline void spawn_forward_task()
	{
		task *tp = create_forward_task();
		if (tp) {
			FLOW_SPAWN(*tp);
		}
	}
}; // function_input_base

//! Implements methods for a function node that takes a type Input as input and sends
//  a type Output to its successors.
template <typename Input, typename Output, typename A>
class function_input : public function_input_base<Input, A, function_input<Input, Output, A> > {
    public:
	typedef Input input_type;
	typedef Output output_type;
	typedef function_body<input_type, output_type> function_body_type;
	typedef function_input<Input, Output, A> my_class;
	typedef function_input_base<Input, A, my_class> base_type;
	typedef function_input_queue<input_type, A> input_queue_type;

	// constructor
	template <typename Body>
	function_input(graph &g, size_t max_concurrency, Body &body, input_queue_type *q = NULL)
	        : base_type(g, max_concurrency, q)
	        , my_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	        , my_init_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	{
	}

	//! Copy constructor
	function_input(const function_input &src, input_queue_type *q = NULL)
	        : base_type(src, q)
	        , my_body(src.my_init_body->clone())
	        , my_init_body(src.my_init_body->clone())
	{
	}

	~function_input()
	{
		delete my_body;
		delete my_init_body;
	}

	template <typename Body> Body copy_function_object()
	{
		function_body_type &body_ref = *this->my_body;
		return dynamic_cast<internal::function_body_leaf<input_type, output_type, Body> &>(body_ref).get_body();
	}

	task *apply_body_impl_bypass(const input_type &i)
	{
#if TBB_PREVIEW_FLOW_GRAPH_TRACE
		// There is an extra copied needed to capture the
		// body execution without the try_put
		tbb::internal::fgt_begin_body(my_body);
		output_type v = (*my_body)(i);
		tbb::internal::fgt_end_body(my_body);
		task *new_task = successors().try_put_task(v);
#else
		task *new_task = successors().try_put_task((*my_body)(i));
#endif
		return new_task;
	}

    protected:
	void reset_function_input(reset_flags f)
	{
		base_type::reset_function_input_base(f);
		if (f & rf_reset_bodies) {
			function_body_type *tmp = my_init_body->clone();
			delete my_body;
			my_body = tmp;
		}
	}

	function_body_type *my_body;
	function_body_type *my_init_body;
	virtual broadcast_cache<output_type> &successors() = 0;

}; // function_input

// helper templates to clear the successor edges of the output ports of an multifunction_node
template <int N> struct clear_element {
	template <typename P> static void clear_this(P &p)
	{
		(void)tbb::flow::get<N - 1>(p).successors().clear();
		clear_element<N - 1>::clear_this(p);
	}
	template <typename P> static bool this_empty(P &p)
	{
		if (tbb::flow::get<N - 1>(p).successors().empty())
			return clear_element<N - 1>::this_empty(p);
		return false;
	}
};

template <> struct clear_element<1> {
	template <typename P> static void clear_this(P &p)
	{
		(void)tbb::flow::get<0>(p).successors().clear();
	}
	template <typename P> static bool this_empty(P &p)
	{
		return tbb::flow::get<0>(p).successors().empty();
	}
};

#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
// helper templates to extract the output ports of an multifunction_node from graph
template <int N> struct extract_element {
	template <typename P> static void extract_this(P &p)
	{
		(void)tbb::flow::get<N - 1>(p).successors().built_successors().sender_extract(tbb::flow::get<N - 1>(p));
		extract_element<N - 1>::extract_this(p);
	}
};

template <> struct extract_element<1> {
	template <typename P> static void extract_this(P &p)
	{
		(void)tbb::flow::get<0>(p).successors().built_successors().sender_extract(tbb::flow::get<0>(p));
	}
};
#endif

//! Implements methods for a function node that takes a type Input as input
//  and has a tuple of output ports specified.
template <typename Input, typename OutputPortSet, typename A>
class multifunction_input : public function_input_base<Input, A, multifunction_input<Input, OutputPortSet, A> > {
    public:
	static const int N = tbb::flow::tuple_size<OutputPortSet>::value;
	typedef Input input_type;
	typedef OutputPortSet output_ports_type;
	typedef multifunction_body<input_type, output_ports_type> multifunction_body_type;
	typedef multifunction_input<Input, OutputPortSet, A> my_class;
	typedef function_input_base<Input, A, my_class> base_type;
	typedef function_input_queue<input_type, A> input_queue_type;

	// constructor
	template <typename Body>
	multifunction_input(graph &g, size_t max_concurrency, Body &body, input_queue_type *q = NULL)
	        : base_type(g, max_concurrency, q)
	        , my_body(new internal::multifunction_body_leaf<input_type, output_ports_type, Body>(body))
	        , my_init_body(new internal::multifunction_body_leaf<input_type, output_ports_type, Body>(body))
	{
	}

	//! Copy constructor
	multifunction_input(const multifunction_input &src, input_queue_type *q = NULL)
	        : base_type(src, q)
	        , my_body(src.my_init_body->clone())
	        , my_init_body(src.my_init_body->clone())
	{
	}

	~multifunction_input()
	{
		delete my_body;
		delete my_init_body;
	}

	template <typename Body> Body copy_function_object()
	{
		multifunction_body_type &body_ref = *this->my_body;
		return dynamic_cast<internal::multifunction_body_leaf<input_type, output_ports_type, Body> &>(body_ref)
		        .get_body();
	}

	// for multifunction nodes we do not have a single successor as such.  So we just tell
	// the task we were successful.
	task *apply_body_impl_bypass(const input_type &i)
	{
		tbb::internal::fgt_begin_body(my_body);
		(*my_body)(i, my_output_ports);
		tbb::internal::fgt_end_body(my_body);
		task *new_task = SUCCESSFULLY_ENQUEUED;
		return new_task;
	}

	output_ports_type &output_ports()
	{
		return my_output_ports;
	}

    protected:
#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
	/*override*/ void extract()
	{
		extract_element<N>::extract_this(my_output_ports);
	}
#endif

	/*override*/ void reset(reset_flags f)
	{
		base_type::reset_function_input_base(f);
		if (f & rf_clear_edges)
			clear_element<N>::clear_this(my_output_ports);
		if (f & rf_reset_bodies) {
			multifunction_body_type *tmp = my_init_body->clone();
			delete my_body;
			my_body = tmp;
		}
		__TBB_ASSERT(!(f & rf_clear_edges) || clear_element<N>::this_empty(my_output_ports),
		             "multifunction_node reset failed");
	}

	multifunction_body_type *my_body;
	multifunction_body_type *my_init_body;
	output_ports_type my_output_ports;

}; // multifunction_input

// template to refer to an output port of a multifunction_node
template <size_t N, typename MOP>
typename tbb::flow::tuple_element<N, typename MOP::output_ports_type>::type &output_port(MOP &op)
{
	return tbb::flow::get<N>(op.output_ports());
}

// helper structs for split_node
template <int N> struct emit_element {
	template <typename T, typename P> static void emit_this(const T &t, P &p)
	{
		(void)tbb::flow::get<N - 1>(p).try_put(tbb::flow::get<N - 1>(t));
		emit_element<N - 1>::emit_this(t, p);
	}
};

template <> struct emit_element<1> {
	template <typename T, typename P> static void emit_this(const T &t, P &p)
	{
		(void)tbb::flow::get<0>(p).try_put(tbb::flow::get<0>(t));
	}
};

//! Implements methods for an executable node that takes continue_msg as input
template <typename Output> class continue_input : public continue_receiver {
    public:
	//! The input type of this receiver
	typedef continue_msg input_type;

	//! The output type of this receiver
	typedef Output output_type;
	typedef function_body<input_type, output_type> function_body_type;

	template <typename Body>
	continue_input(graph &g, Body &body)
	        : my_graph_ptr(&g)
	        , my_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	        , my_init_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	{
	}

	template <typename Body>
	continue_input(graph &g, int number_of_predecessors, Body &body)
	        : continue_receiver(number_of_predecessors)
	        , my_graph_ptr(&g)
	        , my_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	        , my_init_body(new internal::function_body_leaf<input_type, output_type, Body>(body))
	{
	}

	continue_input(const continue_input &src)
	        : continue_receiver(src)
	        , my_graph_ptr(src.my_graph_ptr)
	        , my_body(src.my_init_body->clone())
	        , my_init_body(src.my_init_body->clone())
	{
	}

	~continue_input()
	{
		delete my_body;
		delete my_init_body;
	}

	template <typename Body> Body copy_function_object()
	{
		function_body_type &body_ref = *my_body;
		return dynamic_cast<internal::function_body_leaf<input_type, output_type, Body> &>(body_ref).get_body();
	}

	/*override*/ void reset_receiver(reset_flags f)
	{
		continue_receiver::reset_receiver(f);
		if (f & rf_reset_bodies) {
			function_body_type *tmp = my_init_body->clone();
			delete my_body;
			my_body = tmp;
		}
	}

    protected:
	graph *my_graph_ptr;
	function_body_type *my_body;
	function_body_type *my_init_body;

	virtual broadcast_cache<output_type> &successors() = 0;

	friend class apply_body_task_bypass<continue_input<Output>, continue_msg>;

	//! Applies the body to the provided input
	task *apply_body_bypass(input_type)
	{
#if TBB_PREVIEW_FLOW_GRAPH_TRACE
		// There is an extra copied needed to capture the
		// body execution without the try_put
		tbb::internal::fgt_begin_body(my_body);
		output_type v = (*my_body)(continue_msg());
		tbb::internal::fgt_end_body(my_body);
		return successors().try_put_task(v);
#else
		return successors().try_put_task((*my_body)(continue_msg()));
#endif
	}

	//! Spawns a task that applies the body
	/* override */ task *execute()
	{
		return (my_graph_ptr->is_active()) ?
		               new (task::allocate_additional_child_of(*(my_graph_ptr->root_task())))
		                       apply_body_task_bypass<continue_input<Output>, continue_msg>(*this,
		                                                                                    continue_msg()) :
		               NULL;
	}

}; // continue_input

//! Implements methods for both executable and function nodes that puts Output to its successors
template <typename Output> class function_output : public sender<Output> {
    public:
	template <int N> friend struct clear_element;
	typedef Output output_type;
	typedef typename sender<output_type>::successor_type successor_type;
	typedef broadcast_cache<output_type> broadcast_cache_type;
#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
	typedef typename sender<output_type>::built_successors_type built_successors_type;
	typedef typename sender<output_type>::successor_list_type successor_list_type;
#endif

	function_output()
	{
		my_successors.set_owner(this);
	}
	function_output(const function_output & /*other*/)
	        : sender<output_type>()
	{
		my_successors.set_owner(this);
	}

	//! Adds a new successor to this node
	/* override */ bool register_successor(successor_type &r)
	{
		successors().register_successor(r);
		return true;
	}

	//! Removes a successor from this node
	/* override */ bool remove_successor(successor_type &r)
	{
		successors().remove_successor(r);
		return true;
	}

#if TBB_PREVIEW_FLOW_GRAPH_FEATURES
	built_successors_type &built_successors()
	{
		return successors().built_successors();
	}

	/*override*/ void internal_add_built_successor(successor_type &r)
	{
		successors().internal_add_built_successor(r);
	}

	/*override*/ void internal_delete_built_successor(successor_type &r)
	{
		successors().internal_delete_built_successor(r);
	}

	/*override*/ size_t successor_count()
	{
		return successors().successor_count();
	}

	/*override*/ void copy_successors(successor_list_type &v)
	{
		successors().copy_successors(v);
	}
#endif /* TBB_PREVIEW_FLOW_GRAPH_FEATURES */

	// for multifunction_node.  The function_body that implements
	// the node will have an input and an output tuple of ports.  To put
	// an item to a successor, the body should
	//
	//    get<I>(output_ports).try_put(output_value);
	//
	// if task pointer is returned will always spawn and return true, else
	// return value will be bool returned from successors.try_put.
	task *try_put_task(const output_type &i)
	{
		return my_successors.try_put_task(i);
	}

	broadcast_cache_type &successors()
	{
		return my_successors;
	}

    protected:
	broadcast_cache_type my_successors;

}; // function_output

template <typename Output> class multifunction_output : public function_output<Output> {
    public:
	typedef Output output_type;
	typedef function_output<output_type> base_type;
	using base_type::my_successors;

	multifunction_output()
	        : base_type()
	{
		my_successors.set_owner(this);
	}
	multifunction_output(const multifunction_output & /*other*/)
	        : base_type()
	{
		my_successors.set_owner(this);
	}

	bool try_put(const output_type &i)
	{
		task *res = my_successors.try_put_task(i);
		if (!res)
			return false;
		if (res != SUCCESSFULLY_ENQUEUED)
			FLOW_SPAWN(*res);
		return true;
	}
}; // multifunction_output

//composite_node
#if TBB_PREVIEW_FLOW_GRAPH_TRACE && __TBB_FLOW_GRAPH_CPP11_FEATURES
template <typename CompositeType> void add_nodes_impl(CompositeType *, bool)
{
}

template <typename CompositeType, typename NodeType1, typename... NodeTypes>
void add_nodes_impl(CompositeType *c_node, bool visible, const NodeType1 &n1, const NodeTypes &... n)
{
	void *addr = const_cast<NodeType1 *>(&n1);

	if (visible)
		tbb::internal::itt_relation_add(tbb::internal::ITT_DOMAIN_FLOW, c_node, tbb::internal::FLOW_NODE,
		                                tbb::internal::__itt_relation_is_parent_of, addr,
		                                tbb::internal::FLOW_NODE);
	else
		tbb::internal::itt_relation_add(tbb::internal::ITT_DOMAIN_FLOW, addr, tbb::internal::FLOW_NODE,
		                                tbb::internal::__itt_relation_is_child_of, c_node,
		                                tbb::internal::FLOW_NODE);
	add_nodes_impl(c_node, visible, n...);
}
#endif

} // internal

#endif // __TBB__flow_graph_node_impl_H
