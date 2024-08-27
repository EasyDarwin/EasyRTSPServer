/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// Usage Environment
// Implementation

#include "UsageEnvironment.hh"
#include "../../trace.h"

Boolean UsageEnvironment::reclaim() {
  // We delete ourselves only if we have no remainining state:
  if (liveMediaPriv == NULL && groupsockPriv == NULL) {
    delete this;
    return True;
  }

  return False;
}

UsageEnvironment::UsageEnvironment(TaskScheduler& scheduler, int _envirId, char *_envirName)
  : liveMediaPriv(NULL), groupsockPriv(NULL), fScheduler(scheduler) {

  envirId = _envirId;
  memset(envirName, 0x00, sizeof(envirName));
  strcpy(envirName, _envirName);
  envirCounter = 0;
  mLockFlag	=	0;
  streamStatus = 0;

  mTmpServerMediaSession	=	NULL;
  resetInternalChannel	=	0;
  clientReferenceCount = 0;

  memset(liveStreamName, 0x00, sizeof(liveStreamName));
  InitMutex(&osMutex);
}

UsageEnvironment::~UsageEnvironment() {
	DeinitMutex(&osMutex);
}

void	UsageEnvironment::LockEnvir(char *functionName, unsigned long long ptr)
{
	/*
	if (ptr > 0)
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"%s LockEnvir FunctionName[%s]  0x%X   Lock\n", envirName, functionName, ptr);
	}

	LockMutex(&osMutex);

	if (ptr > 0)
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"%s LockEnvir FunctionName[%s]  0x%X   Lock OK\n", envirName, functionName, ptr);
	}
	*/
}
void	UsageEnvironment::UnlockEnvir(char *functionName, unsigned long long ptr)
{
	/*
	if (ptr > 0)
		_TRACE(TRACE_LOG_DEBUG, (char *)"%s LockEnvir FunctionName[%s]  0x%X   Unlock\n", envirName, functionName, ptr);

	UnlockMutex(&osMutex);

	if (ptr > 0)
	_TRACE(TRACE_LOG_DEBUG, (char *)"%s LockEnvir FunctionName[%s]  0x%X   Unlock OK\n", envirName, functionName, ptr);
	*/
}

// By default, we handle 'should not occur'-type library errors by calling abort().  Subclasses can redefine this, if desired.
// (If your runtime library doesn't define the "abort()" function, then define your own (e.g., that does nothing).)
void UsageEnvironment::internalError() {
  abort();
}
int	UsageEnvironment::GetStreamStatus()			
{
	return streamStatus;
}
void	UsageEnvironment::SetStreamStatus(int _status)	
{
	streamStatus = _status;
}

TaskScheduler::TaskScheduler() {
}

TaskScheduler::~TaskScheduler() {
}

void TaskScheduler::rescheduleDelayedTask(TaskToken& task,
					  int64_t microseconds, TaskFunc* proc,
					  void* clientData) {
  unscheduleDelayedTask(task);
  task = scheduleDelayedTask(microseconds, proc, clientData);
}

// By default, we handle 'should not occur'-type library errors by calling abort().  Subclasses can redefine this, if desired.
void TaskScheduler::internalError() {
  abort();
}
