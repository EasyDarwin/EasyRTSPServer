#include "livebufferqueue.h"
#include <time.h>
//#include "trace.h"

int		BUFQUE_Create(BUFFQUEUE_HANDLE *handle, unsigned int queueId, char *queueName, int queueSize, int record_secs, unsigned char isShareMemory, unsigned char createShareMemory)
{
	int		bufQueCreateRet = -1;
	BUFFER_QUEUE_OBJ_T *pObj = NULL;


	int nFramelistNum = 0;
	int nFrameQueSize = record_secs * 60;
#ifdef _WIN32
	char szHeaderName[36] = {0};
	char szNodeListName[36] = {0};
	char szDataName[36] = {0};
	char szConsumerName[36] = {0};
	char szMutexName[36] = {0};
	if ( (isShareMemory==0x01) && (NULL==queueName || ((int)strlen(queueName) < 1)) )	return -1;
#endif

	//queueSize不能小于两个BUFFER_HEADER_T
	if (queueSize < sizeof(BUFFER_HEADER_T)*2)										return -1;

	if (record_secs > 0)
	{
		nFramelistNum = record_secs * 60;//30;	//每秒30帧
		nFrameQueSize = nFramelistNum* (int)sizeof(BUFFER_NODE_T);
	}

	pObj = (BUFFER_QUEUE_OBJ_T *)malloc(sizeof(BUFFER_QUEUE_OBJ_T));
	if (NULL==pObj)											return -1;

	memset(pObj, 0x00, sizeof(BUFFER_QUEUE_OBJ_T));
	pObj->id = queueId;
	if (NULL != queueName && ((int)strlen(queueName)>0))	strcpy(pObj->name, queueName);

	//创建互斥量, 在进行数据量的加减时使用
#ifdef _WIN32
	sprintf(szMutexName, "%s%d_mutex", queueName, queueId);
	pObj->hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, szMutexName);
	if (NULL == pObj->hMutex)
	{
		pObj->hMutex = CreateMutex(NULL, FALSE, szMutexName);
		if (NULL == pObj->hMutex)
		{
			memset(pObj, 0x00, sizeof(BUFFER_QUEUE_OBJ_T));
			free(pObj);
			return -1;
		}
	}
#else
	pthread_mutex_init (&pObj->hMutex,NULL);
#endif
	//Create Header map

	if (isShareMemory == 0x01)		//共享内存
	{
#ifdef _WIN32
		do
		{
			sprintf(szHeaderName, TEXT("%s%d_h"), queueName, queueId);
			sprintf(szNodeListName, TEXT("%s%d_f"), queueName, queueId);
			sprintf(szConsumerName, TEXT("%s_%d_c"), queueName, queueId);
			sprintf(szDataName, TEXT("%s%d_b"), queueName, queueId);

			//创建queueHeader
			pObj->hQueueHeader = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szHeaderName);
			if (NULL==pObj->hQueueHeader && createShareMemory==0x01)
			{
				pObj->hQueueHeader = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT, 0, sizeof(QUEUE_HEADER_T), szHeaderName);
				if (NULL==pObj->hQueueHeader || pObj->hQueueHeader==INVALID_HANDLE_VALUE)
				{
					memset(pObj, 0x00, sizeof(BUFFER_QUEUE_OBJ_T));
					break;
				}
			}
			pObj->pQueHeader = (QUEUE_HEADER_T*)MapViewOfFile(pObj->hQueueHeader, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
			if (createShareMemory==0x01)
			{
				if (pObj->pQueHeader->queueSize < 1)
				{
					memset(pObj->pQueHeader, 0x00, sizeof(QUEUE_HEADER_T));
					pObj->pQueHeader->queueSize = queueSize;
				}
			}
			else if (NULL==pObj->pQueHeader)
			{
				bufQueCreateRet = -1000;
				break;
			}
			else
			{
				queueSize = pObj->pQueHeader->queueSize;
 			}

			//创建NodeList
			pObj->hNodeList = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szNodeListName);
			if (NULL==pObj->hNodeList && createShareMemory==0x01)
			{
				pObj->hNodeList = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT, 0, nFrameQueSize, szNodeListName);
				if (NULL==pObj->hNodeList || pObj->hNodeList==INVALID_HANDLE_VALUE)
				{
					break;
				}
			}
			pObj->pNodeInfoList = (BUFFER_NODE_T*)MapViewOfFile(pObj->hNodeList, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);
			if (createShareMemory==0x01)
			{
				memset(pObj->pNodeInfoList, 0x00, nFrameQueSize);
				pObj->pQueHeader->buffNodeNum = nFramelistNum;
			}
			else if (NULL==pObj->pNodeInfoList)
			{
				break;
			}

			//创建QueueData
			pObj->hQueueData	= OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szDataName);
			if (NULL==pObj->hQueueData && createShareMemory==0x01)
			{
				pObj->hQueueData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT, 0, queueSize, szDataName);
			}
			if (NULL == pObj->hQueueData || pObj->hQueueData==INVALID_HANDLE_VALUE)
			{
				break;
			}
			pObj->pQueData = (char*)MapViewOfFile(pObj->hQueueData, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);

			//创建Consumer
			pObj->hConsumerList	= OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szConsumerName);
			if (NULL == pObj->hConsumerList && createShareMemory==0x01)
			{
				pObj->hConsumerList = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE|SEC_COMMIT, 0, sizeof(QUEUE_CONSUMER_T) * MAX_CONSUMER_NUM, szConsumerName);
			}
			if (NULL == pObj->hConsumerList || pObj->hConsumerList==INVALID_HANDLE_VALUE)
			{
				break;
			}
			pObj->pConsumer = (QUEUE_CONSUMER_T*)MapViewOfFile(pObj->hConsumerList, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0);



			if (createShareMemory == 0x01)		//创建者首先清空QueueData
			{
				//memset(pQueHeader, 0x00, sizeof(QUEUE_HEADER_T));		//不能memset queueHeader
				memset(pObj->pQueData, 0x00, queueSize);
			}

			bufQueCreateRet = 0;
			pObj->pQueHeader->flag = 0xFFFF;			//初始化完成
		}while (0);
#else
		do
		{
			//Create Queue Header
			int ret = shm_create((SYNC_VID_SHM_KEY<<22)|queueId, &pObj->shmHdrid, sizeof(QUEUE_HEADER_T), (char**)&pObj->pQueHeader);
			if (ret < 0)
			{
				return -1;
			}
			else if (NULL==pObj->pQueHeader)
			{
				bufQueCreateRet = -1000;
				break;
			}

			//_TRACE(TRACE_LOG_DEBUG, "[%d]pQueHeader: %d\n", (SYNC_VID_SHM_KEY<<22)|queueId, pObj->shmHdrid);

			//Create Queue Data
			ret = shm_create((SYNC_VID_SHM_KEY<<23)|queueId, &pObj->shmDatid, queueSize, (char**)&pObj->pQueData);
			if (ret < 0)
			{
				shm_delete(&pObj->shmHdrid, (char*)pObj->pQueHeader);
				return -1;
			}
			pObj->pQueHeader->queueSize = queueSize;
			//_TRACE(TRACE_LOG_DEBUG, "[%d]pQueData: %d\n", (SYNC_VID_SHM_KEY<<23)|queueId, pObj->shmDatid);

			//Create Node List
			ret = shm_create( (SYNC_VID_SHM_KEY << 24)|queueId, &pObj->shmFrameListId, nFrameQueSize, (char **)&pObj->pNodeInfoList);
			if (ret < 0)
			{
				shm_delete(&pObj->shmDatid, (char*)pObj->pQueData);
				shm_delete(&pObj->shmHdrid, (char*)pObj->pQueHeader);
				return -1;
			}
			if (createShareMemory==0x01)
			{
				memset(pObj->pNodeInfoList, 0x00, nFrameQueSize);
				pObj->pQueHeader->buffNodeNum = nFramelistNum;
			}

			bufQueCreateRet = 0;
			pObj->pQueHeader->flag = 0xFFFF;			//初始化完成
		}while (0);
