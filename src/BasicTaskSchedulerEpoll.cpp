
#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include <stdio.h>
#if defined(_QNX4)
#include <sys/select.h>
#include <unix.h>
#endif

#include "BasicTaskSchedulerEpoll.h"
#include "trace.h"
#ifndef _WIN32
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <time.h>


////////// BasicTaskSchedulerEpoll //////////

BasicTaskSchedulerEpoll* BasicTaskSchedulerEpoll::createNew(int id, int maxEventNum, unsigned maxSchedulerGranularity) {
	return new BasicTaskSchedulerEpoll(id, maxEventNum, maxSchedulerGranularity);
}

BasicTaskSchedulerEpoll::BasicTaskSchedulerEpoll(int id, int maxEventNum, unsigned maxSchedulerGranularity)
{
	SetId(id);
	mEpollMaxEventNum = maxEventNum;
    mEpollEventNum = 0;

	pEpoolEvents = new EPOLL_EVENT_T[mEpollMaxEventNum];
	memset(&pEpoolEvents[0], 0x00, sizeof(EPOLL_EVENT_T) * mEpollMaxEventNum);

	fEpollFd = epoll_create(mEpollMaxEventNum + 1);
	if (fEpollFd == -1)
	{
		_TRACE(TRACE_LOG_ERROR, (char *)"[0x%X] BasicTaskSchedulerEpoll epoll_create error: %s\n", this, strerror(errno));
	}
}

BasicTaskSchedulerEpoll::~BasicTaskSchedulerEpoll() {
	if (NULL != pEpoolEvents)
	{
		delete []pEpoolEvents;
		pEpoolEvents = NULL;
	}
}

#ifndef MILLION
#define MILLION 1000000
#endif

static int getIndexEventByFd(struct epoll_event *events, int fd, int eventMax)
{
  int i = -1;
  for (i = 0; i < eventMax; i ++){
	if (events[i].data.fd == fd){
		return i;
	}
  }
  return -1;
}


int BasicTaskSchedulerEpoll::SingleStep(unsigned maxDelayTime, int threadIdx, char *envirName)
{
	struct epoll_event events[128];
	int epollTimeout = 10;
	int eventIdx = -1;
  
	//memset(events, 0, BASIC_TS_EPOLL_EVENT_MAX * sizeof(struct epoll_event));
	DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();
	struct timeval tv_timeToDelay;
	tv_timeToDelay.tv_sec = timeToDelay.seconds();
	tv_timeToDelay.tv_usec = timeToDelay.useconds();
	// Very large "tv_sec" values cause select() to fail.
	// Don't make it any larger than 1 million seconds (11.5 days)
	const long MAX_TV_SEC = MILLION;
	if (tv_timeToDelay.tv_sec > MAX_TV_SEC) 
	{
		tv_timeToDelay.tv_sec = MAX_TV_SEC;
	}
	// Also check our "maxDelayTime" parameter (if it's > 0):
	if (maxDelayTime > 0 &&
	(tv_timeToDelay.tv_sec > (long)maxDelayTime/MILLION ||
	(tv_timeToDelay.tv_sec == (long)maxDelayTime/MILLION &&
	tv_timeToDelay.tv_usec > (long)maxDelayTime%MILLION))) 
	{
		tv_timeToDelay.tv_sec = maxDelayTime/MILLION;
		tv_timeToDelay.tv_usec = maxDelayTime%MILLION;
	}
  
  
	epollTimeout = 10;
	//epollTimeout = tv_timeToDelay.tv_sec * 1000 + tv_timeToDelay.tv_usec / 1000;
	/* Frist process the Delay Queue when time out is 0 */
	if (epollTimeout == 0)
	{
		fDelayQueue.handleAlarm();
		return 0;
	}
  
	//printf((char *)"[0x%X] msleep: %d\n", epollTimeout);
	//epollTimeout = 10;
  
	int ret = epoll_wait(fEpollFd, events, mEpollMaxEventNum, epollTimeout);
	if (ret < 0) 
	{
		usleep(1000);
		if (errno == EINTR)
		{
			return 0;
		}
        
		_TRACE(TRACE_LOG_ERROR, (char *)"[0x%X] BasicTaskSchedulerEpoll epoll_wait error: %s\n", this, strerror(errno));

		return 0;
	}
	else if (ret == 0)
	{
		usleep(1000);
		//printf("[0x%X]epoll_wait == 0\n", this);
		//return 0;
	}
  
  
	//printf("execute... fEpollFd:%d\n", fEpollFd);
  
	// Call the handler function for one readable socket:
	HandlerIterator iter(*fHandlers);
	HandlerDescriptor* handler;
	// To ensure forward progress through the handlers, begin past the last
	// socket number that we handled:
	if (fLastHandledSocketNum >= 0) 
	{
		while ((handler = iter.next()) != NULL) 
		{
			if (handler->socketNum == fLastHandledSocketNum) break;
		}
		if (handler == NULL) 
		{
			fLastHandledSocketNum = -1;
			iter.reset(); // start from the beginning instead
		}
	}

	while ((handler = iter.next()) != NULL) 
	{
		int sock = handler->socketNum; // alias
		int resultConditionSet = 0;
		eventIdx = getIndexEventByFd(events, sock, ret);
		if (eventIdx != -1){
		/*
		if (events[eventIdx].events & EPOLLET){
		printf("%s-----%d sock %d\n", __FILE__, __LINE__, sock);
		}*/
		if (events[eventIdx].events & EPOLLIN || events[eventIdx].events & EPOLLET)
		{
			resultConditionSet |= SOCKET_READABLE;
		}
		if (events[eventIdx].events & EPOLLOUT)
		{
			resultConditionSet |= SOCKET_WRITABLE;
		}
		if (events[eventIdx].events & EPOLLERR)
		{
			resultConditionSet |= SOCKET_EXCEPTION;
		}
		}
		if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) 
		{
			fLastHandledSocketNum = sock;
			// Note: we set "fLastHandledSocketNum" before calling the handler,
			// in case the handler calls "doEventLoop()" reentrantly.
			(*handler->handlerProc)(handler->clientData, resultConditionSet);
			break;
		}
	}
