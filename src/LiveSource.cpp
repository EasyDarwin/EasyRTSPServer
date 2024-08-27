
#include "LiveSource.h"
#include <GroupsockHelper.hh>
#include "libRTSPServerAPI.h"
#include "trace.h"
#include <time.h>
// A common "FramedSource" subclass, used for reading from an open file:


typedef struct __MEDIA_FRAME_DATA_T
{
	unsigned int	bufsize;
	unsigned char	*buf;
	unsigned int	framesize;
	unsigned int	framepos;
	struct timeval  timestamp;
}MEDIA_FRAME_DATA_T;
class LiveStreamSource: public FramedSource 
{
protected:
  LiveStreamSource(UsageEnvironment& env, LiveSource& input);
  virtual ~LiveStreamSource();

  virtual int readFromFile() = 0;

private: // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual unsigned int maxFrameSize() const;

private:
  static void incomingDataHandler(LiveStreamSource* source, int mask);
  void incomingDataHandler1();

protected:
  LiveSource& fInput;
  unsigned int	fMediaType;
  MEDIA_FRAME_DATA_T	framedata;

  CONSUMER_HANDLE		consumerHandle;

  //unsigned char	*fFramedata;
  //unsigned int	fFramesize;
};


////////// LiveVideoStreamSource definition //////////

class LiveVideoStreamSource: public LiveStreamSource 
{
public:
  LiveVideoStreamSource(UsageEnvironment& env, LiveSource& input);
  virtual ~LiveVideoStreamSource();

private:
	unsigned int	mLastTimestamp;
	FILE		*fErrLog;
protected: // redefined virtual functions:
  virtual int readFromFile();
};


////////// LiveAudioStreamSource definition //////////

class LiveAudioStreamSource: public LiveStreamSource 
{
public:
  LiveAudioStreamSource(UsageEnvironment& env, LiveSource& input);
  virtual ~LiveAudioStreamSource();

protected: // redefined virtual functions:
  virtual int readFromFile();
};


////////// LiveAudioStreamSource definition //////////

class LiveMetadataStreamSource: public LiveStreamSource 
{
public:
  LiveMetadataStreamSource(UsageEnvironment& env, LiveSource& input);
  virtual ~LiveMetadataStreamSource();

protected: // redefined virtual functions:
  virtual int readFromFile();
};

////////// LiveSource implementation //////////

LiveSource* LiveSource::createNew(UsageEnvironment& env, ServerMediaSession *pSession, 
										int channelId, char *channelName,RTSP_CHANNEL_HANDLE handle, RTSP_MEDIA_INFO_T *pMediaInfo,  
										LIVE_FRAME_INFO_T *videoFrame, LIVE_FRAME_INFO_T *audioFrame, 
										//SS_QUEUE_OBJ_T *videoQueue, SS_QUEUE_OBJ_T *audioQueue, 
										BUFFQUEUE_HANDLE	videoQueue, BUFFQUEUE_HANDLE audioQueue, BUFFQUEUE_HANDLE metadataQueue,
										void *callbackPtr, void *userPtr, void *channelUserPtr) 
{
  return new LiveSource(env, pSession, channelId, channelName, handle, pMediaInfo, videoFrame, audioFrame, videoQueue, audioQueue, metadataQueue, callbackPtr, userPtr, channelUserPtr);
}

FramedSource* LiveSource::videoSource() 
{
	if (flag != LIVE_FLAG)			return NULL;

	if (fOurVideoSource == NULL)
	{
		fOurVideoSource = new LiveVideoStreamSource(*pEnvironment, *this);
	}
  return fOurVideoSource;
}

FramedSource* LiveSource::audioSource() 
{
	if (flag != LIVE_FLAG)			return NULL;

	if (fOurAudioSource == NULL) 
	{
		fOurAudioSource = new LiveAudioStreamSource(*pEnvironment, *this);
	}
	return fOurAudioSource;
}
FramedSource* LiveSource::metadataSource()
{
	if (flag != LIVE_FLAG)			return NULL;

	if (fOurMetadataSource == NULL) 
	{
		fOurMetadataSource = new LiveMetadataStreamSource(*pEnvironment, *this);
	}
  return fOurMetadataSource;
}


