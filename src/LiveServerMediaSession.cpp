#include "LiveServerMediaSession.h"
#include "LiveRtspServer.h"
#include "trace.h"

LiveServerMediaSession* LiveServerMediaSession::createNew(UsageEnvironment& env,    char const* streamName, char const* info,
	    char const* description, Boolean isSSM, char const* miscSDPLines, void	*liveChannel, void **callback, void *userptr) 
{
  return new LiveServerMediaSession(env, streamName, info, description,
				isSSM, miscSDPLines, liveChannel, callback, userptr);
}

LiveServerMediaSession::LiveServerMediaSession(UsageEnvironment& env,
				       char const* streamName,
				       char const* info,
				       char const* description,
				       Boolean isSSM, char const* miscSDPLines,
					   void	*liveChannel, void **callback, void *userptr)
  : ServerMediaSession(env, streamName, info, description, isSSM, miscSDPLines) 
{
	channelPtr	=	liveChannel;
	callbackPtr		=	callback;
	userPtr			= userptr;
}

LiveServerMediaSession::~LiveServerMediaSession() 
{
	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )*callbackPtr;
	if (NULL == pCallback)
	{
		//_TRACE(TRACE_LOG_ERROR, "Callback not be set.\n");
		//return;
	}

	_TRACE(TRACE_LOG_INFO, (char *)"LiveServerMediaSession::~LiveServerMediaSession()  %s\n", streamName());

	if (NULL != channelPtr)
	{
		LIVE_CHANNEL_OBJ_T	*pChannel = (LIVE_CHANNEL_OBJ_T *)channelPtr;
		if (NULL != pChannel && pChannel->status == LIVE_FLAG)
		{
			if (NULL != pChannel->liveSource && pChannel->liveSource->GetFlag()==LIVE_FLAG)
			{
				Medium::close(pChannel->liveSource, pChannel->liveSource->GetEnvironment());
			}
			pChannel->liveSource = NULL;

			if (NULL != pCallback)
			{
				envir().SetEnvirCount(-1);

				pCallback(RTSP_CHANNEL_STOP_STREAM, pChannel->streamName, (RTSP_CHANNEL_HANDLE*)&pChannel, &pChannel->mediainfo, NULL, userPtr, pChannel->channelUserPtr);
				pCallback(RTSP_CHANNEL_CLOSE_STREAM, pChannel->streamName, (RTSP_CHANNEL_HANDLE*)&pChannel, &pChannel->mediainfo, NULL, userPtr, pChannel->channelUserPtr);

				envir().SetEnvirCount(-1);
#if 1
				envir().ResetInternalChannel(10, NULL);
#else
				BasicTaskScheduler0 *pTaskScheduler0 = NULL;
				if (pTaskScheduler0 = (BasicTaskScheduler0 *)&envir().taskScheduler())
				{
					if (NULL != pTaskScheduler0)
					{
						pTaskScheduler0->ClearDelayQueue();
					}
				}
#endif
			}
		}

		channelPtr = NULL;		//2019.12.02
	}
}
