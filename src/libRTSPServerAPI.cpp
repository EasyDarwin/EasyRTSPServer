
#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32")
#endif


#include "libRTSPServerAPI.h"
#include "LiveRtspServer.h"
#include <BasicUsageEnvironment.hh>
#include "RTSPCommon.hh"
#include "LiveRtspServer.h"
#include "version.hh"
#include "trace.h"

#ifndef _WIN32
#define	Sleep(x)	{usleep(x*1000);}
#include "BasicTaskSchedulerEpoll.h"
#endif

typedef struct __RTSP_SERVER_OBJ_T
{
	unsigned int		mFlag;
	TaskScheduler* scheduler;
	UsageEnvironment* env;
	LiveRtspServer	*rtspServer;
	UserAuthenticationDatabase* authDB;
	int			listenPort;
	void		*pCallback;
	void		*pUserPtr;

	AUTHENTICATION_TYPE_ENUM	AuthorizationType;		//验证类型
	char		realm[64];
	char		username[36];
	char		password[36];

	int			liveChannelNum;
	//LIVE_CHANNEL_INFO_T	liveChannelInfo[MAX_LIVE_CHANNEL_NUM];
#ifdef _WIN32
	HANDLE		hThread;
#else
	pthread_t	threadId;
#endif

	int			tryBind;		//2019.08.20

	int			flag;
	void		*pEx;
}RTSP_SERVER_OBJ_T;

//static	RTSP_SERVER_OBJ_T	*gRtspServerObj = NULL;



#ifdef _WIN32
DWORD WINAPI __RtspServerProcMainThread(LPVOID lpParam);
#else
#ifdef ANDROID
	#include <jni.h>
	JavaVM *g_vm = NULL;
#endif
#include <pthread.h>
void *__RtspServerProcMainThread(void *lpParam);
#endif


#ifdef _WIN32
#include "CreateDump.h"

