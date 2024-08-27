#include "LiveMetadataServerMediaSubsession.h"
#include "SimpleRTPSink.hh"


LiveMetadataServerMediaSubsession* LiveMetadataServerMediaSubsession::createNew(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort) 
{
  return new LiveMetadataServerMediaSubsession(env, pMediaInfo, liveSource, streamName, mediaTypeName, reuseFirstSource, defaultPort);
}

LiveMetadataServerMediaSubsession::LiveMetadataServerMediaSubsession(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort)
	: LiveServerMediaSubsession(env, liveSource, streamName, mediaTypeName, 500, pMediaInfo, reuseFirstSource, MEDIA_TYPE_EVENT, defaultPort)
{

}

LiveMetadataServerMediaSubsession::~LiveMetadataServerMediaSubsession() 
{
}

FramedSource* LiveMetadataServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,  unsigned& estBitrate) 
{
	FramedSource* metadataSource = NULL;
	estBitrate = fEstimatedKbps;

	if (NULL == mMediaInfo)		return NULL;
	if (mMediaInfo->metadataCodec < 1)		return NULL;
	if (NULL == fLiveSource || fLiveSource->GetFlag() != LIVE_FLAG)		return NULL;

	metadataSource = fLiveSource->metadataSource();

	return metadataSource;
}

RTPSink* LiveMetadataServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,  FramedSource* inputSource) 
{
	MultiFramedRTPSink	*rtpSink = NULL;

	if (NULL == mMediaInfo)		return NULL;
	if (mMediaInfo->metadataCodec < 1)		return NULL;
	if (NULL == inputSource)		return NULL;
	if (inputSource->GetFlag() != LIVE_FLAG)		return NULL;


	unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // if dynamic

	char* mimeType = "metadata";
	//payloadFormatCode = 0; // a static RTP payload type
	unsigned rtpTimestampFrequency = 90000;
	unsigned numChannels = 1;
    
	rtpSink =  SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode,
											rtpTimestampFrequency, "metadata",
											mimeType, numChannels, false);

	return rtpSink;
}
