
#ifndef __EASY_RTSP_SERVER_H__
#define __EASY_RTSP_SERVER_H__

#include "EasyTypes.h"
#ifdef _WIN32
#include <winsock2.h>
#endif

typedef enum _EASY_AUTHENTICATION_TYPE_ENUM
{
	EASY_AUTHENTICATION_TYPE_NONE		=	0x00,			//����֤
	EASY_AUTHENTICATION_TYPE_BASIC		=	0x01,
	EASY_AUTHENTICATION_TYPE_DIGEST	=	0x02,
}EASY_AUTHENTICATION_TYPE_ENUM;




/* ý����Ϣ */
typedef struct __EASY_RTSPSERVER_MEDIA_INFO_T
{
	Easy_U32 videoCodec;				//��Ƶ��������
	Easy_U32 videoFps;					//��Ƶ֡��
	Easy_U32 videoQueueSize;			//��Ƶ���д�С	��: 1024 * 1024

	Easy_U32 audioCodec;				//��Ƶ��������
	Easy_U32 audioSampleRate;			//��Ƶ������
	Easy_U32 audioChannel;				//��Ƶͨ����
	Easy_U32 audioBitsPerSample;		//��Ƶ��������
	Easy_U32 audioQueueSize;			//��Ƶ���д�С	��: 1024 * 128

	Easy_U32 metadataCodec;				//Metadata����
	Easy_U32 metadataQueueSize;			//Metadata���д�С	��: 1024 * 512

	Easy_U32 vpsLength;				//��Ƶvps֡����
	Easy_U32 spsLength;				//��Ƶsps֡����
	Easy_U32 ppsLength;				//��Ƶpps֡����
	Easy_U32 seiLength;				//��Ƶsei֡����
	Easy_U8	 vps[256];			//��Ƶvps֡����
	Easy_U8	 sps[256];			//��Ƶsps֡����
	Easy_U8	 pps[128];			//��Ƶsps֡����
	Easy_U8	 sei[128];			//��Ƶsei֡����
}EASY_RTSPSERVER_MEDIA_INFO_T;

typedef enum __EASY_PLAY_CTRL_CMD_ENUM
{
	EASY_PLAY_CTRL_CMD_GET_DURATION	=	0x00000001,	//��ȡ¼��ʱ��
	EASY_PLAY_CTRL_CMD_PAUSE	=	0x00000002,	//��ͣ
	EASY_PLAY_CTRL_CMD_RESUME,					//�ָ�
	EASY_PLAY_CTRL_CMD_SCALE,					//��������
	EASY_PLAY_CTRL_CMD_SEEK_STREAM,				//��תʱ��
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
	EASY_RTSPSERVER_STATE_ERROR	=	0,					//�ڲ�����
	EASY_CHANNEL_OPEN_STREAM			=	0x00000001,	//�����ͨ��
	EASY_CHANNEL_START_STREAM,								//��ʼ����
	EASY_CHANNEL_FIND_STREAM,									//������   ��START_STREAM��, ������������, ������ڲ���ʱ��û���յ���,���ص���״̬
	EASY_CHANNEL_STOP_STREAM,									//ֹͣ����	�����ʶ�Ӧͨ�������пͻ��˶��Ͽ����Ӻ�, �ص���״̬
	EASY_CHANNEL_CLOSE_STREAM,								//�ر�ͨ��  �����ʶ�Ӧͨ�������пͻ��˶��Ͽ����Ӻ�, �ص���״̬
	EASY_CHANNEL_PLAY_CONTROL,								//���ſ���
}EASY_RTSPSERVER_STATE_T;


typedef void *EASY_CHANNEL_HANDLE;

/* �ص��������� userptr��ʾ�û��Զ������� */
//mediaInfo�У�����дvideoCodec��sps��pps
typedef Easy_I32 (CALLBACK *EasyRtspServer_Callback)(EASY_RTSPSERVER_STATE_T serverStatus, const char *resourceName, 
											EASY_CHANNEL_HANDLE *channelHandle, 
											EASY_RTSPSERVER_MEDIA_INFO_T *mediaInfo, 
											EASY_PLAY_CONTROL_INFO_T *playCtrlInfo, 
											void *userPtr, void *channelPtr);

#ifdef __cplusplus
extern "C"
{
#endif

	/* ���� RTSP Server: ���ü����˿�, �ص��������Զ�������  */
	/*
			listenPort: �����˿�
			realm:		����˱�ʶ
			authType: �û���֤����, ��ѡ����֤��Basic��Digest����;
			username: ��֤�û���	  �û��������뻹��ʹ��EasyRtspServer_AddUser��ӻ�EasyRtspServer_DelUserɾ��
			password: ��֤����
			callback: �ص�����
			userPtr:  �û��Զ���ָ��
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Startup(Easy_U16 listenPort, const char *realm, 
												EASY_AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
												EasyRtspServer_Callback callback, void *userPtr);
	
	//��ֹ RTSP Server  ####### ����ڹر�����Դ����øú��� ######## 
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_Shutdown();


	//����ͨ��     �˺����ڻص������е���
	/*
			resourceName: 	ͬ�ص������е�resourceName
			channelHandle: 	���ص�ͨ�����
			channelPtr:   	ͨ���Զ���ָ��
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_CreateChannel(const char *resourceName, 
												EASY_CHANNEL_HANDLE *channelHandle, void *channelPtr);

	//����ý������		����ҪStartCode(0x00 0x00 0x00 0x01)
	/*
			channelHandle: 	ʹ��EasyRtspServer_CreateChannel������ͨ�����
			mediaType:			ý������
			frameInfo:			֡��Ϣ
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_PushFrame(EASY_CHANNEL_HANDLE channelHandle, 
													EASY_AV_Frame *frame);

	//ɾ��ͨ��			�˺����ڻص������е���
	/*
			channelHandle: 	ʹ��EasyRtspServer_CreateChannel������ͨ�����
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DeleteChannel(EASY_CHANNEL_HANDLE *channelHandle);


	//��λͨ��	�ֱ��ʱ仯 ֻҪ��Ƶ����Ƶ�����仯ʱ, ���� EasyRtspServer_ResetChannel
	/*
			channelHandle: 	ʹ��EasyRtspServer_CreateChannel������ͨ�����
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_ResetChannel(EASY_CHANNEL_HANDLE channelHandle);


	//����û�  ��    �����û����޸��û�����
	//�����ӵ��û���������,��Ϊ����, ���Ѵ���,��Ϊ�޸�����
	/*
			username:  			����ӵ��û���
			clientNum:			�û�����Ӧ������
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_AddUser(const Easy_U8 *username, const Easy_U8 *password);
	//ɾ���û�
	/*
			username:  			��ɾ�����û���
	*/
	Easy_API Easy_I32 Easy_APICALL EasyRtspServer_DelUser(const Easy_U8 *username);


#ifdef __cplusplus
}
#endif


#endif
