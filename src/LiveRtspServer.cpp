/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2016, Live Networks, Inc.  All rights reserved

#include <BasicUsageEnvironment.hh>
#include "LiveRtspServer.h"
#include "LiveServerMediaSession.h"
#include "version.hh"


#include "LiveRtspServer.h"
#include <liveMedia.hh>
#include <string.h>
#include "LiveH264VideoServerMediaSubsession.h"
#include "LiveH265VideoServerMediaSubsession.h"
#include "LiveJPEGVideoServerMediaSubsession.h"
#include "LiveAudioServerMediaSubsession.h"
#include "LiveMetadataServerMediaSubsession.h"
#include "libRTSPServerAPI.h"
#include "trace.h"
#include <GroupsockHelper.hh>
#include "live/liveMedia/include/RTSPCommon.hh"


int	CreateLiveFrame(LIVE_FRAME_INFO_T *frame, int size)
{
	if (NULL == frame)		return -1;

	if (NULL == frame->framedata)
	{
		frame->size = size;
		frame->framedata = new unsigned char[frame->size];
		if (NULL != frame->framedata)		memset(frame->framedata, 0x00, frame->size);
		frame->framesize = 0;
	}
	if (NULL == frame->framedata)		return -1;

	return 0;
}
void DeleteLiveFrame(LIVE_FRAME_INFO_T *frame)
{
	if (NULL == frame)		return;

	if (NULL != frame->framedata)
	{
		delete []frame->framedata;
		frame->framedata = NULL;
	}
	frame->framesize = 0;
	frame->size = 0;
}

LiveRtspServer	*LiveRtspServer::createNew(UsageEnvironment& env, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, void *_callback, void *_userptr) 
{
  int ourSocket4 = setUpOurSocket4(env, ourPort);
  if (ourSocket4 == -1) return NULL;

  int ourSocket6 = setUpOurSocket6(env, ourPort);
  //if (ourSocket6 == -1) return NULL;	//2018.09.13  部分设备不支持IPV6
  return new LiveRtspServer(env, ourSocket4, ourSocket6, ourPort, authDatabase, reclamationTestSeconds, _callback, _userptr);
}

LiveRtspServer::LiveRtspServer(UsageEnvironment& env,int ourSocketV4, int ourSocketV6, Port ourPort, UserAuthenticationDatabase* authDatabase,unsigned reclamationTestSeconds, void *_callback, void *_userptr)
  : RTSPServer(env, ourSocketV4, ourSocketV6, ourPort, authDatabase, reclamationTestSeconds, _callback, _userptr) 
{
	liveChannelNum	=	0;
	pLiveChannel	=	NULL;
	rtpInitialPortNum	=	9670;

	InitMutex(&mutexChannel);
}

LiveRtspServer::~LiveRtspServer() 
{
	//cleanup();
	DeleteAllChannel();

	DeinitMutex(&mutexChannel);
}

int	LiveRtspServer::CreateChannel(const char *streamName, RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr)
{
	LIVE_CHANNEL_OBJ_T		*pNewChannel = new LIVE_CHANNEL_OBJ_T;
	if (NULL == pNewChannel)			return RTSP_SERVER_ERR_ALLOC_MEMORY;

	memset(pNewChannel, 0x00, sizeof(LIVE_CHANNEL_OBJ_T));
	strcpy(pNewChannel->streamName, streamName);
	pNewChannel->status = LIVE_FLAG;

	pNewChannel->channelUserPtr = channelPtr;
	*channelHandle = pNewChannel;


	LockMutex(&mutexChannel);		//Lock
	if (NULL == pLiveChannel)		pLiveChannel = pNewChannel;
	else
	{
		LIVE_CHANNEL_OBJ_T *pLast = pLiveChannel;
		while (NULL != pLast->next)
		{
			pLast = pLast->next;
		}
		pLast->next = pNewChannel;
	}
	UnlockMutex(&mutexChannel);		//Unlock

	return RTSP_SERVER_ERR_NoErr;
}

//删除指定通道
int	LiveRtspServer::DeleteChannel(RTSP_CHANNEL_HANDLE *channelHandle)
{
	LockMutex(&mutexChannel);		//Lock

	int iDelRet = -1;

	LIVE_CHANNEL_OBJ_T	*pLast = pLiveChannel;
	LIVE_CHANNEL_OBJ_T	*pChannel = pLiveChannel;
	while (NULL != pChannel)
	{
		if (pChannel == *channelHandle)
		{
			iDelRet = 0;

			if (pChannel == pLiveChannel)				//删除的是第一个
			{
				pLiveChannel = pChannel->next;			//指向第二个

				LIVE_CHANNEL_OBJ_T *pDelChannel = pChannel;

				pDelChannel->status = 0x00;

				//销毁通道
				if (NULL != pDelChannel->liveSource)
				{
					//ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(&pDelChannel->liveSource->envir(), 0, NULL, pDelChannel->streamName);
					//if (NULL != sms)
					//{
					//	closeAllClientSessionsForServerMediaSession(sms);
					//	sms = NULL;
					//}

					Medium::close(pDelChannel->liveSource, pDelChannel->liveSource->GetEnvironment());
					pDelChannel->liveSource = NULL;
				}
				if (NULL != pDelChannel->videoQueue)
				{
					//SSQ_Deinit(pDelChannel->videoQueue);
					//delete pDelChannel->videoQueue;
					BUFQUE_Release(&pDelChannel->videoQueue);
					pDelChannel->videoQueue = NULL;
				}
				if (NULL != pDelChannel->audioQueue)
				{
					//SSQ_Deinit(pDelChannel->audioQueue);
					//delete pDelChannel->audioQueue;
					BUFQUE_Release(&pDelChannel->audioQueue);
					pDelChannel->audioQueue = NULL;
				}
				if (NULL != pDelChannel->metadataQueue)
				{
					BUFQUE_Release(&pDelChannel->metadataQueue);
					pDelChannel->metadataQueue = NULL;
				}

				DeleteLiveFrame(&pDelChannel->videoFrame);
				DeleteLiveFrame(&pDelChannel->audioFrame);

				delete pDelChannel;
			}
			else
			{
				LIVE_CHANNEL_OBJ_T *pDelChannel = pChannel;
				pLast->next = pChannel->next;					//指向下一个
				pChannel = pLast->next;

				pDelChannel->status = 0x00;

				//销毁通道
				if (NULL != pDelChannel->liveSource)
				{
					////以下sms的部分, 不需要
					//ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(&pDelChannel->liveSource->envir(), 0, NULL, pDelChannel->streamName);
					//if (NULL != sms)
					//{
					//	closeAllClientSessionsForServerMediaSession(sms);
					//	sms = NULL;
					//}


					Medium::close(pDelChannel->liveSource, pDelChannel->liveSource->GetEnvironment());
					pDelChannel->liveSource = NULL;
				}
				if (NULL != pDelChannel->videoQueue)
				{
					//SSQ_Deinit(pDelChannel->videoQueue);
					//delete pDelChannel->videoQueue;
					BUFQUE_Release(&pDelChannel->videoQueue);
					pDelChannel->videoQueue = NULL;
				}
				if (NULL != pDelChannel->audioQueue)
				{
					//SSQ_Deinit(pDelChannel->audioQueue);
					//delete pDelChannel->audioQueue;
					BUFQUE_Release(&pDelChannel->audioQueue);
					pDelChannel->audioQueue = NULL;
				}
				if (NULL != pDelChannel->metadataQueue)
				{
					BUFQUE_Release(&pDelChannel->metadataQueue);
					pDelChannel->metadataQueue = NULL;
				}

				DeleteLiveFrame(&pDelChannel->videoFrame);
				DeleteLiveFrame(&pDelChannel->audioFrame);

				delete pDelChannel;
			}

			break;
		}
		pLast = pChannel;
		if (NULL != pChannel)		pChannel = pChannel->next;
	}

	UnlockMutex(&mutexChannel);		//Unlock

	if (iDelRet == 0x00)
	{
		*channelHandle = NULL;
	}

	return RTSP_SERVER_ERR_NoErr;
}

