
#ifndef __LIB_RTSP_SERVER_API_H__
#define __LIB_RTSP_SERVER_API_H__

#include "RtspTypes.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif

#define RTSP_SERVER_NAME	"EasyRTSPServer v1.0.24.0511"


typedef enum _AUTHENTICATION_TYPE_ENUM
{
	AUTHENTICATION_TYPE_NONE		=	0x00,			//不验证
	AUTHENTICATION_TYPE_BASIC		=	0x01,
	AUTHENTICATION_TYPE_DIGEST		=	0x02,
}AUTHENTICATION_TYPE_ENUM;


//媒体类型
#ifndef MEDIA_TYPE_VIDEO
#define MEDIA_TYPE_VIDEO		0x00000001
#endif
#ifndef MEDIA_TYPE_AUDIO
#define MEDIA_TYPE_AUDIO		0x00000002
#endif
#ifndef MEDIA_TYPE_EVENT
#define MEDIA_TYPE_EVENT		0x00000004
#endif
#ifndef MEDIA_TYPE_RTP
#define MEDIA_TYPE_RTP			0x00000008
#endif
#ifndef MEDIA_TYPE_SDP
#define MEDIA_TYPE_SDP			0x00000010
#endif
#ifndef MEDIA_TYPE_CODEC_INFO
#define MEDIA_TYPE_CODEC_INFO	0x00000020
#endif

typedef struct __MEDIA_FRAME_INFO_T
{
    unsigned int    frameSize;		//帧长
    unsigned int    frameType;		//视频的类型，I帧或P帧
    unsigned char   *pBuffer;		//数据
	unsigned int	rtpTimestamp;	//时间戳
	unsigned char	timestamp[6];	//当前播放时间	年月日时分秒  年为后两位(2018则填18)
}MEDIA_FRAME_INFO_T;


typedef enum __RTSP_SERVER_STATE_T
{
	RTSP_SERVER_STATE_ERROR			=	0,						//内部错误
	RTSP_CHANNEL_OPEN_STREAM		=	0x00000001,				//请求打开通道
	RTSP_CHANNEL_START_STREAM,									//开始推流
	RTSP_CHANNEL_FIND_STREAM,									//查找流   在START_STREAM后, 即正常推送流, 如果库内部长时间没有收到流,则会回调该状态
	RTSP_CHANNEL_STOP_STREAM,									//停止推流	当访问对应通道的所有客户端都断开连接后, 回调该状态
	RTSP_CHANNEL_CLOSE_STREAM,									//关闭通道  当访问对应通道的所有客户端都断开连接后, 回调该状态
	RTSP_CHANNEL_PLAY_CONTROL,									//播放控制
}RTSP_SERVER_STATE_T;


//客户端信息
typedef struct __LIVE_CLIENT_INFO_T
{
	int		connection;		//连接状态
	char	session[200];	//客户端session
	char	ipaddr[66];		//客户端IP
	int		port;			//客户端Port
	char	name[128];		//客户端名称
}RTSP_CLIENT_INFO_T;

typedef void *RTSP_SVR_HANDLE;
typedef void *RTSP_CHANNEL_HANDLE;

/* 回调函数定义 userptr表示用户自定义数据 */
//mediaInfo中，需填写videoCodec、sps、pps
typedef int (CALLBACK *RTSPSvrCallBack)(RTSP_SERVER_STATE_T serverStatus, const char *resourceName, 
											RTSP_CHANNEL_HANDLE *channelHandle, 
											RTSP_MEDIA_INFO_T *mediaInfo, 
											RTSP_PLAY_CONTROL_INFO_T *playCtrlInfo, 
											void *userPtr, void *channelPtr);