#endif
		if (bufQueCreateRet < 0)
		{
			BUFQUE_Release( (BUFFQUEUE_HANDLE*)&pObj);

			return bufQueCreateRet;
		}
	}
	else		//非共享内存
	{
		do
		{
			//创建 QueueHeader
			pObj->pQueHeader = (QUEUE_HEADER_T *) malloc(sizeof(QUEUE_HEADER_T));
			if (NULL == pObj->pQueHeader)			break;
			memset(pObj->pQueHeader, 0x00, sizeof(QUEUE_HEADER_T));

			//创建 Node List
			if (nFramelistNum > 0)
			{
				pObj->pNodeInfoList = (BUFFER_NODE_T *) malloc(sizeof(BUFFER_NODE_T) * nFramelistNum);
				if (NULL == pObj->pNodeInfoList)		break;
				memset(&pObj->pNodeInfoList[0], 0x00, sizeof(BUFFER_NODE_T)*nFramelistNum);
				pObj->pQueHeader->buffNodeNum = nFramelistNum;
			}

			//创建 QueueData
			pObj->pQueData = (char *)malloc(queueSize);
			if (NULL == pObj->pQueData)				break;
			pObj->pQueHeader->queueSize = queueSize;

			//调用者列表
			pObj->pConsumer = (QUEUE_CONSUMER_T *)malloc(sizeof(QUEUE_CONSUMER_T) * MAX_CONSUMER_NUM);
			if (NULL==pObj->pConsumer)				break;
			memset(pObj->pConsumer, 0x00, sizeof(QUEUE_CONSUMER_T) * MAX_CONSUMER_NUM);

			bufQueCreateRet = 0;
			pObj->pQueHeader->flag = 0xFFFF;			//初始化完成
		}while (0);

		if (bufQueCreateRet < 0)
		{
			BUFQUE_Release( (BUFFQUEUE_HANDLE*)&pObj);

			return bufQueCreateRet;
		}
	}

	*handle = pObj;

	return bufQueCreateRet;
}

int		BUFQUE_Release(BUFFQUEUE_HANDLE *handle)
{
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)*handle;
	if (NULL == pObj)		return -1;

#ifdef _WIN32

	if (NULL != pObj->hMutex)
	{
		CloseHandle(pObj->hMutex);
		pObj->hMutex = NULL;
	}

	if (NULL != pObj->hQueueHeader)
	{
		if (! UnmapViewOfFile(pObj->pQueHeader))
		{
		}
		pObj->pQueHeader = NULL;
		CloseHandle(pObj->hQueueHeader);
		pObj->hQueueHeader = NULL;
	}
	if (NULL != pObj->pQueHeader)
	{
		free(pObj->pQueHeader);
		pObj->pQueHeader = NULL;
	}

	if (NULL != pObj->hConsumerList)
	{
		if (! UnmapViewOfFile(pObj->pConsumer))
		{
		}
		CloseHandle(pObj->hConsumerList);
		pObj->pConsumer = NULL;
	}
	if (NULL != pObj->pConsumer)
	{
		free(pObj->pConsumer);
		pObj->pConsumer = NULL;
	}

	if (NULL != pObj->hQueueData)
	{
		if (! UnmapViewOfFile(pObj->pQueData))
		{
		}
		pObj->pQueData = NULL;
		CloseHandle(pObj->hQueueData);
		pObj->hQueueData = NULL;
	}
	if (NULL != pObj->pQueData)
	{
		free(pObj->pQueData);
		pObj->pQueData = NULL;
	}
	if (NULL != pObj->hNodeList)
	{
		if (! UnmapViewOfFile(pObj->pNodeInfoList))
		{
		}
		pObj->pNodeInfoList = NULL;
		CloseHandle(pObj->hNodeList);
		pObj->hNodeList = NULL;
	}
	if (NULL != pObj->pNodeInfoList)
	{
		free(pObj->pNodeInfoList);
		pObj->pNodeInfoList = NULL;
	}

#else
	//if (NULL != pObj->hMutex)
	{
		
		//pObj->hMutex = NULL;
	}

	if (pObj->shmHdrid>0 && pObj->pQueHeader!=NULL)
	{
		shm_delete(&pObj->shmHdrid, (char*)pObj->pQueHeader);
		pObj->pQueHeader = NULL;
	}
	if (pObj->shmDatid>0 && pObj->pQueData!=NULL)
	{
		shm_delete(&pObj->shmDatid, (char*)pObj->pQueData);
		pObj->pQueData = NULL;
	}
	if (pObj->shmFrameListId>0 && pObj->pNodeInfoList!=NULL)
	{
		shm_delete(&pObj->shmFrameListId, (char *)pObj->pNodeInfoList);
		pObj->pNodeInfoList = NULL;
	}

	if (NULL != pObj->pQueHeader)
	{
		free(pObj->pQueHeader);
		pObj->pQueHeader = NULL;
	}
	if (NULL != pObj->pQueData)
	{
		free(pObj->pQueData);
		pObj->pQueData = NULL;
	}

	//if (NULL != pObj->hMutex)
	{
		pthread_mutex_destroy(&pObj->hMutex);
		//pObj->hMutex = NULL;
	}
	if (NULL != pObj->pNodeInfoList)
	{
		free(pObj->pNodeInfoList);
		pObj->pNodeInfoList = NULL;
	}

#endif


	free(pObj);
	pObj = NULL;
	*handle = NULL;

	return 0;
}



int		BUFQUE_Lock(BUFFER_QUEUE_OBJ_T *pObj)
{
	if (NULL == pObj)		return -1;

#ifdef _WIN32
	WaitForSingleObject(pObj->hMutex, INFINITE);		//Lock
#else
	pthread_mutex_lock(&pObj->hMutex);
#endif

	return 0;
}
int		BUFQUE_Unlock(BUFFER_QUEUE_OBJ_T *pObj)
{
	if (NULL == pObj)		return -1;

#ifdef _WIN32
	ReleaseMutex(pObj->hMutex);
#else
	pthread_mutex_unlock(&pObj->hMutex);
#endif
	return 0;
}

unsigned int GetAddVideoDataTime(BUFFQUEUE_HANDLE handle)
{
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL == pObj)		return 0;
	if (NULL == pObj->pQueHeader)		return 0;

	return pObj->addVideoDataTime;
}

unsigned int GetAddAudioDataTime(BUFFQUEUE_HANDLE handle)
{
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL == pObj)		return 0;
	if (NULL == pObj->pQueHeader)		return 0;

	return pObj->addAudioDataTime;
}