//删除所有通道
int		LiveRtspServer::DeleteAllChannel()
{
#ifdef LIVE_MULTI_THREAD_ENABLE
	deleteAllWorkerThread(1);
#endif

	//LockMutex(&mutexChannel);		//Lock

	LIVE_CHANNEL_OBJ_T	*pLast = pLiveChannel;
	LIVE_CHANNEL_OBJ_T	*pChannel = pLiveChannel;
	while (NULL != pChannel)
	{
		LIVE_CHANNEL_OBJ_T *pDelChannel = pChannel;
		pChannel = pChannel->next;

		//pDelChannel->reset = 1;
		//CheckChannelStatus();

		//销毁通道
		if (NULL != pDelChannel->videoQueue)
		{
			//SSQ_Deinit(pDelChannel->videoQueue);
			//delete pDelChannel->videoQueue;
			BUFQUE_Release(&pDelChannel->videoQueue);
			pDelChannel->videoQueue = NULL;
		}
		if (NULL != pDelChannel->audioQueue)
		{
			//SSQ_Deinit(pDelChannel->audioQueue);
			//delete pDelChannel->audioQueue;
			BUFQUE_Release(&pDelChannel->audioQueue);
			pDelChannel->audioQueue = NULL;
		}
		if (NULL != pDelChannel->metadataQueue)
		{
			//SSQ_Deinit(pDelChannel->audioQueue);
			//delete pDelChannel->audioQueue;
			BUFQUE_Release(&pDelChannel->metadataQueue);
			pDelChannel->metadataQueue = NULL;
		}

		DeleteLiveFrame(&pDelChannel->videoFrame);
		DeleteLiveFrame(&pDelChannel->audioFrame);

#ifdef LIVE_MULTI_THREAD_ENABLE
		ServerMediaSession* sms = RTSPServer::lookupServerMediaSession((NULL!=pDelChannel->liveSource)?&pDelChannel->liveSource->envir():NULL, 0, NULL, pDelChannel->streamName, False);
#else
		ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(pDelChannel->streamName);
#endif
		if (NULL != sms)
		{
			_TRACE(TRACE_LOG_DEBUG, (char*)"Close ServerMediaSession: %s\n", pDelChannel->streamName);
			closeAllClientSessionsForServerMediaSession(sms, False);
		}

		if (NULL != pDelChannel)		//已在liveServerMediaSession中释放
		{
			//Medium::close(pDelChannel->liveSource, pDelChannel->liveSource->GetEnvironment());
			//pDelChannel->liveSource = NULL;
		}

		//delete pDelChannel;
	}

	//UnlockMutex(&mutexChannel);		//Unlock

	return RTSP_SERVER_ERR_NoErr;
}



static LiveServerMediaSession* createNewSMS(UsageEnvironment& env,	LIVE_CHANNEL_OBJ_T *pChannel, void **callbackptr, void *userptr, portNumBits &port);//char const* streamName, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource); // forward
static ServerMediaSession* createNewSMS_File(UsageEnvironment& env,	char const* fileName, FILE* /*fid*/);


