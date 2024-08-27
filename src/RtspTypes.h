
#ifndef __RTSP_TYPES_H__
#define __RTSP_TYPES_H__


#ifdef _WIN32
#include <WinSock2.h>
#endif


#ifdef _WIN32
#define RTSPSERVER_API  __declspec(dllexport)
#define RTSPSERVER_APICALL  __stdcall
#define WIN32_LEAN_AND_MEAN
#else
#define RTSPSERVER_API __attribute__ ((visibility("default")))
#define RTSPSERVER_APICALL 
#define CALLBACK
#endif


typedef enum __RTSP_SERVER_ERROR_CODE_ENUM
{
	RTSP_SERVER_ERR_NoErr			=	0,
	RTSP_SERVER_ERR_NotInitialized	=	-1,			//û�г�ʼ��
	RTSP_SERVER_ERR_BadArgument		=	-2,			//��������
	RTSP_SERVER_ERR_ALLOC_MEMORY	=	-3,			//�ڴ����ʧ��
	RTSP_SERVER_ERR_OPERATE			=	-4,			//����ʧ��
}RTSP_SERVER_ERROR_CODE_ENUM;


//��Ƶ����
#define RTSP_VIDEO_CODEC_H264	0x1C				//H264
#define RTSP_VIDEO_CODEC_H265	0xAE				//H265
#define	RTSP_VIDEO_CODEC_MJPEG	0x08				//MJPEG
#define	RTSP_VIDEO_CODEC_MPEG4	0x0D				//MPEG4

//��Ƶ����
#define RTSP_AUDIO_CODEC_AAC	0x15002		//AAC
#define RTSP_AUDIO_CODEC_G711U	0x10006		//G711 ulaw
#define RTSP_AUDIO_CODEC_G711A	0x10007		//G711 alaw
#define RTSP_AUDIO_CODEC_G726	0x1100B		//G726


//��Ƶ�ؼ��ֱ�ʶ
#define RTSP_VIDEO_FRAME_I		0x01		//I֡
#define RTSP_VIDEO_FRAME_P		0x02		//P֡
#define RTSP_VIDEO_FRAME_B		0x03		//B֡
#define RTSP_VIDEO_FRAME_J		0x04		//JPEG

//ý����Ϣ
typedef struct __RTSP_MEDIA_INFO_T
{
	unsigned int videoCodec;				//��Ƶ��������
	unsigned int videoFps;					//��Ƶ֡��
	unsigned int videoQueueSize;			//��Ƶ���д�С	��: 1024 * 1024

	unsigned int audioCodec;				//��Ƶ��������
	unsigned int audioSampleRate;			//��Ƶ������
	unsigned int audioChannel;				//��Ƶͨ����
	unsigned int audioBitsPerSample;		//��Ƶ��������
	unsigned int audioQueueSize;			//��Ƶ���д�С	��: 1024 * 128

	unsigned int metadataCodec;				//Metadata����
	unsigned int metadataQueueSize;			//Metadata���д�С	��: 1024 * 512

	unsigned int vpsLength;				//��Ƶvps֡����
	unsigned int spsLength;				//��Ƶsps֡����
	unsigned int ppsLength;				//��Ƶpps֡����
	unsigned int seiLength;				//��Ƶsei֡����
	unsigned char	 vps[256];				//��Ƶvps֡����
	unsigned char	 sps[256];				//��Ƶsps֡����
	unsigned char	 pps[128];				//��Ƶsps֡����
	unsigned char	 sei[128];				//��Ƶsei֡����
}RTSP_MEDIA_INFO_T;


typedef enum __RTSP_PLAY_CTRL_CMD_ENUM
{
	RTSP_PLAY_CTRL_CMD_GET_DURATION	=	0x00000001,	//��ȡ¼��ʱ��
	RTSP_PLAY_CTRL_CMD_PAUSE	=	0x00000002,		//��ͣ
	RTSP_PLAY_CTRL_CMD_RESUME,						//�ָ�
	RTSP_PLAY_CTRL_CMD_SCALE,						//��������
	RTSP_PLAY_CTRL_CMD_SEEK_STREAM,					//��תʱ��
	//RTSP_PLAY_CTRL_CMD_FRAME_BY_FRAME,			//��֡
}RTSP_PLAY_CTRL_CMD_ENUM;
typedef struct __RTSP_PLAY_CONTROL_INFO_T
{
	RTSP_PLAY_CTRL_CMD_ENUM	ctrlCommand;
	unsigned int	mediaType;
	float		scale;
	char		startTime[36];
	char		endTime[36];

	char		suffix[256];
}RTSP_PLAY_CONTROL_INFO_T;


//�Զ��� Metadata ͷ��Ϣ
typedef struct __CUSTOM_META_DATA_HEADER_T
{
	int nsize;			/* size of structure		   */
	int customid;		/* custom module id			   */
	int moduleid;		/* module id - ordering number */
	int datalen;		/* size of whole data		   */

	char reserved[16];
}CUSTOM_META_DATA_HEADER_T;


#endif
