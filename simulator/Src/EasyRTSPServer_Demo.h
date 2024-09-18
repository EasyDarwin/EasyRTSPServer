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
//32λ����ʹ��Win32�ļ��������ӿ�
#pragma comment(lib, "EasyRTSPServer/Win32/libEasyRTSPServer.lib")
#else
//64λ����ʹ��Win64�ļ��������ӿ�
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
	
	EASY_RTSPSERVER_MEDIA_INFO_T		mediaInfo;		//Դý����Ϣ
	EASY_MEDIA_INFO_T s_mediainfo;

	Easy_Handle	rtspClientHandle;			//rtsp�ͻ��˾��


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