#ifdef LIVE_MULTI_THREAD_ENABLE
ServerMediaSession* LiveRtspServer::lookupServerMediaSession(UsageEnvironment	*pEnv, int iType, void *pClientPtr, char const* streamName, Boolean bLockServerMediaSession, Boolean isFirstLookupInSession) 
#else
ServerMediaSession* LiveRtspServer::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession) 
#endif
{
#if 1

	//只有工作线程才会执行该函数
	if (pEnv->GetEnvirId() == MAIN_THREAD_ID)
	{
		_TRACE(TRACE_LOG_INFO, (char*)"##### 错误:  lookupServerMediaSession 被主线程执行[%d:%s]  streamName[%s]\n",  pEnv->GetEnvirId(), pEnv->GetEnvirName(), streamName);
		return NULL;
	}

	_TRACE(TRACE_LOG_INFO, (char*)"lookupServerMediaSession workerThread[%d:%s] StreamName[%s]\n",  pEnv->GetEnvirId(), pEnv->GetEnvirName(), streamName);

	// Next, check whether we already have a "ServerMediaSession" for this file:
	if (NULL == streamName)					return NULL;
	if ( (int)strlen(streamName) < 1)		return NULL;
#ifdef LIVE_MULTI_THREAD_ENABLE
	RTSPClientConnection	*pRtspClient = (RTSPClientConnection *)pClientPtr;
	if (NULL != pRtspClient)
	{
		//_TRACE(TRACE_LOG_DEBUG, "new request:  RTSPClientConnection[0x%X] streamName[%s]\n", pRtspClient, streamName);
	}
#else
	int	iType = 0;
#endif

	ServerMediaSession* sms = NULL;

	if (bLockServerMediaSession)	LockServerMediaSession(pEnv->GetEnvirName(), "LiveRtspServer::lookupServerMediaSession", (unsigned long long)this);
	do
	{
		//LockServerMediaSession();
#ifdef LIVE_MULTI_THREAD_ENABLE
		sms = RTSPServer::lookupServerMediaSession(pEnv, iType, pClientPtr, streamName, bLockServerMediaSession);
#else
		sms = RTSPServer::lookupServerMediaSession(streamName);
#endif

		
		Boolean smsExists = sms != NULL;
		if (NULL != sms)
		{
			if (&sms->envir() != pEnv)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"lookupServerMediaSession ERROR envir[%s] 不匹配  [%s]\n", sms->envir().GetEnvirName(), pEnv->GetEnvirName());

				FILE *f = fopen("lookupServerMediaSession.txt", "wb");
				if (NULL != f)
				{
					char sztmp[256] = {0};
					sprintf(sztmp, "[%s] lookupServerMediaSession error. envir不匹配 sms.envir[%s]  ptr[0x%X]\n", pEnv->GetEnvirName(), sms->envir().GetEnvirName(), this);

					fwrite(sztmp, 1, (int)strlen(sztmp), f);
					fclose(f);
				}
				sms = NULL;
				break;
			}

			//if (sms->GetStreamStatus() == 0x00 && iType==1)		//iType  1:handleCmd_DESCRIBE 函数中调用    2:handleCmd_SETUP 函数中调用
			if (pEnv->GetStreamStatus() == 0x00 && iType==1)		//iType  1:handleCmd_DESCRIBE 函数中调用    2:handleCmd_SETUP 函数中调用
			{
				//_TRACE(TRACE_LOG_DEBUG, (char*)"当前sms未就绪, 关闭当前连接.\n");
				_TRACE(TRACE_LOG_DEBUG, (char*)"The current sms is not ready, close the current connection.\n");
				
				sms = NULL;
				break;
			}
			/*
			if (NULL != pRtspClient)
			{
				_TRACE(TRACE_LOG_DEBUG, (char*)"sms already exist: 0x%X\t%s\n", pRtspClient, streamName);
			}
			else
			{
				_TRACE(TRACE_LOG_DEBUG, (char*)"sms already exist: %s\n", streamName);
			}
			*/
			break;
		}
		/*		//test
		if (NULL != sms)
		{
		  removeServerMediaSession(sms);
		  sms = NULL;
		  return NULL;
		}
		*/

		RTSPSvrCallBack	pCallback = (RTSPSvrCallBack )mCallbackPtr;
		if (NULL == pCallback)
		{
			_TRACE(TRACE_LOG_ERROR, (char*)"Callback not be set.\n");
			break;
		}

		bool bLock = bLockServerMediaSession;
		if (bLock)
		{
			UnlockServerMediaSession(pEnv->GetEnvirName(), (char*)"LiveRtspServer::lookupServerMediaSession", (unsigned long long)this);
			bLock = false;
		}

		RTSP_MEDIA_INFO_T	mediainfo;
		memset(&mediainfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));
		RTSP_CHANNEL_HANDLE	channelHandle = NULL;
		if (pCallback(RTSP_CHANNEL_OPEN_STREAM, streamName, &channelHandle, &mediainfo, NULL, mUserPtr, NULL) < 0)
		{
			bLockServerMediaSession = False;		//因为当前已解锁, 所以while后不需解锁
			_TRACE(TRACE_LOG_WARNING, (char*)"Application layer return failed. stream name: %s\n", streamName);
			break;	//应用层返回值为失败
		}

		if (mediainfo.vpsLength >= sizeof(mediainfo.vps) ||
			mediainfo.spsLength >= sizeof(mediainfo.sps) ||
			mediainfo.ppsLength >= sizeof(mediainfo.pps))
		{
			bLockServerMediaSession = False;		//因为当前已解锁, 所以while后不需解锁
			
			FILE *f = fopen("mediainfo_error.txt", "wb");
			if (NULL != f)
			{
				char szlog[128] = {0};
				sprintf(szlog, "mediainfo中的sps和pps长度超出最大范围. vpslength[%d] spslength[%d]  ppsLength[%d]\n", 
								mediainfo.vpsLength, mediainfo.spsLength, mediainfo.ppsLength);
				fwrite(szlog, 1, (int)strlen(szlog), f);
				fclose(f);
			}

			//_TRACE(TRACE_LOG_WARNING, "应用层返回值为成功. 但没有设置通道句柄.  stream name: %s\n", streamName);
			break;	//应用层返回值为失败
		}

		if (NULL == channelHandle)
		{
			bLockServerMediaSession = False;		//因为当前已解锁, 所以while后不需解锁
			_TRACE(TRACE_LOG_WARNING, (char*)"The application layer does not have a channel handle set...  stream name: %s\n", streamName);
			//_TRACE(TRACE_LOG_WARNING, "应用层返回值为成功. 但没有设置通道句柄.  stream name: %s\n", streamName);
			break;	//应用层返回值为失败
		}

		//仅在回调中设置了相关的编码信息, 才继续创建
		if ( (RTSP_VIDEO_CODEC_MJPEG==mediainfo.videoCodec) || (mediainfo.videoCodec > 0))// && mediainfo.spsLength>0 && mediainfo.ppsLength>0) )
		//if (mediainfo.videoCodec > 0 && mediainfo.spsLength>0 && mediainfo.ppsLength>0)
		{
			//LockServerMediaSession();	//Lock

			if (bLockServerMediaSession && !bLock)
			{
				LockServerMediaSession(pEnv->GetEnvirName(), "LiveRtspServer::lookupServerMediaSession 2", (unsigned long long)this);

				//再次锁定后检查, 是否已存在该ServerMediaSession
				sms = RTSPServer::lookupServerMediaSession(pEnv, iType, pClientPtr, streamName, bLockServerMediaSession);
				if (NULL != sms)
				{
					_TRACE(TRACE_LOG_WARNING, (char*)"*******************SMS 已被创建. return..... stream name: %s     env[%s]  sms.envir[%s]\n", 
						streamName,  pEnv->GetEnvirName(), sms->envir().GetEnvirName());

					if ( &sms->envir() != pEnv)
					{
						_TRACE(TRACE_LOG_WARNING, (char*)"*******************SMS 已被创建. 但 pEnv 不匹配. 返回NULL. stream name: %s     env[%s]  sms.envir[%s]\n", 
							streamName,  pEnv->GetEnvirName(), sms->envir().GetEnvirName());

						sms = NULL;
					}

					break;
				}
			}

			//printf("创建通道: %s\n", streamName);

			liveChannelNum++;

			LIVE_CHANNEL_OBJ_T		*pNewChannel = (LIVE_CHANNEL_OBJ_T*)channelHandle;
			pNewChannel->id = liveChannelNum;
			//memcpy(&pNewChannel->mediainfo, &mediainfo, sizeof(RTSP_MEDIA_INFO_T));

			//创建视频队列
			if (NULL != pNewChannel->videoQueue)
			{
				//SSQ_Deinit(pNewChannel->videoQueue);
				//delete pNewChannel->videoQueue;
				BUFQUE_Release(&pNewChannel->videoQueue);
				pNewChannel->videoQueue = NULL;
			}
			if (NULL == pNewChannel->videoQueue)
			{
				if (mediainfo.videoQueueSize < LIVE_VIDEO_QUEUE_DEFAULT_SIZE) mediainfo.videoQueueSize = LIVE_VIDEO_QUEUE_DEFAULT_SIZE;
#if 1
				BUFQUE_Create(&pNewChannel->videoQueue, pNewChannel->id, "", mediainfo.videoQueueSize, 1, 0x00, 0x01);
#else
				pNewChannel->videoQueue = new SS_QUEUE_OBJ_T;
				if (NULL == pNewChannel->videoQueue)
				{
					_TRACE(TRACE_LOG_ERROR, (char*)"insufficient memory ???\n");
					break;
				}

				memset(pNewChannel->videoQueue, 0x00, sizeof(SS_QUEUE_OBJ_T));
				SSQ_Init(pNewChannel->videoQueue, 0x00, pNewChannel->id, NULL, mediainfo.videoQueueSize, 1, 0x00);
#endif
			}

			//创建音频队列
			if (mediainfo.audioCodec > 0 && mediainfo.audioBitsPerSample>0 && mediainfo.audioChannel>0 && mediainfo.audioSampleRate>0)
			{
				if (NULL != pNewChannel->audioQueue)
				{
					//SSQ_Deinit(pNewChannel->audioQueue);
					//delete pNewChannel->audioQueue;
					BUFQUE_Release(&pNewChannel->audioQueue);
					pNewChannel->audioQueue = NULL;
				}
				if (NULL == pNewChannel->audioQueue)
				{
					if (mediainfo.audioQueueSize < LIVE_AUDIO_QUEUE_DEFAULT_SIZE) mediainfo.audioQueueSize = LIVE_AUDIO_QUEUE_DEFAULT_SIZE;
#if 1

					BUFQUE_Create(&pNewChannel->audioQueue, pNewChannel->id, "", mediainfo.audioQueueSize, 1, 0x00, 0x01);
#else
					pNewChannel->audioQueue = new SS_QUEUE_OBJ_T;
					if (NULL == pNewChannel->audioQueue)
					{
						_TRACE(TRACE_LOG_ERROR, (char*)"insufficient memory ???\n");
						break;
					}

					memset(pNewChannel->audioQueue, 0x00, sizeof(SS_QUEUE_OBJ_T));
					SSQ_Init(pNewChannel->audioQueue, 0x00, pNewChannel->id, NULL, mediainfo.audioQueueSize, 1, 0x00);
#endif
				}
			}

			if (mediainfo.metadataCodec > 0)
			{
				if (mediainfo.metadataQueueSize < LIVE_METADATA_QUEUE_DEFAULT_SIZE) mediainfo.metadataQueueSize = LIVE_METADATA_QUEUE_DEFAULT_SIZE;

				if (NULL != pNewChannel->metadataQueue)
				{
					BUFQUE_Release(&pNewChannel->metadataQueue);
					pNewChannel->metadataQueue = NULL;
				}
				if (NULL == pNewChannel->metadataQueue)
				{
					BUFQUE_Create(&pNewChannel->metadataQueue, pNewChannel->id, "", mediainfo.metadataQueueSize, 1, 0x00, 0x01);
				}
			}

#ifndef LIVE_MULTI_THREAD_ENABLE
				UsageEnvironment *pEnv = &envir();
#endif

				memcpy(&pNewChannel->mediainfo, &mediainfo, sizeof(RTSP_MEDIA_INFO_T));
			//创建Source
			if (NULL == pNewChannel->liveSource)
			{
				pNewChannel->liveSource = LiveSource::createNew(*pEnv, sms, pNewChannel->id, pNewChannel->streamName, channelHandle, &pNewChannel->mediainfo, 
												&pNewChannel->videoFrame, &pNewChannel->audioFrame, 
												pNewChannel->videoQueue, pNewChannel->audioQueue, pNewChannel->metadataQueue,
												mCallbackPtr, mUserPtr, pNewChannel->channelUserPtr);
			}

			sms = createNewSMS(*pEnv, pNewChannel, (void**)&mCallbackPtr, mUserPtr, rtpInitialPortNum);//streamName, &pRequestChannel->mediainfo, pRequestChannel->liveSource);
#ifdef LIVE_MULTI_THREAD_ENABLE
			addServerMediaSession(pEnv, sms);
#else
			addServerMediaSession(sms);
#endif

			//UnlockServerMediaSession();
		}
		else
		{
			//如果当前创建了通道, 而媒体信息不完整, 则回调关闭该流
			if (NULL != channelHandle)
			{
				LIVE_CHANNEL_OBJ_T	*pChannelObj = (LIVE_CHANNEL_OBJ_T *)channelHandle;

				//printf("当前创建了通道, 而媒体信息不完整, 回调关闭该流: %s\n", streamName);

				_TRACE(TRACE_LOG_WARNING, (char*)"Media information is incomplete, callback closes the stream...  stream name: %s\n", streamName);

				pCallback(RTSP_CHANNEL_CLOSE_STREAM, streamName, &channelHandle, &mediainfo, NULL, mUserPtr, pChannelObj->channelUserPtr);
			}
		}
	}while (0);

	//UnlockMutex(&mutexServerMediaSession);


	if (bLockServerMediaSession)	UnlockServerMediaSession(pEnv->GetEnvirName(), "LiveRtspServer::lookupServerMediaSession", (unsigned long long)this);

	return sms;
