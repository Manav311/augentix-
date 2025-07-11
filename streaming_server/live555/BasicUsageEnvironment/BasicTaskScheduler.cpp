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
// Implementation

#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include <stdio.h>
#include <unistd.h>
#include "sys/timerfd.h"
#include <malloc.h>
#if defined(_QNX4)
#include <sys/select.h>
#include <unix.h>
#endif


#include <sys/epoll.h>
#define EPOLL_INVALID -1
#define epoll_close close

EpollTaskScheduler *EpollTaskScheduler::createNew(unsigned maxSchedulerGranularity)
{
	return new EpollTaskScheduler(maxSchedulerGranularity);
}

EpollTaskScheduler::EpollTaskScheduler(unsigned maxSchedulerGranularity)
        : fMaxSchedulerGranularity(maxSchedulerGranularity)
        , fEpollHandle(EPOLL_INVALID)
{
	fEpollHandle = epoll_create(1024 /*ignored*/);

	if (fEpollHandle == EPOLL_INVALID) {
		internalError();
	}

	if (maxSchedulerGranularity > 0)
		schedulerTickTask(); // ensures that we handle events frequently
}

EpollTaskScheduler::~EpollTaskScheduler()
{
	if (fEpollHandle != EPOLL_INVALID) {
		epoll_close(fEpollHandle);
	}
}

void EpollTaskScheduler::rescheduleDelayedTask(TaskToken &task, int64_t microseconds)
{
	if (task == NULL) {
		return;	
	}
	int *timerfd_tmp = (int*)task;
	
	struct itimerspec timerfd_value;
	if(microseconds == 0) {
		microseconds = 1000;
	}

	timerfd_value.it_value.tv_sec = 0;
	timerfd_value.it_value.tv_nsec = microseconds * 1000;
	timerfd_value.it_interval.tv_sec = 0;
	timerfd_value.it_interval.tv_nsec =  microseconds * 1000 /*10000 us*/;
	if (microseconds > 1000000) {
		timerfd_value.it_value.tv_sec = microseconds / 1000000;
		timerfd_value.it_value.tv_nsec =  (microseconds % 1000000) * 1000;
		timerfd_value.it_interval.tv_sec = microseconds / 1000000;
		timerfd_value.it_interval.tv_nsec = (microseconds % 1000000) * 1000;
	}
	timerfd_settime(*timerfd_tmp, 0, &timerfd_value, NULL);

#if 0
	printf("[%s %d]resched timerfd: %d, interval: %ld s %ld ns \n", __func__, __LINE__, *timerfd_tmp, 
	     timerfd_value.it_interval.tv_sec, timerfd_value.it_interval.tv_nsec);
#endif
}


/*for remove timerfd from epoll and close timerfd*/
void EpollTaskScheduler::unscheduleDelayedTask(TaskToken &prevTask)
{
	if (prevTask == NULL) {
		return;	
	}
	int timerfd_tmp = *(int*)prevTask;

	fHandlers->clearHandler(timerfd_tmp);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = timerfd_tmp; /*union with data.ptr*/
	int ret = epoll_ctl(fEpollHandle, EPOLL_CTL_DEL, timerfd_tmp, &ev);
	if (ret != 0) {
		fprintf(stderr, "epoll del close timer: %d, ret : %d\n", timerfd_tmp, ret);
	}
	close(timerfd_tmp);
	/*don't need to free prevTask (timerfd ptr), it will free at Medium parent class ~Medium()*/
}
/*create timerfd and add to epoll*/
TaskToken EpollTaskScheduler::scheduleDelayedTask(int64_t microseconds, TaskFunc *proc, void *clientData)
{
	int* timerfd_tmp = (int*)malloc(sizeof(int)); 
	*timerfd_tmp = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);

	struct itimerspec timerfd_value;
	if(microseconds == 0) {
		microseconds = 1000;
	}

	timerfd_value.it_value.tv_sec = 0;
	timerfd_value.it_value.tv_nsec = microseconds * 1000;
	timerfd_value.it_interval.tv_sec = 0;
	timerfd_value.it_interval.tv_nsec =  microseconds * 1000 /*10000 us*/;
	if (microseconds > 1000000) {
		timerfd_value.it_value.tv_sec = microseconds / 1000000;
		timerfd_value.it_value.tv_nsec =  (microseconds % 1000000) * 1000;
		timerfd_value.it_interval.tv_sec = microseconds / 1000000;
		timerfd_value.it_interval.tv_nsec = (microseconds % 1000000) * 1000;
	}
	timerfd_settime(*timerfd_tmp, 0, &timerfd_value, NULL);
#if 1
	printf("[%s %d]create timerfd: %d, interval: %ld s %ld ns \n", __func__, __LINE__, *timerfd_tmp, 
	     timerfd_value.it_interval.tv_sec, timerfd_value.it_interval.tv_nsec);
#endif
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = *timerfd_tmp; /*union with data.ptr*/

	const HandlerDescriptor *handler = lookupHandlerDescriptor(*timerfd_tmp);
	if (handler != NULL) {
		epoll_ctl(fEpollHandle, EPOLL_CTL_DEL, *timerfd_tmp, &ev);
	}

	fHandlers->assignHandler(*timerfd_tmp, SOCKET_READABLE, (BackgroundHandlerProc*)proc, clientData);

	epoll_ctl(fEpollHandle, EPOLL_CTL_ADD, *timerfd_tmp, &ev);

	return (TaskToken)timerfd_tmp;
}


