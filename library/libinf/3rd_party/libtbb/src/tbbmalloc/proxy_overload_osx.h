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

// The original source for this code is
// Copyright (c) 2011, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <AvailabilityMacros.h>
#include <malloc/malloc.h>

static kern_return_t enumerator(task_t, void *, unsigned, vm_address_t, memory_reader_t, vm_range_recorder_t)
{
	return KERN_FAILURE;
}

static size_t good_size(malloc_zone_t *, size_t size)
{
	return size;
}

static boolean_t zone_check(malloc_zone_t *) /* Consistency checker */
{
	return true;
}

static void zone_print(malloc_zone_t *, boolean_t)
{
}
static void zone_log(malloc_zone_t *, void *)
{
}
static void zone_force_lock(malloc_zone_t *)
{
}
static void zone_force_unlock(malloc_zone_t *)
{
}

static void zone_statistics(malloc_zone_t *, malloc_statistics_t *s)
{
	s->blocks_in_use = 0;
	s->size_in_use = s->max_size_in_use = s->size_allocated = 0;
}

static boolean_t zone_locked(malloc_zone_t *)
{
	return false;
}

static boolean_t impl_zone_enable_discharge_checking(malloc_zone_t *)
{
	return false;
}

static void impl_zone_disable_discharge_checking(malloc_zone_t *)
{
}
static void impl_zone_discharge(malloc_zone_t *, void *)
{
}
static void impl_zone_destroy(struct _malloc_zone_t *)
{
}

/* note: impl_malloc_usable_size() is called for each free() call, so it must be fast */
static size_t impl_malloc_usable_size(struct _malloc_zone_t *, const void *ptr)
{
	// malloc_usable_size() is used by OS X to recognize which memory manager
	// allocated the address, so our wrapper must not redirect to the original function.
	return __TBB_malloc_safer_msize(const_cast<void *>(ptr), NULL);
}

static void *impl_malloc(struct _malloc_zone_t *, size_t size);
static void *impl_calloc(struct _malloc_zone_t *, size_t num_items, size_t size);
static void *impl_valloc(struct _malloc_zone_t *, size_t size);
static void impl_free(struct _malloc_zone_t *, void *ptr);
static void *impl_realloc(struct _malloc_zone_t *, void *ptr, size_t size);
static void *impl_memalign(struct _malloc_zone_t *, size_t alignment, size_t size);

/* ptr is in zone and have reported size */
static void impl_free_definite_size(struct _malloc_zone_t *, void *ptr, size_t size)
{
	__TBB_malloc_free_definite_size(ptr, size);
}

/* Empty out caches in the face of memory pressure. */
static size_t impl_pressure_relief(struct _malloc_zone_t *, size_t goal)
{
	return 0;
}

static malloc_zone_t *system_zone;

struct DoMallocReplacement {
	DoMallocReplacement()
	{
		static malloc_introspection_t introspect;
		memset(&introspect, 0, sizeof(malloc_introspection_t));
		static malloc_zone_t zone;
		memset(&zone, 0, sizeof(malloc_zone_t));

		introspect.enumerator = &enumerator;
		introspect.good_size = &good_size;
		introspect.check = &zone_check;
		introspect.print = &zone_print;
		introspect.log = zone_log;
		introspect.force_lock = &zone_force_lock;
		introspect.force_unlock = &zone_force_unlock;
		introspect.statistics = zone_statistics;
		introspect.zone_locked = &zone_locked;
		introspect.enable_discharge_checking = &impl_zone_enable_discharge_checking;
		introspect.disable_discharge_checking = &impl_zone_disable_discharge_checking;
		introspect.discharge = &impl_zone_discharge;

		zone.size = &impl_malloc_usable_size;
		zone.malloc = &impl_malloc;
		zone.calloc = &impl_calloc;
		zone.valloc = &impl_valloc;
		zone.free = &impl_free;
		zone.realloc = &impl_realloc;
		zone.destroy = &impl_zone_destroy;
		zone.zone_name = "tbbmalloc";
		zone.introspect = &introspect;
		zone.version = 8;
		zone.memalign = impl_memalign;
		zone.free_definite_size = &impl_free_definite_size;
		zone.pressure_relief = &impl_pressure_relief;

		// make sure that default purgeable zone is initialized
		malloc_default_purgeable_zone();
		// after unregistration of system zone, our zone became default
		malloc_zone_register(&zone);
		system_zone = malloc_default_zone();
		malloc_zone_unregister(system_zone);
		malloc_zone_register(system_zone);
	}
};

static DoMallocReplacement doMallocReplacement;