#else
  // First, check whether the specified "streamName" exists as a local file:
  FILE* fid = fopen(streamName, "rb");
  Boolean fileExists = fid != NULL;

  // Next, check whether we already have a "ServerMediaSession" for this file:
  ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(streamName);
  Boolean smsExists = sms != NULL;

  // Handle the four possibilities for "fileExists" and "smsExists":
  if (!fileExists) {
    if (smsExists) {
      // "sms" was created for a file that no longer exists. Remove it:
      removeServerMediaSession(sms, True);
      sms = NULL;
    }

    return NULL;
  } else {
    if (smsExists && isFirstLookupInSession) { 
      // Remove the existing "ServerMediaSession" and create a new one, in case the underlying
      // file has changed in some way:
      removeServerMediaSession(sms, True); 
      sms = NULL;
    } 

    if (sms == NULL) {
      sms = createNewSMS_File(envir(), streamName, fid); 
      addServerMediaSession(sms);
    }

    fclose(fid);
    return sms;
  }
#endif
}

// Special code for handling Matroska files:
struct MatroskaDemuxCreationState 
{  
	MatroskaFileServerDemux* demux;
	char watchVariable;
};
static void onMatroskaDemuxCreation(MatroskaFileServerDemux* newDemux, void* clientData) 
{
  MatroskaDemuxCreationState* creationState = (MatroskaDemuxCreationState*)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}
// END Special code for handling Matroska files:

// Special code for handling Ogg files:
struct OggDemuxCreationState 
{
  OggFileServerDemux* demux;
  char watchVariable;
};
static void onOggDemuxCreation(OggFileServerDemux* newDemux, void* clientData) 
{
  OggDemuxCreationState* creationState = (OggDemuxCreationState*)clientData;
  creationState->demux = newDemux;
  creationState->watchVariable = 1;
}
// END Special code for handling Ogg files:

#define NEW_SMS(description) do {\
char const* descStr = description\
    ", streamed by the " RTSP_SERVER_NAME;\
sms = LiveServerMediaSession::createNew(env, pChannel->streamName, pChannel->streamName, descStr, False, NULL, pChannel, callbackptr, userptr);\
} while(0)