int		BUFQUE_AddData(BUFFQUEUE_HANDLE handle, unsigned int bufferId, BUFFER_TYPE_ENUM bufferType, int headerSize, char *headerData, int payloadSize, char *payloadData)
{
	int ret = 0;
	BUFFER_HEADER_T		bufferHeader;
	BUFFER_NODE_T		bufferNode;

	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL==pObj || NULL==headerData)			return -1;
	if (NULL == pObj->pQueData)					return -1;
	if (NULL == pObj->pQueHeader)				return -1;
	if (headerSize < 1 || payloadSize<0)
	{
		//_TRACE(TRACE_LOG_DEBUG, "输入参数headerSize长度小于1: %d\n", headerSize);
		return -1;
	}

	if (pObj->pQueHeader->flag != 0xFFFF)			return -1;		//尚未初始化完成

	if (BUFFER_TYPE_VIDEO == bufferType)
	{
		pObj->addVideoDataTime = (unsigned int)time(NULL);		//2020.04.23
	}
	else if (BUFFER_TYPE_AUDIO == bufferType)
	{
		pObj->addAudioDataTime = (unsigned int)time(NULL);		//2020.04.23
	}

	if (headerSize + payloadSize > pObj->pQueHeader->queueSize)
	{
		//_TRACE(TRACE_LOG_WARNING, "Buffer too low.. Current Frame Size: %d\tBuffer Size: %d\n", headerSize+payloadSize, pObj->pQueHeader->queueSize);
		return -1;
	}

#ifdef DEBUG_BUFFER_QUEUE
	//_TRACE(TRACE_LOG_DEBUG, "writePos: %d  framesize:%d\n", pObj->pQueHeader->writePos, headerSize+payloadSize+(int)sizeof(BUFFER_HEADER_T));
