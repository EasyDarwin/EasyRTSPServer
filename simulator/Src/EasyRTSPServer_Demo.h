#pragma once


#include "getsps.h"
#include "EasyRTSPServer/EasyRtspServerAPI.h"
#include <time.h>

#define  EASYSTREALCLIENT

#ifdef EASYSTREALCLIENT
#include "EasyStreamClient/EasyStreamClientAPI.h"
#endif


#ifdef _WIN32
//===============Windows==============
#if 0
//32位程序使用Win32文件夹下链接库
#pragma comment(lib, "EasyRTSPServer/Win32/libEasyRTSPServer.lib")
#else
//64位程序使用Win64文件夹下链接库
#ifdef EASYSTREALCLIENT
#pragma comment(lib, "EasyStreamClient/Win64/libEasyStreamClient.lib")
#endif //EASYSTREALCLIENT
#pragma comment(lib, "EasyRTSPServer/Win64/libEasyRTSPServer.lib")

#endif
#else
//===============Linux=================
#include <unistd.h>
#define	Sleep(x)	{usleep(x*1000);}
#endif

typedef struct __AV_FRAME_PTS_T
{
	long long	pts;

	long long dts;
	float		dtsDecimal;
}AV_FRAME_PTS_T;

typedef struct __RTSP_CHANNEL_T
{
	char		url[256];
	char		resourcename[256];
	char		username[36];
	char		password[36];
	
	EASY_RTSPSERVER_MEDIA_INFO_T		mediaInfo;		//源媒体信息
	EASY_MEDIA_INFO_T s_mediainfo;

	Easy_Handle	rtspClientHandle;			//rtsp客户端句柄


	AV_FRAME_PTS_T		timestamp;

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

