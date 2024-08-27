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


//#define DEBUG_BUFFER_QUEUE			0x01			//�Ƿ��ӡ������Ϣ

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
	unsigned int		size;			//�����ͷ����
	unsigned int		id;
	BUFFER_TYPE_ENUM	type;
	unsigned int		flag;					//BUFFER_QUEUE_FLAG		���ڼ����������
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
	int				flag;							//��ʼ����ʶ
	int				queueSize;				//����������д�С
	int				writePos;				//дλ��
	int				totalSize;				//��ǰ���ֽ���
	int				buffNodeNum;			//BufferNode����
	int				nodeIndex;				//��ǰд�б���±�ֵ
	int				updateNodeList;			//�Ƿ���Ҫ����NodeList
	int				firstNodeIndex;			//��һ��NodeList�±�ֵ, ��ʾ�����һ����¼
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


typedef void *BUFFQUEUE_HANDLE;			//���о��
typedef void *CONSUMER_HANDLE;			//�����߾��, ��BUFQUE_RegisterConsumer����

#if defined (__cplusplus)
extern "C"
{
#endif

	

	/*�����������. ע��queueSize��record_secs��ֵ, 
                    ���queueSize̫С,��record_secs����,����ָ����ʱ��(record_secs)��,
                    ��queueSize���������ɸ������ļ�¼,������ڲ�ǿ��ͬ��Ϊ���µ�, �������ȡ��ʷ����ʧ�� 
					record_secs������СΪ1,��Ҫ����Ϊ0, ������GetData����ʱ�������(�����ݱ�����),�������Զ��л�Ϊ���µ�����λ�ã��������Ϊ0�򲻻���λ��

					ע: ���isShareMemoryΪ1, createShareMemoryΪ0, �ڸù����ڴ����Ʋ�����ʱ,�򷵻�-1000
					*/
	int		BUFQUE_Create(BUFFQUEUE_HANDLE *handle, unsigned int queueId, char *queueName, int queueSize, int record_secs, 
													unsigned char isShareMemory, unsigned char createShareMemory);
	int		BUFQUE_Release(BUFFQUEUE_HANDLE *handle);

	//ע��������ID,��ʾ��id��Ҫ��ȡ�����е�ֵ
	CONSUMER_HANDLE	BUFQUE_RegisterConsumer(BUFFQUEUE_HANDLE handle, unsigned long consumerId, unsigned char getHistoryData/*0x01: ��ȡ��ʷ����   0x00:��ȡ��������*/);
	void	BUFQUE_UnRegisterConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE *consumerHandle);

	//�������
	int		BUFQUE_AddData(BUFFQUEUE_HANDLE handle, unsigned int bufferId, BUFFER_TYPE_ENUM bufferType, 
													int headerSize, char *headerData, int payloadSize, char *payloadData);
	//ȡ������
	int		BUFQUE_GetData(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, unsigned int *bufferId, BUFFER_TYPE_ENUM *bufferType, 
													int *headerSize, char *headerData, int *payloadSize, char *payloadData, unsigned char moveData);

	//��ȡ֡��
	int		BUFQUE_GetFrameNum(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, BUFFER_TYPE_ENUM bufferType);

	//��ȡ�������ߵ�ǰ���ֽ���
	int		BUFQUE_GetTotalBytes(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle);

	//��ȡ��������С
	int		BUFQUE_GetQueueSize(BUFFQUEUE_HANDLE handle);

	//������������Ϣ(���ڼ�ʱ�ط�)
	int		BUFQUE_CopyConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE handleDst, CONSUMER_HANDLE handleSrc);

	unsigned int GetAddVideoDataTime(BUFFQUEUE_HANDLE handle);
	unsigned int GetAddAudioDataTime(BUFFQUEUE_HANDLE handle);

	//===========================================================================================
	//===========================���²���Ϊ˽�к���==============================================
	//===========================================================================================
	//private
	int		BUFQUE_Lock(BUFFER_QUEUE_OBJ_T *pObj);
	int		BUFQUE_Unlock(BUFFER_QUEUE_OBJ_T *pObj);

	//���¶���λ��Ϊ��ǰ����д����λ��
	int		BUFQUE_UpdateReadPos(BUFFER_QUEUE_OBJ_T *pObj, CONSUMER_HANDLE handle);
	int		BUFQUE_CheckBufferHeader(BUFFER_HEADER_T *pBufferHeader);
	int		BUFQUE_Debug(BUFFER_QUEUE_OBJ_T *pObj);

	int		BUFQUE_TRACE(char* szFormat, ...);
#if defined (__cplusplus)
}
#endif











#endif
