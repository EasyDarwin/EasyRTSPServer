#include "EasyStreamingServer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

EasyStreamingServer::EasyStreamingServer(void)
{
}
EasyStreamingServer::~EasyStreamingServer(void)
{
}


//EasyRTSPClient 回调
int CALLBACK __RTSPSourceCallBack( int _channelId, void *_channelPtr, int _frameType, char *pBuf, EASY_FRAME_INFO* frameInfo)
{
	RTSP_CHANNEL_T	*pChannel = (RTSP_CHANNEL_T *)_channelPtr;

	if (EASY_SDK_VIDEO_FRAME_FLAG == _frameType)
	{
		if (pChannel->mediaInfo.spsLength < 1)
		{
			if (frameInfo->codec == EASY_SDK_VIDEO_CODEC_H264)
			{
				GetH264SPSandPPS(pBuf, frameInfo->length, (char*)pChannel->mediaInfo.sps, (int *)&pChannel->mediaInfo.spsLength, (char *)pChannel->mediaInfo.pps, (int *)&pChannel->mediaInfo.ppsLength);
			}
			else if (frameInfo->codec == EASY_SDK_VIDEO_CODEC_H265)
			{
				GetH265VPSandSPSandPPS(pBuf, frameInfo->length, (char*)pChannel->mediaInfo.vps, (int *)&pChannel->mediaInfo.vpsLength,  (char*)pChannel->mediaInfo.sps, (int *)&pChannel->mediaInfo.spsLength, (char *)pChannel->mediaInfo.pps, (int *)&pChannel->mediaInfo.ppsLength);
			}
#if 0
			if (pChannel->mediaInfo.spsLength > 0)
			{
				pChannel->mediaInfo.videoCodec = frameInfo->codec;
				pChannel->mediaInfo.audioCodec = EASY_SDK_AUDIO_CODEC_G711U;

				pChannel->mediaInfo.audioSampleRate = 8000;
				pChannel->mediaInfo.audioChannel = 1;
				pChannel->mediaInfo.audioBitsPerSample = 16;
			}
#endif
		}
		EASY_AV_Frame		avFrameInfo;
		bool bSendSpsAndPps = false;

#if 1
		if (frameInfo->type == 0x01 && pChannel->mediaInfo.spsLength>0 && pChannel->mediaInfo.ppsLength>0)
		{
			bSendSpsAndPps = true;

			if (pChannel->mediaInfo.vpsLength > 0)
			{
				//VPS
				memset(&avFrameInfo, 0x00, sizeof(EASY_AV_Frame));
				avFrameInfo.u32AVFrameFlag = _frameType;
				avFrameInfo.u32AVFrameLen = pChannel->mediaInfo.vpsLength;
				avFrameInfo.u32VFrameType = frameInfo->type;
				avFrameInfo.pBuffer   = (unsigned char *)(pChannel->mediaInfo.vps);
				avFrameInfo.u32TimestampSec = frameInfo->timestamp_sec;
				avFrameInfo.u32TimestampUsec =frameInfo->timestamp_usec;
				EasyRtspServer_PushFrame(pChannel->channelHandle, &avFrameInfo);
			}

			//SPS
			memset(&avFrameInfo, 0x00, sizeof(EASY_AV_Frame));
			avFrameInfo.u32AVFrameFlag = _frameType;
			avFrameInfo.u32AVFrameLen = pChannel->mediaInfo.spsLength;
			avFrameInfo.u32VFrameType = frameInfo->type;
			avFrameInfo.pBuffer   = (unsigned char *)(pChannel->mediaInfo.sps);
			avFrameInfo.u32TimestampSec = frameInfo->timestamp_sec;
			avFrameInfo.u32TimestampUsec =frameInfo->timestamp_usec;
			EasyRtspServer_PushFrame(pChannel->channelHandle, &avFrameInfo);

			//PPS
			memset(&avFrameInfo, 0x00, sizeof(EASY_AV_Frame));
			avFrameInfo.u32AVFrameFlag = _frameType;
			avFrameInfo.u32AVFrameLen = pChannel->mediaInfo.ppsLength;
			avFrameInfo.u32VFrameType = frameInfo->type;
			avFrameInfo.pBuffer   = (unsigned char *)(pChannel->mediaInfo.pps);
			avFrameInfo.u32TimestampSec = frameInfo->timestamp_sec;
			avFrameInfo.u32TimestampUsec =frameInfo->timestamp_usec;
			EasyRtspServer_PushFrame(pChannel->channelHandle, &avFrameInfo);
		}
#endif

		memset(&avFrameInfo, 0x00, sizeof(EASY_AV_Frame));
		avFrameInfo.u32AVFrameFlag = _frameType;
		avFrameInfo.u32AVFrameLen = frameInfo->length-4;
		avFrameInfo.u32VFrameType = frameInfo->type;
		avFrameInfo.pBuffer   = (unsigned char *)(pChannel->mediaInfo.sps);
		avFrameInfo.u32TimestampSec = frameInfo->timestamp_sec;
		avFrameInfo.u32TimestampUsec =frameInfo->timestamp_usec;

		if (bSendSpsAndPps)
		{
			if (pChannel->mediaInfo.videoCodec == EASY_SDK_VIDEO_CODEC_H264)
			{
				int trim_length = 4+pChannel->mediaInfo.spsLength;
				trim_length += 4+pChannel->mediaInfo.ppsLength;
				if (pChannel->mediaInfo.seiLength>0)			trim_length += 4+pChannel->mediaInfo.seiLength;
				trim_length += 4;

				avFrameInfo.u32AVFrameLen = frameInfo->length-trim_length;
				avFrameInfo.pBuffer   = (unsigned char *)(pBuf+trim_length);
			}
			else if (pChannel->mediaInfo.videoCodec == EASY_SDK_VIDEO_CODEC_H265)
			{
				int trim_length = 4+pChannel->mediaInfo.vpsLength;
				trim_length += 4+pChannel->mediaInfo.spsLength;
				trim_length += 4+pChannel->mediaInfo.ppsLength;
				if (pChannel->mediaInfo.seiLength>0)			trim_length += 4+pChannel->mediaInfo.seiLength;
				trim_length += 4;

				avFrameInfo.u32AVFrameLen = frameInfo->length-trim_length;
				avFrameInfo.pBuffer   = (unsigned char *)(pBuf+trim_length);
			}
		}
		else
		{
			avFrameInfo.pBuffer   = (unsigned char *)(pBuf+4);
		}

		EasyRtspServer_PushFrame(pChannel->channelHandle, &avFrameInfo);
	}
	else if (EASY_SDK_AUDIO_FRAME_FLAG == _frameType)
	{
		EASY_AV_Frame		avFrameInfo;
		memset(&avFrameInfo, 0x00, sizeof(EASY_AV_Frame));
		avFrameInfo.u32AVFrameFlag = _frameType;
		avFrameInfo.u32AVFrameLen = frameInfo->length;
		avFrameInfo.u32VFrameType = frameInfo->type;
		avFrameInfo.pBuffer   = (unsigned char *)pBuf;
		avFrameInfo.u32TimestampSec = frameInfo->timestamp_sec;
		avFrameInfo.u32TimestampUsec = frameInfo->timestamp_usec;

		EasyRtspServer_PushFrame(pChannel->channelHandle, &avFrameInfo);
	}
	else if (EASY_SDK_MEDIA_INFO_FLAG == _frameType)
	{
		EASY_MEDIA_INFO_T	*mediainfo = (EASY_MEDIA_INFO_T *)pBuf;
		if (NULL != mediainfo && NULL!=pChannel)
		{
			pChannel->mediaInfo.audioChannel =mediainfo->u32AudioChannel;
			pChannel->mediaInfo.audioSampleRate = mediainfo->u32AudioSamplerate;
			pChannel->mediaInfo.audioBitsPerSample = mediainfo->u32AudioBitsPerSample;

			pChannel->mediaInfo.vpsLength = mediainfo->u32VpsLength;
			if (pChannel->mediaInfo.vpsLength > 0)		memcpy(pChannel->mediaInfo.vps, mediainfo->u8Vps, mediainfo->u32VpsLength);

			pChannel->mediaInfo.spsLength = mediainfo->u32SpsLength;
			if (pChannel->mediaInfo.spsLength > 0)		memcpy(pChannel->mediaInfo.sps, mediainfo->u8Sps, mediainfo->u32SpsLength);

			pChannel->mediaInfo.ppsLength = mediainfo->u32PpsLength;
			if (pChannel->mediaInfo.ppsLength > 0)		memcpy(pChannel->mediaInfo.pps, mediainfo->u8Pps, mediainfo->u32PpsLength);

			//最后再设置编码类型
			pChannel->mediaInfo.audioCodec = mediainfo->u32AudioCodec;
			pChannel->mediaInfo.videoCodec = mediainfo->u32VideoCodec;
		}
	}
	return 0;
}