void EpollTaskScheduler::schedulerTickTask(void *clientData)
{
	((EpollTaskScheduler *)clientData)->schedulerTickTask();
}

void EpollTaskScheduler::schedulerTickTask()
{
	fprintf(stderr, "don't need Tick Task to alarm\n");
}

#ifndef MILLION
#define MILLION 1000000
#endif

void EpollTaskScheduler::SingleStep(unsigned maxDelayTime)
{
	(void)(maxDelayTime);

	epoll_event event;
	int ret = epoll_wait(fEpollHandle, &event, 1, -1);
	if (ret < 0) {
		if (errno != EINTR) {
			internalError();
		}
	}

	ssize_t size;
	uint64_t exp;

	/*read epoll until -EAGAIN*/
	while (1) {
		size = read(event.data.fd, &exp, sizeof(uint64_t));
		if (size != sizeof(uint64_t)) {
			//fprintf(stderr, "not timerfd case\n");
		}

		if (size < 0) {
			//printf("read <0; %d\n", size);
			break;
		}
		//printf("read buffer()\n");
	}

	const HandlerDescriptor *handler = lookupHandlerDescriptor(event.data.fd);
	if (handler == NULL) { /*fixme:need better method to diff timerfd or tcp*/
		handler = static_cast<const HandlerDescriptor *>(event.data.ptr);
	}

	if (handler != NULL) {
		int resultConditionSet = 0;
		if (event.events & EPOLLIN)
			resultConditionSet |= SOCKET_READABLE;
		if (event.events & EPOLLOUT)
			resultConditionSet |= SOCKET_WRITABLE;
		if (event.events & EPOLLERR)
			resultConditionSet |= SOCKET_EXCEPTION;

		if ((resultConditionSet & handler->conditionSet) != 0) {
			(*handler->handlerProc)(handler->clientData, resultConditionSet);
		}
	}

	/*try release VmRss*/
	ret = malloc_trim(0);
	if (ret) {};


	// Also handle any newly-triggered event (Note that we do this *after* calling a socket handler,
	// in case the triggered event handler modifies The set of readable sockets.)
	if (fTriggersAwaitingHandling != 0) {
		if (fTriggersAwaitingHandling == fLastUsedTriggerMask) {
			// Common-case optimization for a single event trigger:
			fTriggersAwaitingHandling &= ~fLastUsedTriggerMask;
			if (fTriggeredEventHandlers[fLastUsedTriggerNum] != NULL) {
				(*fTriggeredEventHandlers[fLastUsedTriggerNum])(
				        fTriggeredEventClientDatas[fLastUsedTriggerNum]);
			}
		} else {
			// Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
			unsigned i = fLastUsedTriggerNum;
			EventTriggerId mask = fLastUsedTriggerMask;

			do {
				i = (i + 1) % MAX_NUM_EVENT_TRIGGERS;
				mask >>= 1;
				if (mask == 0)
					mask = 0x80000000;

				if ((fTriggersAwaitingHandling & mask) != 0) {
					fTriggersAwaitingHandling &= ~mask;
					if (fTriggeredEventHandlers[i] != NULL) {
						(*fTriggeredEventHandlers[i])(fTriggeredEventClientDatas[i]);
					}

					fLastUsedTriggerMask = mask;
					fLastUsedTriggerNum = i;
					break;
				}
			} while (i != fLastUsedTriggerNum);
		}
	}

}

void EpollTaskScheduler ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc *handlerProc,
                                                void *clientData)
{
	if (socketNum < 0)
		return;

	epoll_event ev;
	memset(&ev, 0, sizeof(ev));

	const HandlerDescriptor *handler = lookupHandlerDescriptor(socketNum);
	if (handler != NULL) {
		epoll_ctl(fEpollHandle, EPOLL_CTL_DEL, socketNum, &ev);
	}

	if (conditionSet == 0) {
		fHandlers->clearHandler(socketNum);
	} else {
		fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);

		handler = lookupHandlerDescriptor(socketNum);
		ev.data.ptr = const_cast<HandlerDescriptor *>(handler);

		if (conditionSet & SOCKET_READABLE)
			ev.events |= EPOLLIN;
		if (conditionSet & SOCKET_WRITABLE)
			ev.events |= EPOLLOUT;

		epoll_ctl(fEpollHandle, EPOLL_CTL_ADD, socketNum, &ev);
	}
}

void EpollTaskScheduler::moveSocketHandling(int oldSocketNum, int newSocketNum)
{
	if (oldSocketNum < 0 || newSocketNum < 0)
		return; // sanity check

	const HandlerDescriptor *handler = lookupHandlerDescriptor(oldSocketNum);
	if (handler == NULL) {
		return;
	}

	epoll_event ev;
	memset(&ev, 0, sizeof(ev));

	if (handler->conditionSet & SOCKET_READABLE)
		ev.events |= EPOLLIN;
	if (handler->conditionSet & SOCKET_WRITABLE)
		ev.events |= EPOLLOUT;

	epoll_ctl(fEpollHandle, EPOLL_CTL_DEL, oldSocketNum, &ev);
	epoll_ctl(fEpollHandle, EPOLL_CTL_ADD, newSocketNum, &ev);

	fHandlers->moveHandler(oldSocketNum, newSocketNum);
}

const HandlerDescriptor *EpollTaskScheduler::lookupHandlerDescriptor(int socketNum) const
{
	HandlerDescriptor *handler;
	HandlerIterator iter(*fHandlers);
	while ((handler = iter.next()) != NULL) {
		if (handler->socketNum == socketNum)
			break;
	}
	return handler;
}