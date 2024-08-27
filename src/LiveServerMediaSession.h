#ifndef _LIVE_SERVER_MEDIA_SESSION_H
#define _LIVE_SERVER_MEDIA_SESSION_H

#include <ServerMediaSession.hh>


class LiveServerMediaSession: public ServerMediaSession 
{
public:
  static LiveServerMediaSession* createNew(UsageEnvironment& env,
				       char const* streamName = NULL,
				       char const* info = NULL,
				       char const* description = NULL,
				       Boolean isSSM = False,
				       char const* miscSDPLines = NULL,
					   void	*liveChannel=NULL, void **callback=NULL, void *userptr=NULL);

protected: // we're a virtual base class
  LiveServerMediaSession(UsageEnvironment& env,
				       char const* streamName = NULL,
				       char const* info = NULL,
				       char const* description = NULL,
				       Boolean isSSM = False,
				       char const* miscSDPLines = NULL,
					   void	*liveChannel=NULL, void **callback=NULL, void *userptr=NULL);
  virtual ~LiveServerMediaSession();


  void		*channelPtr;
  void		**callbackPtr;
  void		*userPtr;
};

#endif