#endif


	memset(&bufferHeader, 0x00, sizeof(BUFFER_HEADER_T));
	bufferHeader.size = sizeof(BUFFER_HEADER_T);
	bufferHeader.id = bufferId;
	bufferHeader.type = bufferType;
	bufferHeader.flag = BUFFER_QUEUE_FLAG;
	bufferHeader.timestamp_sec = (unsigned int)time(NULL);
	bufferHeader.headerSize = headerSize;
	bufferHeader.payloadSize = payloadSize;

	////_TRACE(TRACE_LOG_DEBUG, "WritePos: %d    totalSize: %d\n", pQueHeader->writePos, pQueHeader->totalsize);

	memset(&bufferNode, 0x00, sizeof(BUFFER_NODE_T));
	bufferNode.pos = pObj->pQueHeader->writePos;
	bufferNode.timestamp_sec = bufferHeader.timestamp_sec;

	/*
	//_TRACE(TRACE_LOG_DEBUG, (char *)"pObj->pQueHeader->writePos: %d  bufNodeSize[%d]  headerSize[%d] payloadSize[%d]    packetSize[%d]  videoFrame[%d]  audioFrame[%d]\n", 
		pObj->pQueHeader->writePos, (int)sizeof(BUFFER_HEADER_T), headerSize, payloadSize,
		(int)sizeof(BUFFER_HEADER_T) + headerSize + payloadSize, pObj->pQueHeader->videoFrameNum, pObj->pQueHeader->audioFrameNum);
		*/

	//Lock();
	//从头到尾 写
	if ((pObj->pQueHeader->writePos + (int)sizeof(BUFFER_HEADER_T) + headerSize + payloadSize) <= pObj->pQueHeader->queueSize)
	{
		//copy to queue
		int nAdd = 0;

		//if (pObj->pQueHeader->firstframe_timestamp == 0x00)	pObj->pQueHeader->firstframe_timestamp = frameinfo->timestamp;	//记录第一帧的时间,用于得出预录时间

		//记录帧位置
		//if (bufferType==BUFFER_TYPE_VIDEO)	SSQ_AddFrameInfo(pObj, pObj->pQueHeader->writePos, frameinfo);

		nAdd = pObj->pQueHeader->writePos;
		memcpy(pObj->pQueData+nAdd, &bufferHeader, sizeof(BUFFER_HEADER_T));
		nAdd += (int)sizeof(BUFFER_HEADER_T);

		memcpy(pObj->pQueData+nAdd, headerData, headerSize);
		nAdd += headerSize;

		if (NULL != payloadData && payloadSize>0)
		{
			memcpy(pObj->pQueData+nAdd, payloadData, payloadSize);
			nAdd += payloadSize;
		}

		pObj->pQueHeader->writePos = nAdd;



		//pObj->pQueHeader->totalsize+= (sizeof(BUFFER_HEADER_T) + headerSize + payloadSize);

		////_TRACE(TRACE_LOG_DEBUG, "add data..  framesize: %d writePos: %d   /   %d\n", frameinfo->length, pObj->pQueHeader->writePos, pObj->pQueHeader->bufsize);
	}
	//else if (pQueHeader->size - pQueHeader->writePos+pQueHeader->readpos >= (int)(frameinfo->length+sizeof(BUFFER_HEADER_T)))	//从尾写到头
	//else if (pObj->pQueHeader->bufsize - pObj->pQueHeader->writePos+pObj->pQueHeader->readpos >= (int)(frameinfo->length+sizeof(BUFFER_HEADER_T)))	//从尾写到头
	else// if (pObj->pQueHeader->bufsize - pObj->pQueHeader->writePos >= (int)(frameinfo->length+sizeof(BUFFER_HEADER_T)))	//从尾写到头
	{
		int remain = pObj->pQueHeader->queueSize - pObj->pQueHeader->writePos;	//剩余空间
		if (remain >= (int)sizeof(BUFFER_HEADER_T) + headerSize)		//剩余空间可以容纳 sizeof(BUFFER_HEADER_T) 和 headerSize
		{
			int nAdd = pObj->pQueHeader->writePos;

			////_TRACE(TRACE_LOG_INFO, (char *)"1. 剩余空间可以容纳 sizeof(BUFFER_HEADER_T) 和 headerSize\n");

			memcpy(pObj->pQueData+nAdd, &bufferHeader, sizeof(BUFFER_HEADER_T));
			nAdd += sizeof(BUFFER_HEADER_T);
			memcpy(pObj->pQueData+nAdd, headerData, headerSize);
			nAdd += headerSize;
			remain = pObj->pQueHeader->queueSize - nAdd;
			if (payloadSize > 0)
			{
				if (remain > 0)
				{
					memcpy(pObj->pQueData+nAdd, payloadData, remain);						//复制到队尾
					memcpy(pObj->pQueData, payloadData+remain, payloadSize - remain);		//复制到队头
					nAdd = payloadSize - remain;

					pObj->pQueHeader->writePos = nAdd;
				}
				else if (remain == 0)
				{
					memcpy(pObj->pQueData, payloadData, payloadSize);		//复制到队头
					pObj->pQueHeader->writePos = payloadSize;
				}
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char*)"偏移位置错误: %d..\n", remain);
				}
			}
			else
			{
				//_TRACE(TRACE_LOG_ERROR, "判断错误1111..\n");
			}
		}
		else if (remain >= (int)sizeof(BUFFER_HEADER_T))		//剩余空间仅可以容纳 sizeof(BUFFER_HEADER_T)
		{
			int nAdd = pObj->pQueHeader->writePos;
#if 0
			//_TRACE(TRACE_LOG_INFO, (char *)"2. 剩余空间仅可以容纳 sizeof(BUFFER_HEADER_T)\n");
#endif
			memcpy(pObj->pQueData+nAdd, &bufferHeader, sizeof(BUFFER_HEADER_T));
			nAdd += (int)sizeof(BUFFER_HEADER_T);
			remain = pObj->pQueHeader->queueSize - nAdd;
			if (remain > 0)
			{
				memcpy(pObj->pQueData+nAdd, headerData, remain);						//复制到队尾
				memcpy(pObj->pQueData, headerData+remain, headerSize-remain);			//复制到队头
				nAdd = headerSize - remain;
				if (payloadSize > 0)
				{
					memcpy(pObj->pQueData+nAdd, payloadData, payloadSize);
					nAdd += payloadSize;
				}
				pObj->pQueHeader->writePos = nAdd;
			}
			else if (remain == 0)
			{
				memcpy(pObj->pQueData, headerData, headerSize);						//复制到队头
				nAdd = headerSize;
				if (payloadSize > 0)
				{
					memcpy(pObj->pQueData+nAdd, payloadData, payloadSize);
					nAdd += payloadSize;
				}
				pObj->pQueHeader->writePos = nAdd;
			}
			else
			{
				//_TRACE(TRACE_LOG_ERROR, "判断错误2222..\n");
			}
		}
		else		//剩余空间不能容纳 sizeof(BUFFER_HEADER_T)
		{
			char *tmpbuf = (char *)&bufferHeader;
			int nAdd = pObj->pQueHeader->writePos;
#if 0
			//_TRACE(TRACE_LOG_INFO, (char *)"3. 剩余空间不能容纳 sizeof(BUFFER_HEADER_T)  headersize+payloadsize: %d\n", (int)sizeof(BUFFER_HEADER_T) + bufferHeader.headerSize + bufferHeader.payloadSize);
#endif
			memcpy(pObj->pQueData+nAdd, tmpbuf, remain);
			memcpy(pObj->pQueData, tmpbuf+remain, (int)sizeof(BUFFER_HEADER_T)-remain);							//复制到队头
			nAdd = (int)sizeof(BUFFER_HEADER_T)-remain;

			memcpy(pObj->pQueData+nAdd, headerData, headerSize);			//复制headerData
			nAdd += headerSize;
			if (payloadSize>0)
			{
				memcpy(pObj->pQueData+nAdd, payloadData, payloadSize);
				nAdd += payloadSize;
			}
			pObj->pQueHeader->writePos = nAdd;
		}

		pObj->pQueHeader->updateNodeList = 0x01;
	}


	if (bufferType==BUFFER_TYPE_VIDEO)				pObj->pQueHeader->videoFrameNum ++;
	else if (bufferType==BUFFER_TYPE_AUDIO)			pObj->pQueHeader->audioFrameNum ++;
	else if (bufferType==BUFFER_TYPE_EVENT)			pObj->pQueHeader->eventFrameNum ++;
	else if (bufferType==BUFFER_TYPE_TEXT)			pObj->pQueHeader->textFrameNum ++;

	if (NULL != pObj->pNodeInfoList)
	{
		int nodeIndex = pObj->pQueHeader->nodeIndex;

		if (nodeIndex > 0 && pObj->pQueHeader->firstNodeIndex>1)
		{
			//pObj->pQueHeader->firstNodeIndex = nodeIndex-1;
		}


		memcpy(&pObj->pNodeInfoList[nodeIndex], &bufferNode, sizeof(BUFFER_NODE_T));
		pObj->pQueHeader->nodeIndex ++;
		if (pObj->pQueHeader->nodeIndex >= pObj->pQueHeader->buffNodeNum)
		{
			pObj->pQueHeader->nodeIndex = 0;
			pObj->pQueHeader->firstNodeIndex = 1;
		}


		//else if (pObj->pQueHeader->firstNodeIndex>0 && nodeIndex>0)
		if (pObj->pQueHeader->updateNodeList == 0x01)// && nodeIndex>0)
		{
			pObj->pQueHeader->firstNodeIndex = pObj->pQueHeader->nodeIndex;
			////_TRACE(TRACE_LOG_DEBUG, (char *)"更新firstNodeIndex: %d   当前nodeIndex: %d\n", pObj->pQueHeader->firstNodeIndex, nodeIndex);
		}


#if 0
		if (nodeIndex >= pObj->pQueHeader->buffNodeNum || pObj->pQueHeader->updateNodeList==0x01)
		{
			int idx = 0, found=0;
			for (idx=0; idx<nodeIndex; idx++)
			{
				BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[idx].pos);
				if (pBufferHeader->flag == BUFFER_QUEUE_FLAG)
				{
					found = 1;

					if (pBufferHeader->type==BUFFER_TYPE_VIDEO)				pObj->pQueHeader->videoFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_AUDIO)		pObj->pQueHeader->audioFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_EVENT)		pObj->pQueHeader->eventFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_TEXT)			pObj->pQueHeader->textFrameNum --;

					if (idx==0)	break;

					memmove(&pObj->pNodeInfoList[0], &pObj->pNodeInfoList[idx], sizeof(BUFFER_NODE_T) * (idx-1));

					pObj->pQueHeader->nodeIndex = nodeIndex-idx;

					
					break;
				}
			}

			if (found == 0x00)
			{
				//_TRACE(TRACE_LOG_ERROR, (char *)"NodeList中无符合条件的记录...   数据错误\n");
			}
			/*
			do
			{
				BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[0].pos);
				nodeIndex = pObj->pQueHeader->nodeIndex;
				if (pBufferHeader->flag == BUFFER_QUEUE_FLAG)
				{
					if (pBufferHeader->type==BUFFER_TYPE_VIDEO)				pObj->pQueHeader->videoFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_AUDIO)		pObj->pQueHeader->audioFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_EVENT)		pObj->pQueHeader->eventFrameNum --;
					else if (pBufferHeader->type==BUFFER_TYPE_TEXT)			pObj->pQueHeader->textFrameNum --;

					break;
				}
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"标志位错误.\n");		//因队头被最新数据覆盖, 所以此处第1帧的位置信息已错误, 需遍历NodeList,找到正确的位置
				}
				memmove(&pObj->pNodeInfoList[0], &pObj->pNodeInfoList[1], sizeof(BUFFER_NODE_T) * (nodeIndex-1));
				if (pObj->pQueHeader->nodeIndex > 0)	pObj->pQueHeader->nodeIndex --;
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"NodeList中无符合条件的记录...   数据错误\n");
					break;
				}
			}while (1);
			*/
		}
		nodeIndex = pObj->pQueHeader->nodeIndex;
		memcpy(&pObj->pNodeInfoList[nodeIndex], &bufferNode, sizeof(BUFFER_NODE_T));
		//pObj->pNodeInfoList[nodeIndex].pos = bufferNode.pos;
		//pObj->pNodeInfoList[nodeIndex].timestamp_sec = bufferNode.timestamp_sec;

		pObj->pQueHeader->nodeIndex ++;
