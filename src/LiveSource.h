#ifndef _LIVE_INPUT_H
#define _LIVE_INPUT_H

#include <MediaSink.hh>
#include "RtspTypes.h"
//#include "ssqueue.h"
#include "livebufferqueue.h"
#include "RtspTypes.h"
#include "libRTSPServerAPI.h"
#include "ServerMediaSession.hh"

typedef struct __LIVE_FRAME_INFO_T
{
	unsigned int	codec;
	int				frametype;
	int				size;			//谷那??∩車D?
	int				framesize;		//那米?那??∩車D?
	unsigned char	*framedata;		//??那y?Y

	unsigned int	timestamp_sec;
	unsigned int	timestamp_usec;

}LIVE_FRAME_INFO_T;


#define CUSTOM_META_STREAM_TYPE			64
#define OFD_MODULE_ID			0x004F4644	// "OFD"
#define VCA_MODULE_ID			0x00564341	// "VCA"
#define VPB_MODULE_ID			0x00565042	// "VPB"		//Video Playback



class LiveSource: public Medium 
{
public:
  static LiveSource* createNew(UsageEnvironment& env, ServerMediaSession *pSession, 
										int channelId, char *channelName, RTSP_CHANNEL_HANDLE handle, RTSP_MEDIA_INFO_T *pMediaInfo,  
										LIVE_FRAME_INFO_T *videoFrame, LIVE_FRAME_INFO_T *audioFrame, 
										//SS_QUEUE_OBJ_T *videoQueue, SS_QUEUE_OBJ_T *audioQueue, 
										BUFFQUEUE_HANDLE	videoQueue, BUFFQUEUE_HANDLE audioQueue, BUFFQUEUE_HANDLE metadataQueue,
										void *callbackPtr, void *userPtr, void *channelUserPtr);

  FramedSource* videoSource();
  FramedSource* audioSource();
  FramedSource* metadataSource();
  UsageEnvironment	*GetEnvironment()		{return pEnvironment;}
  unsigned int GetFlag()					{return flag;}

private:
  LiveSource(UsageEnvironment& env, ServerMediaSession *pSession, 
								int channelId, char *channelName, RTSP_CHANNEL_HANDLE handle, RTSP_MEDIA_INFO_T *pMediaInfo, 
								LIVE_FRAME_INFO_T *videoFrame, LIVE_FRAME_INFO_T *audioFrame, 
								//SS_QUEUE_OBJ_T *videoQueue, SS_QUEUE_OBJ_T *audioQueue, 
								BUFFQUEUE_HANDLE	videoQueue, BUFFQUEUE_HANDLE audioQueue, BUFFQUEUE_HANDLE metadataQueue,
								void *callbackPtr, void *userPtr, void *channelUserPtr); // called only by createNew()
  virtual ~LiveSource();

public:
	bool	mStartStreaming;
	int		mChannelId;
	char	channelName[255];
	RTSP_CHANNEL_HANDLE	mChannelHandle;
	RTSP_MEDIA_INFO_T	*mMediaInfoPtr;
	//SS_QUEUE_OBJ_T	*mVideoQueue;
	//SS_QUEUE_OBJ_T	*mAudioQueue;
	BUFFQUEUE_HANDLE	mVideoQueue;
	BUFFQUEUE_HANDLE	mAudioQueue;
	BUFFQUEUE_HANDLE	mMetadataQueue;

	LIVE_FRAME_INFO_T	*pLiveVideoFrame;
	LIVE_FRAME_INFO_T	*pLiveAudioFrame;

	void	*mCallbackPtr;
	void	*mUserPtr;
	void	*mChannelUserPtr;

	UsageEnvironment	*pEnvironment;
	ServerMediaSession  *pMediaSession;
	FramedSource* fOurVideoSource;
	FramedSource* fOurAudioSource;
	FramedSource* fOurMetadataSource;

	unsigned int	flag;
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define AUDIO_MAX_FRAME_SIZE 20480
#define VIDEO_MAX_FRAME_SIZE 1048576	//250000
inline void setAudioRTPSinkBufferSize() { OutPacketBuffer::maxSize = AUDIO_MAX_FRAME_SIZE; }
inline void setVideoRTPSinkBufferSize() { OutPacketBuffer::maxSize = VIDEO_MAX_FRAME_SIZE; }

#endif