static LiveServerMediaSession* createNewSMS(UsageEnvironment& env, LIVE_CHANNEL_OBJ_T *pChannel, void **callbackptr, void *userptr, portNumBits &port)//char const* streamName, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource) 
{
	if (NULL == pChannel)								return NULL;
	//if (NULL == pChannel->mediainfo)						return NULL;
	if ( (pChannel->mediainfo.videoCodec < 1) &&
		(pChannel->mediainfo.audioCodec < 1) &&
		(pChannel->mediainfo.metadataCodec<1) )
	{
		return NULL;
	}

	LiveServerMediaSession* sms = NULL;
	Boolean const reuseSource = False;

	//如果为“true”，则其他接入的客户端跟第一个客户端看到一样的视频流，否则其他客户端接入的时候将重新播放
	Boolean reuseFirstSource = True;

	//Video
	switch (pChannel->mediainfo.videoCodec)
	{
	case RTSP_VIDEO_CODEC_H264:
		{
			NEW_SMS("H.264 Video");
			OutPacketBuffer::maxSize = 1024*1024*2;//100000; // allow for some possibly large H.264 frames
			//sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
			sms->addSubsession(LiveH264VideoServerMediaSubsession::createNew(env, OutPacketBuffer::maxSize,  &pChannel->mediainfo, pChannel->liveSource, pChannel->streamName, "Video/H264", reuseFirstSource, port++));
		}
		break;
	case RTSP_VIDEO_CODEC_H265:
		{
			NEW_SMS("H.265 Video");
			OutPacketBuffer::maxSize = 1024*1024*2;//100000; // allow for some possibly large H.265 frames
			//sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
			sms->addSubsession(LiveH265VideoServerMediaSubsession::createNew(env, OutPacketBuffer::maxSize, &pChannel->mediainfo, pChannel->liveSource, pChannel->streamName, "Video/H265", reuseFirstSource, port++));
		}
		break;
	case RTSP_VIDEO_CODEC_MJPEG:
		{
			NEW_SMS("JPEG Video");
			OutPacketBuffer::maxSize = 1024*1024*2;
			sms->addSubsession(LiveJPEGVideoServerMediaSubsession::createNew(env, OutPacketBuffer::maxSize, &pChannel->mediainfo, pChannel->liveSource, pChannel->streamName, "Video/MJPEG", reuseFirstSource, port++));
		}
		break;
	default:
		break;
	}

	//Audio
	if (pChannel->mediainfo.audioCodec > 0)
	{
		if (NULL == sms)		NEW_SMS("Live Audio");
		sms->addSubsession(LiveAudioServerMediaSubsession::createNew(env, &pChannel->mediainfo, pChannel->liveSource, pChannel->streamName, "AUDIO",reuseFirstSource, port++));
	}

	//Metadata
	if (pChannel->mediainfo.metadataCodec > 0)
	{
		if (NULL == sms)		NEW_SMS("Live Metadata");
		sms->addSubsession(LiveMetadataServerMediaSubsession::createNew(env, &pChannel->mediainfo, pChannel->liveSource, pChannel->streamName, "Metadata", reuseFirstSource, port++));
	}

	/*
	switch (pMediaInfo->audioCodec)
	{
	case RTSP_AUDIO_CODEC_G711U:
		break;
	case RTSP_AUDIO_CODEC_G711A:
		break;
	case RTSP_AUDIO_CODEC_G726:
		break;
	case RTSP_AUDIO_CODEC_AAC:
		sms->addSubsession(LiveAudioServerMediaSubsession::createNew(env, pMediaInfo, liveSource));
		break;
	default:
		break;
	}
	*/

	return sms;

#if 0
  if (strcmp(extension, ".aac") == 0) {
    // Assumed to be an AAC Audio (ADTS format) file:
    NEW_SMS("AAC Audio");
    sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".ac3") == 0) {
    // Assumed to be an AC-3 Audio file:
    NEW_SMS("AC-3 Audio");
    sms->addSubsession(AC3AudioFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".m4e") == 0) {
    // Assumed to be a MPEG-4 Video Elementary Stream file:
    NEW_SMS("MPEG-4 Video");
    sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".264") == 0) {
    // Assumed to be a H.264 Video Elementary Stream file:
    NEW_SMS("H.264 Video");
    OutPacketBuffer::maxSize = 1024*1024;//100000; // allow for some possibly large H.264 frames
    //sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
	sms->addSubsession(LiveH264VideoServerMediaSubsession::createNew(env, OutPacketBuffer::maxSize));
  } else if (strcmp(extension, ".265") == 0) {
    // Assumed to be a H.265 Video Elementary Stream file:
    NEW_SMS("H.265 Video");
    OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.265 frames
    sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".mp3") == 0) {
    // Assumed to be a MPEG-1 or 2 Audio file:
    NEW_SMS("MPEG-1 or 2 Audio");
    // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
//#define STREAM_USING_ADUS 1
    // To also reorder ADUs before streaming, uncomment the following:
//#define INTERLEAVE_ADUS 1
    // (For more information about ADUs and interleaving,
    //  see <http://www.live555.com/rtp-mp3/>)
    Boolean useADUs = False;
    Interleaving* interleaving = NULL;
#ifdef STREAM_USING_ADUS
    useADUs = True;
#ifdef INTERLEAVE_ADUS
    unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own...
    unsigned const interleaveCycleSize
      = (sizeof interleaveCycle)/(sizeof (unsigned char));
    interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
    sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(env, fileName, reuseSource, useADUs, interleaving));
  } else if (strcmp(extension, ".mpg") == 0) {
    // Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
    NEW_SMS("MPEG-1 or 2 Program Stream");
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(env, fileName, reuseSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession());
    sms->addSubsession(demux->newAudioServerMediaSubsession());
  } else if (strcmp(extension, ".vob") == 0) {
    // Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
    NEW_SMS("VOB (MPEG-2 video with AC-3 audio)");
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(env, fileName, reuseSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession());
    sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
  } else if (strcmp(extension, ".ts") == 0) {
    // Assumed to be a MPEG Transport Stream file:
    // Use an index file name that's the same as the TS file name, except with ".tsx":
    unsigned indexFileNameLen = strlen(fileName) + 2; // allow for trailing "x\0"
    char* indexFileName = new char[indexFileNameLen];
    sprintf(indexFileName, "%sx", fileName);
    NEW_SMS("MPEG Transport Stream");
    sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(env, fileName, indexFileName, reuseSource));
    delete[] indexFileName;
  } else if (strcmp(extension, ".wav") == 0) {
    // Assumed to be a WAV Audio file:
    NEW_SMS("WAV Audio Stream");
    // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
    // change the following to True:
    Boolean convertToULaw = False;
    sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(env, fileName, reuseSource, convertToULaw));
  } else if (strcmp(extension, ".dv") == 0) {
    // Assumed to be a DV Video file
    // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
    OutPacketBuffer::maxSize = 300000;

    NEW_SMS("DV Video");
    sms->addSubsession(DVVideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".mkv") == 0 || strcmp(extension, ".webm") == 0) {
    // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
    OutPacketBuffer::maxSize = 100000; // allow for some possibly large VP8 or VP9 frames
    NEW_SMS("Matroska video+audio+(optional)subtitles");

    // Create a Matroska file server demultiplexor for the specified file.
    // (We enter the event loop to wait for this to complete.)
    MatroskaDemuxCreationState creationState;
    creationState.watchVariable = 0;
    MatroskaFileServerDemux::createNew(env, fileName, onMatroskaDemuxCreation, &creationState);
    env.taskScheduler().doEventLoop(&creationState.watchVariable);

    ServerMediaSubsession* smss;
    while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
    }
  } else if (strcmp(extension, ".ogg") == 0 || strcmp(extension, ".ogv") == 0 || strcmp(extension, ".opus") == 0) {
    // Assumed to be an Ogg file
    NEW_SMS("Ogg video and/or audio");

    // Create a Ogg file server demultiplexor for the specified file.
    // (We enter the event loop to wait for this to complete.)
    OggDemuxCreationState creationState;
    creationState.watchVariable = 0;
    OggFileServerDemux::createNew(env, fileName, onOggDemuxCreation, &creationState);
    env.taskScheduler().doEventLoop(&creationState.watchVariable);

    ServerMediaSubsession* smss;
    while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
    }
  }

  return sms;