#endif
	}

	//pObj->pQueHeader->totalSize += ((int)sizeof(BUFFER_HEADER_T) + headerSize + payloadSize);


	//只有在更新totalsize时才加锁
	BUFQUE_Lock(pObj);		//LOCK
	{
		//更新consumer的totalSize
		int i=0;
		for (i=0; i<MAX_CONSUMER_NUM; i++)
		{
			if (pObj->pConsumer[i].consumerId>0)
			{
#ifdef DEBUG_BUFFER_QUEUE
				int iOldTotalSize = pObj->pConsumer[i].totalSize;
#endif
				int iSize = ((int)sizeof(BUFFER_HEADER_T) + headerSize + payloadSize);
				pObj->pConsumer[i].totalSize += iSize;

				if (bufferType==BUFFER_TYPE_VIDEO)				pObj->pConsumer[i].videoFrameNum ++;
				else if (bufferType==BUFFER_TYPE_AUDIO)			pObj->pConsumer[i].audioFrameNum ++;
				else if (bufferType==BUFFER_TYPE_EVENT)			pObj->pConsumer[i].eventFrameNum ++;
				else if (bufferType==BUFFER_TYPE_TEXT)			pObj->pConsumer[i].textFrameNum ++;

#ifdef DEBUG_BUFFER_QUEUE
				//_TRACE(TRACE_LOG_DEBUG, "writePos: %d\tconsumerID[%d]  %d + %d = totalsize[%d]\n", pObj->pQueHeader->writePos, pObj->pConsumer[i].consumerId, iOldTotalSize, iSize, pObj->pConsumer[i].totalSize);
#endif
			}
		}
	}


	BUFQUE_Unlock(pObj);
	////_TRACE(TRACE_LOG_DEBUG, "writePos: %d\ttotalsize: %d   bufferId: %d\n", pObj->pQueHeader->writePos, pObj->pQueHeader->totalSize, bufferId);

#ifdef _DEBUG1
	if (bufferType==BUFFER_TYPE_VIDEO)
	{
		//_TRACE(TRACE_LOG_DEBUG, "==========================\n");
		for (int i=0; i<pObj->pQueHeader->maxframeno; i++)
		{
			//_TRACE(TRACE_LOG_DEBUG, "[%d] times: %d   pos: %d   %02X%02X%02X%02X%02X\n", i, pObj->pQueHeader->pFrameinfoList[i].timestamp, pObj->pQueHeader->pFrameinfoList[i].pos,
				(unsigned char)pObj->pQueData[pObj->pQueHeader->pFrameinfoList[i].pos+sizeof(BUFFER_HEADER_T)+0], (unsigned char)pObj->pQueData[pObj->pQueHeader->pFrameinfoList[i].pos+sizeof(BUFFER_HEADER_T)+1], (unsigned char)pObj->pQueData[pObj->pQueHeader->pFrameinfoList[i].pos+sizeof(BUFFER_HEADER_T)+2],
				(unsigned char)pObj->pQueData[pObj->pQueHeader->pFrameinfoList[i].pos+sizeof(BUFFER_HEADER_T)+3], (unsigned char)pObj->pQueData[pObj->pQueHeader->pFrameinfoList[i].pos+sizeof(BUFFER_HEADER_T)+4]);
		}
	}
#endif


	if (ret == 0x00)
	{
		//pObj->pQueHeader->lastframe_timestamp = frameinfo->timestamp;		//更新最后一帧时间

		//pObj->pQueHeader->max_framesize = max_t(pObj->pQueHeader->max_framesize, frameinfo->length);	//更新最大帧大小
	}

	return ret;
}


int		BUFQUE_CopyConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE handleDst, CONSUMER_HANDLE handleSrc)
{
	unsigned int	consumerId = 0;
	QUEUE_CONSUMER_T	*pConsumerSrc = NULL;
	QUEUE_CONSUMER_T	*pConsumerDst = NULL;

	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL == pObj)				return -1;

	if (NULL == handleDst || NULL == handleSrc)		return -1;

	

	pConsumerSrc = (QUEUE_CONSUMER_T *)handleSrc;
	pConsumerDst = (QUEUE_CONSUMER_T *)handleDst;

	consumerId = pConsumerDst->consumerId;
	memcpy(pConsumerDst, pConsumerSrc, sizeof(QUEUE_CONSUMER_T));
	pConsumerDst->consumerId = consumerId;

	return 0;
}

	//注册消费者ID,表示该id将要读取队列中的值
CONSUMER_HANDLE	BUFQUE_RegisterConsumer(BUFFQUEUE_HANDLE handle, unsigned long consumerId, unsigned char getHistoryData)
{
	int i=0;
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	CONSUMER_HANDLE	consumerHandle = NULL;
	if (NULL == pObj)				return NULL;

	//检查是否已存在
	for (i=0; i<MAX_CONSUMER_NUM; i++)
	{
		if (pObj->pConsumer[i].consumerId == consumerId)
		{
			consumerHandle = (CONSUMER_HANDLE)&pObj->pConsumer[i];
			break;
		}
	}

	//不存在, 则分配
	if (NULL == consumerHandle)
	{
		for (i=0; i<MAX_CONSUMER_NUM; i++)
		{
			if (pObj->pConsumer[i].consumerId < 1)
			{
				pObj->pConsumer[i].consumerId = consumerId;

				consumerHandle = (CONSUMER_HANDLE)&pObj->pConsumer[i];

				//BUFQUE_UpdateReadPos(pObj, consumerHandle);
				break;
			}
		}
	}

	if (NULL == consumerHandle)
	{
		//_TRACE(TRACE_LOG_INFO, (char *)"注册调用者[0x%X][%d]失败. 当前调用者队列已满\n", consumerId, consumerId);
		return NULL;
	}


	if (getHistoryData == 0x01)		//如果为获取历史数据
	{
		int initPos = -1;//pObj->pNodeInfoList[pObj->pQueHeader->firstNodeIndex].pos;
		int nodeIndex=0, totalSize=0;

		if (pObj->pQueHeader->updateNodeList == 0x00)
		{
			for (nodeIndex=0; nodeIndex<pObj->pQueHeader->nodeIndex; nodeIndex++)
			{
				BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[nodeIndex].pos);
				if (BUFQUE_CheckBufferHeader(pBufferHeader) == 0x00)
				{
					if (initPos == -1)	initPos = pObj->pNodeInfoList[nodeIndex].pos;
					totalSize += ((int)sizeof(BUFFER_HEADER_T) + pBufferHeader->headerSize + pBufferHeader->payloadSize);
				}
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[0x%X]  遍历NodeList数据错误.. 可能数据已被覆盖\n", consumerId);
					//break;
				}
			}
		}
		else
		{
			for (nodeIndex=pObj->pQueHeader->firstNodeIndex; nodeIndex<pObj->pQueHeader->buffNodeNum; nodeIndex++)
			{
				BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[nodeIndex].pos);
				if (BUFQUE_CheckBufferHeader(pBufferHeader) == 0x00)
				{
					if (initPos == -1)	initPos = pObj->pNodeInfoList[nodeIndex].pos;
					totalSize += ((int)sizeof(BUFFER_HEADER_T) + pBufferHeader->headerSize + pBufferHeader->payloadSize);
				}
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[0x%X]  遍历NodeList数据错误.. 可能数据已被覆盖\n", consumerId);
					//break;
				}
			}
			for (nodeIndex=0; nodeIndex<pObj->pQueHeader->firstNodeIndex; nodeIndex++)
			{
				BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[nodeIndex].pos);
				if (BUFQUE_CheckBufferHeader(pBufferHeader) == 0x00)
				{
					if (initPos == -1)	initPos = pObj->pNodeInfoList[nodeIndex].pos;
					totalSize += ((int)sizeof(BUFFER_HEADER_T) + pBufferHeader->headerSize + pBufferHeader->payloadSize);
				}
				else
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[0x%X]  遍历NodeList数据错误.. 可能数据已被覆盖\n", consumerId);
					//break;
				}
			}
		}

		if (initPos == -1)
		{
			initPos = pObj->pQueHeader->writePos;
			totalSize = 0;
			pObj->pConsumer[i].videoFrameNum = 0;
			pObj->pConsumer[i].audioFrameNum = 0;
			pObj->pConsumer[i].eventFrameNum = 0;
			pObj->pConsumer[i].textFrameNum = 0;
		}
		pObj->pConsumer[i].readPos = initPos;
		pObj->pConsumer[i].totalSize = totalSize;

		//_TRACE(TRACE_LOG_INFO, (char *)"注册调用者[0x%X] 开始读取历史数据位置: %d totalsize:%d  当前最新数据位置: %d\n", 
			//consumerId, pObj->pConsumer[i].readPos, pObj->pConsumer[i].totalSize, pObj->pQueHeader->writePos);
	}
	else		//获取当前最新数据
	{
		pObj->pConsumer[i].readPos = pObj->pQueHeader->writePos;		//指定为当前写位置, 即获取到的是最新数据
		pObj->pConsumer[i].totalSize = 0;
		pObj->pConsumer[i].videoFrameNum = 0;
		pObj->pConsumer[i].audioFrameNum = 0;
		pObj->pConsumer[i].eventFrameNum = 0;
		pObj->pConsumer[i].textFrameNum = 0;

		//_TRACE(TRACE_LOG_INFO, (char *)"注册调用者[0x%X] 开始读取最新数据位置: %d\n", consumerId, pObj->pConsumer[i].readPos);
	}

	return consumerHandle;
}
void	BUFQUE_UnRegisterConsumer(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE *consumerHandle)
{
	int i=0;
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL == pObj)				return;
	if (NULL == *consumerHandle)	return;
	
	for (i=0; i<MAX_CONSUMER_NUM; i++)
	{
		if (&pObj->pConsumer[i] == *consumerHandle)
		{
			//_TRACE(TRACE_LOG_INFO, (char *)"注销调用者[0x%X]\n", pObj->pConsumer[i].consumerId);

			memset(&pObj->pConsumer[i], 0x00, sizeof(QUEUE_CONSUMER_T));
			*consumerHandle = NULL;
			break;
		}
	}
}


