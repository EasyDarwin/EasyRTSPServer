#ifndef __OS_THREAD_H__
#define __OS_THREAD_H__


#ifdef _WIN32
#include <WinSock2.h>
#else
#include <pthread.h>
#endif


#ifdef _WIN32
typedef HANDLE				OSThreadHandle;
#else
typedef pthread_t			OSThreadHandle;
#endif


typedef struct OSTHREAD_OBJ_T
{
	int					threadId;
	OSThreadHandle		hHandle;
	int					flag;
	void				*userPtr;
	void				*pEx;
}OSTHREAD_OBJ_T;


#ifdef _WIN32
int CreateOSThread(OSTHREAD_OBJ_T **handle, void *procFunc, void *userPtr);
#else
int CreateOSThread(OSTHREAD_OBJ_T **handle, void *(*procFunc)(void *), void *userPtr);
#endif
int DeleteOSThread(OSTHREAD_OBJ_T **handle);





#endif
