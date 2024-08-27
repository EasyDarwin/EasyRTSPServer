#ifndef _BASIC_TASK_SCHEDULER_EPOLL_HH
#define _BASIC_TASK_SCHEDULER_EPOLL_HH

#ifndef _BASIC_USAGE_ENVIRONMENT0_HH
#include "BasicUsageEnvironment0.hh"
#endif



typedef struct __EPOLL_EVENT_T
{
	int		fd;
	int		events;
	int		status;
	long last_active;
}EPOLL_EVENT_T;


class BasicTaskSchedulerEpoll: public BasicTaskScheduler0 {
public:
  static BasicTaskSchedulerEpoll* createNew(int id, int maxEventNum, unsigned maxSchedulerGranularity = 10000/*microseconds*/);
  virtual ~BasicTaskSchedulerEpoll();

  
  virtual int SingleStep(unsigned maxDelayTime, int threadIdx, char *envirName);
protected:
  BasicTaskSchedulerEpoll(int id, int maxEventNum, unsigned maxSchedulerGranularity);
      // called only by "createNew()"

protected:
  // Redefined virtual functions:
  

  virtual int setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData);
  virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

  int		epoll_event_set(EPOLL_EVENT_T *ev, int fd, void *arg);
  int		epoll_event_add(int epollFd, int events, EPOLL_EVENT_T *ev);
  int		epoll_event_del(int epollFd, EPOLL_EVENT_T *ev);


protected:
  // To implement background operations:
  int fEpollFd;
  int mEpollMaxEventNum;
  int mEpollEventNum;

  EPOLL_EVENT_T		*pEpoolEvents;
};


#endif