int		BUFQUE_UpdateReadPos(BUFFER_QUEUE_OBJ_T *pObj, CONSUMER_HANDLE handle)
{
	int i=0, ret = -1, readPos = 0, getInitReadPos=0;
	QUEUE_CONSUMER_T	*pConsumer = (QUEUE_CONSUMER_T *)handle;
	if (NULL == pObj)				return -1;
	if (NULL == pObj->pQueHeader)	return -1;
	if (NULL == pConsumer)			return -1;
	
	readPos = pConsumer->readPos;
	if (NULL != pObj->pNodeInfoList)
	{
		pConsumer->readPos = pObj->pQueHeader->writePos;
		pConsumer->totalSize = 0;
		pConsumer->videoFrameNum = 0;
		pConsumer->audioFrameNum = 0;
		pConsumer->eventFrameNum = 0;
		pConsumer->textFrameNum = 0;

		//_TRACE(TRACE_LOG_INFO, (char *)"更新调用者[0x%X][%d] 开始读取位置: %d\n", pConsumer->consumerId, pConsumer->consumerId, pConsumer->readPos);

		/*
		int nodeIndex=0, getTimes=0;
		for (nodeIndex=0; nodeIndex<pObj->pQueHeader->buffNodeNum; nodeIndex++)
		{
			BUFFER_HEADER_T *pBufferHeader = (BUFFER_HEADER_T *)(pObj->pQueData+pObj->pNodeInfoList[nodeIndex].pos);
			if (pBufferHeader->flag == BUFFER_QUEUE_FLAG)
			{
				if (getInitReadPos == 0x00)	getInitReadPos = pObj->pNodeInfoList[nodeIndex].pos;

				getTimes ++;
				if (getTimes>10)
				{
					pConsumer->readPos = pObj->pNodeInfoList[nodeIndex].pos;

					ret = 0;
					//_TRACE(TRACE_LOG_INFO, (char *)"更新调用者[0x%X][%d] 开始读取位置: %d\n", pConsumer->consumerId, pConsumer->consumerId, pConsumer->readPos);
					break;
				}
			}
		}
		*/
	}

	return ret;
}

