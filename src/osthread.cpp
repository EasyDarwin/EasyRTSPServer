#include "osthread.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "live_ipv4_ipv6.h"

#ifdef _WIN32
int CreateOSThread(OSTHREAD_OBJ_T **handle, void *procFunc, void *userPtr)
#else
int CreateOSThread(OSTHREAD_OBJ_T **handle, void *(*procFunc)(void *), void *userPtr)
#endif
{
	if (NULL == procFunc)		return -1;

	OSTHREAD_OBJ_T	*pThread = new OSTHREAD_OBJ_T;
	if (NULL == pThread)		return -2;

	memset(pThread, 0x00, sizeof(OSTHREAD_OBJ_T));
	pThread->flag = 0x01;
	pThread->userPtr = userPtr;

#ifdef _WIN32
	pThread->hHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)procFunc, (LPVOID)pThread, 0, (LPDWORD)&pThread->threadId);
	if (NULL == pThread->hHandle)
	{
		delete pThread;
		return -2;
	}
#else
	if(pthread_create(&pThread->hHandle, NULL, procFunc, pThread) < 0)
    {
		 delete pThread;
		 return -2;
    }
#endif
	while (pThread->flag != 0x02 && pThread->flag!=0x00)	{Sleep(10);}
	if (pThread->flag == 0x00)
	{
		delete pThread;
		return -2;
	}

	*handle = pThread;

	return 0;
}


int DeleteOSThread(OSTHREAD_OBJ_T **handle)
{
	OSTHREAD_OBJ_T *pThread = (OSTHREAD_OBJ_T *)*handle;
	if (NULL == pThread)		return -1;

	if (pThread->flag == 0x02)		pThread->flag = 0x03;
#ifdef _WIN32
	while (pThread->flag != 0x00)	{Sleep(10);}
#else
	while (pThread->flag != 0x00)	{usleep(1000*10);}
#endif

#ifdef _WIN32
	if (NULL != pThread->hHandle)
	{
		if (pThread->flag == 0x02)	pThread->flag = 0x03;
		while (pThread->flag != 0x00)	{Sleep(10);}
		CloseHandle(pThread->hHandle);
		pThread->hHandle = NULL;
	}
#else
	if (pThread->hHandle > 0U)
	{
		if (pThread->flag == 0x02)	pThread->flag = 0x03;
		while (pThread->flag != 0x00)	{usleep(1000*10);}
		pthread_join(pThread->hHandle, NULL);
		pThread->hHandle = 0x00;
	}
#endif

	delete pThread;
	*handle = NULL;

	return 0;
}
