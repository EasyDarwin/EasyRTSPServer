#pragma once


#include "getsps.h"
#include "EasyRTSPClient/EasyRTSPClientAPI.h"
#include "EasyRTSPServer/EasyRtspServerAPI.h"
#include <time.h>
#ifdef _WIN32
//===============Windows==============
#if 0
//32λ����ʹ��Win32�ļ��������ӿ�
#pragma comment(lib, "EasyRTSPClient/Win32/libEasyRTSPClient.lib")
#pragma comment(lib, "EasyRTSPServer/Win32/libEasyRTSPServer.lib")
#else
//64λ����ʹ��Win64�ļ��������ӿ�
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
	
	EASY_RTSPSERVER_MEDIA_INFO_T		mediaInfo;		//Դý����Ϣ

	Easy_Handle	rtspClientHandle;			//rtsp�ͻ��˾��

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

