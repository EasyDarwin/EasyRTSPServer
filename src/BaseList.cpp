#include "BaseList.h"


BaseList::BaseList(void)
{
	pBaseNodeList	=	NULL;
	pBaseNodeLast	=	NULL;
	baseNodeNum		=	0;

#ifdef _WIN32
	InitializeCriticalSection(&baseMutex);
#else
	pthread_mutex_init(&baseMutex, NULL);
#endif
}


BaseList::~BaseList(void)
{
	//DeleteAll();

#ifdef _WIN32
	DeleteCriticalSection(&baseMutex);
#else
	pthread_mutex_destroy(&baseMutex);
#endif
}


int		BaseList::Lock()
{
#ifdef _WIN32
	EnterCriticalSection(&baseMutex);
#else
	(void)pthread_mutex_lock(&baseMutex);
#endif
	return 0;
}
void	BaseList::Unlock()
{
#ifdef _WIN32
	LeaveCriticalSection(&baseMutex);
#else
	pthread_mutex_unlock(&baseMutex);
#endif
}

void	BaseList::DeleteAll()
{
	Lock();

	BASE_NODE_T *pTmpNode = pBaseNodeList;
	while (NULL != pTmpNode)
	{
		BASE_NODE_T *pDelNode = pTmpNode;
		pTmpNode = pTmpNode->pNext;

		if (NULL != pDelNode->userPtr)
		{
			DelNode(pDelNode->userPtr);
		}

		delete pDelNode;
	}

	pBaseNodeList	=	NULL;
	pBaseNodeLast	=	NULL;
	baseNodeNum		=	0;

	Unlock();
}


int		BaseList::Add(void *userPtr)
{
	if (NULL == userPtr)		return -1;

	BASE_NODE_T	*pNewBaseNode = new BASE_NODE_T;
	if (NULL == pNewBaseNode)		return -1;

	memset(pNewBaseNode, 0x00, sizeof(BASE_NODE_T));
	pNewBaseNode->userPtr = userPtr;

	int ret = -1;
	Lock();

	if (NULL == pBaseNodeList)
	{
		pBaseNodeList = pNewBaseNode;
		pBaseNodeLast = pBaseNodeList;
	}
	else
	{
		BASE_NODE_T	*prev = pBaseNodeLast;
		pBaseNodeLast->pNext = pNewBaseNode;
		pBaseNodeLast = pNewBaseNode;
		pBaseNodeLast->pPrev = prev;
	}

	baseNodeNum ++;
	ret = 0;

	Unlock();

#ifdef _DEBUG
	PrintList();
#endif


	return ret;
}
int		BaseList::Add(void *userPtr, int userPtrSize)
{
	if (NULL == userPtr)		return -1;
	if (userPtrSize < 1)		return -1;

	char *ptr = new char[userPtrSize];
	if (NULL == ptr)			return -2;

	memcpy(ptr, userPtr, userPtrSize);

	return Add(ptr);
}


int		BaseList::Del(void *userPtr)
{
	if (NULL == userPtr)		return -1;

	BASE_NODE_T *pNode = NULL;
	Lock();

	BASE_NODE_T *pTmpNode = pBaseNodeList;
	while (NULL != pTmpNode)
	{
		if (pTmpNode->userPtr == userPtr)
		{
			pNode = pTmpNode;
			break;
		}
		pTmpNode = pTmpNode->pNext;
	}

	Unlock();

	if (NULL != pNode)
	{
		//调用虚函数,删除该成员, 当返回0时, 删除对应的节点
		if (0 == DelNode(userPtr))
		{
			Del(pNode);
		}
	}

	return 0;
}
int		BaseList::Del(BASE_NODE_T *pNode)
{
	if (NULL == pNode)				return 0;
	if (NULL == pBaseNodeList)		return 0;

	int ret = -1;

	Lock();

	do
	{
		BASE_NODE_T *pClientPrevious = pNode->pPrev;
		BASE_NODE_T *pClientNext = pNode->pNext;

		if (NULL == pClientPrevious)
		{
			//头
			if (NULL!=pClientNext)		pClientNext->pPrev = NULL;
			pBaseNodeList = pClientNext;
			pBaseNodeLast = pClientNext;
		}
		else
		{
			if (pBaseNodeLast == pNode)		//删除的是最后一个成员
			{
				pBaseNodeLast = pNode->pPrev;
				if (NULL == pBaseNodeLast)
				{
					pBaseNodeLast = pBaseNodeList = NULL;
				}
				else
				{
					pBaseNodeLast->pNext = NULL;
				}
			}
			else
			{
				pClientPrevious->pNext = pClientNext;
				if (NULL != pClientNext)	pClientNext->pPrev = pClientPrevious;
			}
		}

		//GB28181StreamSocket::Release(&pNode->pStreamSocket);
		//DeleteOSThread(&pNode->pThread);

		delete pNode;

		baseNodeNum --;

	}while (0);

	Unlock();

#ifdef _DEBUG
	PrintList();
#endif
	
	return ret;
}

int BaseList::DelNode(void *userPtr)
{
	return 0;
}

void	BaseList::PrintList()
{
	if (NULL != pBaseNodeList)
	{
		//libLogger_Print(LOG_TYPE_DEBUG, (char *)"客户端列表 BEGIN----------------------\n");

		int idx = 0;
		BASE_NODE_T *pTmpNode = pBaseNodeList;
		while (NULL != pTmpNode)
		{
			//libLogger_Print(LOG_TYPE_DEBUG, (char *)"客户端[%d] [%s][0x%X]...\n", idx++,  pNode->pStreamSocket->GetSocketType()==SOCK_STREAM?"TCP":"UDP",  pNode);

			pTmpNode = pTmpNode->pNext;
		}

		//libLogger_Print(LOG_TYPE_DEBUG, (char *)"客户端列表 END----------------------\n");
	}
	else
	{
		//libLogger_Print(LOG_TYPE_INFO, (char *)"############# 当前客户端列表为空 #############\n");
	}

}