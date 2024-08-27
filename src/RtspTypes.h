
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
	RTSP_SERVER_ERR_NotInitialized	=	-1,			//没有初始化
	RTSP_SERVER_ERR_BadArgument		=	-2,			//参数错误
	RTSP_SERVER_ERR_ALLOC_MEMORY	=	-3,			//内存分配失败
	RTSP_SERVER_ERR_OPERATE			=	-4,			//操作失败
}RTSP_SERVER_ERROR_CODE_ENUM;


//视频编码
#define RTSP_VIDEO_CODEC_H264	0x1C				//H264
#define RTSP_VIDEO_CODEC_H265	0xAE				//H265
#define	RTSP_VIDEO_CODEC_MJPEG	0x08				//MJPEG
#define	RTSP_VIDEO_CODEC_MPEG4	0x0D				//MPEG4

//音频编码
#define RTSP_AUDIO_CODEC_AAC	0x15002		//AAC
#define RTSP_AUDIO_CODEC_G711U	0x10006		//G711 ulaw
#define RTSP_AUDIO_CODEC_G711A	0x10007		//G711 alaw
#define RTSP_AUDIO_CODEC_G726	0x1100B		//G726


//视频关键字标识
#define RTSP_VIDEO_FRAME_I		0x01		//I帧
#define RTSP_VIDEO_FRAME_P		0x02		//P帧
#define RTSP_VIDEO_FRAME_B		0x03		//B帧
#define RTSP_VIDEO_FRAME_J		0x04		//JPEG

//媒体信息
typedef struct __RTSP_MEDIA_INFO_T
{
	unsigned int videoCodec;				//视频编码类型
	unsigned int videoFps;					//视频帧率
	unsigned int videoQueueSize;			//视频队列大小	如: 1024 * 1024

	unsigned int audioCodec;				//音频编码类型
	unsigned int audioSampleRate;			//音频采样率
	unsigned int audioChannel;				//音频通道数
	unsigned int audioBitsPerSample;		//音频采样精度
	unsigned int audioQueueSize;			//音频队列大小	如: 1024 * 128

	unsigned int metadataCodec;				//Metadata类型
	unsigned int metadataQueueSize;			//Metadata队列大小	如: 1024 * 512

	unsigned int vpsLength;				//视频vps帧长度
	unsigned int spsLength;				//视频sps帧长度
	unsigned int ppsLength;				//视频pps帧长度
	unsigned int seiLength;				//视频sei帧长度
	unsigned char	 vps[256];				//视频vps帧内容
	unsigned char	 sps[256];				//视频sps帧内容
	unsigned char	 pps[128];				//视频sps帧内容
	unsigned char	 sei[128];				//视频sei帧内容
}RTSP_MEDIA_INFO_T;


typedef enum __RTSP_PLAY_CTRL_CMD_ENUM
{
	RTSP_PLAY_CTRL_CMD_GET_DURATION	=	0x00000001,	//获取录像时长
	RTSP_PLAY_CTRL_CMD_PAUSE	=	0x00000002,		//暂停
	RTSP_PLAY_CTRL_CMD_RESUME,						//恢复
	RTSP_PLAY_CTRL_CMD_SCALE,						//调整倍率
	RTSP_PLAY_CTRL_CMD_SEEK_STREAM,					//跳转时间
	//RTSP_PLAY_CTRL_CMD_FRAME_BY_FRAME,			//逐帧
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


//自定义 Metadata 头信息
typedef struct __CUSTOM_META_DATA_HEADER_T
{
	int nsize;			/* size of structure		   */
	int customid;		/* custom module id			   */
	int moduleid;		/* module id - ordering number */
	int datalen;		/* size of whole data		   */

	char reserved[16];
}CUSTOM_META_DATA_HEADER_T;


#endif
