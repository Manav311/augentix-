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

#ifndef __TBB_profiling_H
#define __TBB_profiling_H

namespace tbb
{
namespace internal
{
//
// This is not under __TBB_ITT_STRUCTURE_API because these values are used directly in flow_graph.h.
//

// include list of index names
#define TBB_STRING_RESOURCE(index_name, str) index_name,
enum string_index {
#include "internal/_tbb_strings.h"
	NUM_STRINGS
};
#undef TBB_STRING_RESOURCE

enum itt_relation {
	__itt_relation_is_unknown = 0,
	__itt_relation_is_dependent_on, /**< "A is dependent on B" means that A cannot start until B completes */
	__itt_relation_is_sibling_of, /**< "A is sibling of B" means that A and B were created as a group */
	__itt_relation_is_parent_of, /**< "A is parent of B" means that A created B */
	__itt_relation_is_continuation_of, /**< "A is continuation of B" means that A assumes the dependencies of B */
	__itt_relation_is_child_of, /**< "A is child of B" means that A was created by B (inverse of is_parent_of) */
	__itt_relation_is_continued_by, /**< "A is continued by B" means that B assumes the dependencies of A (inverse of is_continuation_of) */
	__itt_relation_is_predecessor_to /**< "A is predecessor to B" means that B cannot start until A completes (inverse of is_dependent_on) */
};

}
}

// Check if the tools support is enabled
#if (_WIN32 || _WIN64 || __linux__) && !__MINGW32__ && TBB_USE_THREADING_TOOLS

#if _WIN32 || _WIN64
#include <stdlib.h> /* mbstowcs_s */
#endif
#include "tbb_stddef.h"

namespace tbb
{
namespace internal
{
#if _WIN32 || _WIN64
void __TBB_EXPORTED_FUNC itt_set_sync_name_v3(void *obj, const wchar_t *name);
inline size_t multibyte_to_widechar(wchar_t *wcs, const char *mbs, size_t bufsize)
{
#if _MSC_VER >= 1400
	size_t len;
	mbstowcs_s(&len, wcs, bufsize, mbs, _TRUNCATE);
	return len; // mbstowcs_s counts null terminator
#else
	size_t len = mbstowcs(wcs, mbs, bufsize);
	if (wcs && len != size_t(-1))
		wcs[len < bufsize - 1 ? len : bufsize - 1] = wchar_t('\0');
	return len + 1; // mbstowcs does not count null terminator
#endif
}
#else
void __TBB_EXPORTED_FUNC itt_set_sync_name_v3(void *obj, const char *name);
#endif
} // namespace internal
} // namespace tbb

//! Macro __TBB_DEFINE_PROFILING_SET_NAME(T) defines "set_name" methods for sync objects of type T
/** Should be used in the "tbb" namespace only.
    Don't place semicolon after it to avoid compiler warnings. **/
#if _WIN32 || _WIN64
#define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)                         \
	namespace profiling                                                       \
	{                                                                         \
	inline void set_name(sync_object_type &obj, const wchar_t *name)          \
	{                                                                         \
		tbb::internal::itt_set_sync_name_v3(&obj, name);                  \
	}                                                                         \
	inline void set_name(sync_object_type &obj, const char *name)             \
	{                                                                         \
		size_t len = tbb::internal::multibyte_to_widechar(NULL, name, 0); \
		wchar_t *wname = new wchar_t[len];                                \
		tbb::internal::multibyte_to_widechar(wname, name, len);           \
		set_name(obj, wname);                                             \
		delete[] wname;                                                   \
	}                                                                         \
	}
#else /* !WIN */
#define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)             \
	namespace profiling                                           \
	{                                                             \
	inline void set_name(sync_object_type &obj, const char *name) \
	{                                                             \
		tbb::internal::itt_set_sync_name_v3(&obj, name);      \
	}                                                             \
	}
#endif /* !WIN */

#else /* no tools support */

#if _WIN32 || _WIN64
#define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)         \
	namespace profiling                                       \
	{                                                         \
	inline void set_name(sync_object_type &, const wchar_t *) \
	{                                                         \
	}                                                         \
	inline void set_name(sync_object_type &, const char *)    \
	{                                                         \
	}                                                         \
	}
#else /* !WIN */
#define __TBB_DEFINE_PROFILING_SET_NAME(sync_object_type)      \
	namespace profiling                                    \
	{                                                      \
	inline void set_name(sync_object_type &, const char *) \
	{                                                      \
	}                                                      \
	}
#endif /* !WIN */

#endif /* no tools support */

