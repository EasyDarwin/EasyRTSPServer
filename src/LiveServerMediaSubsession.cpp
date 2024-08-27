#include "LiveServerMediaSubsession.h"

LiveServerMediaSubsession::LiveServerMediaSubsession(UsageEnvironment& env, LiveSource *LiveSource, char *streamName, char *mediaTypeName, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *mediainfo, Boolean reuseFirstSource, unsigned int mediaType, portNumBits defaultPort)
  : OnDemandServerMediaSubsession(env, reuseFirstSource /*reuse the first source*/, streamName, mediaTypeName, defaultPort, false),
    fLiveSource(LiveSource), mMediaInfo(mediainfo), mMediaType(mediaType)
{
  //fEstimatedKbps = (estimatedBitrate + 500)/1000;

	fEstimatedKbps = estimatedBitrate;
}

LiveServerMediaSubsession::~LiveServerMediaSubsession() {
}

#if 0
float LiveServerMediaSubsession::getCurrentNPT(void* streamToken)
{
	static float ffNpt = 0.0f;
	ffNpt += 1.0f;
	return ffNpt;
}
#endif


void LiveServerMediaSubsession::getAbsoluteTimeRange(char*& absStartTime, char*& absEndTime)// const
{
#if 1
	absStartTime = absEndTime = NULL;
#else
	absStartTime = "20180101T083000Z";
	absEndTime  =  "20180101T120000Z";
#endif
	//20100929T095038.00Z-20100929T102038.00Z

	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fLiveSource->mCallbackPtr;
	if (NULL == pCallback)		return;
	
	RTSP_PLAY_CONTROL_INFO_T	durationCtlInfo;
	memset(&durationCtlInfo, 0x00, sizeof(RTSP_PLAY_CONTROL_INFO_T));
	durationCtlInfo.ctrlCommand = RTSP_PLAY_CTRL_CMD_GET_DURATION;
	durationCtlInfo.mediaType = mMediaType;

	if (NULL == fLiveSource || fLiveSource->GetFlag() != LIVE_FLAG)	return;

	pCallback(RTSP_CHANNEL_PLAY_CONTROL, fLiveSource->channelName, &fLiveSource->mChannelHandle, fLiveSource->mMediaInfoPtr, &durationCtlInfo, fLiveSource->mUserPtr, (void *)fLiveSource->mChannelUserPtr);

	if ( (int)strlen((char *)durationCtlInfo.startTime) > 0 )
	{
		absStartTime = (char *)durationCtlInfo.startTime;
	}

	if ( (int)strlen((char *)durationCtlInfo.endTime) > 0 )
	{
		absEndTime = (char *)durationCtlInfo.endTime;
	}
}


//pause play
void	LiveServerMediaSubsession::pauseStreamSource()
{
	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fLiveSource->mCallbackPtr;
	if (NULL == pCallback)			return;

	RTSP_PLAY_CONTROL_INFO_T	playCtrlInfo;
	memset(&playCtrlInfo, 0x00, sizeof(RTSP_PLAY_CONTROL_INFO_T));
	playCtrlInfo.ctrlCommand = RTSP_PLAY_CTRL_CMD_PAUSE;
	playCtrlInfo.mediaType = mMediaType;

	pCallback(RTSP_CHANNEL_PLAY_CONTROL, fLiveSource->channelName, 
											&fLiveSource->mChannelHandle, 
											fLiveSource->mMediaInfoPtr, 
											&playCtrlInfo, fLiveSource->mUserPtr, 
											(void *)fLiveSource->mChannelUserPtr);
}

//Resume play
void  LiveServerMediaSubsession::resumeStreamSource()
{
	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fLiveSource->mCallbackPtr;
	if (NULL == pCallback)			return;

	RTSP_PLAY_CONTROL_INFO_T	playCtrlInfo;
	memset(&playCtrlInfo, 0x00, sizeof(RTSP_PLAY_CONTROL_INFO_T));
	playCtrlInfo.ctrlCommand = RTSP_PLAY_CTRL_CMD_RESUME;
	playCtrlInfo.mediaType = mMediaType;

	pCallback(RTSP_CHANNEL_PLAY_CONTROL, fLiveSource->channelName, 
											&fLiveSource->mChannelHandle, 
											fLiveSource->mMediaInfoPtr, 
											&playCtrlInfo, 
											fLiveSource->mUserPtr, 
											(void *)fLiveSource->mChannelUserPtr);
}


