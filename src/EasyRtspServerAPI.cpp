

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "aes256.h"
#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")
#else

#endif


#include "EasyRtspServerAPI.h"
#include "libRTSPServerAPI.h"

static RTSP_SVR_HANDLE	gRTSPServerHandle = NULL;
char gLicenseStr[1024] = {0};


Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Startup(Easy_U16 listenPort, const char *realm, 
											EASY_AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
											EasyRtspServer_Callback callback, void *userPtr)
{
	if (NULL == gRTSPServerHandle)
	{
		libRTSPSvr_Create(&gRTSPServerHandle);

		if (NULL == gRTSPServerHandle)			return Easy_RequestFailed;

		return libRTSPSvr_Startup(gRTSPServerHandle, listenPort, realm, 
							(AUTHENTICATION_TYPE_ENUM)authType, username, password, 
							(RTSPSvrCallBack)callback, userPtr, 0);
	}
	
	return Easy_RequestFailed;
}


Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Shutdown()
{
	if (NULL != gRTSPServerHandle)
	{
		libRTSPSvr_Shutdown(gRTSPServerHandle);
		libRTSPSvr_Release(&gRTSPServerHandle);
	}

	return 0;
}


Easy_API Easy_I32 Easy_APICALL EasyRtspServer_CreateChannel(const char *resourceName, 
											EASY_CHANNEL_HANDLE *channelHandle, void *channelPtr)
{
	if (NULL == gRTSPServerHandle)				return Easy_RequestFailed;

	int ret = libRTSPSvr_CreateChannel(gRTSPServerHandle, resourceName, channelHandle, channelPtr);

	return ret;
}





Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DeleteChannel(EASY_CHANNEL_HANDLE *channelHandle)
{
	if (NULL == gRTSPServerHandle)				return Easy_RequestFailed;

	int ret = libRTSPSvr_DeleteChannel(gRTSPServerHandle, channelHandle);

	return ret;
}

Easy_API Easy_I32 Easy_APICALL EasyRtspServer_PushFrame(EASY_CHANNEL_HANDLE channelHandle, 
													EASY_AV_Frame *frame )
{
	if (NULL == frame)				return -1;
	if (NULL == gRTSPServerHandle)		return -1;

	MEDIA_FRAME_INFO_T	mediaFrame;
	memset(&mediaFrame, 0x00, sizeof(MEDIA_FRAME_INFO_T));
	mediaFrame.frameSize = frame->u32AVFrameLen;
	mediaFrame.frameType = frame->u32VFrameType;
	mediaFrame.pBuffer   = frame->pBuffer;
	mediaFrame.rtpTimestamp = (unsigned int)(frame->u32TimestampSec * 1000 + frame->u32TimestampUsec / 1000);
	//mediaFrame.timestamp

	int ret = libRTSPSvr_PutFrame(gRTSPServerHandle, channelHandle, frame->u32AVFrameFlag, &mediaFrame);

	return ret;
}

Easy_API Easy_I32 Easy_APICALL EasyRtspServer_ResetChannel(EASY_CHANNEL_HANDLE channelHandle)
{
	if (NULL == gRTSPServerHandle)		return -1;

	int ret = libRTSPSvr_ResetChannel(gRTSPServerHandle, channelHandle);

	return ret;
}




Easy_API Easy_I32 Easy_APICALL EasyRtspServer_AddUser(const Easy_U8 *username, const Easy_U8 *password)
{
	if (NULL == gRTSPServerHandle)				return Easy_RequestFailed;

	if (NULL == username || NULL == password)	return Easy_BadArgument;
	if ( (int)strlen((char*)username) < 1)		return Easy_BadArgument;
	if ( (int)strlen((char*)password) < 1)		return Easy_BadArgument;

	int ret = libRTSPSvr_AddUser(gRTSPServerHandle, (const char *)username, (const char *)password);
	return ret;
}

Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DelUser(const Easy_U8 *username)
{
	if (NULL == gRTSPServerHandle)					return Easy_RequestFailed;
	

	int ret = libRTSPSvr_DelUser(gRTSPServerHandle, (const char *)username);

	return ret;
}