#endif
}



int	LiveRtspServer::PutFrame(RTSP_CHANNEL_HANDLE channelHandle, unsigned int mediaType, MEDIA_FRAME_INFO_T *frame)
{
	if (NULL == channelHandle)		return RTSP_SERVER_ERR_BadArgument;
	if (NULL == frame)				return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pLiveChannel)		return RTSP_SERVER_ERR_NotInitialized;
	

	LIVE_CHANNEL_OBJ_T	*pStreamChannel = (LIVE_CHANNEL_OBJ_T *)channelHandle;

	if (0xFFFFFFFF != pStreamChannel->status)	return RTSP_SERVER_ERR_NotInitialized;

	if (pStreamChannel->resetFlag == 0x01)
	{
		_TRACE(TRACE_LOG_WARNING, (char*)"The channel is reset: [%d@%s]\n", pStreamChannel->id, pStreamChannel->streamName);
		return -1;
	}

	if (NULL != pStreamChannel)
	{
#if 1
		LIVE_FRAME_INFO_T		frameinfo;
		memset(&frameinfo, 0x00, sizeof(LIVE_FRAME_INFO_T));
		frameinfo.size = frame->frameSize;
		frameinfo.frametype = frame->frameType;
		frameinfo.timestamp_sec = frame->rtpTimestamp / 1000;
		frameinfo.timestamp_usec = (frame->rtpTimestamp % 1000) * 1000;

		if (mediaType == MEDIA_TYPE_VIDEO)
		{
			frameinfo.codec = pStreamChannel->mediainfo.videoCodec;

			if (NULL != pStreamChannel->videoQueue)
			{
				if ( ((unsigned char)frame->pBuffer[0] == 0x00) &&
					 ((unsigned char)frame->pBuffer[1] == 0x00) &&
					 ((unsigned char)frame->pBuffer[2] == 0x00) &&
					 ((unsigned char)frame->pBuffer[3] == 0x01) )
				{
					frameinfo.size -= 4;
					BUFQUE_AddData(pStreamChannel->videoQueue, 
									pStreamChannel->id, 
									BUFFER_TYPE_VIDEO, 
									sizeof(LIVE_FRAME_INFO_T), 
									(char *)&frameinfo, frame->frameSize-4, 
									(char*)(frame->pBuffer+4));
				}
				else
				{
					BUFQUE_AddData(pStreamChannel->videoQueue, 
									pStreamChannel->id, 
									BUFFER_TYPE_VIDEO, 
									sizeof(LIVE_FRAME_INFO_T), 
									(char *)&frameinfo, frame->frameSize, (char*)frame->pBuffer);
				}
				
				//自定义时间
				if (frame->timestamp[0] > 0 || frame->timestamp[1] > 0 || frame->timestamp[2] > 0 || 
					frame->timestamp[3] > 0 || frame->timestamp[4] > 0 || frame->timestamp[5] > 0)		//有带帧时间
				{
					char pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 20] = {0};
					CUSTOM_META_DATA_HEADER_T	*pMetaHdr = (CUSTOM_META_DATA_HEADER_T *)pbuf;
					memset(pMetaHdr, 0x00, sizeof(CUSTOM_META_DATA_HEADER_T));
					pMetaHdr->nsize = sizeof(CUSTOM_META_DATA_HEADER_T);
					pMetaHdr->moduleid = VPB_MODULE_ID;
					pMetaHdr->customid = VPB_MODULE_ID;
					pMetaHdr->datalen = 6;
					/*
					time_t t = time(NULL);
					struct tm *_timetmp = NULL;
					_timetmp = localtime(&t);
					*/
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 0]  = frame->timestamp[0];//_timetmp->tm_year+1900-2000;
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 1]  = frame->timestamp[1];//_timetmp->tm_mon+1;
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 2]  = frame->timestamp[2];//_timetmp->tm_mday;
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 3]  = frame->timestamp[3];//_timetmp->tm_hour;
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 4]  = frame->timestamp[4];//_timetmp->tm_min;
					pbuf[sizeof(CUSTOM_META_DATA_HEADER_T) + 5]  = frame->timestamp[5];//_timetmp->tm_sec;

					MEDIA_FRAME_INFO_T		mediaFrameInfo;
					memset(&mediaFrameInfo, 0x00, sizeof(MEDIA_FRAME_INFO_T));
					mediaFrameInfo.frameSize = pMetaHdr->nsize + pMetaHdr->datalen;
					mediaFrameInfo.frameType = 0;
					mediaFrameInfo.pBuffer   = (unsigned char *)pbuf;
					//mediaFrameInfo.rtpTimestamp = pChannel->audioPts.pts;//frameInfo->rtptimestamp_sec * 1000 + frameInfo->rtptimestamp_usec / 1000;
					//mediaFrameInfo.rtpTimestamp = frameInfo->rtptimestamp_sec * 1000 + frameInfo->rtptimestamp_usec / 1000;
					mediaFrameInfo.rtpTimestamp = frame->rtpTimestamp;
					//mediaFrameInfo.rtpTimestamp = pChannel->audioPts.pts;

					PutFrame(channelHandle, MEDIA_TYPE_EVENT, &mediaFrameInfo);
				}
			}
			else if (NULL != pStreamChannel->videoFrame.framedata)
			{
				memcpy(pStreamChannel->videoFrame.framedata, frame->pBuffer, frame->frameSize);
				//pStreamChannel->videoFrame.timestamp_sec = frame->u32TimestampSec;
				//pStreamChannel->videoFrame.timestamp_usec = frame->u32TimestampUsec;
				pStreamChannel->videoFrame.framesize = frame->frameSize;
			}
		}
		else if (mediaType == MEDIA_TYPE_AUDIO)
		{
			frameinfo.codec = pStreamChannel->mediainfo.audioCodec;

			if (NULL != pStreamChannel->audioQueue)
			{
				BUFQUE_AddData(pStreamChannel->audioQueue, pStreamChannel->id, BUFFER_TYPE_AUDIO, sizeof(LIVE_FRAME_INFO_T), (char *)&frameinfo, frame->frameSize, (char*)frame->pBuffer);
			}
			else if (NULL != pStreamChannel->audioFrame.framedata)
			{
				memcpy(pStreamChannel->audioFrame.framedata, frame->pBuffer, frame->frameSize);
				//pStreamChannel->audioFrame.timestamp_sec = frame->rtpTimestamp;
				//pStreamChannel->audioFrame.timestamp_usec = frame->u32TimestampUsec;
				pStreamChannel->audioFrame.framesize = frame->frameSize;
			}
		}
		else if (mediaType == MEDIA_TYPE_EVENT)
		{
			frameinfo.codec = pStreamChannel->mediainfo.metadataCodec;

			if (NULL != pStreamChannel->metadataQueue)
			{
				BUFQUE_AddData(pStreamChannel->metadataQueue, pStreamChannel->id, BUFFER_TYPE_EVENT, sizeof(LIVE_FRAME_INFO_T), (char *)&frameinfo, frame->frameSize, (char*)frame->pBuffer);
			}
		}

