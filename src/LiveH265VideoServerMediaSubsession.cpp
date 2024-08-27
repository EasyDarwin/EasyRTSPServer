
#include "LiveH265VideoServerMediaSubsession.h"
#include <H265VideoRTPSink.hh>
#include <H265VideoStreamDiscreteFramer.hh>


LiveH265VideoServerMediaSubsession* LiveH265VideoServerMediaSubsession::createNew(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort) 
{
  return new LiveH265VideoServerMediaSubsession(env, estimatedBitrate, pMediaInfo, liveSource, streamName, mediaTypeName, reuseFirstSource, defaultPort);
}

LiveH265VideoServerMediaSubsession::LiveH265VideoServerMediaSubsession(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort)
  : LiveServerMediaSubsession(env, liveSource, streamName, mediaTypeName, estimatedBitrate, pMediaInfo, reuseFirstSource, MEDIA_TYPE_VIDEO, defaultPort) 
{
	mCounter	=	0;
}

LiveH265VideoServerMediaSubsession::~LiveH265VideoServerMediaSubsession() 
{
	if (NULL != fLiveSource)
	{
		Medium::close(fLiveSource, &envir());
		fLiveSource = NULL;
	}
}

static void afterPlayingDummy(void* clientData) 
{
  LiveH265VideoServerMediaSubsession* subsess   = (LiveH265VideoServerMediaSubsession*)clientData;
  // Signal the event loop that we're done:
  subsess->setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) 
{
  LiveH265VideoServerMediaSubsession* subsess    = (LiveH265VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void LiveH265VideoServerMediaSubsession::checkForAuxSDPLine1() 
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

char const* LiveH265VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
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

FramedSource* LiveH265VideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
  estBitrate = fEstimatedKbps;

  // Create a framer for the Video Elementary Stream:
  if (NULL == fLiveSource)		return NULL;
  if (fLiveSource->GetFlag() != LIVE_FLAG)		return NULL;

  return H265VideoStreamDiscreteFramer::createNew(envir(), fLiveSource->videoSource());
}

RTPSink* LiveH265VideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* inputSource) 
{
	if (NULL == mMediaInfo)		return NULL;
	if (NULL == inputSource)		return NULL;
	if (inputSource->GetFlag() != LIVE_FLAG)		return NULL;

	if ((mMediaInfo->vpsLength >= sizeof(mMediaInfo->vps)) ||
		(mMediaInfo->spsLength >= sizeof(mMediaInfo->sps)) ||
		(mMediaInfo->ppsLength >= sizeof(mMediaInfo->pps)))
	{
		FILE *f = fopen("LiveH265VideoServerMediaSubsession.txt", "wb");
		if (NULL != f)
		{
			fwrite(mMediaInfo->vps, 1, sizeof(mMediaInfo->vps), f);
			fwrite(mMediaInfo->sps, 1, sizeof(mMediaInfo->sps), f);
			fwrite(mMediaInfo->pps, 1, sizeof(mMediaInfo->pps), f);

			fclose(f);
		}
		return NULL;
	}


	setVideoRTPSinkBufferSize();


	if ( (mMediaInfo->vpsLength > 0) && (mMediaInfo->spsLength > 0) && (mMediaInfo->ppsLength > 0) )
	{
		return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, mMediaInfo->vps, mMediaInfo->vpsLength, mMediaInfo->sps, mMediaInfo->spsLength, mMediaInfo->pps, mMediaInfo->ppsLength);
	}

	return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