LONG CrashHandler_libRTSPServer(EXCEPTION_POINTERS *pException)
{
	SYSTEMTIME	systemTime;
	GetLocalTime(&systemTime);

	char szFile[MAX_PATH] = {0,};
	sprintf(szFile, TEXT("libRTSPServer_%04d%02d%02d %02d%02d%02d.dmp"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
	CreateDumpFile(szFile, pException);

	return EXCEPTION_EXECUTE_HANDLER;		//返回值EXCEPTION_EXECUTE_HANDLER	EXCEPTION_CONTINUE_SEARCH	EXCEPTION_CONTINUE_EXECUTION
}
#endif


//初始化
int RTSPSERVER_API	libRTSPSvr_Create(RTSP_SVR_HANDLE *handle)
{
	RTSP_SERVER_OBJ_T	*pRtspSvrObj = new RTSP_SERVER_OBJ_T;
	if (NULL != pRtspSvrObj)		memset(pRtspSvrObj, 0x00, sizeof(RTSP_SERVER_OBJ_T));

	pRtspSvrObj->mFlag = LIVE_FLAG;

	*handle = (RTSP_SVR_HANDLE)pRtspSvrObj;
	return 0;
}

//启动服务
int RTSPSERVER_API	libRTSPSvr_Startup(RTSP_SVR_HANDLE handle, int listenPort, const char *realm, 
											AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
											RTSPSvrCallBack callback, void *userPtr, int tryBind)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;

	if (NULL == pRtspSvrObj)			return RTSP_SERVER_ERR_BadArgument;
	if (listenPort < 1)					return RTSP_SERVER_ERR_BadArgument;

#ifdef _WIN32
#ifdef _DEBUG
	_TRACE_Init();
	_TRACE_OpenLogFile("LiveRtspServer");
#endif
#endif

#ifdef _WIN32
	if (NULL == pRtspSvrObj->hThread)
#else
	if (pRtspSvrObj->threadId == 0x00)
#endif
	{

		memset(pRtspSvrObj->username, 0x00, sizeof(pRtspSvrObj->username));
		memset(pRtspSvrObj->password, 0x00, sizeof(pRtspSvrObj->password));
		if (NULL != username && (int)strlen((char*)username)>0)
		{
			strcpy(pRtspSvrObj->username, (char*)username);
		}
		if (NULL != password && (int)strlen((char*)password) > 0)
		{
			strcpy(pRtspSvrObj->password, (char*)password);
		}
		if (NULL != realm && (int)strlen(realm) > 0)
		{
			memset(pRtspSvrObj->realm, 0x00, sizeof(pRtspSvrObj->realm));
			strcpy(pRtspSvrObj->realm, realm);
		}


		pRtspSvrObj->AuthorizationType = authType;
		pRtspSvrObj->listenPort = listenPort;
		pRtspSvrObj->pCallback = (void*)callback;
		pRtspSvrObj->pUserPtr  = userPtr;
		pRtspSvrObj->tryBind = tryBind;
		pRtspSvrObj->flag = 0x01;
		pRtspSvrObj->pEx	 = pRtspSvrObj;
#ifdef _WIN32
		pRtspSvrObj->hThread = CreateThread(NULL, 0, __RtspServerProcMainThread, pRtspSvrObj, 0, NULL);
		if (NULL == pRtspSvrObj->hThread)		pRtspSvrObj->flag = 0x00;
#else
		if(pthread_create(&pRtspSvrObj->threadId, NULL, __RtspServerProcMainThread, pRtspSvrObj) < 0)
        {
             pRtspSvrObj->flag = 0x00;
        }
#endif
		while (pRtspSvrObj->flag != 0x02 && pRtspSvrObj->flag!=0x00)	{Sleep(10);}

		while (NULL == pRtspSvrObj->rtspServer)
		{
			if (pRtspSvrObj->flag == 0x00)		break;
			Sleep(100);
		}
	}

	return pRtspSvrObj->flag==0x00?-1:0;
}

//创建通道
int RTSPSERVER_API	libRTSPSvr_CreateChannel(RTSP_SVR_HANDLE handle, const char *streamName, RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;

	return pRtspSvrObj->rtspServer->CreateChannel(streamName, channelHandle, channelPtr);
}

//推送媒体数据
int RTSPSERVER_API	libRTSPSvr_PutFrame(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle, unsigned int mediaType, MEDIA_FRAME_INFO_T *frameInfo)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == channelHandle)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == frameInfo)						return RTSP_SERVER_ERR_BadArgument;

	if (pRtspSvrObj->mFlag != LIVE_FLAG)		return RTSP_SERVER_ERR_NotInitialized;

	return pRtspSvrObj->rtspServer->PutFrame(channelHandle, mediaType, frameInfo);

}


//删除通道
int RTSPSERVER_API	libRTSPSvr_DeleteChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE *channelHandle)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;

	return pRtspSvrObj->rtspServer->DeleteChannel(channelHandle);
}


//添加用户
RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_AddUser(RTSP_SVR_HANDLE handle, const char *username, const char *password)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pRtspSvrObj->authDB)			return RTSP_SERVER_ERR_NotInitialized;

	if (NULL == username || NULL == password)	return RTSP_SERVER_ERR_BadArgument;
	if ( (int)strlen((char*)username) < 1)		return RTSP_SERVER_ERR_BadArgument;
	if ( (int)strlen((char*)password) < 1)		return RTSP_SERVER_ERR_BadArgument;

	pRtspSvrObj->authDB->addUserRecord((const char *)username, (const char *)password);
	return RTSP_SERVER_ERR_NoErr;
}

RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_DelUser(RTSP_SVR_HANDLE handle, const char *username)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pRtspSvrObj->authDB)			return RTSP_SERVER_ERR_NotInitialized;

	if (NULL == username)						return RTSP_SERVER_ERR_BadArgument;
	if ( (int)strlen((char*)username) < 1)		return RTSP_SERVER_ERR_BadArgument;
	
	pRtspSvrObj->authDB->removeUserRecord((const char *)username);
	return RTSP_SERVER_ERR_NoErr;
}

