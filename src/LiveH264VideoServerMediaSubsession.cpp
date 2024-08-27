
#include "LiveH264VideoServerMediaSubsession.h"
#include <H264VideoRTPSink.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <H264VideoStreamFramer.hh>


LiveH264VideoServerMediaSubsession* LiveH264VideoServerMediaSubsession::createNew(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *LiveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort) 
{
  return new LiveH264VideoServerMediaSubsession(env, estimatedBitrate, pMediaInfo, LiveSource, streamName, mediaTypeName, reuseFirstSource, defaultPort);
}

LiveH264VideoServerMediaSubsession::LiveH264VideoServerMediaSubsession(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *LiveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort)
  : LiveServerMediaSubsession(env, LiveSource, streamName, mediaTypeName, estimatedBitrate, pMediaInfo, reuseFirstSource, MEDIA_TYPE_VIDEO, defaultPort) 
{
	  mCounter	=	0;
}

LiveH264VideoServerMediaSubsession::~LiveH264VideoServerMediaSubsession() 
{
	if (NULL != fLiveSource)
	{
		//Medium::close(fLiveSource, &envir());
		//fLiveSource = NULL;
	}
}

static void afterPlayingDummy(void* clientData) 
{
  LiveH264VideoServerMediaSubsession* subsess   = (LiveH264VideoServerMediaSubsession*)clientData;
  // Signal the event loop that we're done:
  subsess->setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) 
{
  LiveH264VideoServerMediaSubsession* subsess    = (LiveH264VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void LiveH264VideoServerMediaSubsession::checkForAuxSDPLine1() 
{
  if (fDummyRTPSink->auxSDPLine() != NULL) 
  {
    // Signal the event loop that we're done:
    setDoneFlag();
  } 
  else 
  {
	  if (GetCounter() < 5)
	  {
		  IncreaseCount();

		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
					  (TaskFunc*)checkForAuxSDPLine, this);
	  }
	  else
	  {
		  //如果5次都失败,则退出循环
		  setDoneFlag();
	  }
  }
}

char const* LiveH264VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) 
{
  // Note: For H264 video buffer, the 'sps' information isn't known
  // until we start reading the Buffer.  This means that "rtpSink"s
  // "auxSDPLine()" will be NULL initially, and we need to start reading
  // data from our buffer until this changes.
  fDummyRTPSink = rtpSink;
    
  // Start reading the buffer:
  fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
   
  fDoneFlag = 0;

  // Check whether the sink's 'auxSDPLine()' is ready:
  checkForAuxSDPLine(this);

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  char const* auxSDPLine = fDummyRTPSink->auxSDPLine();
  return auxSDPLine;
  //return NULL;
}

FramedSource* LiveH264VideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
  estBitrate = fEstimatedKbps;
	//estBitrate = 500; // kbps, estimate

  // Create a framer for the Video Elementary Stream:
  if (NULL == fLiveSource)		return NULL;

  if (fLiveSource->GetFlag() != LIVE_FLAG)		return NULL;

  //return H264VideoStreamFramer::createNew(envir(), fLiveSource->videoSource(), false);
  return H264VideoStreamDiscreteFramer::createNew(envir(), fLiveSource->videoSource());
}

RTPSink* LiveH264VideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* inputSource) 
{
	if (NULL == mMediaInfo)		return NULL;
	if (NULL == inputSource)		return NULL;
	if (inputSource->GetFlag() != LIVE_FLAG)		return NULL;

	//setVideoRTPSinkBufferSize();

	if ((mMediaInfo->spsLength >= sizeof(mMediaInfo->sps)) ||
		(mMediaInfo->ppsLength >= sizeof(mMediaInfo->pps)))
	{
		FILE *f = fopen("LiveH264VideoServerMediaSubsession.txt", "wb");
		if (NULL != f)
		{
			fwrite(mMediaInfo->sps, 1, sizeof(mMediaInfo->sps), f);
			fwrite(mMediaInfo->pps, 1, sizeof(mMediaInfo->pps), f);

			fclose(f);
		}
		return NULL;
	}


	if (mMediaInfo->spsLength > 0 && mMediaInfo->ppsLength > 0)
	{
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, mMediaInfo->sps, mMediaInfo->spsLength, mMediaInfo->pps, mMediaInfo->ppsLength);
	}


	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
