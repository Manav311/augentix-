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

#ifndef __TBB_tbb_allocator_H
#define __TBB_tbb_allocator_H

#include "tbb_stddef.h"
#include <new>
#if __TBB_ALLOCATOR_CONSTRUCT_VARIADIC
#include <utility> // std::forward
#endif

#if !TBB_USE_EXCEPTIONS && _MSC_VER
// Suppress "C++ exception handler used, but unwind semantics are not enabled" warning in STL headers
#pragma warning(push)
#pragma warning(disable : 4530)
#endif

#include <cstring>

#if !TBB_USE_EXCEPTIONS && _MSC_VER
#pragma warning(pop)
#endif

namespace tbb
{
//! @cond INTERNAL
namespace internal
{
//! Deallocates memory using FreeHandler
/** The function uses scalable_free if scalable allocator is available and free if not*/
void __TBB_EXPORTED_FUNC deallocate_via_handler_v3(void *p);

//! Allocates memory using MallocHandler
/** The function uses scalable_malloc if scalable allocator is available and malloc if not*/
void *__TBB_EXPORTED_FUNC allocate_via_handler_v3(size_t n);

//! Returns true if standard malloc/free are used to work with memory.
bool __TBB_EXPORTED_FUNC is_malloc_used_v3();
}
//! @endcond

#if _MSC_VER && !defined(__INTEL_COMPILER)
// Workaround for erroneous "unreferenced parameter" warning in method destroy.
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

//! Meets "allocator" requirements of ISO C++ Standard, Section 20.1.5
/** The class selects the best memory allocation mechanism available
    from scalable_malloc and standard malloc.
    The members are ordered the same way they are in section 20.4.1
    of the ISO C++ standard.
    @ingroup memory_allocation */
template <typename T> class tbb_allocator {
    public:
	typedef typename internal::allocator_type<T>::value_type value_type;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type &reference;
	typedef const value_type &const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	template <typename U> struct rebind {
		typedef tbb_allocator<U> other;
	};

	//! Specifies current allocator
	enum malloc_type { scalable, standard };

	tbb_allocator() throw()
	{
	}
	tbb_allocator(const tbb_allocator &) throw()
	{
	}
	template <typename U> tbb_allocator(const tbb_allocator<U> &) throw()
	{
	}

	pointer address(reference x) const
	{
		return &x;
	}
	const_pointer address(const_reference x) const
	{
		return &x;
	}

	//! Allocate space for n objects.
	pointer allocate(size_type n, const void * /*hint*/ = 0)
	{
		return pointer(internal::allocate_via_handler_v3(n * sizeof(value_type)));
	}

	//! Free previously allocated block of memory.
	void deallocate(pointer p, size_type)
	{
		internal::deallocate_via_handler_v3(p);
	}

	//! Largest value for which method allocate might succeed.
	size_type max_size() const throw()
	{
		size_type max = static_cast<size_type>(-1) / sizeof(value_type);
		return (max > 0 ? max : 1);
	}

	//! Copy-construct value at location pointed to by p.
#if __TBB_ALLOCATOR_CONSTRUCT_VARIADIC
	template <typename U, typename... Args> void construct(U *p, Args &&... args)
	{
		::new ((void *)p) U(std::forward<Args>(args)...);
	}
#else // __TBB_ALLOCATOR_CONSTRUCT_VARIADIC
#if __TBB_CPP11_RVALUE_REF_PRESENT
	void construct(pointer p, value_type &&value)
	{
		::new ((void *)(p)) value_type(std::move(value));
	}
#endif
	void construct(pointer p, const value_type &value)
	{
		::new ((void *)(p)) value_type(value);
	}
#endif // __TBB_ALLOCATOR_CONSTRUCT_VARIADIC

	//! Destroy value at location pointed to by p.
	void destroy(pointer p)
	{
		p->~value_type();
	}

	//! Returns current allocator
	static malloc_type allocator_type()
	{
		return internal::is_malloc_used_v3() ? standard : scalable;
	}
};

#if _MSC_VER && !defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif // warning 4100 is back

//! Analogous to std::allocator<void>, as defined in ISO C++ Standard, Section 20.4.1
/** @ingroup memory_allocation */
template <> class tbb_allocator<void> {
    public:
	typedef void *pointer;
	typedef const void *const_pointer;
	typedef void value_type;
	template <typename U> struct rebind {
		typedef tbb_allocator<U> other;
	};
};

template <typename T, typename U> inline bool operator==(const tbb_allocator<T> &, const tbb_allocator<U> &)
{
	return true;
}

template <typename T, typename U> inline bool operator!=(const tbb_allocator<T> &, const tbb_allocator<U> &)
{
	return false;
}

//! Meets "allocator" requirements of ISO C++ Standard, Section 20.1.5
/** The class is an adapter over an actual allocator that fills the allocation
    using memset function with template argument C as the value.
    The members are ordered the same way they are in section 20.4.1
    of the ISO C++ standard.
    @ingroup memory_allocation */
template <typename T, template <typename X> class Allocator = tbb_allocator>
class zero_allocator : public Allocator<T> {
    public:
	typedef Allocator<T> base_allocator_type;
	typedef typename base_allocator_type::value_type value_type;
	typedef typename base_allocator_type::pointer pointer;
	typedef typename base_allocator_type::const_pointer const_pointer;
	typedef typename base_allocator_type::reference reference;
	typedef typename base_allocator_type::const_reference const_reference;
	typedef typename base_allocator_type::size_type size_type;
	typedef typename base_allocator_type::difference_type difference_type;
	template <typename U> struct rebind {
		typedef zero_allocator<U, Allocator> other;
	};

	zero_allocator() throw()
	{
	}
	zero_allocator(const zero_allocator &a) throw()
	        : base_allocator_type(a)
	{
	}
	template <typename U>
	zero_allocator(const zero_allocator<U> &a) throw()
	        : base_allocator_type(Allocator<U>(a))
	{
	}

	pointer allocate(const size_type n, const void *hint = 0)
	{
		pointer ptr = base_allocator_type::allocate(n, hint);
		std::memset(ptr, 0, n * sizeof(value_type));
		return ptr;
	}
};

//! Analogous to std::allocator<void>, as defined in ISO C++ Standard, Section 20.4.1
/** @ingroup memory_allocation */
template <template <typename T> class Allocator> class zero_allocator<void, Allocator> : public Allocator<void> {
    public:
	typedef Allocator<void> base_allocator_type;
	typedef typename base_allocator_type::value_type value_type;
	typedef typename base_allocator_type::pointer pointer;
	typedef typename base_allocator_type::const_pointer const_pointer;
	template <typename U> struct rebind {
		typedef zero_allocator<U, Allocator> other;
	};
};

template <typename T1, template <typename X1> class B1, typename T2, template <typename X2> class B2>
inline bool operator==(const zero_allocator<T1, B1> &a, const zero_allocator<T2, B2> &b)
{
	return static_cast<B1<T1> >(a) == static_cast<B2<T2> >(b);
}
template <typename T1, template <typename X1> class B1, typename T2, template <typename X2> class B2>
inline bool operator!=(const zero_allocator<T1, B1> &a, const zero_allocator<T2, B2> &b)
{
	return static_cast<B1<T1> >(a) != static_cast<B2<T2> >(b);
}

} // namespace tbb

#endif /* __TBB_tbb_allocator_H */