#if 1
	if (handler == NULL && fLastHandledSocketNum >= 0) 
	{
		// We didn't call a handler, but we didn't get to check all of them,
		// so try again from the beginning:
		iter.reset();
		while ((handler = iter.next()) != NULL) 
		{
			int sock = handler->socketNum; // alias
			int resultConditionSet = 0;
			eventIdx = getIndexEventByFd(events, sock, ret);
			if (eventIdx != -1)
			{
				if (events[eventIdx].events & EPOLLIN || events[eventIdx].events & EPOLLET)
				{
					resultConditionSet |= SOCKET_READABLE;
				}
				if (events[eventIdx].events & EPOLLOUT)
				{
					resultConditionSet |= SOCKET_WRITABLE;
				}
				if (events[eventIdx].events & EPOLLERR)
				{
					resultConditionSet |= SOCKET_EXCEPTION;
				}
			}
			if ((resultConditionSet&handler->conditionSet) != 0 && handler->handlerProc != NULL) 
			{
				fLastHandledSocketNum = sock;
				// Note: we set "fLastHandledSocketNum" before calling the handler,
				// in case the handler calls "doEventLoop()" reentrantly.
				(*handler->handlerProc)(handler->clientData, resultConditionSet);
				break;
			}
		}
		if (handler == NULL) fLastHandledSocketNum = -1;//because we didn't call a handler
	}