#else
		MEDIA_FRAME_INFO		frameinfo;
		memset(&frameinfo, 0x00, sizeof(MEDIA_FRAME_INFO));
		frameinfo.length = frame->frameSize;
		frameinfo.type   = frame->frameType;
		frameinfo.rtpTimestamp_sec = frame->rtpTimestamp / 1000;
		frameinfo.rtpTimestamp_usec = (frame->rtpTimestamp % 1000) * 1000;
		if (mediaType == MEDIA_TYPE_VIDEO)
		{
			frameinfo.codec = pStreamChannel->mediainfo.videoCodec;

			if (NULL != pStreamChannel->videoQueue)
			{
				SSQ_AddData(pStreamChannel->videoQueue, pStreamChannel->id, mediaType, &frameinfo, (char*)frame->pBuffer, 0x01);
			}
			else if (NULL != pStreamChannel->videoFrame.framedata)
			{
				memcpy(pStreamChannel->videoFrame.framedata, frame->pBuffer, frame->frameSize);
				//pStreamChannel->videoFrame.timestamp_sec = frame->u32TimestampSec;
				//pStreamChannel->videoFrame.timestamp_usec = frame->u32TimestampUsec;
				pStreamChannel->videoFrame.framesize = frame->frameSize;
			}
		}
		else if (mediaType == MEDIA_TYPE_AUDIO)
		{
			frameinfo.codec = pStreamChannel->mediainfo.audioCodec;

			if (NULL != pStreamChannel->audioQueue)
			{
				SSQ_AddData(pStreamChannel->audioQueue, pStreamChannel->id, mediaType, &frameinfo, (char*)frame->pBuffer, 0x01);
			}
			else if (NULL != pStreamChannel->audioFrame.framedata)
			{
				memcpy(pStreamChannel->audioFrame.framedata, frame->pBuffer, frame->frameSize);
				//pStreamChannel->audioFrame.timestamp_sec = frame->rtpTimestamp;
				//pStreamChannel->audioFrame.timestamp_usec = frame->u32TimestampUsec;
				pStreamChannel->audioFrame.framesize = frame->frameSize;
			}			
		}
#endif
	}
	else
	{
		_TRACE(TRACE_LOG_ERROR, (char*)"Channel queue is NULL: [%d@%s]\n", pStreamChannel->id, pStreamChannel->streamName);
		return -1;
	}

	return RTSP_SERVER_ERR_NoErr;
}


int LiveRtspServer::ResetChannel(RTSP_CHANNEL_HANDLE channelHandle)
{
	if (NULL == channelHandle)		return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pLiveChannel)		return RTSP_SERVER_ERR_NotInitialized;


	LockMutex(&mutexChannel);		//Lock

	LIVE_CHANNEL_OBJ_T	*pStreamChannel = NULL;
	LIVE_CHANNEL_OBJ_T	*pChannel = pLiveChannel;
	while (NULL != pChannel)
	{
		if (pChannel == channelHandle)
		{
			pStreamChannel = pChannel;
			break;
		}

		pChannel = pChannel->next;
	}

	if (NULL == pStreamChannel)
	{
		UnlockMutex(&mutexChannel);		//Unlock
		return RTSP_SERVER_ERR_OPERATE;
	}


#ifdef LIVE_MULTI_THREAD_ENABLE
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(NULL, 0, NULL, pStreamChannel->streamName, True);
#else
	ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(pStreamChannel->streamName);
#endif
	if (NULL != sms)
	{
		pStreamChannel->resetFlag = 0x01;


		//UsageEnvironment *pEnv = FindEnvByStreamName(pStreamChannel->streamName);
		//pEnv->ResetChannel(1);

		//for (int i=0; i<10; i++)
		//{
		//	if (pEnv->IsResetChannel() == 0x02)
		//	{
		//		if (NULL != sms)
		//		{
		//			closeAllClientSessionsForServerMediaSession(sms);
		//			CloseAllConnections(pChannel->streamName);
		//			sms = NULL;
		//		}
		//		memset(&pChannel->mediainfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));

		//		pEnv->ResetChannel(0);

		//		break;
		//	}
		//	Sleep(200);
		//}
		//pEnv->ResetChannel(0);
	}
	else
	{
		memset(&pStreamChannel->mediainfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));
	}

	UnlockMutex(&mutexChannel);		//Unlock

	return RTSP_SERVER_ERR_NoErr;
}


int		LiveRtspServer::CheckChannelStatus()
{
	if (NULL == pLiveChannel)		return -1;

	LockMutex(&mutexChannel);		//Lock

	LIVE_CHANNEL_OBJ_T	*pChannel = pLiveChannel;
	while (NULL != pChannel)
	{
		if (pChannel->resetFlag == 0x01)
		{
#ifdef LIVE_MULTI_THREAD_ENABLE
			ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(NULL, 0, NULL, pChannel->streamName, True);
#else
			ServerMediaSession* sms = RTSPServer::lookupServerMediaSession(pChannel->streamName);
#endif

			if (NULL != sms)
			{
				//char streamName[512] = {0};
				//strcpy(streamName, pChannel->streamName);

				UsageEnvironment *pEnv = FindEnvByStreamName(pChannel->streamName);
				if (NULL != pEnv)
				{
					pEnv->ResetInternalChannel(1, (void*)sms);

					for (int i=0; i<10; i++)
					{
						if (pEnv->IsResetInternalChannel() == 0x02)
						{
							_TRACE(TRACE_LOG_ERROR, (char*)"[%s] Reset Channel: %s\n", pEnv->GetEnvirName(),  pChannel->streamName);

							//if (NULL != sms)
							//{
								//closeAllClientSessionsForServerMediaSession(sms);
								//CloseAllConnections(streamName);
								//sms = NULL;
							//}
							memset(&pChannel->mediainfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));

							//pEnv->ResetChannel(0, NULL);

							break;
						}
						Sleep(200);
					}
				}

				//closeAllClientSessionsForServerMediaSession(sms);
				//CloseAllConnections(pChannel->streamName);
				//sms = NULL;
			}
			//memset(&pChannel->mediainfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));
			pChannel->resetFlag = 0x00;
			break;
		}

		pChannel = pChannel->next;
	}
	UnlockMutex(&mutexChannel);		//Unlock

	return RTSP_SERVER_ERR_NoErr;
}


