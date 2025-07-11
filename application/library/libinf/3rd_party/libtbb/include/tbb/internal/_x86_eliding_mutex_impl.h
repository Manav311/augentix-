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

#ifndef __TBB__x86_eliding_mutex_impl_H
#define __TBB__x86_eliding_mutex_impl_H

#ifndef __TBB_spin_mutex_H
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#if (__TBB_x86_32 || __TBB_x86_64)

namespace tbb
{
namespace interface7
{
namespace internal
{
template <typename Mutex, bool is_rw> class padded_mutex;

//! An eliding lock that occupies a single byte.
/** A x86_eliding_mutex is an HLE-enabled spin mutex. It is recommended to
    put the mutex on a cache line that is not shared by the data it protects.
    It should be used for locking short critical sections where the lock is
    contended but the data it protects are not.  If zero-initialized, the
    mutex is considered unheld.
    @ingroup synchronization */
class x86_eliding_mutex : tbb::internal::mutex_copy_deprecated_and_disabled {
	//! 0 if lock is released, 1 if lock is acquired.
	__TBB_atomic_flag flag;

	friend class padded_mutex<x86_eliding_mutex, false>;

    public:
	//! Construct unacquired lock.
	/** Equivalent to zero-initialization of *this. */
	x86_eliding_mutex()
	        : flag(0)
	{
	}

// bug in gcc 3.x.x causes syntax error in spite of the friend declaration above.
// Make the scoped_lock public in that case.
#if __TBB_USE_X86_ELIDING_MUTEX || __TBB_GCC_VERSION < 40000
#else
	// by default we will not provide the scoped_lock interface.  The user
	// should use the padded version of the mutex.  scoped_lock is used in
	// padded_mutex template.
    private:
#endif
	// scoped_lock in padded_mutex<> is the interface to use.
	//! Represents acquisition of a mutex.
	class scoped_lock : tbb::internal::no_copy {
	    private:
		//! Points to currently held mutex, or NULL if no lock is held.
		x86_eliding_mutex *my_mutex;

	    public:
		//! Construct without acquiring a mutex.
		scoped_lock()
		        : my_mutex(NULL)
		{
		}

		//! Construct and acquire lock on a mutex.
		scoped_lock(x86_eliding_mutex &m)
		        : my_mutex(NULL)
		{
			acquire(m);
		}

		//! Acquire lock.
		void acquire(x86_eliding_mutex &m)
		{
			__TBB_ASSERT(!my_mutex, "already holding a lock");

			my_mutex = &m;
			my_mutex->lock();
		}

		//! Try acquiring lock (non-blocking)
		/** Return true if lock acquired; false otherwise. */
		bool try_acquire(x86_eliding_mutex &m)
		{
			__TBB_ASSERT(!my_mutex, "already holding a lock");

			bool result = m.try_lock();
			if (result) {
				my_mutex = &m;
			}
			return result;
		}

		//! Release lock
		void release()
		{
			__TBB_ASSERT(my_mutex, "release on scoped_lock that is not holding a lock");

			my_mutex->unlock();
			my_mutex = NULL;
		}

		//! Destroy lock.  If holding a lock, releases the lock first.
		~scoped_lock()
		{
			if (my_mutex) {
				release();
			}
		}
	};
#if __TBB_USE_X86_ELIDING_MUTEX || __TBB_GCC_VERSION < 40000
#else
    public:
#endif /* __TBB_USE_X86_ELIDING_MUTEX */

	// Mutex traits
	static const bool is_rw_mutex = false;
	static const bool is_recursive_mutex = false;
	static const bool is_fair_mutex = false;

	// ISO C++0x compatibility methods

	//! Acquire lock
	void lock()
	{
		__TBB_LockByteElided(flag);
	}

	//! Try acquiring lock (non-blocking)
	/** Return true if lock acquired; false otherwise. */
	bool try_lock()
	{
		return __TBB_TryLockByteElided(flag);
	}

	//! Release lock
	void unlock()
	{
		__TBB_UnlockByteElided(flag);
	}
}; // end of x86_eliding_mutex

} // namespace internal
} // namespace interface7
} // namespace tbb

#endif /* ( __TBB_x86_32 || __TBB_x86_64 ) */

#endif /* __TBB__x86_eliding_mutex_impl_H */