//停止服务
int RTSPSERVER_API	libRTSPSvr_Shutdown(RTSP_SVR_HANDLE handle)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)		return RTSP_SERVER_ERR_BadArgument;

	if (pRtspSvrObj->flag == 0x02)		pRtspSvrObj->flag = 0x03;
	while (pRtspSvrObj->flag != 0x00)	{Sleep(10);}

#ifdef _WIN32
	if (NULL != pRtspSvrObj->hThread)
	{
		if (pRtspSvrObj->flag == 0x02)	pRtspSvrObj->flag = 0x03;
		while (pRtspSvrObj->flag != 0x00)	{Sleep(10);}
		CloseHandle(pRtspSvrObj->hThread);
		pRtspSvrObj->hThread = NULL;
	}
#else
	if (pRtspSvrObj->threadId > 0U)
	{
		if (pRtspSvrObj->flag == 0x02)	pRtspSvrObj->flag = 0x03;
		while (pRtspSvrObj->flag != 0x00)	{usleep(1000*10);}
		pthread_join(pRtspSvrObj->threadId, NULL);
		pRtspSvrObj->threadId = 0x00;
	}
#endif

	return RTSP_SERVER_ERR_NoErr;
}

//反初始化
int RTSPSERVER_API	libRTSPSvr_Release(RTSP_SVR_HANDLE *handle)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)*handle;
	if (NULL == pRtspSvrObj)		return RTSP_SERVER_ERR_BadArgument;

	delete pRtspSvrObj;
	*handle = NULL;
	return RTSP_SERVER_ERR_NoErr;
}

int RTSPSERVER_API	libRTSPSvr_ResetChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pRtspSvrObj->rtspServer)		return RTSP_SERVER_ERR_NotInitialized;

	return pRtspSvrObj->rtspServer->ResetChannel(channelHandle);
	return RTSP_SERVER_ERR_NoErr;
}

//获取连接的客户端列表
int RTSPSERVER_API	libRTSPSvr_GetClientList(RTSP_SVR_HANDLE handle, RTSP_CLIENT_INFO_T	**pClientInfo, int *clientNum)
{
	RTSP_SERVER_OBJ_T *pRtspSvrObj = (RTSP_SERVER_OBJ_T *)handle;
	if (NULL == pRtspSvrObj)					return RTSP_SERVER_ERR_BadArgument;
	if (NULL == pRtspSvrObj->rtspServer)		return RTSP_SERVER_ERR_NotInitialized;
#if ENABLE_CLIENT_LIST == 0x01
	return pRtspSvrObj->rtspServer->GetRTSPClientList(pClientInfo, clientNum);
#endif

	return -1000;
}

void	ShutdownRtspServer(RTSP_SERVER_OBJ_T *serverObj)
{
	if (NULL == serverObj)			return;

	if (NULL != serverObj->rtspServer)
	{
		//rtspServer->removeServerMediaSession(sms);
		serverObj->rtspServer->DeleteAllChannel();
		Medium::close(serverObj->rtspServer, &serverObj->rtspServer->envir());
		serverObj->rtspServer = NULL;
		//shutdownStream(rtspObj.rtspClient, 0x00);
		//Medium::close(rtspObj.rtspClient);
		//rtspObj.rtspClient = NULL;
	}

	if (NULL != serverObj->authDB)
	{
		delete serverObj->authDB;
		serverObj->authDB = NULL;
	}

	if (NULL != serverObj->env)
	{
		serverObj->env->reclaim();
		serverObj->env = NULL;
	}
	if (NULL != serverObj->scheduler)
	{
		delete serverObj->scheduler;
		serverObj->scheduler = NULL;
	}
}