LiveSource::LiveSource(UsageEnvironment& env, ServerMediaSession *pSession, 
						int channelId, char *_channelName, RTSP_CHANNEL_HANDLE handle, RTSP_MEDIA_INFO_T *pMediaInfo,  
						LIVE_FRAME_INFO_T *videoFrame, LIVE_FRAME_INFO_T *audioFrame, 
						//SS_QUEUE_OBJ_T *videoQueue, SS_QUEUE_OBJ_T *audioQueue, 
						BUFFQUEUE_HANDLE	videoQueue, BUFFQUEUE_HANDLE audioQueue, BUFFQUEUE_HANDLE metadataQueue,
						void *callbackPtr, void *userPtr, void *channelUserPtr) : Medium(env) 
{
	mStartStreaming = false ;
	mChannelId = channelId;
	mChannelHandle	=	handle;
	mMediaInfoPtr = pMediaInfo;
	pLiveVideoFrame = videoFrame;
	pLiveAudioFrame = audioFrame;
	pEnvironment = &env;
	pMediaSession = pSession;
	flag = LIVE_FLAG;

	if (NULL != _channelName)
	{
		memset(channelName, 0x00, sizeof(channelName));
		strcpy(channelName, _channelName);
	}

	mVideoQueue	=	videoQueue;
	mAudioQueue =   audioQueue;
	mMetadataQueue = metadataQueue;
	mCallbackPtr = callbackPtr;
	mUserPtr = userPtr;
	mChannelUserPtr = channelUserPtr;

	fOurVideoSource = NULL;
	fOurAudioSource = NULL;
	fOurMetadataSource= NULL;
}

LiveSource::~LiveSource() 
{
  if (NULL != fOurVideoSource) 
  {
	  Medium::close(fOurVideoSource, GetEnvironment());
	  fOurVideoSource = NULL;
  }
  if (NULL != fOurAudioSource) 
  {
	  Medium::close(fOurAudioSource, GetEnvironment());
	  fOurAudioSource = NULL;
  }
  if (NULL != fOurMetadataSource)
  {
	  Medium::close(fOurMetadataSource, GetEnvironment());
	  fOurMetadataSource = NULL;
  }

  flag = 0;

  //OutputDebugString("LiveSource::~LiveSource() ...\n");
}

//FramedSource* LiveSource::fOurVideoSource = NULL;
//FramedSource* LiveSource::fOurAudioSource = NULL;

////////// LiveStreamSource implementation //////////

LiveStreamSource::LiveStreamSource(UsageEnvironment& env, LiveSource& input)
  : FramedSource(env),   fInput(input), consumerHandle(NULL)
{

}

LiveStreamSource::~LiveStreamSource() 
{

  //envir().taskScheduler().turnOffBackgroundReadHandling(fFileNo);
}

