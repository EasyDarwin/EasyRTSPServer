#include "RtspClientList.h"


RtspClientList::RtspClientList(void)
{
}


RtspClientList::~RtspClientList(void)
{
	DeleteAll();
}


int RtspClientList::Update(RTSP_CLIENT_INFO_T *pClient)
{
	int ret = -1;

	BASE_NODE_T	*pBaseNodeList = GetBaseNodeList();
	if (NULL == pBaseNodeList)		return -2;
	Lock();

	while (NULL != pBaseNodeList)
	{
		RTSP_CLIENT_INFO_T *pTmpContext = (RTSP_CLIENT_INFO_T *)pBaseNodeList->userPtr;
		//if (pBaseNodeList->userPtr == pSipDeviceInfoContext)
		if (0 == strcmp(pTmpContext->sip_auth_info.usernameId, pSipDeviceInfoContext->sip_auth_info.usernameId))
		{
			memcpy(pBaseNodeList->userPtr, pSipDeviceInfoContext, sizeof(RTSP_CLIENT_INFO_T));
			ret = 0;
			break;
		}

		pBaseNodeList = pBaseNodeList->pNext;
	}

	Unlock();

	return ret;
}

int RtspClientList::Del(char *cameraIndexCode)
{



	return 0;
}

int RtspClientList::DelNode(void *userPtr)
{
	if (NULL == userPtr)			return 0;

	RTSP_CLIENT_INFO_T *pTmpContext = (RTSP_CLIENT_INFO_T *)userPtr;

	delete pTmpContext;

	return 0;
}

int RtspClientList::Find(char *cameraIndexCode, RTSP_CLIENT_INFO_T **pClient)
{
	int ret = -1;
#if 0
	BASE_NODE_T	*pBaseNodeList = GetBaseNodeList();
	if (NULL == pBaseNodeList)		return -2;
	Lock();

	while (NULL != pBaseNodeList)
	{
		RTSP_CLIENT_INFO_T *pTmpContext = (RTSP_CLIENT_INFO_T *)pBaseNodeList->userPtr;
		//if (pBaseNodeList->userPtr == pSipDeviceInfoContext)
		if (0 == strcmp(pTmpContext->sip_auth_info.usernameId, cameraIndexCode))
		{
			if (NULL != ppSipDeviceInfoContext)	*ppSipDeviceInfoContext = pTmpContext;
			ret = 0;
			break;
		}

		pBaseNodeList = pBaseNodeList->pNext;
	}

	Unlock();
#endif
	return ret;
}