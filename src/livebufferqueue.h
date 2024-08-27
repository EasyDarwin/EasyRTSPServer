#ifndef __BUFFER_QUEUE_H__
#define __BUFFER_QUEUE_H__


#ifdef _WIN32
#include <winsock2.h>
#else
#include "sync_shm.h"
#include <pthread.h>
#define	SYNC_VID_SHM_KEY		2000
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "common.h"


//#define DEBUG_BUFFER_QUEUE			0x01			//是否打印调试信息

typedef enum __BUFFER_TYPE_ENUM
{
	BUFFER_TYPE_UNKNOWN		=		0x00000000,
	BUFFER_TYPE_VIDEO		=		0x00000001,
	BUFFER_TYPE_AUDIO,
	BUFFER_TYPE_EVENT,
	BUFFER_TYPE_TEXT
}BUFFER_TYPE_ENUM;




#define		LOCK_WAIT_TIMES		1000

#define BUFFER_QUEUE_FLAG	0x0FFFFFFF
typedef struct __BUFFER_HEADER_T
{
	unsigned int		size;			//自身包头长度
	unsigned int		id;
	BUFFER_TYPE_ENUM	type;
	unsigned int		flag;					//BUFFER_QUEUE_FLAG		用于检验包完整性
	unsigned int		timestamp_sec;
	int		headerSize;
	int		payloadSize;
}
#ifndef _WIN32
__attribute__ ((packed)) BUFFER_HEADER_T;
#else
BUFFER_HEADER_T;
#endif

typedef struct __BUFFER_NODE_T
{
	int				pos;
	unsigned int	timestamp_sec;
}
#ifndef _WIN32
__attribute__ ((packed)) BUFFER_NODE_T;
#else
BUFFER_NODE_T;
#endif

typedef struct __QUEUE_HEADER_T
{
	int				flag;							//初始化标识
	int				queueSize;				//整个缓冲队列大小
	int				writePos;				//写位置
	int				totalSize;				//当前总字节数
	int				buffNodeNum;			//BufferNode个数
	int				nodeIndex;				//当前写列表的下标值
	int				updateNodeList;			//是否需要更新NodeList
	int				firstNodeIndex;			//第一个NodeList下标值, 表示最早的一条记录
	int				videoFrameNum;
	int				audioFrameNum;
	int				eventFrameNum;
	int				textFrameNum;
}QUEUE_HEADER_T;


#define		MAX_CONSUMER_NUM	5
typedef struct __QUEUE_CONSUMER_T
{
	unsigned long	consumerId;
	int				readPos;
	int				totalSize;

	int				videoFrameNum;
	int				audioFrameNum;
	int				eventFrameNum;
	int				textFrameNum;
}QUEUE_CONSUMER_T;

typedef struct __BUFFER_QUEUE_OBJ_T
{
	int				id;
	char			name[36];

	unsigned int	addVideoDataTime;
	unsigned int	addAudioDataTime;

#ifdef _WIN32
	HANDLE			hQueueHeader;
	HANDLE			hQueueData;
	HANDLE			hNodeList;
	HANDLE			hConsumerList;

	HANDLE			hMutex;
#else
	key_t			shmkey;
	int				shmHdrid;
	int				shmDatid;
	int				shmFrameListId;
	pthread_mutex_t hMutex;
#endif
	QUEUE_HEADER_T	*pQueHeader;
	char			*pQueData;
	BUFFER_NODE_T	*pNodeInfoList;
	QUEUE_CONSUMER_T	*pConsumer;
}BUFFER_QUEUE_OBJ_T;


typedef void *BUFFQUEUE_HANDLE;			//队列句柄
typedef void *CONSUMER_HANDLE;			//调用者句柄, 由BUFQUE_RegisterConsumer创建

#if defined (__cplusplus)
extern "C"
{
#endif

	

	/*创建缓冲队列. 注意queueSize和record_secs的值, 
                    如果queueSize太小,而record_secs过大,即在指定的时间(record_secs)内,
                    该queueSize不足以容纳该数量的记录,则会在内部强制同步为最新的, 后果就是取历史数据失败 
					record_secs建议最小为1,不要设置为0, 这样在GetData不及时的情况下(即数据被覆盖),还可以自动切换为最新的数据位置，如果设置为0则不缓存位置

					注: 如果isShareMemory为1, createShareMemory为0, 在该共享内存名称不存在时,则返回-1000
					*/
	int		BUFQUE_Create(BUFFQUEUE_HANDLE *handle, unsigned int queueId, char *queueName, int queueSize, int record_secs, 
													unsigned char isShareMemory, unsigned char createShareMemory);
	int		BUFQUE_Release(BUFFQUEUE_HANDLE *handle);

	//注册消费者ID,表示该id将要读取队列中的值
	CONSUMER_HANDLE	BUFQUE_RegisterConsumer(BUFFQUEUE_HANDLE handle, unsigned long consumerId, unsigned char getHistoryData/*0x01: 获取历史数据   0x00:获取最新数据*/);
	void	BUFQUE_UnRegisterConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE *consumerHandle);

	//添加数据
	int		BUFQUE_AddData(BUFFQUEUE_HANDLE handle, unsigned int bufferId, BUFFER_TYPE_ENUM bufferType, 
													int headerSize, char *headerData, int payloadSize, char *payloadData);
	//取出数据
	int		BUFQUE_GetData(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, unsigned int *bufferId, BUFFER_TYPE_ENUM *bufferType, 
													int *headerSize, char *headerData, int *payloadSize, char *payloadData, unsigned char moveData);

	//获取帧数
	int		BUFQUE_GetFrameNum(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, BUFFER_TYPE_ENUM bufferType);

	//获取该消费者当前总字节数
	int		BUFQUE_GetTotalBytes(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle);

	//获取缓冲区大小
	int		BUFQUE_GetQueueSize(BUFFQUEUE_HANDLE handle);

	//拷贝消费者信息(用于即时回放)
	int		BUFQUE_CopyConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE handleDst, CONSUMER_HANDLE handleSrc);

	unsigned int GetAddVideoDataTime(BUFFQUEUE_HANDLE handle);
	unsigned int GetAddAudioDataTime(BUFFQUEUE_HANDLE handle);

	//===========================================================================================
	//===========================以下部分为私有函数==============================================
	//===========================================================================================
	//private
	int		BUFQUE_Lock(BUFFER_QUEUE_OBJ_T *pObj);
	int		BUFQUE_Unlock(BUFFER_QUEUE_OBJ_T *pObj);

	//更新读端位置为当前最新写数据位置
	int		BUFQUE_UpdateReadPos(BUFFER_QUEUE_OBJ_T *pObj, CONSUMER_HANDLE handle);
	int		BUFQUE_CheckBufferHeader(BUFFER_HEADER_T *pBufferHeader);
	int		BUFQUE_Debug(BUFFER_QUEUE_OBJ_T *pObj);

	int		BUFQUE_TRACE(char* szFormat, ...);
#if defined (__cplusplus)
}
#endif











#endif