void LiveServerMediaSubsession::testScaleFactor(float& scale)
{
	if ( (scale >= 1.0000000f && scale <= 32.00000000f) ||
		 (scale >= 0.0312500f && scale<=0.500000000f)  ||
		 (scale >= -32.00000000f && scale <= 0.00000000f) )
	{
		return;
	}
	else
	{
		scale = 1.0f;
	}
}

#if 0
void LiveServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/,
						   void* streamToken, float scale) {
  // Changing the scale factor isn't allowed if multiple clients are receiving data
  // from the same source:
  //if (fReuseFirstSource) return;

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL && streamState->mediaSource() != NULL) {
    setStreamSourceScale(streamState->mediaSource(), scale);
  }
}
#endif

void LiveServerMediaSubsession::setStreamSourceScale(FramedSource* inputSource, float scale)
{
	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fLiveSource->mCallbackPtr;
	if (NULL == pCallback)			return;

	RTSP_PLAY_CONTROL_INFO_T	playCtrlInfo;
	memset(&playCtrlInfo, 0x00, sizeof(RTSP_PLAY_CONTROL_INFO_T));
	playCtrlInfo.ctrlCommand = RTSP_PLAY_CTRL_CMD_SCALE;
	playCtrlInfo.mediaType = mMediaType;
	playCtrlInfo.scale = scale;

	pCallback(RTSP_CHANNEL_PLAY_CONTROL, fLiveSource->channelName, &fLiveSource->mChannelHandle, fLiveSource->mMediaInfoPtr, &playCtrlInfo, fLiveSource->mUserPtr, (void *)fLiveSource->mChannelUserPtr);
}

void LiveServerMediaSubsession::seekStreamSource(FramedSource* inputSource, char*& absStart, char*& absEnd)
{
	RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fLiveSource->mCallbackPtr;
	if (NULL == pCallback)			return;

	RTSP_PLAY_CONTROL_INFO_T	playCtrlInfo;
	memset(&playCtrlInfo, 0x00, sizeof(RTSP_PLAY_CONTROL_INFO_T));
	playCtrlInfo.ctrlCommand = RTSP_PLAY_CTRL_CMD_SEEK_STREAM;
	playCtrlInfo.mediaType = mMediaType;
	if (NULL!=absStart && ((int)strlen(absStart)>0))		strcpy((char*)playCtrlInfo.startTime, absStart);
	if (NULL!=absEnd && ((int)strlen(absEnd)>0))		strcpy((char*)playCtrlInfo.endTime, absStart);

	pCallback(RTSP_CHANNEL_PLAY_CONTROL, fLiveSource->channelName, &fLiveSource->mChannelHandle, fLiveSource->mMediaInfoPtr, &playCtrlInfo, fLiveSource->mUserPtr, (void *)fLiveSource->mChannelUserPtr);
}


#if 0
void LiveServerMediaSubsession::seekStreamSource(FramedSource* inputSource, double& seekNPT, double streamDuration, u_int64_t& numBytes)
{
	printf("LiveServerMediaSubsession::seekStreamSource  seekNPT[%s]  absEnd[%.2f]\n", seekNPT, streamDuration);
}
    // This routine is used to seek by relative (i.e., NPT) time.
    // "streamDuration", if >0.0, specifies how much data to stream, past "seekNPT".  (If <=0.0, all remaining data is streamed.)
    // "numBytes" returns the size (in bytes) of the data to be streamed, or 0 if unknown or unlimited.
void LiveServerMediaSubsession::seekStreamSource(FramedSource* inputSource, char*& absStart, char*& absEnd)
{
	printf("LiveServerMediaSubsession::seekStreamSource  absStart[%s]  absEnd[%s]\n", absStart, absEnd);
}

    // This routine is used to seek by 'absolute' time.
    // "absStart" should be a string of the form "YYYYMMDDTHHMMSSZ" or "YYYYMMDDTHHMMSS.<frac>Z".
    // "absEnd" should be either NULL (for no end time), or a string of the same form as "absStart".
    // These strings may be modified in-place, or can be reassigned to a newly-allocated value (after delete[]ing the original).


void LiveServerMediaSubsession::setStreamSourceDuration(FramedSource* inputSource, double streamDuration, u_int64_t& numBytes)
{
	printf("LiveServerMediaSubsession::setStreamSourceDuration  streamDuration: %.2f\n", streamDuration);
}

void LiveServerMediaSubsession::closeStreamSource(FramedSource* inputSource)
{

	printf("LiveServerMediaSubsession::closeStreamSource\n");
}
#endif