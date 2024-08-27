
#ifndef __LIB_RTSP_SERVER_API_H__
#define __LIB_RTSP_SERVER_API_H__

#include "RtspTypes.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif

#define RTSP_SERVER_NAME	"EasyRTSPServer v1.0.24.0511"


typedef enum _AUTHENTICATION_TYPE_ENUM
{
	AUTHENTICATION_TYPE_NONE		=	0x00,			//����֤
	AUTHENTICATION_TYPE_BASIC		=	0x01,
	AUTHENTICATION_TYPE_DIGEST		=	0x02,
}AUTHENTICATION_TYPE_ENUM;


//ý������
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
    unsigned int    frameSize;		//֡��
    unsigned int    frameType;		//��Ƶ�����ͣ�I֡��P֡
    unsigned char   *pBuffer;		//����
	unsigned int	rtpTimestamp;	//ʱ���
	unsigned char	timestamp[6];	//��ǰ����ʱ��	������ʱ����  ��Ϊ����λ(2018����18)
}MEDIA_FRAME_INFO_T;


typedef enum __RTSP_SERVER_STATE_T
{
	RTSP_SERVER_STATE_ERROR			=	0,						//�ڲ�����
	RTSP_CHANNEL_OPEN_STREAM		=	0x00000001,				//�����ͨ��
	RTSP_CHANNEL_START_STREAM,									//��ʼ����
	RTSP_CHANNEL_FIND_STREAM,									//������   ��START_STREAM��, ������������, ������ڲ���ʱ��û���յ���,���ص���״̬
	RTSP_CHANNEL_STOP_STREAM,									//ֹͣ����	�����ʶ�Ӧͨ�������пͻ��˶��Ͽ����Ӻ�, �ص���״̬
	RTSP_CHANNEL_CLOSE_STREAM,									//�ر�ͨ��  �����ʶ�Ӧͨ�������пͻ��˶��Ͽ����Ӻ�, �ص���״̬
	RTSP_CHANNEL_PLAY_CONTROL,									//���ſ���
}RTSP_SERVER_STATE_T;


//�ͻ�����Ϣ
typedef struct __LIVE_CLIENT_INFO_T
{
	int		connection;		//����״̬
	char	session[200];	//�ͻ���session
	char	ipaddr[66];		//�ͻ���IP
	int		port;			//�ͻ���Port
	char	name[128];		//�ͻ�������
}RTSP_CLIENT_INFO_T;

typedef void *RTSP_SVR_HANDLE;
typedef void *RTSP_CHANNEL_HANDLE;

/* �ص��������� userptr��ʾ�û��Զ������� */
//mediaInfo�У�����дvideoCodec��sps��pps
typedef int (CALLBACK *RTSPSvrCallBack)(RTSP_SERVER_STATE_T serverStatus, const char *resourceName, 
											RTSP_CHANNEL_HANDLE *channelHandle, 
											RTSP_MEDIA_INFO_T *mediaInfo, 
											RTSP_PLAY_CONTROL_INFO_T *playCtrlInfo, 
											void *userPtr, void *channelPtr);