#ifdef __cplusplus
extern "C"
{
#endif

	//初始化, 创建RTSP Server句柄
	int RTSPSERVER_API	libRTSPSvr_Create(RTSP_SVR_HANDLE *handle);

	//启动服务
	/*
			handle:   使用libRTSPSvr_Create创建的句柄
			listenPort: 监听端口
			realm:		服务端标识
			authType: 用户验证类型, 可选择不验证、Basic、Digest三种;
			username: 验证用户名	  用户名和密码还可使用libRTSPSvr_AddUser添加或libRTSPSvr_DelUser删除
			password: 验证密码
			callback: 回调函数
			userPtr:  用户自定义指针
			tryBind: 尝试绑定, 0为一次操作，如果绑定失败则返回; 1在绑定失败情况下继续尝试绑定
	*/
	int RTSPSERVER_API	libRTSPSvr_Startup(RTSP_SVR_HANDLE handle, int listenPort, const char *realm, 
												AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
												RTSPSvrCallBack callback, void *userPtr, int tryBind);

	//创建通道
	/*
			handle:   		使用libRTSPSvr_Create创建的句柄
			resourceName: 	同回调函数中的resourceName
			channelHandle: 	返回的通道句柄
			channelPtr:   	通道自定义指针
	*/
	int RTSPSERVER_API	libRTSPSvr_CreateChannel(RTSP_SVR_HANDLE handle, const char *resourceName, 
												RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr);

	//推送媒体数据		不需要StartCode(0x00 0x00 0x00 0x01)
	/*
			handle:   		使用libRTSPSvr_Create创建的句柄
			channelHandle: 	使用libRTSPSvr_CreateChannel创建的通道句柄
			mediaType:		媒体类型
			frameInfo:		帧信息
	*/
	int RTSPSERVER_API	libRTSPSvr_PutFrame(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle, 
													unsigned int mediaType, MEDIA_FRAME_INFO_T *frameInfo);

	//删除通道
	/*
			handle:   		使用libRTSPSvr_Create创建的句柄
			channelHandle: 	使用libRTSPSvr_CreateChannel创建的通道句柄
	*/
	int RTSPSERVER_API	libRTSPSvr_DeleteChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE *channelHandle);

	//复位通道
	/*
			handle:   		使用libRTSPSvr_Create创建的句柄
			channelHandle: 	使用libRTSPSvr_CreateChannel创建的通道句柄
	*/
	int RTSPSERVER_API	libRTSPSvr_ResetChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle);

	//获取连接的客户端列表
	/*
			handle:   			使用libRTSPSvr_Create创建的句柄
			pClientInfo:		客户端信息
			clientNum:			客户端个数		
	*/
	int RTSPSERVER_API	libRTSPSvr_GetClientList(RTSP_SVR_HANDLE handle, RTSP_CLIENT_INFO_T	**pClientInfo, int *clientNum);


	//添加用户  或    根据用户名修改用户密码
	//如果添加的用户名不存在,则为新增, 如已存在,则为修改密码
	/*
			handle:   			使用libRTSPSvr_Create创建的句柄
			username:  			待添加的用户名
			clientNum:			用户名对应的密码
	*/
	RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_AddUser(RTSP_SVR_HANDLE handle, const char *username, const char *password);
	//删除用户
	/*
			handle:   			使用libRTSPSvr_Create创建的句柄
			username:  			待删除的用户名
	*/
	RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_DelUser(RTSP_SVR_HANDLE handle, const char *username);

	//停止服务  ####### 务必在关闭所有源后调用该函数 ######## 
	/*
			handle:   			使用libRTSPSvr_Create创建的句柄
	*/
	int RTSPSERVER_API	libRTSPSvr_Shutdown(RTSP_SVR_HANDLE handle);


	//释放RTSP Server句柄
	/*
			handle:   			使用libRTSPSvr_Create创建的句柄
	*/
	int RTSPSERVER_API	libRTSPSvr_Release(RTSP_SVR_HANDLE *handle);
#ifdef __cplusplus
}
#endif



/*
Linux .so  函数声明
int (*func_RTSPServer_Create)(RTSP_SVR_HANDLE *handle);
int (*func_RTSPServer_Startup)(RTSP_SVR_HANDLE handle, int listenPort, const char *realm, 
												AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
												RTSPSvrCallBack callback, void *userPtr);
int (*func_RTSPServer_CreateChannel)(RTSP_SVR_HANDLE handle, const char *resourceName, 
												RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr);
int (*func_RTSPServer_PutFrame)(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle, 
													unsigned int mediaType, MEDIA_FRAME_INFO_T *frameInfo);
int (*func_RTSPServer_DeleteChannel)(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE *channelHandle);
int (*func_RTSPServer_Shutdown)(RTSP_SVR_HANDLE handle);
int (*func_RTSPServer_Release)(RTSP_SVR_HANDLE *handle);
*/

/*
流程:
1. 调用 libRTSPSvr_Startup 设置监听端口、回调函数和自定义数据指针
2. 启动后，程序进入监听状态
	2.1		接收到客户端请求, 回调 状态:RTSP_CHANNEL_OPEN_STREAM          上层程序在填充完mediainfo后，返回0, 则RtspServer响应客户端ok
	2.2		libRTSPSvr回调状态 RTSP_CHANNEL_START_STREAM , 则表示rtsp交互完成, 开始发送流, 上层程序调用libRTSPSvr_PushFrame 发送帧数据
	2.3		libRTSPSvr回调状态 RTSP_CHANNEL_STOP_STREAM , 则表示客户端已发送teaardown, 要求停止发送帧数据
3.	调用 libRTSPSvr_Shutdown(), 关闭libRTSPSvr，释放相关资源

*/


#endif