#include "atomic.h"
// Need these to work regardless of tools support
namespace tbb
{
namespace internal
{
enum notify_type { prepare = 0, cancel, acquired, releasing };

const uintptr_t NUM_NOTIFY_TYPES = 4; // set to # elements in enum above

void __TBB_EXPORTED_FUNC call_itt_notify_v5(int t, void *ptr);
void __TBB_EXPORTED_FUNC itt_store_pointer_with_release_v3(void *dst, void *src);
void *__TBB_EXPORTED_FUNC itt_load_pointer_with_acquire_v3(const void *src);
void *__TBB_EXPORTED_FUNC itt_load_pointer_v3(const void *src);
#if __TBB_ITT_STRUCTURE_API
enum itt_domain_enum { ITT_DOMAIN_FLOW = 0 };

void __TBB_EXPORTED_FUNC itt_make_task_group_v7(itt_domain_enum domain, void *group, unsigned long long group_extra,
                                                void *parent, unsigned long long parent_extra, string_index name_index);
void __TBB_EXPORTED_FUNC itt_metadata_str_add_v7(itt_domain_enum domain, void *addr, unsigned long long addr_extra,
                                                 string_index key, const char *value);
void __TBB_EXPORTED_FUNC itt_relation_add_v7(itt_domain_enum domain, void *addr0, unsigned long long addr0_extra,
                                             itt_relation relation, void *addr1, unsigned long long addr1_extra);
void __TBB_EXPORTED_FUNC itt_task_begin_v7(itt_domain_enum domain, void *task, unsigned long long task_extra,
                                           void *parent, unsigned long long parent_extra, string_index name_index);
void __TBB_EXPORTED_FUNC itt_task_end_v7(itt_domain_enum domain);

void __TBB_EXPORTED_FUNC itt_region_begin_v9(itt_domain_enum domain, void *region, unsigned long long region_extra,
                                             void *parent, unsigned long long parent_extra, string_index name_index);
void __TBB_EXPORTED_FUNC itt_region_end_v9(itt_domain_enum domain, void *region, unsigned long long region_extra);
#endif // __TBB_ITT_STRUCTURE_API

// two template arguments are to workaround /Wp64 warning with tbb::atomic specialized for unsigned type
template <typename T, typename U> inline void itt_store_word_with_release(tbb::atomic<T> &dst, U src)
{
#if TBB_USE_THREADING_TOOLS
	// This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized.");
	itt_store_pointer_with_release_v3(&dst, (void *)uintptr_t(src));
#else
	dst = src;
#endif // TBB_USE_THREADING_TOOLS
}

template <typename T> inline T itt_load_word_with_acquire(const tbb::atomic<T> &src)
{
#if TBB_USE_THREADING_TOOLS
	// This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized.");
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
// Workaround for overzealous compiler warnings
#pragma warning(push)
#pragma warning(disable : 4311)
#endif
	T result = (T)itt_load_pointer_with_acquire_v3(&src);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif
	return result;
#else
	return src;
#endif // TBB_USE_THREADING_TOOLS
}

template <typename T> inline void itt_store_word_with_release(T &dst, T src)
{
#if TBB_USE_THREADING_TOOLS
	// This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized.");
	itt_store_pointer_with_release_v3(&dst, (void *)src);
#else
	__TBB_store_with_release(dst, src);
#endif // TBB_USE_THREADING_TOOLS
}

template <typename T> inline T itt_load_word_with_acquire(const T &src)
{
#if TBB_USE_THREADING_TOOLS
	// This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized");
	return (T)itt_load_pointer_with_acquire_v3(&src);
#else
	return __TBB_load_with_acquire(src);
#endif // TBB_USE_THREADING_TOOLS
}

template <typename T> inline void itt_hide_store_word(T &dst, T src)
{
#if TBB_USE_THREADING_TOOLS
	//TODO: This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized");
	itt_store_pointer_with_release_v3(&dst, (void *)src);
#else
	dst = src;
#endif
}

//TODO: rename to itt_hide_load_word_relaxed
template <typename T> inline T itt_hide_load_word(const T &src)
{
#if TBB_USE_THREADING_TOOLS
	//TODO: This assertion should be replaced with static_assert
	__TBB_ASSERT(sizeof(T) == sizeof(void *), "Type must be word-sized.");
	return (T)itt_load_pointer_v3(&src);
#else
	return src;
#endif
}

#if TBB_USE_THREADING_TOOLS
inline void call_itt_notify(notify_type t, void *ptr)
{
	call_itt_notify_v5((int)t, ptr);
}

#else
inline void call_itt_notify(notify_type /*t*/, void * /*ptr*/)
{
}

#endif // TBB_USE_THREADING_TOOLS

#if __TBB_ITT_STRUCTURE_API
inline void itt_make_task_group(itt_domain_enum domain, void *group, unsigned long long group_extra, void *parent,
                                unsigned long long parent_extra, string_index name_index)
{
	itt_make_task_group_v7(domain, group, group_extra, parent, parent_extra, name_index);
}

inline void itt_metadata_str_add(itt_domain_enum domain, void *addr, unsigned long long addr_extra, string_index key,
                                 const char *value)
{
	itt_metadata_str_add_v7(domain, addr, addr_extra, key, value);
}

inline void itt_relation_add(itt_domain_enum domain, void *addr0, unsigned long long addr0_extra, itt_relation relation,
                             void *addr1, unsigned long long addr1_extra)
{
	itt_relation_add_v7(domain, addr0, addr0_extra, relation, addr1, addr1_extra);
}

inline void itt_task_begin(itt_domain_enum domain, void *task, unsigned long long task_extra, void *parent,
                           unsigned long long parent_extra, string_index name_index)
{
	itt_task_begin_v7(domain, task, task_extra, parent, parent_extra, name_index);
}

inline void itt_task_end(itt_domain_enum domain)
{
	itt_task_end_v7(domain);
}

inline void itt_region_begin(itt_domain_enum domain, void *region, unsigned long long region_extra, void *parent,
                             unsigned long long parent_extra, string_index name_index)
{
	itt_region_begin_v9(domain, region, region_extra, parent, parent_extra, name_index);
}

inline void itt_region_end(itt_domain_enum domain, void *region, unsigned long long region_extra)
{
	itt_region_end_v9(domain, region, region_extra);
}
#endif // __TBB_ITT_STRUCTURE_API

} // namespace internal
} // namespace tbb

#endif /* __TBB_profiling_H */