#endif

	// Also handle any newly-triggered event (Note that we do this *after* calling a socket handler,
	// in case the triggered event handler modifies The set of readable sockets.)
	if (fTriggersAwaitingHandling != 0) 
	{
		if (fTriggersAwaitingHandling == fLastUsedTriggerMask) 
		{
			// Common-case optimization for a single event trigger:
			fTriggersAwaitingHandling &=~ fLastUsedTriggerMask;
			if (fTriggeredEventHandlers[fLastUsedTriggerNum] != NULL) 
			{
				(*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
			}
		} 
		else 
		{
			// Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
			unsigned i = fLastUsedTriggerNum;
			EventTriggerId mask = fLastUsedTriggerMask;

			do {
				i = (i+1)%MAX_NUM_EVENT_TRIGGERS;
				mask >>= 1;
				if (mask == 0) mask = 0x80000000;

				if ((fTriggersAwaitingHandling&mask) != 0) 
				{
					fTriggersAwaitingHandling &=~ mask;
					if (fTriggeredEventHandlers[i] != NULL) 
					{
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
#if 0
void BasicTaskSchedulerEpoll
  ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData) {
  struct epoll_event ev;
  
  if (socketNum < 0 || fEpollFd == -1) return;
  
  memset(&ev, 0, sizeof(struct epoll_event));

 
  //read
  ev.data.fd = socketNum;
  ev.events = EPOLLIN;// | EPOLLOUT | EPOLLERR;
  if (-1 == epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev))
  {
      //printf("[0x%X]epoll_ctrl delete fail: %s\n", this, strerror(errno));
  }
  else
  {
      //printf("[0x%X]epoll_ctrl delete ok\n", this);
  }

  //write
  ev.data.fd = socketNum;
  ev.events = EPOLLOUT;
  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev);

  //error
  ev.data.fd = socketNum;
  ev.events = EPOLLERR;
  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev);  

  if (conditionSet == 0) {
    fHandlers->clearHandler(socketNum);
    
    //printf("[0x%X]setBackgroundHandling[epoll:%d]:   socketNum[%d]  clearHandler\n", this, fEpollFd, socketNum);
    
  } else 
  {
    fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);

    //printf("[0x%X]setBackgroundHandling[epoll:%d]:   socketNum[%d]  add\n", this, fEpollFd, socketNum);
    
#if 1
	ev.data.fd = socketNum;
    ev.events = 0x00;
    if (conditionSet&SOCKET_READABLE)       ev.events = EPOLLIN;
    if (conditionSet&SOCKET_WRITABLE)       ev.events |= EPOLLOUT;
    if (conditionSet&SOCKET_EXCEPTION)       ev.events |= EPOLLERR;

	if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1)
    {
		_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] BasicTaskSchedulerEpoll epollFd[%d] epoll_ctl  EPOLL_CTL_ADD SOCKET_READABLE error: %d  %s\n", 
						this, fEpollFd, errno, strerror(errno));
	}
#else
    if (conditionSet&SOCKET_READABLE){
	    ev.data.fd = socketNum;
		//ev.events = EPOLLIN | EPOLLET;
		ev.events = EPOLLIN;
		 if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			//printf("BasicTaskSchedulerEpoll epoll_ctl  EPOLL_CTL_ADD SOCKET_READABLE error\n");
		 }
	}
    if (conditionSet&SOCKET_WRITABLE){
	    ev.data.fd = socketNum;
		ev.events = EPOLLOUT;
		 if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			//printf("BasicTaskSchedulerEpoll epoll_ctl  EPOLL_CTL_ADD SOCKET_WRITABLE error\n");
		 }
	}
    if (conditionSet&SOCKET_EXCEPTION){
	    ev.data.fd = socketNum;
		ev.events = EPOLLERR;
		 if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1){
			//printf("BasicTaskSchedulerEpoll epoll_ctl  EPOLL_CTL_ADD SOCKET_EXCEPTION error: %d   %s\n", 
                         //       errno, strerror(errno));
		 }
	}
#endif        
  }
  

  //printf("[0x%X]setBackgroundHandling:   socketNum[%d] OK\n", this, socketNum);
}
#endif

void BasicTaskSchedulerEpoll::moveSocketHandling(int oldSocketNum, int newSocketNum) {
  struct epoll_event ev;
  
  if (oldSocketNum < 0 || newSocketNum < 0 || fEpollFd == -1) return; // sanity check
  
  //printf("moveSocketHandling...    [%d] --> [%d]\n", oldSocketNum, newSocketNum);

  memset(&ev, 0, sizeof(struct epoll_event));
  epoll_ctl(fEpollFd, EPOLL_CTL_DEL, oldSocketNum, &ev);
  ev.data.fd = newSocketNum;
  ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
  epoll_ctl(fEpollFd, EPOLL_CTL_ADD, newSocketNum, &ev);
  fHandlers->moveHandler(oldSocketNum, newSocketNum);
}

int		BasicTaskSchedulerEpoll::epoll_event_set(EPOLL_EVENT_T *ev, int fd, void *arg)
{
    ev->fd = fd;  
    //ev->call_back = call_back;  
    ev->events = 0;  
    //ev->arg = arg;  
    ev->status = 0;
    ev->last_active = time(NULL);  
    
    return 0;
}

