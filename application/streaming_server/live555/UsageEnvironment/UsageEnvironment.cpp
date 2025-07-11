/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// Usage Environment
// Implementation

#include "UsageEnvironment.hh"
#include "agtx_types.h"
#include <malloc.h>

void UsageEnvironment::reclaim()
{
	// We delete ourselves only if we have no remainining state:
	if (liveMediaPriv == NULL && groupsockPriv == NULL)
		delete this;
}

UsageEnvironment::UsageEnvironment(TaskScheduler &scheduler)
        : liveMediaPriv(NULL)
        , groupsockPriv(NULL)
        , fScheduler(scheduler)
{
}

UsageEnvironment::~UsageEnvironment()
{
}

// By default, we handle 'should not occur'-type library errors by calling
// abort().  Subclasses can redefine this, if desired.
// (If your runtime library doesn't define the "abort()" function, then define
// your own (e.g., that does nothing).)
void UsageEnvironment::internalError()
{
	abort();
}

TaskScheduler::TaskScheduler()
{
}

TaskScheduler::~TaskScheduler()
{
}

void TaskScheduler::rescheduleDelayedTask(TaskToken &task, int64_t microseconds)
{
	AGTX_UNUSED(task);
	AGTX_UNUSED(microseconds);
#if 0 /*is replaced by epollscheduler implement*/

#endif
}

// By default, we handle 'should not occur'-type library errors by calling
// abort().  Subclasses can redefine this, if desired.
void TaskScheduler::internalError()
{
	abort();
}