#ifdef __cplusplus
extern "C"
{
#endif

	//��ʼ��, ����RTSP Server���
	int RTSPSERVER_API	libRTSPSvr_Create(RTSP_SVR_HANDLE *handle);

	//��������
	/*
			handle:   ʹ��libRTSPSvr_Create�����ľ��
			listenPort: �����˿�
			realm:		����˱�ʶ
			authType: �û���֤����, ��ѡ����֤��Basic��Digest����;
			username: ��֤�û���	  �û��������뻹��ʹ��libRTSPSvr_AddUser��ӻ�libRTSPSvr_DelUserɾ��
			password: ��֤����
			callback: �ص�����
			userPtr:  �û��Զ���ָ��
			tryBind: ���԰�, 0Ϊһ�β����������ʧ���򷵻�; 1�ڰ�ʧ������¼������԰�
	*/
	int RTSPSERVER_API	libRTSPSvr_Startup(RTSP_SVR_HANDLE handle, int listenPort, const char *realm, 
												AUTHENTICATION_TYPE_ENUM authType, const char *username, const char *password, 
												RTSPSvrCallBack callback, void *userPtr, int tryBind);

	//����ͨ��
	/*
			handle:   		ʹ��libRTSPSvr_Create�����ľ��
			resourceName: 	ͬ�ص������е�resourceName
			channelHandle: 	���ص�ͨ�����
			channelPtr:   	ͨ���Զ���ָ��
	*/
	int RTSPSERVER_API	libRTSPSvr_CreateChannel(RTSP_SVR_HANDLE handle, const char *resourceName, 
												RTSP_CHANNEL_HANDLE *channelHandle, void *channelPtr);

	//����ý������		����ҪStartCode(0x00 0x00 0x00 0x01)
	/*
			handle:   		ʹ��libRTSPSvr_Create�����ľ��
			channelHandle: 	ʹ��libRTSPSvr_CreateChannel������ͨ�����
			mediaType:		ý������
			frameInfo:		֡��Ϣ
	*/
	int RTSPSERVER_API	libRTSPSvr_PutFrame(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle, 
													unsigned int mediaType, MEDIA_FRAME_INFO_T *frameInfo);

	//ɾ��ͨ��
	/*
			handle:   		ʹ��libRTSPSvr_Create�����ľ��
			channelHandle: 	ʹ��libRTSPSvr_CreateChannel������ͨ�����
	*/
	int RTSPSERVER_API	libRTSPSvr_DeleteChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE *channelHandle);

	//��λͨ��
	/*
			handle:   		ʹ��libRTSPSvr_Create�����ľ��
			channelHandle: 	ʹ��libRTSPSvr_CreateChannel������ͨ�����
	*/
	int RTSPSERVER_API	libRTSPSvr_ResetChannel(RTSP_SVR_HANDLE handle, RTSP_CHANNEL_HANDLE channelHandle);

	//��ȡ���ӵĿͻ����б�
	/*
			handle:   			ʹ��libRTSPSvr_Create�����ľ��
			pClientInfo:		�ͻ�����Ϣ
			clientNum:			�ͻ��˸���		
	*/
	int RTSPSERVER_API	libRTSPSvr_GetClientList(RTSP_SVR_HANDLE handle, RTSP_CLIENT_INFO_T	**pClientInfo, int *clientNum);


	//����û�  ��    �����û����޸��û�����
	//�����ӵ��û���������,��Ϊ����, ���Ѵ���,��Ϊ�޸�����
	/*
			handle:   			ʹ��libRTSPSvr_Create�����ľ��
			username:  			����ӵ��û���
			clientNum:			�û�����Ӧ������
	*/
	RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_AddUser(RTSP_SVR_HANDLE handle, const char *username, const char *password);
	//ɾ���û�
	/*
			handle:   			ʹ��libRTSPSvr_Create�����ľ��
			username:  			��ɾ�����û���
	*/
	RTSPSERVER_API int RTSPSERVER_APICALL libRTSPSvr_DelUser(RTSP_SVR_HANDLE handle, const char *username);

	//ֹͣ����  ####### ����ڹر�����Դ����øú��� ######## 
	/*
			handle:   			ʹ��libRTSPSvr_Create�����ľ��
	*/
	int RTSPSERVER_API	libRTSPSvr_Shutdown(RTSP_SVR_HANDLE handle);


	//�ͷ�RTSP Server���
	/*
			handle:   			ʹ��libRTSPSvr_Create�����ľ��
	*/
	int RTSPSERVER_API	libRTSPSvr_Release(RTSP_SVR_HANDLE *handle);
#ifdef __cplusplus
}
#endif



/*
Linux .so  ��������
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
����:
1. ���� libRTSPSvr_Startup ���ü����˿ڡ��ص��������Զ�������ָ��
2. �����󣬳���������״̬
	2.1		���յ��ͻ�������, �ص� ״̬:RTSP_CHANNEL_OPEN_STREAM          �ϲ�����������mediainfo�󣬷���0, ��RtspServer��Ӧ�ͻ���ok
	2.2		libRTSPSvr�ص�״̬ RTSP_CHANNEL_START_STREAM , ���ʾrtsp�������, ��ʼ������, �ϲ�������libRTSPSvr_PushFrame ����֡����
	2.3		libRTSPSvr�ص�״̬ RTSP_CHANNEL_STOP_STREAM , ���ʾ�ͻ����ѷ���teaardown, Ҫ��ֹͣ����֡����
3.	���� libRTSPSvr_Shutdown(), �ر�libRTSPSvr���ͷ������Դ

*/


#endif
