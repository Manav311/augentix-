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
// Basic Usage Environment: for a simple, non-scripted, console application
// C++ header

#ifndef _BASIC_USAGE_ENVIRONMENT_HH
#define _BASIC_USAGE_ENVIRONMENT_HH

#ifndef _BASIC_USAGE_ENVIRONMENT0_HH
#include "BasicUsageEnvironment0.hh"
#endif

class BasicUsageEnvironment : public BasicUsageEnvironment0 {
    public:
	static BasicUsageEnvironment *createNew(TaskScheduler &taskScheduler);

	// redefined virtual functions:
	virtual int getErrno() const;

	virtual UsageEnvironment &operator<<(char const *str);
	virtual UsageEnvironment &operator<<(int i);
	virtual UsageEnvironment &operator<<(unsigned u);
	virtual UsageEnvironment &operator<<(double d);
	virtual UsageEnvironment &operator<<(void *p);

    protected:
	BasicUsageEnvironment(TaskScheduler &taskScheduler);
	// called only by "createNew()" (or subclass constructors)
	virtual ~BasicUsageEnvironment();
};



#include "HandlerSet.hh"
class EpollTaskScheduler : public BasicTaskScheduler0 {
    public:
	static EpollTaskScheduler *createNew(unsigned maxSchedulerGranularity = 10000 /*microseconds*/);
	virtual ~EpollTaskScheduler();
	/*inherit scheduleDelayedTask/unscheduleDelayedTask functions to implement timerfd periodical trigger*/
	virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc *proc, void *clientData);
	virtual void unscheduleDelayedTask(TaskToken &prevTask);
	virtual void rescheduleDelayedTask(TaskToken &task, int64_t microseconds);

    private:
	EpollTaskScheduler(unsigned maxSchedulerGranularity);

	static void schedulerTickTask(void *clientData);
	void schedulerTickTask();

	const HandlerDescriptor *lookupHandlerDescriptor(int socketNum) const;

    private:
	virtual void SingleStep(unsigned maxDelayTime);

	virtual void setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc *handlerProc,
	                                   void *clientData);
	virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

    private:
	unsigned fMaxSchedulerGranularity;

    private:
#if defined(__WIN32__) || defined(_WIN32)
	void *fEpollHandle;
#else
	int fEpollHandle;
#endif
};

#endif
