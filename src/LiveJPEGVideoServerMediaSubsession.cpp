
#include "LiveJPEGVideoServerMediaSubsession.h"
#include <JPEGVideoRTPSink.hh>
#include "LiveJPEGStreamSource.h"
#include "MJPEGVideoSource.h"

LiveJPEGVideoServerMediaSubsession* LiveJPEGVideoServerMediaSubsession::createNew(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName,Boolean reuseFirstSource, portNumBits defaultPort) 
{
  return new LiveJPEGVideoServerMediaSubsession(env, estimatedBitrate, pMediaInfo, liveSource, streamName, mediaTypeName, reuseFirstSource, defaultPort);
}

LiveJPEGVideoServerMediaSubsession::LiveJPEGVideoServerMediaSubsession(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName,Boolean reuseFirstSource, portNumBits defaultPort)
  : LiveServerMediaSubsession(env, liveSource, streamName, mediaTypeName, estimatedBitrate, pMediaInfo, reuseFirstSource, MEDIA_TYPE_VIDEO, defaultPort) 
{
}

LiveJPEGVideoServerMediaSubsession::~LiveJPEGVideoServerMediaSubsession() 
{
	if (NULL != fLiveSource)
	{
		Medium::close(fLiveSource);
		fLiveSource = NULL;
	}
}

static void afterPlayingDummy(void* clientData) 
{
  LiveJPEGVideoServerMediaSubsession* subsess   = (LiveJPEGVideoServerMediaSubsession*)clientData;
  // Signal the event loop that we're done:
  subsess->setDoneFlag();
}
#if 0
static void checkForAuxSDPLine(void* clientData) 
{
  LiveJPEGVideoServerMediaSubsession* subsess    = (LiveJPEGVideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void LiveJPEGVideoServerMediaSubsession::checkForAuxSDPLine1() 
{
  if (fDummyRTPSink->auxSDPLine() != NULL) 
  {
    // Signal the event loop that we're done:
    setDoneFlag();
  } 
  else 
  {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* LiveJPEGVideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
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
#endif
FramedSource* LiveJPEGVideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
  estBitrate = fEstimatedKbps;

  // Create a framer for the Video Elementary Stream:
  if (NULL == fLiveSource)		return NULL;
  if (fLiveSource->GetFlag() != LIVE_FLAG)		return NULL;

  //return LiveJPEGStreamSource::createNew(fLiveSource->videoSource());
  return MJPEGVideoSource::createNew(envir(), fLiveSource->videoSource());
}

RTPSink* LiveJPEGVideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* inputSource) 
{
	if (NULL == mMediaInfo)		return NULL;
	if (NULL == inputSource)		return NULL;
	if (inputSource->GetFlag() != LIVE_FLAG)		return NULL;

	setVideoRTPSinkBufferSize();

	return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}
