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

#ifndef _BASIC_USAGE_ENVIRONMENT0_HH
#define _BASIC_USAGE_ENVIRONMENT0_HH

#ifndef _BASICUSAGEENVIRONMENT_VERSION_HH
#include "BasicUsageEnvironment_version.hh"
#endif

#ifndef _USAGE_ENVIRONMENT_HH
#include "UsageEnvironment.hh"
#endif

#ifndef _DELAY_QUEUE_HH
#include "DelayQueue.hh"
#endif

#include "mpi_limits.h"

#define RESULT_MSG_BUFFER_MAX 1000

// An abstract base class, useful for subclassing
// (e.g., to redefine the implementation of "operator<<")
class BasicUsageEnvironment0 : public UsageEnvironment {
    public:
	// redefined virtual functions:
	virtual MsgString getResultMsg() const;

	virtual void setResultMsg(MsgString msg);
	virtual void setResultMsg(MsgString msg1, MsgString msg2);
	virtual void setResultMsg(MsgString msg1, MsgString msg2, MsgString msg3);
	virtual void setResultErrMsg(MsgString msg, int err = 0);

	virtual void appendToResultMsg(MsgString msg);

	virtual void reportBackgroundError();

    protected:
	BasicUsageEnvironment0(TaskScheduler &taskScheduler);
	virtual ~BasicUsageEnvironment0();

    private:
	void reset();

	char fResultMsgBuffer[RESULT_MSG_BUFFER_MAX];
	unsigned fCurBufferSize;
	unsigned fBufferMaxSize;
};

class HandlerSet; // forward

#define MAX_NUM_EVENT_TRIGGERS 32

// An abstract base class, useful for subclassing
// (e.g., to redefine the implementation of socket event handling)
class BasicTaskScheduler0 : public TaskScheduler {
    public:
	virtual ~BasicTaskScheduler0();

	virtual void SingleStep(unsigned maxDelayTime = 0) = 0;
	// "maxDelayTime" is in microseconds.  It allows a subclass to impose a limit
	// on how long "select()" can delay, in case it wants to also do polling.
	// 0 (the default value) means: There's no maximum; just look at the delay
	// queue

    public:
	// Redefined virtual functions:
	virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc *proc, void *clientData);
	virtual void unscheduleDelayedTask(TaskToken &prevTask);

	virtual void doEventLoop(char *watchVariable);

	virtual EventTriggerId createEventTrigger(TaskFunc *eventHandlerProc);
	virtual void deleteEventTrigger(EventTriggerId eventTriggerId);
	virtual void triggerEvent(EventTriggerId eventTriggerId, void *clientData = NULL);

    protected:
	BasicTaskScheduler0();

    protected:
	// To implement delayed operations:
	DelayQueue fDelayQueue;

	// To implement background reads:
	HandlerSet *fHandlers;
	int fLastHandledSocketNum;

	// To implement event triggers:
	EventTriggerId fTriggersAwaitingHandling,
	        fLastUsedTriggerMask; // implemented as 32-bit bitmaps
	TaskFunc *fTriggeredEventHandlers[MAX_NUM_EVENT_TRIGGERS];
	void *fTriggeredEventClientDatas[MAX_NUM_EVENT_TRIGGERS];
	unsigned fLastUsedTriggerNum; // in the range [0,MAX_NUM_EVENT_TRIGGERS)
};

enum Mode { DftMode = 0, ManualMode };
#define MAX_STREAM_CNT (MPI_MAX_ENC_CHN_NUM * 2) /*Augentix has max video chn * (audio + video)*/
typedef struct {
	Mode launchMode;
	int streamCount;
	int portNum;
	char channels[MAX_STREAM_CNT];
	char Urls[MAX_STREAM_CNT][32];
	void *sms[MAX_STREAM_CNT];
	int chnRestartTrigger;
	bool isBlocking;
} RtspServerConf;

#endif