int		BasicTaskSchedulerEpoll::epoll_event_add(int epollFd, int events, EPOLL_EVENT_T *ev)
{
    struct epoll_event epv = {0, {0}};  
    int op; 

    epv.data.ptr = ev;  
    epv.events = ev->events = events;  

    if(ev->status == 1)       op = EPOLL_CTL_MOD;  
    else
	{
        op = EPOLL_CTL_ADD;  
        ev->status = 1;  
    }  

    if(epoll_ctl(epollFd, op, ev->fd, &epv) < 0)
	{
        printf("Event Add failed[fd=%d], evnets[%d]\n", ev->fd, events);
	}
    else 
	{
        printf("Event Add OK[fd=%d], op=%d, evnets[%0X]\n", ev->fd, op, events); 
	}
    
    
    return 0;
}

int		BasicTaskSchedulerEpoll::epoll_event_del(int epollFd, EPOLL_EVENT_T *ev)
{
    struct epoll_event epv = {0, {0}};  

    if(ev->status != 1) return 0;

    epv.data.ptr = ev;
    ev->status = 0;

    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, ev->fd, &epv) < 0)
	{
		printf("epoll_event_del fail. [fd=%d]\n", ev->fd); 
	}
    return 0;
}


#if 1
int BasicTaskSchedulerEpoll
  ::setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData) 
{
	int ret = 0;
	struct epoll_event ev;
	memset(&ev, 0, sizeof(struct epoll_event));
  
	if (socketNum < 0 || fEpollFd == -1) return 0;

	ev.data.fd = socketNum;
	if (conditionSet == 0)
	{
#if 0
		if (-1 == epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev))
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] setBackgroundHandling epoll_ctrl delete fail: %s\n", this, strerror(errno));
		}
#else

		for (int i=0; i<mEpollMaxEventNum; i++)
		{
			if (pEpoolEvents[i].fd == socketNum)
			{
				ev.events = pEpoolEvents[i].events;

				if (-1 == epoll_ctl(fEpollFd, EPOLL_CTL_DEL, socketNum, &ev))
				{
					_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] setBackgroundHandling epollFd[%d] epoll_ctrl delete fail: %s\n", 
						this, fEpollFd, strerror(errno));
				}
                else
                {
                    pEpoolEvents[i].fd = 0;
                    pEpoolEvents[i].events = 0x00;
                    if (mEpollEventNum>0)   mEpollEventNum--;
                }

				break;
			}
		}
#endif
		fHandlers->clearHandler(socketNum);
	}
	else
	{
		fHandlers->assignHandler(socketNum, conditionSet, handlerProc, clientData);

        int  iFreeIdx = -1;
		bool exist = false;
		for (int i=0; i<mEpollMaxEventNum; i++)
		{
            if (pEpoolEvents[i].fd == 0 && iFreeIdx<0)
            {
                iFreeIdx = i;
            }
            else if (pEpoolEvents[i].fd == socketNum)
            {
				exist = true;
				break;
            }
		}

		if ((!exist) && (iFreeIdx>=0) )
		{
			ev.events = 0x00;
			if (conditionSet&SOCKET_READABLE)       ev.events = EPOLLIN;
			if (conditionSet&SOCKET_WRITABLE)       ev.events |= EPOLLOUT;
			if (conditionSet&SOCKET_EXCEPTION)       ev.events |= EPOLLERR;

			if (epoll_ctl(fEpollFd, EPOLL_CTL_ADD, socketNum, &ev) == -1)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] BasicTaskSchedulerEpoll epoll_ctl epollFd[%d] EPOLL_CTL_ADD SOCKET_READABLE error: %s\n",
												this, fEpollFd, strerror(errno));
			}
            else
            {
                pEpoolEvents[iFreeIdx].fd = socketNum;
				pEpoolEvents[iFreeIdx].events = ev.events;
                mEpollEventNum++;

				//_TRACE(TRACE_LOG_DEBUG, (char *)"[0x%X] Add socket to worker thread[%d] epollFd[%d] freeIndex[%d] socketNum[%d]\n",
					//							this, GetId(), fEpollFd, iFreeIdx, socketNum);
            }
		}
        else if ( (!exist) && (iFreeIdx<1) )
        {
            printf("ERROR ERROR ERROR...  epollFd[%d] socket[%d]\n", fEpollFd, socketNum);
			ret = -1;
        }
	}

	return ret;
}
#endif

#endif