#define NEW_SMS_FILE(description) do {\
char const* descStr = description\
    ", streamed by the LIVE555 Media Server";\
sms = ServerMediaSession::createNew(env, fileName, fileName, descStr);\
} while(0)


static ServerMediaSession* createNewSMS_File(UsageEnvironment& env,
					char const* fileName, FILE* /*fid*/) {
  // Use the file name extension to determine the type of "ServerMediaSession":
  char const* extension = strrchr(fileName, '.');
  if (extension == NULL) return NULL;

  ServerMediaSession* sms = NULL;
  Boolean const reuseSource = False;
  if (strcmp(extension, ".aac") == 0) {
    // Assumed to be an AAC Audio (ADTS format) file:
    NEW_SMS_FILE("AAC Audio");
    sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".ac3") == 0) {
    // Assumed to be an AC-3 Audio file:
    NEW_SMS_FILE("AC-3 Audio");
    sms->addSubsession(AC3AudioFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".m4e") == 0) {
    // Assumed to be a MPEG-4 Video Elementary Stream file:
    NEW_SMS_FILE("MPEG-4 Video");
    sms->addSubsession(MPEG4VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".264") == 0) {
    // Assumed to be a H.264 Video Elementary Stream file:
    NEW_SMS_FILE("H.264 Video");
    OutPacketBuffer::maxSize = 1024*1024; // allow for some possibly large H.264 frames
    sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".265") == 0) {
    // Assumed to be a H.265 Video Elementary Stream file:
    NEW_SMS_FILE("H.265 Video");
    OutPacketBuffer::maxSize = 100000; // allow for some possibly large H.265 frames
    sms->addSubsession(H265VideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".mp3") == 0) {
    // Assumed to be a MPEG-1 or 2 Audio file:
    NEW_SMS_FILE("MPEG-1 or 2 Audio");
    // To stream using 'ADUs' rather than raw MP3 frames, uncomment the following:
//#define STREAM_USING_ADUS 1
    // To also reorder ADUs before streaming, uncomment the following:
//#define INTERLEAVE_ADUS 1
    // (For more information about ADUs and interleaving,
    //  see <http://www.live555.com/rtp-mp3/>)
    Boolean useADUs = False;
    Interleaving* interleaving = NULL;
#ifdef STREAM_USING_ADUS
    useADUs = True;
#ifdef INTERLEAVE_ADUS
    unsigned char interleaveCycle[] = {0,2,1,3}; // or choose your own...
    unsigned const interleaveCycleSize
      = (sizeof interleaveCycle)/(sizeof (unsigned char));
    interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
#endif
#endif
    sms->addSubsession(MP3AudioFileServerMediaSubsession::createNew(env, fileName, reuseSource, useADUs, interleaving));
  } else if (strcmp(extension, ".mpg") == 0) {
    // Assumed to be a MPEG-1 or 2 Program Stream (audio+video) file:
    NEW_SMS_FILE("MPEG-1 or 2 Program Stream");
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(env, fileName, reuseSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession());
    sms->addSubsession(demux->newAudioServerMediaSubsession());
  } else if (strcmp(extension, ".vob") == 0) {
    // Assumed to be a VOB (MPEG-2 Program Stream, with AC-3 audio) file:
    NEW_SMS_FILE("VOB (MPEG-2 video with AC-3 audio)");
    MPEG1or2FileServerDemux* demux
      = MPEG1or2FileServerDemux::createNew(env, fileName, reuseSource);
    sms->addSubsession(demux->newVideoServerMediaSubsession());
    sms->addSubsession(demux->newAC3AudioServerMediaSubsession());
  } else if (strcmp(extension, ".ts") == 0) {
    // Assumed to be a MPEG Transport Stream file:
    // Use an index file name that's the same as the TS file name, except with ".tsx":
    unsigned indexFileNameLen = (unsigned)strlen(fileName) + 2; // allow for trailing "x\0"
    char* indexFileName = new char[indexFileNameLen];
    sprintf(indexFileName, "%sx", fileName);
    NEW_SMS_FILE("MPEG Transport Stream");
    sms->addSubsession(MPEG2TransportFileServerMediaSubsession::createNew(env, fileName, indexFileName, reuseSource));
    delete[] indexFileName;
  } else if (strcmp(extension, ".wav") == 0) {
    // Assumed to be a WAV Audio file:
    NEW_SMS_FILE("WAV Audio Stream");
    // To convert 16-bit PCM data to 8-bit u-law, prior to streaming,
    // change the following to True:
    Boolean convertToULaw = False;
    sms->addSubsession(WAVAudioFileServerMediaSubsession::createNew(env, fileName, reuseSource, convertToULaw));
  } else if (strcmp(extension, ".dv") == 0) {
    // Assumed to be a DV Video file
    // First, make sure that the RTPSinks' buffers will be large enough to handle the huge size of DV frames (as big as 288000).
    OutPacketBuffer::maxSize = 300000;

    NEW_SMS_FILE("DV Video");
    sms->addSubsession(DVVideoFileServerMediaSubsession::createNew(env, fileName, reuseSource));
  } else if (strcmp(extension, ".mkv") == 0 || strcmp(extension, ".webm") == 0) {
    // Assumed to be a Matroska file (note that WebM ('.webm') files are also Matroska files)
    OutPacketBuffer::maxSize = 100000; // allow for some possibly large VP8 or VP9 frames
    NEW_SMS_FILE("Matroska video+audio+(optional)subtitles");

    // Create a Matroska file server demultiplexor for the specified file.
    // (We enter the event loop to wait for this to complete.)
    MatroskaDemuxCreationState creationState;
    creationState.watchVariable = 0;
    MatroskaFileServerDemux::createNew(env, fileName, onMatroskaDemuxCreation, &creationState);
    env.taskScheduler().doEventLoop(&creationState.watchVariable);

    ServerMediaSubsession* smss;
    while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
    }
  } else if (strcmp(extension, ".ogg") == 0 || strcmp(extension, ".ogv") == 0 || strcmp(extension, ".opus") == 0) {
    // Assumed to be an Ogg file
    NEW_SMS_FILE("Ogg video and/or audio");

    // Create a Ogg file server demultiplexor for the specified file.
    // (We enter the event loop to wait for this to complete.)
    OggDemuxCreationState creationState;
    creationState.watchVariable = 0;
    OggFileServerDemux::createNew(env, fileName, onOggDemuxCreation, &creationState);
    env.taskScheduler().doEventLoop(&creationState.watchVariable);

    ServerMediaSubsession* smss;
    while ((smss = creationState.demux->newServerMediaSubsession()) != NULL) {
      sms->addSubsession(smss);
    }
  }

  return sms;
}
