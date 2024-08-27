#ifndef _LIVE_RTSP_SERVER_H
#define _LIVE_RTSP_SERVER_H

#ifndef _RTSP_SERVER_SUPPORTING_HTTP_STREAMING_HH
#include "RTSPServerSupportingHTTPStreaming.hh"
#endif

#include "osmutex.h"
#include "libRTSPServerAPI.h"
//#include "ssqueue.h"
#include "LiveSource.h"
#include "osmutex.h"


#define	LIVE_USE_QUEUE			0x01

#define	LIVE_VIDEO_QUEUE_DEFAULT_SIZE		1024*1024*2
#define	LIVE_AUDIO_QUEUE_DEFAULT_SIZE		1024*128
#define	LIVE_METADATA_QUEUE_DEFAULT_SIZE	1024*512


#define	MAX_LIVE_CHANNEL_NUM	1024
typedef struct __LIVE_CHANNEL_OBJ_T
{
	//LIVE_CHANNEL_INFO_T	info;
	int					resetFlag;
	unsigned int		status;			//状态, 创建时赋值为0xFFFFFFFF, 删除前置为0, 即置为0后，PutFrame执行会直接返回

	int					id;
	char				streamName[256];

	RTSP_MEDIA_INFO_T	mediainfo;
	LIVE_FRAME_INFO_T	videoFrame;
	LIVE_FRAME_INFO_T	audioFrame;
	//SS_QUEUE_OBJ_T		*videoQueue;
	//SS_QUEUE_OBJ_T		*audioQueue;
	BUFFQUEUE_HANDLE	videoQueue;
	BUFFQUEUE_HANDLE	audioQueue;
	BUFFQUEUE_HANDLE	metadataQueue;

	LiveSource *liveSource;

	void				*channelUserPtr;

	__LIVE_CHANNEL_OBJ_T	*next;
}LIVE_CHANNEL_OBJ_T;

class LiveRtspServer : public RTSPServer 
{
public:
  static LiveRtspServer* createNew(UsageEnvironment& env, Port ourPort,
				      UserAuthenticationDatabase* authDatabase,
				      unsigned reclamationTestSeconds = 65, void *_callback=NULL, void *_userptr=NULL);

  int		CreateChannel(const char *streamName, RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr);
  int		PutFrame(RTSP_CHANNEL_HANDLE channelHandle, unsigned int mediaType, MEDIA_FRAME_INFO_T *frameInfo);
  int		DeleteChannel(RTSP_CHANNEL_HANDLE *channelHandle);
  int		DeleteAllChannel();

  int		CheckChannelStatus();
  int		ResetChannel(RTSP_CHANNEL_HANDLE channelHandle);
protected:
  LiveRtspServer(UsageEnvironment& env, int ourSocketV4, int ourSocketV6, Port ourPort,
		    UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, void *_callback, void *_userptr);
  // called only by createNew();
  virtual ~LiveRtspServer();

protected: // redefined virtual functions
  virtual ServerMediaSession*
#ifdef LIVE_MULTI_THREAD_ENABLE
  lookupServerMediaSession(UsageEnvironment	*pEnv, int iType,  void *pClientPtr, char const* streamName, Boolean bLockServerMediaSession, Boolean isFirstLookupInSession);
#else
	  lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession);
#endif

  OSMutex	mutexChannel;
  int		liveChannelNum;
  LIVE_CHANNEL_OBJ_T		*pLiveChannel;
  portNumBits rtpInitialPortNum;



};


#endif