int		BUFQUE_GetData(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, unsigned int *bufferId, BUFFER_TYPE_ENUM *bufferType, 
					int *headerSize, char *headerData, int *payloadSize, char *payloadData, unsigned char moveData)
{
	int ret = -1;
	int remain = 0;
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	QUEUE_CONSUMER_T	*pConsumer = (QUEUE_CONSUMER_T *)consumerHandle;

#ifdef DEBUG_BUFFER_QUEUE
	int		nCustomerTotalSize = pConsumer->totalSize;
#endif

	if (NULL == consumerHandle)		return -1;
	if (NULL == pObj)				return -1;
	if (NULL == pObj->pQueHeader)	return -1;
	//if (NULL == headerData)			return -1;
	if (pConsumer->readPos < 0)
	{
		//_TRACE(TRACE_LOG_ERROR, (char *)"ConsumerID[%d] 读取位置错误: %d\n", pConsumer->consumerId,   pConsumer->readPos);

		BUFQUE_UpdateReadPos(pObj, consumerHandle);

		return -1;
	}

	//if (pObj->pQueHeader->totalSize < 0)
	if (pConsumer->totalSize < 0)
	{
		//_TRACE(TRACE_LOG_ERROR, "pConsumer->totalSize<0: %d\n", pObj->pQueHeader->totalSize);
		return -1;
	}

	//if (pObj->pQueHeader->totalSize <= (int)sizeof(BUFFER_HEADER_T))
	if (pConsumer->totalSize <= (int)sizeof(BUFFER_HEADER_T))
	{
		return -1;
	}

	if (pConsumer->readPos > pObj->pQueHeader->queueSize)
	{
		pConsumer->readPos = 0;
	}

	if (pConsumer->readPos == pObj->pQueHeader->queueSize)
	{
		pConsumer->readPos = 0;
	}

	if (pConsumer->readPos < pObj->pQueHeader->queueSize)		//从头到尾读
	{
		if ( (pConsumer->readPos + (int)sizeof(BUFFER_HEADER_T)) <= pObj->pQueHeader->queueSize )	//从头到尾读, 包含BUFFER_HEADER_T
		{
			int iReadPos = pConsumer->readPos;
			BUFFER_HEADER_T	tmpNode;
			BUFFER_HEADER_T *pNode = (BUFFER_HEADER_T *)(pObj->pQueData + pConsumer->readPos);
			memcpy(&tmpNode, pNode, sizeof(BUFFER_HEADER_T));

			if (-1 == BUFQUE_CheckBufferHeader(pNode))		//check data
			{
				//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[%d]  头-->尾  读数据错误. totalSize:%d\n", pConsumer->consumerId, pConsumer->totalSize);

				//if (pConsumer->readPos < pObj->pQueHeader->writePos)
				if (pConsumer->readPos != pObj->pQueHeader->writePos)
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[%d]  pConsumer->readPos[%d] < pObj->pQueHeader->writePos[%d]\n", 
						//pConsumer->consumerId, pConsumer->readPos, pObj->pQueHeader->writePos);
					BUFQUE_UpdateReadPos(pObj, consumerHandle);
					//return BUFQUE_GetData(pObj, consumerHandle, bufferId, bufferType, headerSize, headerData, payloadSize, payloadData, moveData);
				}
				return -1;
			}

			if (pConsumer->totalSize < (int)sizeof(BUFFER_HEADER_T) + tmpNode.headerSize + tmpNode.payloadSize)
			{
				////_TRACE(TRACE_LOG_DEBUG, (char *)"数据量不足. pConsumerId[%d] totalsize[%d]  heaersize+payloadSize[%d]\n", pConsumer->consumerId, pConsumer->totalSize, tmpNode.headerSize+tmpNode.payloadSize+(int)sizeof(BUFFER_HEADER_T));
				return -1;
			}

			if (NULL != bufferId)		*bufferId = tmpNode.id;
			if (NULL != bufferType)		*bufferType = tmpNode.type;
			if (NULL != headerSize)		*headerSize = tmpNode.headerSize;
			if (NULL != payloadSize)	*payloadSize = tmpNode.payloadSize;
			if (tmpNode.type == BUFFER_TYPE_VIDEO && pConsumer->videoFrameNum>0)			pConsumer->videoFrameNum --;
			else if (tmpNode.type == BUFFER_TYPE_AUDIO && pConsumer->audioFrameNum>0)	pConsumer->audioFrameNum --;
			else if (tmpNode.type == BUFFER_TYPE_EVENT && pConsumer->eventFrameNum>0)	pConsumer->eventFrameNum --;
			else if (tmpNode.type == BUFFER_TYPE_TEXT && pConsumer->textFrameNum>0)		pConsumer->textFrameNum --;
			else
			{


			}
			
			iReadPos += (int)sizeof(BUFFER_HEADER_T);
			if (iReadPos == pObj->pQueHeader->queueSize)
			{
				iReadPos = 0;		//reset		到达队尾,从头开始
#if 0
				//_TRACE(TRACE_LOG_WARNING, (char *)"到达队尾,从头开始: %d\n", pConsumer->readPos);
#endif
			}
			if (iReadPos + tmpNode.headerSize <= pObj->pQueHeader->queueSize)		//包含 headerData
			{
				if (NULL != headerData)		memcpy(headerData, pObj->pQueData+iReadPos, tmpNode.headerSize);
				iReadPos += tmpNode.headerSize;
				if (iReadPos == pObj->pQueHeader->queueSize)
				{
					iReadPos = 0;		//reset		到达队尾,从头开始
#if 0
					//_TRACE(TRACE_LOG_WARNING, (char *)"到达队尾,从头开始: %d\n", pConsumer->readPos);
#endif
				}
				if (iReadPos + tmpNode.payloadSize <= pObj->pQueHeader->queueSize)	//包含payloadData
				{
					if (NULL != payloadData)		memcpy(payloadData, pObj->pQueData+iReadPos, tmpNode.payloadSize);
					iReadPos += tmpNode.payloadSize;
				}
				else		//payloadData 在队尾和队头
				{
					int remain = pObj->pQueHeader->queueSize - iReadPos;
					if (NULL != payloadData)
					{
						memcpy(payloadData, pObj->pQueData+iReadPos, remain);
						memcpy(payloadData+remain, pObj->pQueData, tmpNode.payloadSize - remain);
					}
					iReadPos = tmpNode.payloadSize - remain;
				}

				if (moveData == 0x01)
				{
					pConsumer->readPos = iReadPos;

					if (pConsumer->readPos > pObj->pQueHeader->queueSize)
					{
						//libLogger_Print(LOG_TYPE_DEBUG, (char *)"偏移位置错误: %d\n", pConsumer->readPos);
					}

					if (pConsumer->readPos == pObj->pQueHeader->queueSize)
					{
						//libLogger_Print(LOG_TYPE_DEBUG, (char *)"Reset queue\n");
						pConsumer->readPos = 0;
					}

					if (pConsumer->readPos < 0)
					{
						//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误: %d\n", pConsumer->readPos);
					}
					//pObj->pQueHeader->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+tmpNode.headerSize+tmpNode.payloadSize);
					//if (pObj->pQueHeader->totalSize < 0)	pObj->pQueHeader->totalSize = 0;
					BUFQUE_Lock(pObj);	//Lock
					pConsumer->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+tmpNode.headerSize+tmpNode.payloadSize);
					if (pConsumer->totalSize < 0)
					{
						//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误.\n");
						pConsumer->totalSize = 0;
					}
					BUFQUE_Unlock(pObj);	//Unlock
				}
			}
			else	//headerData 在 队尾和队头
			{
				int remain = pObj->pQueHeader->queueSize - iReadPos;
				if (NULL != headerData)
				{
					memcpy(headerData, pObj->pQueData+iReadPos, remain);
					memcpy(headerData+remain, pObj->pQueData, tmpNode.headerSize-remain);
				}
				iReadPos = tmpNode.headerSize-remain;
				if (NULL != payloadData)
				{
					memcpy(payloadData, pObj->pQueData+iReadPos, tmpNode.payloadSize);
				}
				iReadPos += tmpNode.payloadSize;

				if (moveData == 0x01)
				{
					pConsumer->readPos = iReadPos;
					if (pConsumer->readPos < 0)
					{
						//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误: %d\n", pConsumer->readPos);
					}
					//pObj->pQueHeader->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+tmpNode.headerSize+tmpNode.payloadSize);
					//if (pObj->pQueHeader->totalSize < 0)	pObj->pQueHeader->totalSize = 0;

					BUFQUE_Lock(pObj);	//Lock
					pConsumer->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+tmpNode.headerSize+tmpNode.payloadSize);
					if (pConsumer->totalSize < 0)
					{
						//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误.\n");
						pConsumer->totalSize = 0;
					}
					BUFQUE_Unlock(pObj);	//Unlock
				}
			}

			ret = 0;
		}
		else	//队尾中不足一个 sizeof(BUFFER_HEADER_T)
		{
			int iReadPos = pConsumer->readPos;
			int remain = pObj->pQueHeader->queueSize - iReadPos;
			BUFFER_HEADER_T bufferHeader;
			char *tmpbuf = (char *)&bufferHeader;
			BUFFER_HEADER_T *pNode = (BUFFER_HEADER_T *)&bufferHeader;

			memset(&bufferHeader, 0x00, sizeof(BUFFER_HEADER_T));
			memcpy(tmpbuf, pObj->pQueData+iReadPos, remain);
			memcpy(tmpbuf+remain, pObj->pQueData, (int)sizeof(BUFFER_HEADER_T)-remain);
			iReadPos = (int)sizeof(BUFFER_HEADER_T)-remain;

#if 0
			//_TRACE(TRACE_LOG_INFO, (char *)"consumerId[0x%X][%d]队尾不足一个sizeof(BUFFER_HEADER_T)  headersize+payloadsize: %d\n", pConsumer->consumerId, pConsumer->consumerId, 
				(int)sizeof(BUFFER_HEADER_T) + bufferHeader.headerSize + bufferHeader.payloadSize);
