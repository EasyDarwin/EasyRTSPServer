#include "LiveAudioServerMediaSubsession.h"
#include "MPEG4GenericRTPSource.hh"
#include "MPEG4GenericRTPSink.hh"
#include "SimpleRTPSink.hh"


LiveAudioServerMediaSubsession* LiveAudioServerMediaSubsession::createNew(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort) 
{
  return new LiveAudioServerMediaSubsession(env, pMediaInfo, liveSource, streamName, mediaTypeName, reuseFirstSource, defaultPort);
}

LiveAudioServerMediaSubsession::LiveAudioServerMediaSubsession(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort)
	: LiveServerMediaSubsession(env, liveSource, streamName, mediaTypeName, 500, pMediaInfo, reuseFirstSource, MEDIA_TYPE_AUDIO, defaultPort)
{

}

LiveAudioServerMediaSubsession::~LiveAudioServerMediaSubsession() 
{
}

FramedSource* LiveAudioServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,  unsigned& estBitrate) 
{
	FramedSource* audioSource = NULL;
	estBitrate = fEstimatedKbps;

	if (NULL == mMediaInfo)		return NULL;
	if (mMediaInfo->audioCodec < 1 || mMediaInfo->audioChannel < 1 || mMediaInfo->audioSampleRate < 1)		return NULL;
	if (NULL == fLiveSource || fLiveSource->GetFlag() != LIVE_FLAG)		return NULL;

	audioSource = fLiveSource->audioSource();
/*
	switch (mMediaInfo->audioCodec)
	{
	case RTSP_AUDIO_CODEC_G711U:
	case RTSP_AUDIO_CODEC_G711A:
		audioSource = fLiveSource->audioSource();
		break;
	case RTSP_AUDIO_CODEC_G726:
		break;
	case RTSP_AUDIO_CODEC_AAC:
		{
			audioSource = fLiveSource->audioSource();
		}
		break;
	default:
		break;
	}
*/
	return audioSource;
}

RTPSink* LiveAudioServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,  FramedSource* inputSource) 
{
	MultiFramedRTPSink	*rtpSink = NULL;

	if (NULL == mMediaInfo)		return NULL;
	if (mMediaInfo->audioCodec < 1 || mMediaInfo->audioChannel < 1 || mMediaInfo->audioSampleRate < 1)		return NULL;
	if (NULL == inputSource)		return NULL;
	if (inputSource->GetFlag() != LIVE_FLAG)		return NULL;

	switch (mMediaInfo->audioCodec)
	{
	case RTSP_AUDIO_CODEC_G711U:
	case RTSP_AUDIO_CODEC_G711A:
		{
			unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // if dynamic

			char* mimeType = NULL;
			if (mMediaInfo->audioCodec == RTSP_AUDIO_CODEC_G711U)		// stream u-law
			{
				mimeType = "PCMU";
				if (mMediaInfo->audioSampleRate == 8000 && mMediaInfo->audioChannel == 1)
				{
					payloadFormatCode = 0; // a static RTP payload type			//对接海康NVR, 必须要将payloadFormatCode置为0, 否则NVR不能播放声音
				}
			}
			else if (mMediaInfo->audioCodec == RTSP_AUDIO_CODEC_G711A)		// stream A-law
			{
				  mimeType = "PCMA";
				  if (mMediaInfo->audioSampleRate == 8000 && mMediaInfo->audioChannel == 1) 
				  {
					payloadFormatCode = 8; // a static RTP payload type			//对接海康NVR, 必须要将payloadFormatCode置为8, 否则NVR不能播放声音
				  }
			}
			else		// stream raw PCM
			{ 
				mimeType = "L16";
				if (mMediaInfo->audioSampleRate == 44100 && mMediaInfo->audioChannel == 2) 
				{
					payloadFormatCode = 10; // a static RTP payload type
				}
				else if (mMediaInfo->audioSampleRate == 44100 && mMediaInfo->audioChannel == 1) 
				{
					payloadFormatCode = 11; // a static RTP payload type
				}
			}
    
			rtpSink =  SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode,
												 mMediaInfo->audioSampleRate, "audio",
												 mimeType, mMediaInfo->audioChannel, false);
		}
		break;
	case RTSP_AUDIO_CODEC_G726:
		{
			unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; // if dynamic

			char mimeType[24] = {0};
			memset(mimeType, 0x00, sizeof(mimeType));
			if (mMediaInfo->audioBitsPerSample == 16)		sprintf(mimeType, "G726-16/%d", mMediaInfo->audioSampleRate);
			else if (mMediaInfo->audioBitsPerSample == 24)	sprintf(mimeType, "G726-24/%d", mMediaInfo->audioSampleRate);
			else if (mMediaInfo->audioBitsPerSample == 32)	sprintf(mimeType, "G726-32/%d", mMediaInfo->audioSampleRate);
			else if (mMediaInfo->audioBitsPerSample == 40)	sprintf(mimeType, "G726-40/%d", mMediaInfo->audioSampleRate);

			if ( (int)strlen(mimeType) < 1)	sprintf(mimeType, "G726-16/%d", mMediaInfo->audioSampleRate);
    
			//对接海康NVR, 726需要小包发送才能正常解码
			rtpSink =  SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode,
												 mMediaInfo->audioSampleRate, "audio",
												 mimeType, mMediaInfo->audioChannel, false);
		}
		break;
	case RTSP_AUDIO_CODEC_AAC:
		{
			//char const* encoderConfigStr = mMediaInfo->audioChannel == 2 ? "1210": "1208";
			char const* encoderConfigStr = "1208";		//16KHz   44.1Khz
			//char const* encoderConfigStr = "1588";		//16KHz   44.1Khz

			//参照海康相机: DS-2CD4026FWD
			if (mMediaInfo->audioSampleRate == 32000)		encoderConfigStr = "1288";		//32KHz
			else if (mMediaInfo->audioSampleRate == 48000)		encoderConfigStr = "1190";		//48KHz
			else if (mMediaInfo->audioSampleRate == 16000)		encoderConfigStr = "1408";		//16KHz		//对接海康NVR, 需要改成1408,否则NVR不能播放声音

			rtpSink = MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, mMediaInfo->audioSampleRate, "audio", "AAC-hbr", encoderConfigStr, mMediaInfo->audioChannel);
		}
	default:
		break;
	}

	return rtpSink;
}
