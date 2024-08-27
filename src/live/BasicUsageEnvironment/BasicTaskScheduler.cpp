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
// Basic Usage Environment: for a simple, non-scripted, console application
// Implementation


#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include <stdio.h>
#if defined(_QNX4)
#include <sys/select.h>
#include <unix.h>
#endif

#include <time.h>
#include "../../trace.h"
#include "../../live_ipv4_ipv6.h"
////////// BasicTaskScheduler //////////

BasicTaskScheduler* BasicTaskScheduler::createNew(int id, int maxEventNum, unsigned maxSchedulerGranularity) {
	return new BasicTaskScheduler(id, maxEventNum, maxSchedulerGranularity);
}

BasicTaskScheduler::BasicTaskScheduler(int id, int maxEventNum, unsigned maxSchedulerGranularity)
  : fMaxSchedulerGranularity(maxSchedulerGranularity), fMaxNumSockets(0)
#if defined(__WIN32__) || defined(_WIN32)
  , fDummySocketNum(-1)
#endif
{
  FD_ZERO(&fReadSet);
  FD_ZERO(&fWriteSet);
  FD_ZERO(&fExceptionSet);

  SetId(id);

  if (maxSchedulerGranularity > 0) schedulerTickTask(); // ensures that we handle events frequently
}

BasicTaskScheduler::~BasicTaskScheduler() {
#if defined(__WIN32__) || defined(_WIN32)
  if (fDummySocketNum >= 0) closeSocket(fDummySocketNum);
#endif
}

void BasicTaskScheduler::schedulerTickTask(void* clientData) {
  ((BasicTaskScheduler*)clientData)->schedulerTickTask();
}

void BasicTaskScheduler::schedulerTickTask() {
  scheduleDelayedTask(fMaxSchedulerGranularity, schedulerTickTask, this);
}

#ifndef MILLION
#define MILLION 1000000
#endif