void LiveStreamSource::doGetNextFrame()
{
	//OutputDebugString("LiveStreamSource::doGetNextFrame()...\n");

	if (mFlag != LIVE_FLAG)		return;

	if (! fInput.mStartStreaming)		//回调:  开始播放流
	{
		RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )fInput.mCallbackPtr;
		if (NULL != pCallback)
		{
			pCallback(RTSP_CHANNEL_START_STREAM, fInput.channelName, 
								&fInput.mChannelHandle, fInput.mMediaInfoPtr, NULL, 
								fInput.mUserPtr, (void *)fInput.mChannelUserPtr);
		}
		fInput.mStartStreaming = true;
	}

	if (!isCurrentlyAwaitingData())
	{
		FILE *f = fopen("liveSouce.txt", "a+");
		if (NULL != f)
		{
			char sztmp[128] = {0};
			sprintf(sztmp, "liveSource: 0x%X   flag: 0x%X\n", this, mFlag);

			char szTime[64] = {0,};
			time_t tt = time(NULL);
			struct tm *_timetmp = NULL;
			_timetmp = localtime(&tt);
			if (NULL != _timetmp)   strftime(szTime, 32, "%Y-%m-%d_%H:%M:%S",_timetmp);

			fwrite(szTime, 1, (int)strlen(szTime), f);
			fwrite(sztmp, 1, (int)strlen(sztmp), f);
			fclose(f);
		}

		int to_delay = 0;//delay * 1000;  //us
		to_delay = 1000;
		nextTask() = fInput.GetEnvironment()->taskScheduler().scheduleDelayedTask(to_delay, (TaskFunc*)incomingDataHandler, this);

		return; // we're not ready for the data yet
	}

	if (fMediaType == MEDIA_TYPE_VIDEO)
	{
#if 0
		int fps = 30;
		//if (NULL != fInput.mMediaInfoPtr)	fps = fInput.mMediaInfoPtr->u32VideoFps;	//FPS in MediaInfo
		if (fps < 1)	fps = 30;

		// 根据 fps，计算等待时间
		int delay = (int)((float)1000.0f / (float)fps);  // ms

		int nQueueFrame = 0;
		if (NULL != fInput.mVideoQueue && NULL!=fInput.mVideoQueue->pQueHeader)
		{
			nQueueFrame = fInput.mVideoQueue->pQueHeader->videoframes;		//当前队列中帧数
			int iOneFrameUsec = delay;		//每帧耗时
			int iCache = 5;					//设置缓存帧数

			if (nQueueFrame > 1)
			{
				int nn = 1000 / fps * nQueueFrame;
			
				//delay = (int)((float)1000.0f / (float)nn);
			}

			if (nQueueFrame<iCache && fps>0)
			{
				int ii = ((iOneFrameUsec * (iCache-nQueueFrame))/fps);
				delay += ii;
			}
			else if (nQueueFrame>iCache && fps>0)
			{
				int ii = ((iOneFrameUsec * (nQueueFrame-iCache))/fps);
				delay -= ii;
			}
		}
#endif
		//_TRACE(TRACE_LOG_DEBUG, "delay: %d\tvideo frame num: %d\n", delay, nQueueFrame);

		int to_delay = 0;//delay * 1000;  //us
		to_delay = 1000;
		nextTask() = fInput.GetEnvironment()->taskScheduler().scheduleDelayedTask(to_delay, (TaskFunc*)incomingDataHandler, this);
	}
	else if (fMediaType == MEDIA_TYPE_AUDIO)
	{
		nextTask() = fInput.GetEnvironment()->taskScheduler().scheduleDelayedTask(1000, (TaskFunc*)incomingDataHandler, this);  
	}
	else if (fMediaType == MEDIA_TYPE_EVENT)
	{
		nextTask() = fInput.GetEnvironment()->taskScheduler().scheduleDelayedTask(1000, (TaskFunc*)incomingDataHandler, this);  
	}

	return;
}
unsigned int LiveStreamSource::maxFrameSize() const
{
	return 1024*1024;
}

void LiveStreamSource::incomingDataHandler(LiveStreamSource* source, int /*mask*/) 
{
	source->incomingDataHandler1();
}

void LiveStreamSource::incomingDataHandler1() 
{
	// Read the data from our file into the client's buffer:
	if (mFlag != LIVE_FLAG)		return;

	if ( 0 == readFromFile())
	{
		// Tell our client that we have new data:

		FramedSource::afterGetting(this);
	}
	else
	{
		doGetNextFrame();
	}
}


////////// LiveVideoStreamSource implementation //////////

LiveVideoStreamSource::LiveVideoStreamSource(UsageEnvironment& env, LiveSource& input)
  : LiveStreamSource(env, input) 
{
	memset(&framedata, 0x00, sizeof(MEDIA_FRAME_DATA_T));

	fMediaType = MEDIA_TYPE_VIDEO;
	mLastTimestamp = (unsigned int)time(NULL);
	fErrLog = NULL;
}