int CALLBACK __EasyRtspServer_Callback(EASY_RTSPSERVER_STATE_T serverStatus, const char *resourceName, 
											EASY_CHANNEL_HANDLE *channelHandle, 
											EASY_RTSPSERVER_MEDIA_INFO_T *mediaInfo, 
											EASY_PLAY_CONTROL_INFO_T *playCtrlInfo, 
											void *userPtr, void *channelPtr)
{
	int ret = -1;
	if (EASY_CHANNEL_OPEN_STREAM == serverStatus)
	{
		EASY_CHANNEL_HANDLE pChannelHandle = NULL;

		printf("EasyStreamingServer New Request: %s\n", resourceName);

		RTSP_CHANNEL_T	*pRtspChannel = new RTSP_CHANNEL_T;
		if (NULL == pRtspChannel)		return -1;

		memset(pRtspChannel, 0x00, sizeof(RTSP_CHANNEL_T));

		strcpy(pRtspChannel->url, resourceName);

		//初始化RTSP Client 句柄
		EasyRTSP_Init(&pRtspChannel->rtspClientHandle);
		//设置相应回调函数
		EasyRTSP_SetCallback(pRtspChannel->rtspClientHandle, __RTSPSourceCallBack);
		//向前端获取音视频流, 此处使用TCP
		EasyRTSP_OpenStream(pRtspChannel->rtspClientHandle, 0, pRtspChannel->url, EASY_RTP_OVER_TCP, 
													EASY_SDK_VIDEO_FRAME_FLAG|EASY_SDK_AUDIO_FRAME_FLAG, 
													pRtspChannel->username, pRtspChannel->password, (void*)pRtspChannel, 1000, 0, 1, 0);

		for (int i=0; i<10*5; i++)		//等待5秒
		{
			if (pRtspChannel->mediaInfo.videoCodec>0)	break;// && pRtspChannel->mediaInfo.audioCodec)		break;

			Sleep(100);
		}

		//设置媒体信息
		mediaInfo->videoCodec = pRtspChannel->mediaInfo.videoCodec;
		mediaInfo->audioCodec = pRtspChannel->mediaInfo.audioCodec;
		mediaInfo->audioSampleRate = pRtspChannel->mediaInfo.audioSampleRate;
		mediaInfo->audioChannel = pRtspChannel->mediaInfo.audioChannel;
		mediaInfo->audioBitsPerSample = pRtspChannel->mediaInfo.audioBitsPerSample;


#if 0
		//此处可指定sps & pps, 也可以不指定
		mediaInfo->spsLength = pRtspChannel->mediaInfo.vpsLength;
		memcpy(mediaInfo->vps, pRtspChannel->mediaInfo.vps, mediaInfo->vpsLength);
		mediaInfo->spsLength = pRtspChannel->mediaInfo.spsLength;
		memcpy(mediaInfo->sps, pRtspChannel->mediaInfo.sps, mediaInfo->spsLength);
		mediaInfo->ppsLength = pRtspChannel->mediaInfo.ppsLength;
		memcpy(mediaInfo->pps, pRtspChannel->mediaInfo.pps, mediaInfo->ppsLength);
#endif
		mediaInfo->videoFps = 25;

		if (pRtspChannel->mediaInfo.videoCodec > 0)
		{
			printf("create channel: %s\n", resourceName);

			//创建通道
			EasyRtspServer_CreateChannel(resourceName, &pChannelHandle, (void *)pRtspChannel);
			*channelHandle = pChannelHandle;
			pRtspChannel->channelHandle = pChannelHandle;

			ret = 0;
		}
		else		//关闭源
		{
			printf("Get Source[%s][%s] sdp fail.\n", resourceName, pRtspChannel->url);
                        EasyRTSP_CloseStream(pRtspChannel->rtspClientHandle);
                        EasyRTSP_Deinit(&pRtspChannel->rtspClientHandle);

			delete pRtspChannel;
			pRtspChannel = NULL;
		}
	}
	if (EASY_CHANNEL_START_STREAM == serverStatus)
	{
		RTSP_CHANNEL_T	*pRtspChannel = (RTSP_CHANNEL_T *)channelPtr;
		pRtspChannel->status = 0x01;		//开始发送流
	}
	if (EASY_CHANNEL_STOP_STREAM == serverStatus)
	{
		RTSP_CHANNEL_T	*pRtspChannel = (RTSP_CHANNEL_T *)channelPtr;
		pRtspChannel->status = 0x00;		//停止发送流
	}
	else if (EASY_CHANNEL_CLOSE_STREAM == serverStatus)
	{
		printf("#######################Close Stream: %s\n", resourceName);

		RTSP_CHANNEL_T	*pRtspChannel = (RTSP_CHANNEL_T *)channelPtr;
		if (NULL != pRtspChannel)
		{
			if (NULL != pRtspChannel->rtspClientHandle)
			{
				//关闭与前端的连接
                EasyRTSP_CloseStream(pRtspChannel->rtspClientHandle);
                EasyRTSP_Deinit(&pRtspChannel->rtspClientHandle);
			}
			printf("Close Source OK: %s    %s\n", resourceName, pRtspChannel->url);

			EasyRtspServer_DeleteChannel(&pRtspChannel->channelHandle);
			delete pRtspChannel;
			pRtspChannel = NULL;
		}
		ret = 0;
	}
	else if (EASY_CHANNEL_PLAY_CONTROL == serverStatus)
	{
		//以下为播放控制, 用于录像回放时使用
#if 0
		printf("RTSP_CHANNEL_PLAY_CONTROL: %s\n", resourceName);

		RTSP_CHANNEL_T	*pRtspChannel = (RTSP_CHANNEL_T *)channelPtr;
		if (NULL != pRtspChannel)
		{
			char szMediaType[16] = {0};
			if (EASY_SDK_VIDEO_FRAME_FLAG == playCtrlInfo->mediaType)		strcpy(szMediaType, "Video");
			else if (EASY_SDK_AUDIO_FRAME_FLAG == playCtrlInfo->mediaType)	strcpy(szMediaType, "Audio");
			else if (EASY_SDK_EVENT_FRAME_FLAG == playCtrlInfo->mediaType)	strcpy(szMediaType, "Metadata");

			if (EASY_PLAY_CTRL_CMD_GET_DURATION == playCtrlInfo->ctrlCommand)
				printf("GetDuration... %s\n", szMediaType);
			else if (EASY_PLAY_CTRL_CMD_PAUSE == playCtrlInfo->ctrlCommand)
				printf("Pause...%s\n", szMediaType);
			else if (EASY_PLAY_CTRL_CMD_RESUME == playCtrlInfo->ctrlCommand)
				printf("Resume...%s\n", szMediaType);
			else if (EASY_PLAY_CTRL_CMD_SCALE == playCtrlInfo->ctrlCommand)
				printf("Scale[%s]: %.2f\n", szMediaType, playCtrlInfo->scale);
			else if (EASY_PLAY_CTRL_CMD_SEEK_STREAM == playCtrlInfo->ctrlCommand)
				printf("Seek[%s]: %s - %s \n", szMediaType, playCtrlInfo->startTime, playCtrlInfo->endTime);
		}
#endif
	}

	return ret;
}

int		EasyStreamingServer::Startup(int rtspPort)
{
	if (rtspPort < 554)				return -1;

	if (0 != EasyRtspServer_Startup(rtspPort, "EasyStreamingServer", EASY_AUTHENTICATION_TYPE_NONE,  "admin", "12345", __EasyRtspServer_Callback, (void*)this))
	{
		printf("startup fail.\n");
		EasyRtspServer_Shutdown();
		return -2;
	}

	printf("URL:  rtsp://serverip:%d/rtsp://admin:12345@192.168.1.100/ch1\n", rtspPort);


	return 0;
}

void	EasyStreamingServer::Shutdown()
{
	EasyRtspServer_Shutdown();
}


void	EasyStreamingServer::ResetChannel(int channelId)
{
}