int BasicTaskScheduler::SingleStep(unsigned maxDelayTime, int threadIdx, char *envirName) {
  fd_set readSet = fReadSet; // make a copy for this select() call
  fd_set writeSet = fWriteSet; // ditto
  fd_set exceptionSet = fExceptionSet; // ditto

  DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();
  struct timeval tv_timeToDelay;
  tv_timeToDelay.tv_sec = timeToDelay.seconds();
  tv_timeToDelay.tv_usec = timeToDelay.useconds();
  // Very large "tv_sec" values cause select() to fail.
  // Don't make it any larger than 1 million seconds (11.5 days)
  const long MAX_TV_SEC = MILLION;
  if (tv_timeToDelay.tv_sec > MAX_TV_SEC) {
    tv_timeToDelay.tv_sec = MAX_TV_SEC;
  }
  // Also check our "maxDelayTime" parameter (if it's > 0):
  if (maxDelayTime > 0 &&
      (tv_timeToDelay.tv_sec > (long)maxDelayTime/MILLION ||
       (tv_timeToDelay.tv_sec == (long)maxDelayTime/MILLION &&
	tv_timeToDelay.tv_usec > (long)maxDelayTime%MILLION))) {
    tv_timeToDelay.tv_sec = maxDelayTime/MILLION;
    tv_timeToDelay.tv_usec = maxDelayTime%MILLION;
  }

  //if (readSet.fd_count < 1 && writeSet.fd_count<1 && exceptionSet.fd_count<1)
  if (fMaxNumSockets < 1)
  {
	  Sleep(1);
	  return 0;
  }

#ifdef _DEBUG1
	unsigned int uiTime = (unsigned int)time(NULL);
	if (m_uiLastTime != uiTime)
	{
		m_uiLastTime = uiTime;

		if (threadIdx < MAIN_THREAD_ID)		//main thread idx is 1000, other is worker thread
		{
			if (readSet.fd_count>0 || writeSet.fd_count>0 || exceptionSet.fd_count>0)
			{
				if (NULL != envirName)
				{
					_TRACE(TRACE_LOG_DEBUG, (char*)"ThreadIdx[%d] Name[%s]. fd_count[%d - %d - %d]  \n", threadIdx, envirName, readSet.fd_count, writeSet.fd_count, exceptionSet.fd_count);


#ifdef _DEBUG
  if (readSet.fd_count > 0)
  {
	  char szLog[1024] = {0};
	  char *pLog = (char *)szLog;

	  pLog += sprintf(pLog, "Thread[%s]  socket列表------\n", envirName);
	  for (u_int i=0; i<readSet.fd_count; i++)
	  {
		  pLog += sprintf(pLog, "%d---\n", readSet.fd_array[i]);
	  }
	  _TRACE(TRACE_LOG_DEBUG, szLog);
  }
#endif

				}
				else
				{
					_TRACE(TRACE_LOG_DEBUG, (char*)"ThreadIdx[%d] Name is NULL. fd_count[%d - %d - %d]\n", threadIdx, readSet.fd_count, writeSet.fd_count, exceptionSet.fd_count);
				}
			}	
		  }
		  else if (threadIdx == MAIN_THREAD_ID)
		  {
			  //_TRACE(TRACE_LOG_DEBUG, "Main Thread[%d] Name[%s]. fd_count[%d - %d - %d]  \n", threadIdx, envirName, readSet.fd_count, writeSet.fd_count, exceptionSet.fd_count);
		  }

#ifdef _DEBUG1
  char szTmp[1024] = {0};
  char *pTmp = (char *)szTmp;
  for (int i=0; i<readSet.fd_count; i++)
  {
	  pTmp += sprintf(pTmp, "%d\n", readSet.fd_array[i]);
  }
  _TRACE(TRACE_LOG_DEBUG, (char *)"[%s]BasicTaskScheduler[0x%X] readSet列表[%d - %d - %d]===\n%s\n", 
					envirName, this, readSet.fd_count, writeSet.fd_count, exceptionSet.fd_count, szTmp);
#endif

	}


#endif


    //tv_timeToDelay.tv_sec = 0;
    //tv_timeToDelay.tv_usec = 1000;

	if (tv_timeToDelay.tv_sec > 0 || tv_timeToDelay.tv_usec>=1000000)
	{
		//_TRACE(TRACE_LOG_DEBUG, (char *)"tv_timeToDelay.tv_sec: %d\n", tv_timeToDelay.tv_sec);
		tv_timeToDelay.tv_sec = 0;
		tv_timeToDelay.tv_usec = 9999;
	}

  int selectResult = select(fMaxNumSockets, &readSet, &writeSet, &exceptionSet, &tv_timeToDelay);
  if (selectResult < 0) {
#if defined(__WIN32__) || defined(_WIN32)
    int err = WSAGetLastError();
    // For some unknown reason, select() in Windoze sometimes fails with WSAEINVAL if
    // it was called with no entries set in "readSet".  If this happens, ignore it:
    if (err == WSAEINVAL && readSet.fd_count == 0) {
      err = EINTR;
#if 0
      // To stop this from happening again, create a dummy socket:
      if (fDummySocketNum >= 0) closeSocket(fDummySocketNum);
      fDummySocketNum = socket(AF_INET, SOCK_DGRAM, 0);
      FD_SET((unsigned)fDummySocketNum, &fReadSet);
#else
	  Sleep(1);
	  return 0;
#endif


    }
    if (err != EINTR) {
#else
    if (errno != EINTR && errno != EAGAIN) {
#endif
	// Unexpected error - treat this as fatal:
#if !defined(_WIN32_WCE)
	perror("BasicTaskScheduler::SingleStep(): select() fails");
	// Because this failure is often "Bad file descriptor" - which is caused by an invalid socket number (i.e., a socket number
	// that had already been closed) being used in "select()" - we print out the sockets that were being used in "select()",
	// to assist in debugging:
	fprintf(stderr, "socket numbers used in the select() call:");

#ifdef _WIN32
	for (int i=0; i<fReadSet.fd_count; i++)
	{
		fprintf(stderr, " (%d)", fReadSet.fd_array[i]);
		disableBackgroundHandling(fReadSet.fd_array[i]);
		closeSocket(fReadSet.fd_array[i]);
	}
	for (int i=0; i<fWriteSet.fd_count; i++)
	{
		fprintf(stderr, " (%d)", fWriteSet.fd_array[i]);
		disableBackgroundHandling(fWriteSet.fd_array[i]);
		closeSocket(fWriteSet.fd_array[i]);
	}
	for (int i=0; i<fExceptionSet.fd_count; i++)
	{
		fprintf(stderr, " (%d)", fExceptionSet.fd_array[i]);
		disableBackgroundHandling(fExceptionSet.fd_array[i]);
		closeSocket(fExceptionSet.fd_array[i]);
	}
#else
	for (int i=1; i<fMaxNumSockets; i++)
	{
	  if (FD_ISSET(i, &fReadSet) || FD_ISSET(i, &fWriteSet) || FD_ISSET(i, &fExceptionSet)) {
	    fprintf(stderr, " %d(", i);
	    if (FD_ISSET(i, &fReadSet))
		{
			fprintf(stderr, "r");
			disableBackgroundHandling(i);
		}
	    if (FD_ISSET(i, &fWriteSet))
		{
			fprintf(stderr, "w");
			disableBackgroundHandling(i);
		}
	    if (FD_ISSET(i, &fExceptionSet))
		{
			fprintf(stderr, "e");
			disableBackgroundHandling(i);
		}
		closeSocket(i);
	    fprintf(stderr, ")");
		/*
#ifdef _WIN32
		closesocket(i);
#else
		close(i);
#endif
		*/
	  }
	}
#endif
	fprintf(stderr, "\n");
#endif

	_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] select 失败.\n", envirName);
	//return 0;	//2018.03.29
	return -1;

	internalError();
      }
  }


  // Call the handler function for one readable socket:
  HandlerIterator iter(*fHandlers);
  HandlerDescriptor* handler;
  // To ensure forward progress through the handlers, begin past the last
  // socket number that we handled:
  if (fLastHandledSocketNum >= 0) {
    while ((handler = iter.next()) != NULL) {
      if (handler->socketNum == fLastHandledSocketNum) break;
    }
    if (handler == NULL) {
      fLastHandledSocketNum = -1;
      iter.reset(); // start from the beginning instead
    }
  }
  while ((handler = iter.next()) != NULL) {
    int sock = handler->socketNum; // alias
#ifdef _DEBUG1
	_TRACE(TRACE_LOG_DEBUG, (char*)"*********  handler.  socket[%d]\n", handler->socketNum);
#endif
    int resultConditionSet = 0;
    if (FD_ISSET(sock, &readSet) && FD_ISSET(sock, &fReadSet)/*sanity check*/) resultConditionSet |= SOCKET_READABLE;
    if (FD_ISSET(sock, &writeSet) && FD_ISSET(sock, &fWriteSet)/*sanity check*/) resultConditionSet |= SOCKET_WRITABLE;
    if (FD_ISSET(sock, &exceptionSet) && FD_ISSET(sock, &fExceptionSet)/*sanity check*/) resultConditionSet |= SOCKET_EXCEPTION;
    if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) {
      fLastHandledSocketNum = sock;
          // Note: we set "fLastHandledSocketNum" before calling the handler,
          // in case the handler calls "doEventLoop()" reentrantly.
      (*handler->handlerProc)(handler->clientData, resultConditionSet);
      break;
    }
  }
  if (handler == NULL && fLastHandledSocketNum >= 0) {
    // We didn't call a handler, but we didn't get to check all of them,
    // so try again from the beginning:
    iter.reset();
    while ((handler = iter.next()) != NULL) {

#ifdef _DEBUG1
	_TRACE(TRACE_LOG_DEBUG, (char*)"*********  handler222.  socket[%d]  [%s]\n", handler->socketNum, envirName);
#endif

      int sock = handler->socketNum; // alias
      int resultConditionSet = 0;
      if (FD_ISSET(sock, &readSet) && FD_ISSET(sock, &fReadSet)/*sanity check*/) resultConditionSet |= SOCKET_READABLE;
      if (FD_ISSET(sock, &writeSet) && FD_ISSET(sock, &fWriteSet)/*sanity check*/) resultConditionSet |= SOCKET_WRITABLE;
      if (FD_ISSET(sock, &exceptionSet) && FD_ISSET(sock, &fExceptionSet)/*sanity check*/) resultConditionSet |= SOCKET_EXCEPTION;
      if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) {
	fLastHandledSocketNum = sock;
	    // Note: we set "fLastHandledSocketNum" before calling the handler,
            // in case the handler calls "doEventLoop()" reentrantly.
	(*handler->handlerProc)(handler->clientData, resultConditionSet);
	break;
      }
    }
    if (handler == NULL) fLastHandledSocketNum = -1;//because we didn't call a handler
  }

  // Also handle any newly-triggered event (Note that we do this *after* calling a socket handler,
  // in case the triggered event handler modifies The set of readable sockets.)
  if (fTriggersAwaitingHandling != 0) {
    if (fTriggersAwaitingHandling == fLastUsedTriggerMask) {
      // Common-case optimization for a single event trigger:
      fTriggersAwaitingHandling &=~ fLastUsedTriggerMask;
      if (fTriggeredEventHandlers[fLastUsedTriggerNum] != NULL) {
	(*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
      }
    } else {
      // Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
      unsigned i = fLastUsedTriggerNum;
      EventTriggerId mask = fLastUsedTriggerMask;

      do {
	i = (i+1)%MAX_NUM_EVENT_TRIGGERS;
	mask >>= 1;
	if (mask == 0) mask = 0x80000000;

	if ((fTriggersAwaitingHandling&mask) != 0) {
	  fTriggersAwaitingHandling &=~ mask;
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

  // Also handle any delayed event that may have come due.
  fDelayQueue.handleAlarm();

  return 0;
}

int BasicTaskScheduler
  ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData) {
  if (socketNum < 0) return 0;
#if !defined(__WIN32__) && !defined(_WIN32) && defined(FD_SETSIZE)
  if (socketNum >= (int)(FD_SETSIZE)) return -1;
#endif

#ifdef _DEBUG1
  if ( (conditionSet==0x00) && (! FD_ISSET(socketNum, &fReadSet)) &&
	  (! FD_ISSET(socketNum, &fWriteSet)) && (! FD_ISSET(socketNum, &fExceptionSet)))
  {
	  _TRACE(TRACE_LOG_DEBUG, (char*)"该Socket[%d]不存在于当前列表中.\n", socketNum);
  }

#endif


  FD_CLR((unsigned)socketNum, &fReadSet);
  FD_CLR((unsigned)socketNum, &fWriteSet);
  FD_CLR((unsigned)socketNum, &fExceptionSet);
  if (conditionSet == 0) {
    fHandlers->clearHandler(socketNum);
    if (socketNum+1 == fMaxNumSockets) {
      --fMaxNumSockets;
    }
  } else {
    fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);
    if (socketNum+1 > fMaxNumSockets) {
      fMaxNumSockets = socketNum+1;
    }
    if (conditionSet&SOCKET_READABLE) FD_SET((unsigned)socketNum, &fReadSet);
    if (conditionSet&SOCKET_WRITABLE) FD_SET((unsigned)socketNum, &fWriteSet);
    if (conditionSet&SOCKET_EXCEPTION) FD_SET((unsigned)socketNum, &fExceptionSet);


	_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] Add socket to thread[%d] socketNum[%d]\n",
									this, GetId(), socketNum);
  }

#ifdef _DEBUG1
  //envirName
  char szLog[1024] = {0};
  char *pLog = (char *)szLog;
  pLog += sprintf(pLog, "--------------------------------\n");
  for (int i=0; i< fReadSet.fd_count; i++)
  {
	  pLog += sprintf(pLog, "socket[%d]\n", fReadSet.fd_array[i]);
  }
  _TRACE(TRACE_LOG_DEBUG, szLog);

#endif

  return 0;
}