LiveVideoStreamSource::~LiveVideoStreamSource() 
{
	if (NULL != fInput.fOurVideoSource)
	{
		Medium::close(fInput.fOurVideoSource, fInput.GetEnvironment());//&fInput.envir());
		fInput.fOurVideoSource = NULL;
	}

	if (NULL != framedata.buf)
	{
		delete []framedata.buf;
		framedata.buf = NULL;
	}
	memset(&framedata, 0x00, sizeof(MEDIA_FRAME_DATA_T));

	if (NULL != fErrLog)
	{
		fclose(fErrLog);
		fErrLog = NULL;
	}
}


int LiveVideoStreamSource::readFromFile() 
{
	int ret = -1;
	if (NULL == fInput.pLiveVideoFrame && NULL == fInput.mVideoQueue)			return ret;

	if (NULL != fInput.pLiveVideoFrame->framedata)
	{
		if (fInput.pLiveVideoFrame->framesize > 0)
		{
			memcpy(fTo, fInput.pLiveVideoFrame->framedata, fInput.pLiveVideoFrame->framesize);
			fFrameSize = fInput.pLiveVideoFrame->framesize;
			if (fInput.pLiveVideoFrame->timestamp_sec > 0U || fInput.pLiveVideoFrame->timestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = fInput.pLiveVideoFrame->timestamp_sec;
				fPresentationTime.tv_usec = fInput.pLiveVideoFrame->timestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
			fNumTruncatedBytes = 0;

			ret = 0;
		}
	}
	else if (NULL != fInput.mVideoQueue)
	{
#if 1
		LIVE_FRAME_INFO_T	frameinfo;
		memset(&frameinfo, 0x00, sizeof(LIVE_FRAME_INFO_T));

		ret = -1;
		if (NULL == consumerHandle)	consumerHandle = BUFQUE_RegisterConsumer(fInput.mVideoQueue, (unsigned long)this, 0x01);
		if (NULL != consumerHandle)
		{
			ret = BUFQUE_GetData(fInput.mVideoQueue, consumerHandle, NULL, NULL, 
													NULL, (char *)&frameinfo, NULL, (char *)fTo, 0x01);
		}
		fFrameSize = 0;
		if (ret == 0x00)
		{
			fInput.GetEnvironment()->IncEnvirCount();	//增加计数
			mLastTimestamp = (unsigned int)time(NULL);

			//if (NULL!=fInput.pMediaSession)		fInput.pMediaSession->SetStreamStatus(1);		//add by gavin 2018.05.09

			if (frameinfo.timestamp_sec > 0U || frameinfo.timestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = frameinfo.timestamp_sec;
				fPresentationTime.tv_usec = frameinfo.timestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
			fFrameSize = frameinfo.size;
			fNumTruncatedBytes = 0;
		}
		else
		{
			unsigned int uiTime = (unsigned int)time(NULL);
			if (uiTime - mLastTimestamp > 2 && mLastTimestamp>0)
			{
				char filename[260] = {0};
				sprintf(filename, "Worker%d_Video.log", envir().GetEnvirId());
				if (NULL == fErrLog)
				{
					fErrLog = fopen(filename, "wb");
				}
				if (NULL != fErrLog)
				{
					char szTime[64] = {0,};
					time_t tt = uiTime;
					struct tm *_timetmp = NULL;
					_timetmp = localtime(&tt);
					if (NULL != _timetmp)	strftime(szTime, 32, "%Y-%m-%d %H:%M:%S ", _timetmp);

					fwrite(szTime, 1, (int)strlen(szTime), fErrLog);

					char szLastVideoTime[64] = {0};
					char szLastAudioTime[64] = {0};
					char log[1024] = {0};
					memset(szTime, 0x00, sizeof(szTime));
					tt = GetAddVideoDataTime(fInput.mVideoQueue);
					_timetmp = localtime(&tt);
					if (NULL != _timetmp)	strftime(szLastVideoTime, 32, "%Y-%m-%d %H:%M:%S ", _timetmp);

					tt = GetAddAudioDataTime(fInput.mAudioQueue);
					_timetmp = localtime(&tt);
					if (NULL != _timetmp)	strftime(szLastAudioTime, 32, "%Y-%m-%d %H:%M:%S ", _timetmp);

					sprintf(log, ":\t\tLastAddVideoTime:  %s...  LastAddAudioTime: %s\n", szLastVideoTime, szLastAudioTime);
					fwrite(log, 1, (int)strlen(log), fErrLog);
					fflush(fErrLog);
				}
				
				//更新一次
				mLastTimestamp = (unsigned int)time(NULL);
			}

		}

#else
		MEDIA_FRAME_INFO	frameinfo;
		memset(&frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
		ret = SSQ_GetData(fInput.mVideoQueue, NULL, NULL, &frameinfo, (char *)fTo, NULL, 0x01);
		if (ret == 0x00)
		{
			/*
			struct timeval tv={0,0};
			gettimeofday(&tv, NULL);
			int iTimes = (tv.tv_sec*1000+tv.tv_usec/1000) - (frameinfo.rtpTimestamp_sec*1000+frameinfo.rtpTimestamp_usec/1000);
			_TRACE(TRACE_LOG_INFO, (char*)"间隔时长: %d   缓存帧数: %d\n", iTimes, fInput.mVideoQueue->pQueHeader->videoframes);
			*/
			//if ( (unsigned char)fTo[0] == 0x67 || (unsigned char)fTo[0] == 0x61
			if (frameinfo.rtpTimestamp_sec > 0U || frameinfo.rtpTimestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = frameinfo.rtpTimestamp_sec;
				fPresentationTime.tv_usec = frameinfo.rtpTimestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
			fFrameSize = frameinfo.length;
			fNumTruncatedBytes = 0;
		}
#endif
	}


	return ret;
}


////////// LiveAudioStreamSource implementation //////////

LiveAudioStreamSource::LiveAudioStreamSource(UsageEnvironment& env, LiveSource& input)
  : LiveStreamSource(env, input) 
{
	fMediaType = MEDIA_TYPE_AUDIO;
}

LiveAudioStreamSource::~LiveAudioStreamSource() 
{
	if (NULL != fInput.fOurAudioSource)
	{
		Medium::close(fInput.fOurAudioSource, fInput.GetEnvironment());//.envir());
		fInput.fOurAudioSource = NULL;
	}
}

int LiveAudioStreamSource::readFromFile() 
{
	int ret = -1;
	if (NULL == fInput.pLiveAudioFrame && NULL == fInput.mAudioQueue)			return ret;

	if (NULL != fInput.pLiveAudioFrame->framedata)
	{
		if (fInput.pLiveAudioFrame->framesize < 1)		return -1;

		memcpy(fTo, fInput.pLiveAudioFrame->framedata, fInput.pLiveAudioFrame->framesize);
		fFrameSize = fInput.pLiveAudioFrame->framesize;
		if (fInput.pLiveAudioFrame->timestamp_sec > 0U || fInput.pLiveAudioFrame->timestamp_usec > 0U)
		{
			fPresentationTime.tv_sec = fInput.pLiveAudioFrame->timestamp_sec;
			fPresentationTime.tv_usec = fInput.pLiveAudioFrame->timestamp_usec;
		}
		else
		{
			gettimeofdayEx(&fPresentationTime, 0);
		}
		fNumTruncatedBytes = 0;

		fInput.pLiveAudioFrame->framesize = 0;

		ret = 0;
	}
	else if (NULL != fInput.mAudioQueue)
	{
#if 1
		LIVE_FRAME_INFO_T	frameinfo;
		memset(&frameinfo, 0x00, sizeof(LIVE_FRAME_INFO_T));

		ret = -1;
		if (NULL == consumerHandle)	consumerHandle = BUFQUE_RegisterConsumer(fInput.mAudioQueue, (unsigned long)this, 0);
		if (NULL != consumerHandle)
		{
			ret = BUFQUE_GetData(fInput.mAudioQueue, consumerHandle, NULL, NULL, 
													NULL, (char *)&frameinfo, NULL, (char *)fTo, 0x01);
		}
		fFrameSize = 0;

		if (ret == 0x00)
		{
			fInput.GetEnvironment()->IncEnvirCount();	//增加计数

			fFrameSize = frameinfo.size;

			if (fFrameSize > fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			else
			{
				fNumTruncatedBytes = 0;
			}

			if (frameinfo.timestamp_sec > 0U || frameinfo.timestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = frameinfo.timestamp_sec;
				fPresentationTime.tv_usec = frameinfo.timestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
		}

#else
		MEDIA_FRAME_INFO	frameinfo;
		memset(&frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
		//_TRACE(TRACE_LOG_INFO, "Get Audio Data...\n");

		ret = SSQ_GetData(fInput.mAudioQueue, NULL, NULL, &frameinfo, (char *)fTo, NULL, 0x01);
		//_TRACE(TRACE_LOG_INFO, "Get Audio Data %s...\n", ret==0x00?"ok":"fail");
		fFrameSize = 0;

		if (ret == 0x00)
		{
			fFrameSize = frameinfo.length;

			if (fFrameSize > fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			else
			{
				fNumTruncatedBytes = 0;
			}

			if (frameinfo.rtpTimestamp_sec > 0U || frameinfo.rtpTimestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = frameinfo.rtpTimestamp_sec;
				fPresentationTime.tv_usec = frameinfo.rtpTimestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
		}
#endif
	}
	return ret;
}







////////// LiveMetadataStreamSource implementation //////////

LiveMetadataStreamSource::LiveMetadataStreamSource(UsageEnvironment& env, LiveSource& input)
  : LiveStreamSource(env, input) 
{
	fMediaType = MEDIA_TYPE_EVENT;
}

LiveMetadataStreamSource::~LiveMetadataStreamSource() 
{
	if (NULL != fInput.fOurMetadataSource)
	{
		Medium::close(fInput.fOurMetadataSource, fInput.GetEnvironment());//.envir());
		fInput.fOurMetadataSource = NULL;
	}
}

int LiveMetadataStreamSource::readFromFile() 
{
	int ret = -1;
	if (NULL == fInput.mMetadataQueue)			return ret;

	if (NULL != fInput.mMetadataQueue)
	{
		LIVE_FRAME_INFO_T	frameinfo;
		memset(&frameinfo, 0x00, sizeof(LIVE_FRAME_INFO_T));

		ret = -1;
		if (NULL == consumerHandle)	consumerHandle = BUFQUE_RegisterConsumer(fInput.mMetadataQueue, (unsigned long)this, 0);
		if (NULL != consumerHandle)
		{
			ret = BUFQUE_GetData(fInput.mMetadataQueue, consumerHandle, NULL, NULL, 
													NULL, (char *)&frameinfo, NULL, (char *)fTo, 0x01);
		}
		fFrameSize = 0;

		if (ret == 0x00)
		{

			fInput.GetEnvironment()->IncEnvirCount();	//增加计数

/*
			CUSTOM_META_DATA_HEADER_T *pMetaHdr = (CUSTOM_META_DATA_HEADER_T *)fTo;
			pMetaHdr->nsize = sizeof(CUSTOM_META_DATA_HEADER_T);

			if (fInput.mMediaInfoPtr && (fInput.mMediaInfoPtr->metadataCodec == CUSTOM_META_STREAM_TYPE) )
			{
				pMetaHdr->moduleid = VCA_MODULE_ID;
				pMetaHdr->customid = VCA_MODULE_ID;
			}
			else
			{

			}
			pMetaHdr->datalen = frameinfo.size;
			//fFrameSize = frameinfo.size + sizeof(CUSTOM_META_DATA_HEADER_T);
*/
			fFrameSize = frameinfo.size;

			if (fFrameSize > fMaxSize)
			{
				fNumTruncatedBytes = fFrameSize - fMaxSize;
				fFrameSize = fMaxSize;
			}
			else
			{
				fNumTruncatedBytes = 0;
			}

			if (frameinfo.timestamp_sec > 0U || frameinfo.timestamp_usec > 0U)
			{
				fPresentationTime.tv_sec = frameinfo.timestamp_sec;
				fPresentationTime.tv_usec = frameinfo.timestamp_usec;
			}
			else
			{
				gettimeofdayEx(&fPresentationTime, 0);
			}
		}
	}
	return ret;
}

