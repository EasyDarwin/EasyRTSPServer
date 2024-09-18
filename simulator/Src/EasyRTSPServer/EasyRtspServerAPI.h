
#ifndef __EASY_RTSP_SERVER_H__
#define __EASY_RTSP_SERVER_H__

#include "EasyTypes.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

typedef enum _EASY_AUTHENTICATION_TYPE_ENUM
{
	EASY_AUTHENTICATION_TYPE_NONE		=	0x00,			//不验证
	EASY_AUTHENTICATION_TYPE_BASIC		=	0x01,
	EASY_AUTHENTICATION_TYPE_DIGEST	=	0x02,
}EASY_AUTHENTICATION_TYPE_ENUM;




/* 媒体信息 */
typedef struct __EASY_RTSPSERVER_MEDIA_INFO_T
{
	Easy_U32 videoCodec;				//视频编码类型
	Easy_U32 videoFps;					//视频帧率
	Easy_U32 videoQueueSize;			//视频队列大小	如: 1024 * 1024

	Easy_U32 audioCodec;				//音频编码类型
	Easy_U32 audioSampleRate;			//音频采样率
	Easy_U32 audioChannel;				//音频通道数
	Easy_U32 audioBitsPerSample;		//音频采样精度
	Easy_U32 audioQueueSize;			//音频队列大小	如: 1024 * 128

	Easy_U32 metadataCodec;				//Metadata类型
	Easy_U32 metadataQueueSize;			//Metadata队列大小	如: 1024 * 512

	Easy_U32 vpsLength;				//视频vps帧长度
	Easy_U32 spsLength;				//视频sps帧长度
	Easy_U32 ppsLength;				//视频pps帧长度
	Easy_U32 seiLength;				//视频sei帧长度
	Easy_U8	 vps[256];			//视频vps帧内容
	Easy_U8	 sps[256];			//视频sps帧内容
	Easy_U8	 pps[128];			//视频sps帧内容
	Easy_U8	 sei[128];			//视频sei帧内容
}EASY_RTSPSERVER_MEDIA_INFO_T;

typedef enum __EASY_PLAY_CTRL_CMD_ENUM
{
	EASY_PLAY_CTRL_CMD_GET_DURATION	=	0x00000001,	//获取录像时长
	EASY_PLAY_CTRL_CMD_PAUSE	=	0x00000002,	//暂停
	EASY_PLAY_CTRL_CMD_RESUME,					//恢复
	EASY_PLAY_CTRL_CMD_SCALE,					//调整倍率
	EASY_PLAY_CTRL_CMD_SEEK_STREAM,				//跳转时间
}EASY_PLAY_CTRL_CMD_ENUM;

typedef struct __EASY_PLAY_CONTROL_INFO_T
{
	EASY_PLAY_CTRL_CMD_ENUM	ctrlCommand;
	unsigned int	mediaType;
	float		scale;
	char		startTime[36];
	char		endTime[36];
	char		suffix[256];
}EASY_PLAY_CONTROL_INFO_T;

typedef enum __EASY_RTSPSERVER_STATE_T
{
	EASY_RTSPSERVER_STATE_ERROR	=	0,					//内部错误
	EASY_CHANNEL_OPEN_STREAM			=	0x00000001,	//请求打开通道
	EASY_CHANNEL_START_STREAM,								//开始推流
	EASY_CHANNEL_FIND_STREAM,									//查找流   在START_STREAM后, 即正常推送流, 如果库内部长时间没有收到流,则会回调该状态
	EASY_CHANNEL_STOP_STREAM,									//停止推流	当访问对应通道的所有客户端都断开连接后, 回调该状态
	EASY_CHANNEL_CLOSE_STREAM,								//关闭通道  当访问对应通道的所有客户端都断开连接后, 回调该状态
	EASY_CHANNEL_PLAY_CONTROL,								//播放控制
}EASY_RTSPSERVER_STATE_T;


typedef void *EASY_CHANNEL_HANDLE;

/* 回调函数定义 userptr表示用户自定义数据 */
//mediaInfo中，需填写videoCodec、sps、pps
typedef Easy_I32 (CALLBACK *EasyRtspServer_Callback)(EASY_RTSPSERVER_STATE_T serverStatus, const char *resourceName, 
											EASY_CHANNEL_HANDLE *channelHandle, 
											EASY_RTSPSERVER_MEDIA_INFO_T *mediaInfo, 
											EASY_PLAY_CONTROL_INFO_T *playCtrlInfo, 
											void *userPtr, void *channelPtr);

#ifdef __cplusplus
extern "C"
{
#endif

	/* 启动 RTSP Server: 设置监听端口, 回调函数及自定义数据  */
	/*
			listenPort: 监听端口
			realm:		服务端标识
			authType: 用户验证类型, 可选择不验证、Basic、Digest三种;
			username: 验证用户名	  用户名和密码还可使用EasyRtspServer_AddUser添加或EasyRtspServer_DelUser删除
			password: 验证密码
			callback: 回调函数
			userPtr:  用户自定义指针
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Startup(Easy_U16 listenPort, const char *realm, 
												EASY_AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
												EasyRtspServer_Callback callback, void *userPtr);
	
	//终止 RTSP Server  ####### 务必在关闭所有源后调用该函数 ######## 
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Shutdown();


	//创建通道     此函数在回调函数中调用
	/*
			resourceName: 	同回调函数中的resourceName
			channelHandle: 	返回的通道句柄
			channelPtr:   	通道自定义指针
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_CreateChannel(const char *resourceName, 
												EASY_CHANNEL_HANDLE *channelHandle, void *channelPtr);

	//推送媒体数据		不需要StartCode(0x00 0x00 0x00 0x01)
	/*
			channelHandle: 	使用EasyRtspServer_CreateChannel创建的通道句柄
			mediaType:			媒体类型
			frameInfo:			帧信息
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_PushFrame(EASY_CHANNEL_HANDLE channelHandle, 
													EASY_AV_Frame *frame);

	//删除通道			此函数在回调函数中调用
	/*
			channelHandle: 	使用EasyRtspServer_CreateChannel创建的通道句柄
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DeleteChannel(EASY_CHANNEL_HANDLE *channelHandle);


	//复位通道	分辨率变化 只要视频或音频参数变化时, 调用 EasyRtspServer_ResetChannel
	/*
			channelHandle: 	使用EasyRtspServer_CreateChannel创建的通道句柄
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_ResetChannel(EASY_CHANNEL_HANDLE channelHandle);


	//添加用户  或    根据用户名修改用户密码
	//如果添加的用户名不存在,则为新增, 如已存在,则为修改密码
	/*
			username:  			待添加的用户名
			clientNum:			用户名对应的密码
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_AddUser(const Easy_U8 *username, const Easy_U8 *password);
	//删除用户
	/*
			username:  			待删除的用户名
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DelUser(const Easy_U8 *username);


#ifdef __cplusplus
}
#endif


#endif