void BasicTaskScheduler::moveSocketHandling(int oldSocketNum, int newSocketNum) {
  if (oldSocketNum < 0 || newSocketNum < 0) return; // sanity check
#if !defined(__WIN32__) && !defined(_WIN32) && defined(FD_SETSIZE)
  if (oldSocketNum >= (int)(FD_SETSIZE) || newSocketNum >= (int)(FD_SETSIZE)) return; // sanity check
#endif
  if (FD_ISSET(oldSocketNum, &fReadSet)) {FD_CLR((unsigned)oldSocketNum, &fReadSet); FD_SET((unsigned)newSocketNum, &fReadSet);}
  if (FD_ISSET(oldSocketNum, &fWriteSet)) {FD_CLR((unsigned)oldSocketNum, &fWriteSet); FD_SET((unsigned)newSocketNum, &fWriteSet);}
  if (FD_ISSET(oldSocketNum, &fExceptionSet)) {FD_CLR((unsigned)oldSocketNum, &fExceptionSet); FD_SET((unsigned)newSocketNum, &fExceptionSet);}
  fHandlers->moveHandler(oldSocketNum, newSocketNum);

  if (oldSocketNum+1 == fMaxNumSockets) {
    --fMaxNumSockets;
  }
  if (newSocketNum+1 > fMaxNumSockets) {
    fMaxNumSockets = newSocketNum+1;
  }
}
