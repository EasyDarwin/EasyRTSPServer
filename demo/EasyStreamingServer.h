#pragma once


#include "getsps.h"
#include "EasyRTSPClient/EasyRTSPClientAPI.h"
#include "EasyRTSPServer/EasyRtspServerAPI.h"
#include <time.h>
#ifdef _WIN32
//===============Windows==============
#if 0
//32位程序使用Win32文件夹下链接库
#pragma comment(lib, "EasyRTSPClient/Win32/libEasyRTSPClient.lib")
#pragma comment(lib, "EasyRTSPServer/Win32/libEasyRTSPServer.lib")
#else
//64位程序使用Win64文件夹下链接库
#pragma comment(lib, "EasyRTSPClient/Win64/libEasyRTSPClient.lib")
#pragma comment(lib, "EasyRTSPServer/Win64/libEasyRTSPServer.lib")
#endif
#else
//===============Linux=================
#include <unistd.h>
#define	Sleep(x)	{usleep(x*1000);}
#endif


typedef struct __RTSP_CHANNEL_T
{
	char		url[256];
	char		resourcename[256];
	char		username[36];
	char		password[36];
	
	EASY_RTSPSERVER_MEDIA_INFO_T		mediaInfo;		//源媒体信息

	Easy_Handle	rtspClientHandle;			//rtsp客户端句柄

	int			status;		//
	EASY_CHANNEL_HANDLE	channelHandle;
}RTSP_CHANNEL_T;





class EasyStreamingServer
{
public:
	EasyStreamingServer(void);
	~EasyStreamingServer(void);

	int		Startup(int rtspPort);
	void	Shutdown();

	void	ResetChannel(int channelId);
};

