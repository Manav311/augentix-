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

/*
    Platform isolation layer for the ARMv7-a architecture.
*/

#ifndef __TBB_machine_H
#error Do not include this file directly; include tbb_machine.h instead
#endif

//TODO: is ARMv7 is the only version ever to support?
#if !(__ARM_ARCH_7A__)
#error compilation requires an ARMv7-a architecture.
#endif

#include <sys/param.h>
#include <unistd.h>

#define __TBB_WORDSIZE 4

// Traditionally ARM is little-endian.
// Note that, since only the layout of aligned 32-bit words is of interest,
// any apparent PDP-endianness of 32-bit words at half-word alignment or
// any little-endian ordering of big-endian 32-bit words in 64-bit quantities
// may be disregarded for this setting.
#if __BIG_ENDIAN__ || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define __TBB_ENDIANNESS __TBB_ENDIAN_BIG
#elif __LITTLE_ENDIAN__ || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define __TBB_ENDIANNESS __TBB_ENDIAN_LITTLE
#elif defined(__BYTE_ORDER__)
#define __TBB_ENDIANNESS __TBB_ENDIAN_UNSUPPORTED
#else
#define __TBB_ENDIANNESS __TBB_ENDIAN_DETECT
#endif

#define __TBB_compiler_fence() __asm__ __volatile__("" : : : "memory")
#define __TBB_full_memory_fence() __asm__ __volatile__("dmb ish" : : : "memory")
#define __TBB_control_consistency_helper() __TBB_full_memory_fence()
#define __TBB_acquire_consistency_helper() __TBB_full_memory_fence()
#define __TBB_release_consistency_helper() __TBB_full_memory_fence()

//--------------------------------------------------
// Compare and swap
//--------------------------------------------------

/**
 * Atomic CAS for 32 bit values, if *ptr==comparand, then *ptr=value, returns *ptr
 * @param ptr pointer to value in memory to be swapped with value if *ptr==comparand
 * @param value value to assign *ptr to if *ptr==comparand
 * @param comparand value to compare with *ptr
 * @return value originally in memory at ptr, regardless of success
*/
static inline int32_t __TBB_machine_cmpswp4(volatile void *ptr, int32_t value, int32_t comparand)
{
	int32_t oldval, res;

	__TBB_full_memory_fence();

	do {
		__asm__ __volatile__("ldrex      %1, [%3]\n"
		                     "mov        %0, #0\n"
		                     "cmp        %1, %4\n"
		                     "it         eq\n"
		                     "strexeq    %0, %5, [%3]\n"
		                     : "=&r"(res), "=&r"(oldval), "+Qo"(*(volatile int32_t *)ptr)
		                     : "r"((int32_t *)ptr), "Ir"(comparand), "r"(value)
		                     : "cc");
	} while (res);

	__TBB_full_memory_fence();

	return oldval;
}

/**
 * Atomic CAS for 64 bit values, if *ptr==comparand, then *ptr=value, returns *ptr
 * @param ptr pointer to value in memory to be swapped with value if *ptr==comparand
 * @param value value to assign *ptr to if *ptr==comparand
 * @param comparand value to compare with *ptr
 * @return value originally in memory at ptr, regardless of success
 */
static inline int64_t __TBB_machine_cmpswp8(volatile void *ptr, int64_t value, int64_t comparand)
{
	int64_t oldval;
	int32_t res;

	__TBB_full_memory_fence();

	do {
		__asm__ __volatile__("mov        %0, #0\n"
		                     "ldrexd     %1, %H1, [%3]\n"
		                     "cmp        %1, %4\n"
		                     "it         eq\n"
		                     "cmpeq      %H1, %H4\n"
		                     "it         eq\n"
		                     "strexdeq   %0, %5, %H5, [%3]"
		                     : "=&r"(res), "=&r"(oldval), "+Qo"(*(volatile int64_t *)ptr)
		                     : "r"((int64_t *)ptr), "r"(comparand), "r"(value)
		                     : "cc");
	} while (res);

	__TBB_full_memory_fence();

	return oldval;
}

static inline int32_t __TBB_machine_fetchadd4(volatile void *ptr, int32_t addend)
{
	unsigned long tmp;
	int32_t result, tmp2;

	__TBB_full_memory_fence();

	__asm__ __volatile__("1:     ldrex   %0, [%4]\n"
	                     "       add     %3, %0, %5\n"
	                     "       strex   %1, %3, [%4]\n"
	                     "       cmp     %1, #0\n"
	                     "       bne     1b\n"
	                     : "=&r"(result), "=&r"(tmp), "+Qo"(*(volatile int32_t *)ptr), "=&r"(tmp2)
	                     : "r"((int32_t *)ptr), "Ir"(addend)
	                     : "cc");

	__TBB_full_memory_fence();

	return result;
}

static inline int64_t __TBB_machine_fetchadd8(volatile void *ptr, int64_t addend)
{
	unsigned long tmp;
	int64_t result, tmp2;

	__TBB_full_memory_fence();

	__asm__ __volatile__("1:     ldrexd  %0, %H0, [%4]\n"
	                     "       adds    %3, %0, %5\n"
	                     "       adc     %H3, %H0, %H5\n"
	                     "       strexd  %1, %3, %H3, [%4]\n"
	                     "       cmp     %1, #0\n"
	                     "       bne     1b"
	                     : "=&r"(result), "=&r"(tmp), "+Qo"(*(volatile int64_t *)ptr), "=&r"(tmp2)
	                     : "r"((int64_t *)ptr), "r"(addend)
	                     : "cc");

	__TBB_full_memory_fence();

	return result;
}

inline void __TBB_machine_pause(int32_t delay)
{
	while (delay > 0) {
		__TBB_compiler_fence();
		delay--;
	}
}

namespace tbb
{
namespace internal
{
template <typename T, size_t S> struct machine_load_store_relaxed {
	static inline T load(const volatile T &location)
	{
		const T value = location;

		/*
            * An extra memory barrier is required for errata #761319
            * Please see http://infocenter.arm.com/help/topic/com.arm.doc.uan0004a
            */
		__TBB_acquire_consistency_helper();
		return value;
	}

	static inline void store(volatile T &location, T value)
	{
		location = value;
	}
};
}
} // namespaces internal, tbb

// Machine specific atomic operations

#define __TBB_CompareAndSwap4(P, V, C) __TBB_machine_cmpswp4(P, V, C)
#define __TBB_CompareAndSwap8(P, V, C) __TBB_machine_cmpswp8(P, V, C)
#define __TBB_Pause(V) __TBB_machine_pause(V)

// Use generics for some things
#define __TBB_USE_GENERIC_PART_WORD_CAS 1
#define __TBB_USE_GENERIC_PART_WORD_FETCH_ADD 1
#define __TBB_USE_GENERIC_PART_WORD_FETCH_STORE 1
#define __TBB_USE_GENERIC_FETCH_STORE 1
#define __TBB_USE_GENERIC_HALF_FENCED_LOAD_STORE 1
#define __TBB_USE_GENERIC_DWORD_LOAD_STORE 1
#define __TBB_USE_GENERIC_SEQUENTIAL_CONSISTENCY_LOAD_STORE 1