#ifdef _WIN32
DWORD WINAPI __RtspServerProcMainThread(LPVOID lpParam)
#else
#include <pthread.h>
void *__RtspServerProcMainThread(void *lpParam)
#endif
{
#ifdef ANDROID
	JNIEnv* env;  
	g_vm->AttachCurrentThread(&env, NULL);
#endif

	RTSP_SERVER_OBJ_T		*pServerObj = (RTSP_SERVER_OBJ_T *)lpParam;
	if (NULL == pServerObj)		return 0;

	int		select_error = 0;
	pServerObj->flag = 0x02;

	while (1)
	{
		if (pServerObj->flag == 0x03)			break;

#ifdef _EPOLL_
		if (NULL == pServerObj->scheduler)		pServerObj->scheduler = BasicTaskSchedulerEpoll::createNew(MAIN_THREAD_ID, MAX_EPOLL_MAIN_THREAD_EVENT);
#else
		if (NULL == pServerObj->scheduler)		pServerObj->scheduler = BasicTaskScheduler::createNew(MAIN_THREAD_ID, MAX_EPOLL_MAIN_THREAD_EVENT);
#endif
		if (NULL == pServerObj->scheduler)		break;
		if (NULL == pServerObj->env)			pServerObj->env = BasicUsageEnvironment::createNew(*pServerObj->scheduler, MAIN_THREAD_ID, "Main thread");
		if (NULL == pServerObj->env)			break;

		if ( (NULL==pServerObj->authDB) && (AUTHENTICATION_TYPE_NONE!=pServerObj->AuthorizationType) )
		{
			pServerObj->authDB = new UserAuthenticationDatabase(pServerObj->AuthorizationType, (int)strlen(pServerObj->realm)>0?pServerObj->realm:NULL);

			if ( (int)strlen(pServerObj->username) > 0 && (int)strlen(pServerObj->password) > 0)
			{
				pServerObj->authDB->addUserRecord(pServerObj->username, pServerObj->password);
			}
		}

		// Create the RTSP server.  Try first with the default port number (554),
		// and then with the alternative port number (8554):
		portNumBits rtspServerPortNum = pServerObj->listenPort;
		//pServerObj->rtspServer = LiveRtspServer::createNew(*pServerObj->env, rtspServerPortNum, pServerObj->authDB, 65U, pServerObj->pCallback, pServerObj->pUserPtr);
		pServerObj->rtspServer = LiveRtspServer::createNew(*pServerObj->env, rtspServerPortNum, pServerObj->authDB, 65U, pServerObj->pCallback, pServerObj->pUserPtr);
		if (pServerObj->rtspServer == NULL) 
		{
			//rtspServerPortNum = 8554;
			//rtspServer = LiveRtspServer::createNew(*pServerObj->env, rtspServerPortNum, authDB);
		}
		if (pServerObj->rtspServer == NULL && select_error == 0x00) 
		{
			*pServerObj->env << "Failed to create RTSP server: " << pServerObj->env->getResultMsg() << "\n";

			_TRACE(TRACE_LOG_DEBUG, (char *)"RTSP Server Startup Fail: %s\n", pServerObj->env->getResultMsg());

			if (pServerObj->tryBind == 0x01)
			{
				ShutdownRtspServer(pServerObj);

				Sleep(2000);

				continue;
			}

			break;
		}


		_TRACE(TRACE_LOG_DEBUG, (char *)"RTSP Server Startup Success...\n");


#if 0
		*pServerObj->env << "LIVE555 Media Server\n";
		*pServerObj->env << "\tversion " << MEDIA_SERVER_VERSION_STRING	
			<< " (LIVE555 Streaming Media library version "
			<< LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";
#endif

		/*
		char* urlPrefix = pServerObj->rtspServer->rtspURLPrefix();
		*pServerObj->env << "Play streams from this server using the URL\n\t"
			<< urlPrefix << "<filename>\nwhere <filename> is a file present in the current directory.\n";
		*pServerObj->env << "Each file's type is inferred from its name suffix:\n";
		*pServerObj->env << "\t\".264\" => a H.264 Video Elementary Stream file\n";
		*pServerObj->env << "\t\".265\" => a H.265 Video Elementary Stream file\n";
		*pServerObj->env << "\t\".aac\" => an AAC Audio (ADTS format) file\n";
		*pServerObj->env << "\t\".ac3\" => an AC-3 Audio file\n";
		*pServerObj->env << "\t\".amr\" => an AMR Audio file\n";
		*pServerObj->env << "\t\".dv\" => a DV Video file\n";
		*pServerObj->env << "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
		*pServerObj->env << "\t\".mkv\" => a Matroska audio+video+(optional)subtitles file\n";
		*pServerObj->env << "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
		*pServerObj->env << "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
		*pServerObj->env << "\t\".ogg\" or \".ogv\" or \".opus\" => an Ogg audio and/or video file\n";
		*pServerObj->env << "\t\".ts\" => a MPEG Transport Stream file\n";
		*pServerObj->env << "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
		*pServerObj->env << "\t\".vob\" => a VOB (MPEG-2 video with AC-3 audio) file\n";
		*pServerObj->env << "\t\".wav\" => a WAV Audio file\n";
		*pServerObj->env << "\t\".webm\" => a WebM audio(Vorbis)+video(VP8) file\n";
		*pServerObj->env << "See http://www.live555.com/mediaServer/ for additional documentation.\n";

		delete []urlPrefix;
		*/

		

		// Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
		// Try first with the default HTTP port (80), and then with the alternative HTTP
		// port numbers (8000 and 8080).
#if 0
		if (pServerObj->rtspServer->setUpTunnelingOverHTTP(80) || pServerObj->rtspServer->setUpTunnelingOverHTTP(8000) || pServerObj->rtspServer->setUpTunnelingOverHTTP(8080)) 
		{
			*pServerObj->env << "(We use port " << pServerObj->rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
		} else 
		{
			*pServerObj->env << "(RTSP-over-HTTP tunneling is not available.)\n";
		}
#endif

		while (1)
		{
			if (pServerObj->flag == 0x03)			break;

#ifdef _EPOLL_
			BasicTaskSchedulerEpoll *pTaskSchedulerEpoll = NULL;
			if (NULL != pServerObj->env && (pTaskSchedulerEpoll = (BasicTaskSchedulerEpoll *)&pServerObj->env->taskScheduler()))
#else
			BasicTaskScheduler0 *pTaskScheduler0 = NULL;
			if (NULL != pServerObj->env && (pTaskScheduler0 = (BasicTaskScheduler0 *)&pServerObj->env->taskScheduler()))
#endif
			{
				//pServerObj->rtspServer->LockClientConnection();
#ifdef _EPOLL_
				int ret = pTaskSchedulerEpoll->SingleStep(0, MAIN_THREAD_ID, pServerObj->env->GetEnvirName());
#else
				int ret = pTaskScheduler0->SingleStep(0, MAIN_THREAD_ID, pServerObj->env->GetEnvirName());
#endif
				if (ret < 0)
				{
					//pServerObj->rtspServer->UnlockClientConnection();

					select_error = 0x01;
					//_RTSPCLIENT_TRACE(TRACE_LOG_INFO, "channel[%d]     pTaskScheduler0->SingleStep(0) Ret:%d \n", pClient->channelInfo.id, ret);
					break;
				}
				//pServerObj->rtspServer->UnlockClientConnection();
			}

			if (NULL != pServerObj->rtspServer)		pServerObj->rtspServer->CheckChannelStatus();
		}

		//pServerObj->env->taskScheduler().doEventLoop(); // does not return

		ShutdownRtspServer(pServerObj);
	}

	ShutdownRtspServer(pServerObj);

	_TRACE(TRACE_LOG_DEBUG, (char *)"RTSP Server Shutdown Success...\n");

	pServerObj->flag = 0x00;

#ifndef _WIN32
    pthread_detach(pthread_self()); 
#else
	ExitThread(0);
#endif

#ifdef ANDROID
	g_vm->DetachCurrentThread();
#endif

	return 0;
}