#endif
			if (pConsumer->totalSize < (int)sizeof(BUFFER_HEADER_T) + bufferHeader.headerSize + bufferHeader.payloadSize)
			{
				////_TRACE(TRACE_LOG_DEBUG, (char *)"数据量不足. pConsumerId[%d] totalsize[%d]  heaersize+payloadSize[%d]\n", pConsumer->consumerId, pConsumer->totalSize, tmpNode.headerSize+tmpNode.payloadSize+(int)sizeof(BUFFER_HEADER_T));
				return -1;
			}

			if (-1 == BUFQUE_CheckBufferHeader(pNode))		//check data
			{
				//_TRACE(TRACE_LOG_ERROR, (char *)"consumerId[%d]  从尾到头读数据错误. totalSize:%d\n", pConsumer->consumerId, pConsumer->totalSize);
				////_TRACE(TRACE_LOG_ERROR, (char *)"-1 == BUFQUE_CheckBufferHeader(pNode)");
				BUFQUE_UpdateReadPos(pObj, consumerHandle);
				return -1;
			}

			if (NULL != bufferId)		*bufferId = pNode->id;
			if (NULL != bufferType)		*bufferType = pNode->type;
			if (NULL != headerSize)		*headerSize = pNode->headerSize;
			if (NULL != payloadSize)	*payloadSize = pNode->payloadSize;

			if (NULL != headerData)		memcpy(headerData, pObj->pQueData+iReadPos, pNode->headerSize);
			iReadPos += pNode->headerSize;
			if (pNode->payloadSize > 0)
			{
				if (NULL != payloadData)		memcpy(payloadData, pObj->pQueData+iReadPos, pNode->payloadSize);
				iReadPos += pNode->payloadSize;
			}
			
			if (moveData == 0x01)
			{
				if (pNode->type == BUFFER_TYPE_VIDEO && pConsumer->videoFrameNum>0)			pConsumer->videoFrameNum --;
				else if (pNode->type == BUFFER_TYPE_AUDIO && pConsumer->audioFrameNum>0)	pConsumer->audioFrameNum --;
				else if (pNode->type == BUFFER_TYPE_EVENT && pConsumer->eventFrameNum>0)	pConsumer->eventFrameNum --;
				else if (pNode->type == BUFFER_TYPE_TEXT && pConsumer->textFrameNum>0)		pConsumer->textFrameNum --;


				pConsumer->readPos = iReadPos;
				if (pConsumer->readPos < 0)
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误: %d\n", pConsumer->readPos);
				}

				BUFQUE_Lock(pObj);	//Lock
				//pObj->pQueHeader->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+bufferHeader.headerSize+bufferHeader.payloadSize);
				//if (pObj->pQueHeader->totalSize < 0)	pObj->pQueHeader->totalSize = 0;
				pConsumer->totalSize -= ((int)sizeof(BUFFER_HEADER_T)+bufferHeader.headerSize+bufferHeader.payloadSize);
				if (pConsumer->totalSize < 0)
				{
					//_TRACE(TRACE_LOG_ERROR, (char *)"偏移位置错误.\n");
					pConsumer->totalSize = 0;
				}
				BUFQUE_Unlock(pObj);	//Unlock
			}

			ret = 0;
		}
	}
	else
	{
		//_TRACE(TRACE_LOG_ERROR, (char *)"读取错误: %d\n", pConsumer->readPos);


		BUFQUE_UpdateReadPos(pObj, consumerHandle);

	}


	//if (pConsumer->consumerId == 1)
	////_TRACE(TRACE_LOG_DEBUG, (char *)"consumerId[%d] 读取位置: %d totalsize: %d   bufferId:%d\n", pConsumer->consumerId, pConsumer->readPos, pConsumer->totalSize, *bufferId);


#ifdef DEBUG_BUFFER_QUEUE
	//_TRACE(TRACE_LOG_DEBUG, (char *)"consumerId[%d] 读取位置: %d totalsize: %d - %d  = %d\n", pConsumer->consumerId, pConsumer->readPos, 
		nCustomerTotalSize, nCustomerTotalSize-pConsumer->totalSize, pConsumer->totalSize);
#endif

	

	return ret;
}

//获取帧数
int		BUFQUE_GetFrameNum(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle, BUFFER_TYPE_ENUM bufferType)
{
	int ret = -1;
	int remain = 0;
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	QUEUE_CONSUMER_T	*pConsumer = (QUEUE_CONSUMER_T *)consumerHandle;

	if (NULL == consumerHandle)		return -1;
	if (NULL == pObj)				return -1;
	if (NULL == pObj->pQueHeader)	return -1;

	if		(BUFFER_TYPE_VIDEO == bufferType)			return pConsumer->videoFrameNum;
	else if (BUFFER_TYPE_AUDIO == bufferType)			return pConsumer->audioFrameNum;
	else if (BUFFER_TYPE_EVENT == bufferType)			return pConsumer->eventFrameNum;
	else if (BUFFER_TYPE_TEXT == bufferType)			return pConsumer->textFrameNum;

	return 0;
}

//获取该消费者当前总字节数
int		BUFQUE_GetTotalBytes(BUFFQUEUE_HANDLE handle, CONSUMER_HANDLE consumerHandle)
{
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	QUEUE_CONSUMER_T	*pConsumer = (QUEUE_CONSUMER_T *)consumerHandle;

	if (NULL == consumerHandle)		return -1;
	if (NULL == pObj)				return -1;
	if (NULL == pObj->pQueHeader)	return -1;

	return pConsumer->totalSize;
}

//获取缓冲区大小
int		BUFQUE_GetQueueSize(BUFFQUEUE_HANDLE handle)
{
	BUFFER_QUEUE_OBJ_T *pObj = (BUFFER_QUEUE_OBJ_T *)handle;
	if (NULL == pObj)			return 0;
	if (NULL == pObj->pQueHeader)	return 0;

	return pObj->pQueHeader->queueSize;

}


int		BUFQUE_CheckBufferHeader(BUFFER_HEADER_T *pBufferHeader)
{
	if (NULL == pBufferHeader)		return 0;

	if (pBufferHeader->flag != BUFFER_QUEUE_FLAG)		return -1;
	if (pBufferHeader->size != sizeof(BUFFER_HEADER_T))	return -1;

	return 0;
}


int		BUFQUE_Debug(BUFFER_QUEUE_OBJ_T *pObj)
{
	if (NULL == pObj)				return -1;
	if (NULL == pObj->pQueHeader)	return -1;


	if (NULL != pObj->pNodeInfoList)
	{
		int i=0;
		for (i=0; i<pObj->pQueHeader->buffNodeNum; i++)
		{
			//_TRACE(TRACE_LOG_DEBUG, (char *)"[%d] offset[%d] timestamp[%d]\n", i, pObj->pNodeInfoList[i].pos, pObj->pNodeInfoList[i].timestamp_sec);


		}
	}

	return 0;
}
