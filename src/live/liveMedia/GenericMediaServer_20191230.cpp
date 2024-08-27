/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// A generic media server class, used to implement a RTSP server, and any other server that uses
//  "ServerMediaSession" objects to describe media to be served.
// Implementation

#include "GenericMediaServer.hh"
#include <GroupsockHelper.hh>
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
#define snprintf _snprintf
#endif
#include "../../trace.h"
#include "./include/RTSPServer.hh"
#include <time.h>
#ifndef _WIN32
#include "../../BasicTaskSchedulerEpoll.h"
#endif

#include "include/RTSPCommon.hh"

////////// GenericMediaServer implementation //////////

void GenericMediaServer::addServerMediaSession(UsageEnvironment	*pEnv, ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;

  char const* streamName = serverMediaSession->streamName();
  if (streamName == NULL) streamName = "";

#ifdef _DEBUG
	ServerMediaSession* pServerMediaSession = (ServerMediaSession *)fServerMediaSessions->Lookup(streamName);
	if (NULL != pServerMediaSession)
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"StreamName[%s] 已存在.\n", streamName);
	}

#endif

  //int ret = removeServerMediaSession(pEnv, sessionName, False); // in case an existing "ServerMediaSession" with this name already exists

  //if (ret == 0)		//正常情况下，此处应返回1000, 即该sessionName不存在
  //{
	 // _TRACE(TRACE_LOG_DEBUG, 
		//			(char *)"####[%s] GenericMediaServer::addServerMediaSession  当前sessionName[%s]名称已存在且被删除..  异常###\n",
		//			pEnv->GetEnvirName(), sessionName);
  //}
  //
  //此处无需加锁, 因外面调用位置已加锁
  fServerMediaSessions->Add(streamName, (void*)serverMediaSession);
}

void	GenericMediaServer::ResetStreamNameInWorkThread(UsageEnvironment *pEnv, Boolean bLock)//, char *streamName)		//2018.09.10	当源返回失败时,需清空当前线程所占用的通道, 进行回收
{
	if (NULL == multiThreadCore.threadTask)		return;

	if (bLock)	LockServerMediaSession(pEnv->GetEnvirName(), "GenericMediaServer::ResetStreamNameInWorkThread", (unsigned long long)this);

	char szStreamName[512] = {0};
	for (int i=0; i<multiThreadCore.threadNum; i++)
	{
		if (multiThreadCore.threadTask[i].pSubEnv == pEnv)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] 准备清空当前通道.  名称: %s\n", pEnv->GetEnvirName(), multiThreadCore.threadTask[i].liveURLSuffix);

			if ( (int)strlen(multiThreadCore.threadTask[i].liveURLSuffix) < 1)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] 异常情况回收\n", pEnv->GetEnvirName());
			}

			strcpy(szStreamName, multiThreadCore.threadTask[i].liveURLSuffix);

			multiThreadCore.threadTask[i].releaseChannel = 0x01;
			break;
		}
	}

	if ( (int)strlen(szStreamName) > 0)
	{
		ServerMediaSession* pServerMediaSession = (ServerMediaSession *)fServerMediaSessions->Lookup(szStreamName);
		if (NULL != pServerMediaSession)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] #####异常情况处理:  当前serverMediaSesion应为空,但此时不为空, 在此处进行删除: %s\n", pEnv->GetEnvirName(), szStreamName);
			//removeServerMediaSession(&pServerMediaSession->envir(), pServerMediaSession, False, False); 
		}
	}

	if (bLock)	UnlockServerMediaSession(pEnv->GetEnvirName(), "GenericMediaServer::ResetStreamNameInWorkThread", (unsigned long long)this);

#if 0
	//gavin 2018.09.10
	for (int i=0; i<multiThreadCore.threadNum; i++)
	{
		if ( 0 == strcmp(multiThreadCore.threadTask[i].urlSuffix, streamName) )
		{
			multiThreadCore.threadTask[i].release = 0x01;
			memset(multiThreadCore.threadTask[i].urlSuffix, 0x00, sizeof(multiThreadCore.threadTask[i].urlSuffix));
			break;
		}
	}
#endif
}

ServerMediaSession* GenericMediaServer
::lookupServerMediaSession(UsageEnvironment	*pEnv, int iType, void *pClientPtr, char const* streamName, Boolean bLockServerMediaSession, Boolean /*isFirstLookupInSession*/) {
  // Default implementation:
  //return (ServerMediaSession*)(fServerMediaSessions->Lookup(streamName));

	return (ServerMediaSession*)(fServerMediaSessions->Lookup(streamName));
}

int	 GenericMediaServer::removeServerMediaSession(UsageEnvironment *pEnv, ServerMediaSession* serverMediaSession, 
																					Boolean bLock, Boolean resetStreamName) {
  if (serverMediaSession == NULL) return 1000;

  //pEnv->Lock();
  //pEnv->SetLockFlag(0x01);

  if (pEnv != &serverMediaSession->envir())
  {
	  _TRACE(TRACE_LOG_DEBUG, (char *)"[%s] #####异常情况:  待删除的serverMediaSession中的envir[%s]与当前pEnv[%s]不匹配\n", 
						serverMediaSession->envir().GetEnvirName(), pEnv->GetEnvirName());
  }
  
  if (bLock)	LockServerMediaSession(pEnv->GetEnvirName(), "GenericMediaServer::removeServerMediaSession", (unsigned long long)this);
  //2018.09.10
  char streamName[512] = {0};
  strcpy(streamName, serverMediaSession->streamName());

  fServerMediaSessions->Remove(serverMediaSession->streamName());

  if (serverMediaSession->referenceCount() == 0) {
    Medium::close(serverMediaSession, pEnv);
  } else {
    serverMediaSession->deleteWhenUnreferenced() = True;
  }

  if (resetStreamName)
  {
	 ResetStreamNameInWorkThread(pEnv, False);//, streamName);
  }
  if (bLock)	UnlockServerMediaSession(pEnv->GetEnvirName(), "GenericMediaServer::removeServerMediaSession", (unsigned long long)this);


  //pEnv->SetLockFlag(0x00);
  //pEnv->Unlock();

  return 0;
}

void	GenericMediaServer::LockServerMediaSession(char *threadName, char *functionName, unsigned long long ptr)
{

	//_TRACE(TRACE_LOG_DEBUG, (char *)"%s  %s  0x%X Lock\n", threadName, functionName, ptr);
	LockMutex(&mutexServerMediaSession);
	//_TRACE(TRACE_LOG_DEBUG, (char *)"%s  %s  0x%X Lock OK.\n", threadName, functionName, ptr);

}
void	GenericMediaServer::UnlockServerMediaSession(char *threadName, char *functionName, unsigned long long ptr)
{

	//_TRACE(TRACE_LOG_DEBUG, (char *)"%s  %s  0x%X Unlock\n", threadName, functionName, ptr);
	UnlockMutex(&mutexServerMediaSession);
	//_TRACE(TRACE_LOG_DEBUG, (char *)"%s  %s  0x%X Unlock OK\n", threadName, functionName, ptr);

}

void	GenericMediaServer::LockClientConnection()
{
	LockMutex(&mutexClientConnection);
}
void	GenericMediaServer::UnlockClientConnection()
{
	UnlockMutex(&mutexClientConnection);
}
void	GenericMediaServer::LockClientSession()
{
	LockMutex(&mutexClientSession);
}
void	GenericMediaServer::UnlockClientSession()
{
	UnlockMutex(&mutexClientSession);
}



int	 GenericMediaServer::removeServerMediaSession(UsageEnvironment *pEnv, char const* streamName, Boolean bLock) {
  return removeServerMediaSession(pEnv, (ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)), bLock);
}

void GenericMediaServer::closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession, Boolean bLock) {
  if (serverMediaSession == NULL) return;
  
  UsageEnvironment *pEnv = &serverMediaSession->envir();

  char streamName[512] = {0};
  if (NULL != serverMediaSession && serverMediaSession->GetFlag() == LIVE_FLAG)
  {
	strcpy(streamName, serverMediaSession->streamName());
  }

  //LockClientSession();		//Lock clientSession

  HashTable::Iterator* iter = HashTable::Iterator::create(*fClientSessions);
  GenericMediaServer::ClientSession* clientSession;
  char const* key; // dummy
  while ((clientSession = (GenericMediaServer::ClientSession*)(iter->next(key))) != NULL) {
    if (clientSession->fOurServerMediaSession == serverMediaSession) {
		clientSession->lockClientFlag = False;
      delete clientSession;
    }
  }
  delete iter;

  //UnlockClientSession();	//Unlock clientSession

  if (NULL != pEnv && ((int)strlen(streamName) > 0))
  {
	removeServerMediaSession(pEnv, streamName, bLock);
  }
}

void GenericMediaServer::closeAllClientSessionsForServerMediaSession(char const* streamName, Boolean bLock) {
  closeAllClientSessionsForServerMediaSession((ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)), bLock);
}

void GenericMediaServer::deleteServerMediaSession(ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;
  
  closeAllClientSessionsForServerMediaSession(serverMediaSession, True);
  removeServerMediaSession(&serverMediaSession->envir(), serverMediaSession, True);
}

void GenericMediaServer::deleteServerMediaSession(char const* streamName) {
  deleteServerMediaSession((ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)));
}

void	GenericMediaServer::CloseAllConnections(char *streamName)
{
    LockMutex(&mutexClose);
    LockClientConnection();
    
  HashTable::Iterator* iter = HashTable::Iterator::create(*fClientConnections);
  GenericMediaServer::ClientConnection* connection;
  char const* key; // dummy
  while ((connection = (GenericMediaServer::ClientConnection*)(iter->next(key))) != NULL) {
      if (connection->GetFlag() != LIVE_FLAG)		continue;
	  if (0 == strcmp(streamName, connection->GetStreamName()) )
	  {
              connection->SetLockFlag(0);
			  connection->ResetConnectionTask();
		connection->handleRequestBytes(-1, connection->pClientConnectionEnv);
	  }
  }
  delete iter;
  
  UnlockClientConnection();
  UnlockMutex(&mutexClose);
}


#ifdef LIVE_MULTI_THREAD_ENABLE

#ifdef _WIN32
DWORD WINAPI __WorkerThread_Proc(LPVOID param)
#else
void *__WorkerThread_Proc(void *param)
#endif
{
#ifdef ANDROID
	JNIEnv* env;  
	g_vm->AttachCurrentThread(&env, NULL);
#endif

	OSTHREAD_OBJ_T	*pThread = (OSTHREAD_OBJ_T *)param;
	LIVE_THREAD_TASK_T	*pThreadTask = (LIVE_THREAD_TASK_T *)pThread->userPtr;
	GenericMediaServer	*pThis = (GenericMediaServer *)pThreadTask->extPtr;

	int			envirCount = 0;
	int			tally = 0;
#ifdef _DEBUG1
	unsigned int lastTime = 0;
#endif

	_TRACE(TRACE_LOG_INFO, (char *)"Startup Worker Thread[%d]  BasicTaskScheduler[0x%X]  [%s]\n", 
							pThreadTask->id, pThreadTask->pSubScheduler, pThreadTask->pSubEnv->GetEnvirName());

	//pThreadTask->pSubEnv->taskScheduler().turnOnBackgroundReadHandling( pThreadTask->subSocket, NULL, pThread->pEx);

	pThread->flag = 0x02;
	while (1)
	{
		if (pThread->flag == 0x03)		break;

		tally ++;

#ifdef _EPOLL_
		BasicTaskSchedulerEpoll	*pTaskChedulerEpoll = NULL;

		if (NULL != pThreadTask->pSubEnv && (pTaskChedulerEpoll = (BasicTaskSchedulerEpoll *)&pThreadTask->pSubEnv->taskScheduler()))
		{
			if (pThreadTask->pSubEnv->IsResetChannel() > 0x00)
			{
				if (pThreadTask->pSubEnv->IsResetChannel() == 0x01 || pThreadTask->pSubEnv->IsResetChannel() == 0x05)
				{
					ServerMediaSession* sms = (ServerMediaSession*)pThreadTask->pSubEnv->GetTmpServerMediaSession();
					char streamName[512] = {0};

					if (pThreadTask->pSubEnv->IsResetChannel() == 0x05)
					{
						FILE *f = fopen("resetchannel[5].txt", "wb");
						if (NULL != f)
						{
							char szLog[128] = {0};
							if (NULL != sms && sms->GetFlag() == LIVE_FLAG)
								sprintf(szLog, "sms: %s\n", sms->streamName());
							else
								sprintf(szLog, "sms is error.\n");
							fwrite(szLog, 1, (int)strlen(szLog), f);
							fclose(f);
						}
					}

					if (NULL != sms && sms->GetFlag() == LIVE_FLAG)
					{
						strcpy(streamName, sms->streamName());

						UsageEnvironment *pEnv = &sms->envir();

						pThreadTask->pSubEnv->ResetChannel(0x02, NULL);

						if (NULL != pEnv)
						{
							pThis->removeServerMediaSession(pEnv, streamName, False);
						}

						pThis->closeAllClientSessionsForServerMediaSession(sms, True);
						pThis->CloseAllConnections(streamName);
					}

					pThreadTask->pSubEnv->ResetChannel(0x00, NULL);
					//pTaskChedulerEpoll->ClearDelayQueue();
				}
				else if (pThreadTask->pSubEnv->IsResetChannel() == 10)
				{
					ServerMediaSession* sms = (ServerMediaSession*)pThreadTask->pSubEnv->GetTmpServerMediaSession();
					//char streamName[512] = {0};
					//if (NULL != sms)
					//{
						//strcpy(streamName, sms->streamName());
					//}

					pThreadTask->pSubEnv->ResetChannel(0x00, NULL);
					//pTaskChedulerEpoll->ClearDelayQueue();
				}

				Sleep(10);
				continue;
			}

			int ret = pTaskChedulerEpoll->SingleStep(0, pThreadTask->id, pThreadTask->pSubEnv->GetEnvirName());
			if (ret < 0)
			{
				//select_error = 0x01;
				//_RTSPCLIENT_TRACE(TRACE_LOG_INFO, (char*)"channel[%d]     pTaskScheduler0->SingleStep(0) Ret:%d \n", pClient->channelInfo.id, ret);
				break;
			}

			if (pThreadTask->releaseChannel > 0)
			{
				pThis->LockServerMediaSession(pThreadTask->pSubEnv->GetEnvirName(), "ResetChannel", (unsigned long long)pThreadTask);

				if (pThreadTask->pSubEnv->GetReferenceCount() < 1)
				{
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel 通道已复位Release=0 streamName[%s]  BEGIN\n",  pThreadTask->pSubEnv->GetEnvirName(), pThreadTask->liveURLSuffix);

					pThis->removeServerMediaSession(pThreadTask->pSubEnv, pThreadTask->liveURLSuffix, False);	//2019.12.30
					pThis->CloseAllConnections(pThreadTask->liveURLSuffix);
					memset(pThreadTask->liveURLSuffix, 0x00, sizeof(pThreadTask->liveURLSuffix));
					pTaskChedulerEpoll->ClearDelayQueue();

					pThreadTask->pSubEnv->SetStreamStatus(1);
					pThreadTask->useStatus = 0x00;
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel 通道已复位Release=0 streamName[%s]  END\n",  pThreadTask->pSubEnv->GetEnvirName(), pThreadTask->liveURLSuffix);

				}
				else
				{
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel Fail: chReferenceCount[%d]>0 streamName[%s]\n",  pThreadTask->pSubEnv->GetEnvirName(), pThreadTask->pSubEnv->GetReferenceCount(), pThreadTask->liveURLSuffix);
				}

				pThreadTask->releaseChannel = 0x00;

				pThis->UnlockServerMediaSession(pThreadTask->pSubEnv->GetEnvirName(), "ResetChannel", (unsigned long long)pThreadTask);
			}
		}
#else
		BasicTaskScheduler0 *pTaskScheduler0 = NULL;

		if (NULL != pThreadTask->pSubEnv && (pTaskScheduler0 = (BasicTaskScheduler0 *)&pThreadTask->pSubEnv->taskScheduler()))
		{
			if (pThreadTask->pSubEnv->IsResetChannel() > 0x00)
			{
				pThreadTask->pSubEnv->LockEnvir("pThreadTask->pSubEnv->IsResetChannel() > 0x00", (unsigned long long)pThreadTask);

				if (pThreadTask->pSubEnv->IsResetChannel() == 0x01 || pThreadTask->pSubEnv->IsResetChannel() == 0x05)
				{
					ServerMediaSession* sms = (ServerMediaSession*)pThreadTask->pSubEnv->GetTmpServerMediaSession();
					char streamName[512] = {0};

					if (pThreadTask->pSubEnv->IsResetChannel() == 0x05)
					{
						FILE *f = fopen("resetchannel[5].txt", "wb");
						if (NULL != f)
						{
							char szLog[128] = {0};
							if (NULL != sms && sms->GetFlag() == LIVE_FLAG)
								sprintf(szLog, "sms: %s\n", sms->streamName());
							else
								sprintf(szLog, "sms is error.\n");
							fwrite(szLog, 1, (int)strlen(szLog), f);
							fclose(f);
						}
					}

					if (NULL != sms && sms->GetFlag() == LIVE_FLAG)
					{
						strcpy(streamName, sms->streamName());

						UsageEnvironment *pEnv = &sms->envir();

						pThreadTask->pSubEnv->ResetChannel(0x02, NULL);

						if (NULL != pEnv)
						{
							pThis->removeServerMediaSession(pEnv, streamName, False);
						}

						pThis->closeAllClientSessionsForServerMediaSession(sms, True);
						pThis->CloseAllConnections(streamName);
					}
					pThreadTask->pSubEnv->ResetChannel(0x00, NULL);
					//pTaskScheduler0->ClearDelayQueue();
				}
				else if (pThreadTask->pSubEnv->IsResetChannel() == 10)
				{
					ServerMediaSession* sms = (ServerMediaSession*)pThreadTask->pSubEnv->GetTmpServerMediaSession();
					//char streamName[512] = {0};
					//if (NULL != sms)
					//{
					//	strcpy(streamName, sms->streamName());
					//}

					pThreadTask->pSubEnv->ResetChannel(0x00, NULL);
					//pTaskScheduler0->ClearDelayQueue();
				}

				pThreadTask->pSubEnv->UnlockEnvir("pThreadTask->pSubEnv->IsResetChannel() > 0x00", (unsigned long long)pThreadTask);

				Sleep(10);
				continue;
			}

			pThreadTask->pSubEnv->LockEnvir("pTaskScheduler0->SingleStep", 0ll);

			int ret = pTaskScheduler0->SingleStep(0, pThreadTask->id, pThreadTask->pSubEnv->GetEnvirName());
			if (ret < 0)
			{
				//select_error = 0x01;
				//_RTSPCLIENT_TRACE(TRACE_LOG_INFO, (char*)"channel[%d]     pTaskScheduler0->SingleStep(0) Ret:%d \n", pClient->channelInfo.id, ret);

				//pThreadTask->pSubEnv->Unlock();

				break;
			}
			pThreadTask->pSubEnv->UnlockEnvir("pTaskScheduler0->SingleStep", 0ll);

			if (pThreadTask->releaseChannel > 0)
			{
				pThis->LockServerMediaSession(pThreadTask->pSubEnv->GetEnvirName(), "ResetChannel", (unsigned long long)pThreadTask);

				if (pThreadTask->pSubEnv->GetReferenceCount() < 1)
				{
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel 通道已复位Release=0 streamName[%s]  BEGIN\n",  
																pThreadTask->pSubEnv->GetEnvirName(), 
																pThreadTask->liveURLSuffix);

					pThis->removeServerMediaSession(pThreadTask->pSubEnv, pThreadTask->liveURLSuffix, False);	//2019.12.30
					pThis->CloseAllConnections(pThreadTask->liveURLSuffix);
					memset(pThreadTask->liveURLSuffix, 0x00, sizeof(pThreadTask->liveURLSuffix));
					pTaskScheduler0->ClearDelayQueue();
					pThreadTask->pSubEnv->SetStreamStatus(1);
					pThreadTask->useStatus = 0x00;

					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel 通道已复位Release=0 streamName[%s]  END\n",  
																pThreadTask->pSubEnv->GetEnvirName(), 
																pThreadTask->liveURLSuffix);
				}
				else
				{
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] ResetChannel Fail: chReferenceCount[%d]>0 streamName[%s]\n",  
																pThreadTask->pSubEnv->GetEnvirName(), 
																pThreadTask->pSubEnv->GetReferenceCount(), 
																pThreadTask->liveURLSuffix);
				}

				pThreadTask->releaseChannel = 0x00;

				pThis->UnlockServerMediaSession(pThreadTask->pSubEnv->GetEnvirName(), "ResetChannel", (unsigned long long)pThreadTask);
			}
		}
#endif

		if (pThreadTask->id == pThis->GetTaskFullThreadId())
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"[%s]  Task is full.\n", pThreadTask->pSubEnv->GetEnvirName());
		}

		if (tally >= 20000)
		{
#ifdef _DEBUG1
			if (lastTime > 0U)
			{
				_TRACE(TRACE_LOG_DEBUG, (char*)"检测间隔: %u\n", (unsigned int)time(NULL) - lastTime);
			}
			lastTime = (unsigned int)time(NULL);
#endif
			int tmpEnvirCount = pThreadTask->pSubEnv->GetEnvirCount();
			if (( tmpEnvirCount>= 0) && (tmpEnvirCount == envirCount) )
			{
				//回调
				RTSPSvrCallBack	pCallback = (RTSPSvrCallBack)pThis->GetCallbackPtr();
				if (NULL != pCallback)
				{
					pCallback(RTSP_CHANNEL_FIND_STREAM, pThreadTask->liveURLSuffix, NULL, NULL, NULL, pThis->GetUserPtr(), NULL);
				}
			}
			tally = 0;
			envirCount = pThreadTask->pSubEnv->GetEnvirCount();
		}

		//if (pThreadTask->handleDescribe > 0x00)
		if (pThreadTask->clientConnectionNum > 0)
		{
			pThreadTask->pSubEnv->LockEnvir("pThreadTask->clientNum > 0", (unsigned long long)pThreadTask);

			bool bProc = false;
			if (pThreadTask->pSubEnv->GetLockFlag() == 0x00)
			{
				for (int k=0; k<MAX_BATCH_CLIENT_NUM; k++)
				{
					if (pThread->flag == 0x03)		break;

					if (NULL != pThreadTask->pClientConnectionPtr[k])
					{
						GenericMediaServer::ClientConnection *pConnection = (GenericMediaServer::ClientConnection *)pThreadTask->pClientConnectionPtr[k];
						if (NULL == pConnection)		continue;
						//pConnection->incomingRequestHandler(pConnection, 0);

						int addSocketRet = pConnection->OnIncomingRequestHandler(pThreadTask->pSubEnv, 0, 0);		//将socket添加到对应线程中
						if (addSocketRet == 0)
						{
							pConnection->OnHandleRequestBytes(pConnection->GetRecvBytes(), pThreadTask->pSubEnv);
						}
						else
						{
							pConnection->OnHandleRequestBytes(-1, pThreadTask->pSubEnv);
						}
						//_TRACE(TRACE_LOG_DEBUG, (char*)"[%s]  [%s] Process Index[%d]\n", pThreadTask->pSubEnv->GetEnvirName(), pThreadTask->urlSuffix, k);

						pThreadTask->pClientConnectionPtr[k] = NULL;

						if (pThreadTask->clientConnectionNum>0)
						{
							pThreadTask->clientConnectionNum --;

							if (pThreadTask->clientConnectionNum < 1)
							{
								//memset(pThreadTask->urlSuffix, 0x00, sizeof(pThreadTask->urlSuffix));

								pThreadTask->releaseChannel = 0x01;		//此处置为1不会销毁, 因为在销毁前会判断引用计数是否为0
							}
						}

						bProc = true;
					}
				}

				if (! bProc)
				{
					pThreadTask->clientConnectionNum = 0;

					_TRACE(TRACE_LOG_WARNING, (char *)"##########[%s] 当前客户端计数为[%d], 但数组中没有发现有效值, 将计数重置为0.\n", pThreadTask->pSubEnv->GetEnvirName(), pThreadTask->clientConnectionNum);
				}
			}
			else
			{
				_TRACE(TRACE_LOG_WARNING, (char *)"########## Worker Thread[%s] 当前被锁定,不能进行当前操作.\n", pThreadTask->pSubEnv->GetEnvirName());
			}

			pThreadTask->pSubEnv->UnlockEnvir("pThreadTask->clientNum > 0", (unsigned long long)pThreadTask);

			//pThreadTask->handleDescribe = 0;
		}

		if (pThreadTask->clientDisconnectNum > 0)
		{
				for (int k=0; k<MAX_BATCH_CLIENT_NUM; k++)
				{
					if (pThread->flag == 0x03)		break;

					if (NULL != pThreadTask->pClientDisconnectPtr[k])
					{
						GenericMediaServer::ClientConnection *pConnection = (GenericMediaServer::ClientConnection *)pThreadTask->pClientDisconnectPtr[k];
						if (NULL == pConnection)		continue;

						if (pConnection->GetFlag() == LIVE_FLAG)
						{
							pConnection->CloseConnection(0);
						}

						pThreadTask->clientDisconnectNum --;
						pThreadTask->pClientDisconnectPtr[k] = NULL;
					}

				}
		}

		//if (pThreadTask->subSocket > 0)
		//{
			//GenericMediaServer::ClientConnection	*pClientConnection = (GenericMediaServer::ClientConnection *)pThreadTask->pClientConnectionPtr;
			//if (NULL != pClientConnection)
			//{
				//pClientConnection->OnIncomingRequestHandler(pThreadTask->pSubEnv, 0, 0);
			//}

			//pThreadTask->pSubEnv->taskScheduler().turnOnBackgroundReadHandling(pThreadTask->subSocket,  (TaskScheduler::BackgroundHandlerProc*)GenericMediaServer::incomingConnectionHandler, pClientConnection);
			//pThreadTask->pSubEnv->taskScheduler().turnOnBackgroundReadHandling(pThreadTask->subSocket,  (TaskScheduler::BackgroundHandlerProc*)pClientConnection->incomingRequestHandler, pClientConnection);
			//pThreadTask->pSubEnv->taskScheduler().turnOnBackgroundReadHandling(pThreadTask->subSocket,  (TaskScheduler::BackgroundHandlerProc*)&ClientConnection::incomingRequestHandler, pThreadTask->extPtr);
			//pThreadTask->subSocket = 0;
		//}
	}

	_TRACE(TRACE_LOG_WARNING, (char *)"Worker Thread[%d] Exit...\n", pThreadTask->id);

	pThread->flag = 0x00;

#ifdef ANDROID
	g_vm->DetachCurrentThread();
#endif

#ifdef _WIN32
	ExitThread(0);
#endif
	return 0;
}
#endif

GenericMediaServer
::GenericMediaServer(UsageEnvironment& env, int ourSocketV4, int ourSocketV6, Port ourPort,
		     unsigned reclamationSeconds, void *_callback, void *_userptr)
  : Medium(env),
    fServerSocket4(ourSocketV4), fServerSocket6(ourSocketV6), 
	fServerPort(ourPort), fReclamationSeconds(reclamationSeconds),
    fServerMediaSessions(HashTable::create(STRING_HASH_KEYS)),
    fClientConnections(HashTable::create(ONE_WORD_HASH_KEYS)),
    fClientSessions(HashTable::create(STRING_HASH_KEYS)) {
  ignoreSigPipeOnSocket(fServerSocket4); // so that clients on the same host that are killed don't also kill us

  if (fServerSocket6 > 0)
  {
	ignoreSigPipeOnSocket(fServerSocket6); // so that clients on the same host that are killed don't also kill us
  }


#ifdef LIVE_MULTI_THREAD_ENABLE

  InitMutex(&mutexServerMediaSession);
  InitMutex(&mutexClientConnection);
  InitMutex(&mutexClientSession);
  InitMutex(&mutexClose);
  
  mCallbackPtr	=	_callback;
  mUserPtr		=	_userptr;

  taskFullThreadId = -1;

  memset(&multiThreadCore, 0x00, sizeof(MultiThread_CORE_T));
  multiThreadCore.threadNum = MAX_DEFAULT_MULTI_THREAD_NUM;
  multiThreadCore.threadTask = new LIVE_THREAD_TASK_T[multiThreadCore.threadNum];
  memset(&multiThreadCore.threadTask[0], 0x00, sizeof(LIVE_THREAD_TASK_T) * multiThreadCore.threadNum);
  for (int i=0; i<multiThreadCore.threadNum; i++)
  {
	  char szName[36] = {0};
	  sprintf(szName, "worker thread %d", i+1);
	  multiThreadCore.threadTask[i].id = i;
	  multiThreadCore.threadTask[i].extPtr = this;
#ifdef _EPOLL_
	  multiThreadCore.threadTask[i].pSubScheduler = BasicTaskSchedulerEpoll::createNew(i+1, MAX_EPOLL_WORKER_THREAD_EVENT);
#else
	  multiThreadCore.threadTask[i].pSubScheduler = BasicTaskScheduler::createNew(i+1, MAX_EPOLL_WORKER_THREAD_EVENT);
#endif
	  multiThreadCore.threadTask[i].pSubEnv = BasicUsageEnvironment::createNew(*multiThreadCore.threadTask[i].pSubScheduler, i+1, szName);
	  
#if 0
	  while (1)
	  {
		  portNumBits port = 7600+ (unsigned char)rand();
		  multiThreadCore.threadTask[i].subSocket = setUpOurSocket(*multiThreadCore.threadTask[i].pSubEnv, (Port&)port);
		  if (multiThreadCore.threadTask[i].subSocket == -1)
		  {
			  ::closeSocket(multiThreadCore.threadTask[i].subSocket);
			  printf("Create sub thread failed.\n");
		  }
		  else
		  {
			  multiThreadCore.threadTask[i].port = port;
			  break;
		  }
	  }
	  //multiThreadCore.threadTask[i].pSubEnv->taskScheduler().turnOnBackgroundReadHandling( multiThreadCore.threadTask[i].subSocket, \
		  //NULL, this);
		  //(TaskScheduler::BackgroundHandlerProc*)incomingConnectionHandler, this);
#endif


	  //2018.04.27  待有请求时再创建
	  //CreateOSThread( &multiThreadCore.threadTask[i].osThread, __WorkerThread_Proc, (void *)&multiThreadCore.threadTask[i] );
	  

  }
#endif

  // Arrange to handle connections from others:
  env.taskScheduler().turnOnBackgroundReadHandling(fServerSocket4, incomingConnectionHandler4, this);
  if (fServerSocket6 > 0)
  {
	env.taskScheduler().turnOnBackgroundReadHandling(fServerSocket6, incomingConnectionHandler6, this);
  }
}

GenericMediaServer::~GenericMediaServer() {

  mCallbackPtr	=	NULL;
  mUserPtr		=	NULL;

  // Turn off background read handling:
  envir().taskScheduler().turnOffBackgroundReadHandling(fServerSocket4);
  if (fServerSocket6 > 0)	envir().taskScheduler().turnOffBackgroundReadHandling(fServerSocket6);
  ::closeSocket(fServerSocket4);
  if (fServerSocket6 > 0)	::closeSocket(fServerSocket6);

  deleteAllWorkerThread(2);

  DeinitMutex(&mutexClientSession);
  DeinitMutex(&mutexClientConnection);
  DeinitMutex(&mutexServerMediaSession);
  DeinitMutex(&mutexClose);
}

void	GenericMediaServer::deleteAllWorkerThread(int flag)
{

#ifdef LIVE_MULTI_THREAD_ENABLE
  if (NULL != multiThreadCore.threadTask)
  {
	  for (int i=0; i<multiThreadCore.threadNum; i++)
	  {
		  if (flag & 1)
		  {
			DeleteOSThread(&multiThreadCore.threadTask[i].osThread);
		  }
		  else if (flag & 2)
		  {
#ifdef _EPOLL_
			  BasicTaskSchedulerEpoll *pTaskSchedulerEpoll = NULL;
			  if (NULL != multiThreadCore.threadTask[i].pSubEnv && (pTaskSchedulerEpoll = (BasicTaskSchedulerEpoll *)&multiThreadCore.threadTask[i].pSubEnv->taskScheduler()))
#else
			BasicTaskScheduler0 *pTaskScheduler0 = NULL;
			if (NULL != multiThreadCore.threadTask[i].pSubEnv && (pTaskScheduler0 = (BasicTaskScheduler0 *)&multiThreadCore.threadTask[i].pSubEnv->taskScheduler()))
#endif
			
			{
				//for (int i=0; i<10; i++)
				{
					//int ret = pTaskScheduler0->SingleStep(0);
				}
			}

			  if (NULL != multiThreadCore.threadTask[i].pSubEnv)
			  {
				  //if (multiThreadCore.threadTask[i].subSocket>0)
				  {
					//multiThreadCore.threadTask[i].pSubEnv->taskScheduler().turnOffBackgroundReadHandling(multiThreadCore.threadTask[i].subSocket);
				  }

				  multiThreadCore.threadTask[i].pSubEnv->reclaim();
				  multiThreadCore.threadTask[i].pSubEnv = NULL;
			  }
			  if (NULL != multiThreadCore.threadTask[i].pSubScheduler)
			  {
				  delete multiThreadCore.threadTask[i].pSubScheduler;
				  multiThreadCore.threadTask[i].pSubScheduler = NULL;
			  }
		  }
	  }

	  if (flag & 2)
	  {
		  delete []multiThreadCore.threadTask;
		  multiThreadCore.threadTask = NULL;
	  }
  }
#endif
}

void GenericMediaServer::cleanup() {
  // This member function must be called in the destructor of any subclass of
  // "GenericMediaServer".  (We don't call this in the destructor of "GenericMediaServer" itself,
  // because by that time, the subclass destructor will already have been called, and this may
  // affect (break) the destruction of the "ClientSession" and "ClientConnection" objects, which
  // themselves will have been subclassed.)

	//LockClientConnection();

  // Close all client session objects:
  GenericMediaServer::ClientSession* clientSession;
  while ((clientSession = (GenericMediaServer::ClientSession*)fClientSessions->getFirst()) != NULL) {
    delete clientSession;
  }
  if (NULL != fClientSessions)
  {
	  delete fClientSessions;
	  fClientSessions = NULL;
  }
  
  // Close all client connection objects:
  GenericMediaServer::ClientConnection* connection;
  while ((connection = (GenericMediaServer::ClientConnection*)fClientConnections->getFirst()) != NULL) {
    delete connection;
  }
  if (NULL != fClientConnections)
  {
	delete fClientConnections;
	fClientConnections = NULL;
  }
  
  // Delete all server media sessions
  ServerMediaSession* serverMediaSession;
  while ((serverMediaSession = (ServerMediaSession*)fServerMediaSessions->getFirst()) != NULL) {
	  removeServerMediaSession(&serverMediaSession->envir(), serverMediaSession, False); // will delete it, because it no longer has any 'client session' objects using it
  }
  if (NULL != fServerMediaSessions)
  {
	delete fServerMediaSessions;
	fServerMediaSessions = NULL;
  }


  //UnlockClientConnection();
}

#define LISTEN_BACKLOG_SIZE 20

int GenericMediaServer::setUpOurSocket4(UsageEnvironment& env, Port& ourPort) {
  int ourSocket = -1;
  
  do {
    // The following statement is enabled by default.
    // Don't disable it (by defining ALLOW_SERVER_PORT_REUSE) unless you know what you're doing.
#if !defined(ALLOW_SERVER_PORT_REUSE) && !defined(ALLOW_RTSP_SERVER_PORT_REUSE)
    // ALLOW_RTSP_SERVER_PORT_REUSE is for backwards-compatibility #####
    NoReuse dummy(env); // Don't use this socket if there's already a local server using it
#endif
    
    ourSocket = setupStreamSocket4(env, ourPort);
    if (ourSocket < 0) break;
    
    // Make sure we have a big send buffer:
    //if (!increaseSendBufferTo(env, ourSocket, 50*1024)) break;
	if (!increaseSendBufferTo(env, ourSocket, 512*1024)) break;
    
    // Allow multiple simultaneous connections:
    if (listen(ourSocket, LISTEN_BACKLOG_SIZE) < 0) {
      env.setResultErrMsg("listen() failed: ");
      break;
    }
    
    if (ourPort.num() == 0) {
      // bind() will have chosen a port for us; return it also:
      if (!getSourcePort(env, ourSocket, ourPort, LIVE_IP_VER_4)) break;
    }
    
    return ourSocket;
  } while (0);
  
  if (ourSocket != -1) ::closeSocket(ourSocket);
  return -1;
}
int GenericMediaServer::setUpOurSocket6(UsageEnvironment& env, Port& ourPort) {
  int ourSocket = -1;
  
  do {
    // The following statement is enabled by default.
    // Don't disable it (by defining ALLOW_SERVER_PORT_REUSE) unless you know what you're doing.
#if !defined(ALLOW_SERVER_PORT_REUSE) && !defined(ALLOW_RTSP_SERVER_PORT_REUSE)
    // ALLOW_RTSP_SERVER_PORT_REUSE is for backwards-compatibility #####
    NoReuse dummy(env); // Don't use this socket if there's already a local server using it
#endif
    
    ourSocket = setupStreamSocket6(env, ourPort);
    if (ourSocket < 0) break;
    
    // Make sure we have a big send buffer:
    //if (!increaseSendBufferTo(env, ourSocket, 50*1024)) break;
	if (!increaseSendBufferTo(env, ourSocket, 512*1024)) break;
    
    // Allow multiple simultaneous connections:
    if (listen(ourSocket, LISTEN_BACKLOG_SIZE) < 0) {
      env.setResultErrMsg("listen() failed: ");
      break;
    }
    
    if (ourPort.num() == 0) {
      // bind() will have chosen a port for us; return it also:
      if (!getSourcePort(env, ourSocket, ourPort, LIVE_IP_VER_6)) break;
    }
    
    return ourSocket;
  } while (0);
  
  if (ourSocket != -1) ::closeSocket(ourSocket);
  return -1;
}


void GenericMediaServer::incomingConnectionHandler4(void* instance, int /*mask*/) {
  GenericMediaServer* server = (GenericMediaServer*)instance;
  server->incomingConnectionHandler4();
}
void GenericMediaServer::incomingConnectionHandler4() {
  incomingConnectionHandlerOnSocket(fServerSocket4, LIVE_IP_VER_4);
}

void GenericMediaServer::incomingConnectionHandler6(void* instance, int /*mask*/) {
  GenericMediaServer* server = (GenericMediaServer*)instance;
  server->incomingConnectionHandler6();
}
void GenericMediaServer::incomingConnectionHandler6() {
  incomingConnectionHandlerOnSocket(fServerSocket6, LIVE_IP_VER_6);
}

void GenericMediaServer::incomingConnectionHandlerOnSocket(int serverSocket, LIVE_IP_VER_ENUM ipVer) {
  LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr;
  SOCKLEN_T clientAddrLen = sizeof clientAddr;
  int clientSocket = (int)accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
  if (clientSocket < 0) {
    int err = envir().getErrno();
    if (err != EWOULDBLOCK) {
      envir().setResultErrMsg("accept() failed: ");
    }
    return;
  }

  //_TRACE(TRACE_LOG_INFO, (char*)"accept New Connection.  socket[%d]\n", clientSocket);

	//========================================================
//#ifdef _DEBUG
	//调试代码
	Boolean socketIsExist = False;
	LockClientConnection();
	HashTable::Iterator* iter = HashTable::Iterator::create(*fClientConnections);
	GenericMediaServer::ClientConnection* connection;
	char const* key; // dummy
	while ((connection = (GenericMediaServer::ClientConnection*)(iter->next(key))) != NULL) 
	{
		if (clientSocket == connection->fOurSocket)
		{
			if (connection->pClientConnectionEnv)
			{
				_TRACE(TRACE_LOG_ERROR, 
								(char*)"#######[ERROR]########accept New Connection.  socket[%d].  The socket already exist[%s]\n",								clientSocket, connection->pClientConnectionEnv->GetEnvirName());

				connection->pClientConnectionEnv->ResetChannel(0x05, NULL);

				if (NULL == connection->pClientConnectionEnv->GetTmpServerMediaSession())
				{
					//需要在此处删除相应的ServerMediaSession
					_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] accept New Connection. removeServerMediaSession:%s  BEGIN\n", 
						connection->pClientConnectionEnv->GetEnvirName(), connection->GetStreamName());

					removeServerMediaSession(connection->pClientConnectionEnv, connection->GetStreamName(), False);

					_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] accept New Connection. removeServerMediaSession:%s  END\n", 
						connection->pClientConnectionEnv->GetEnvirName(), connection->GetStreamName());

				}
			}
			else
			{
				_TRACE(TRACE_LOG_ERROR, 
								(char*)"#######[ERROR]########accept New Connection.  socket[%d].  The socket already exist\n",								clientSocket);
			}

			socketIsExist = True;
			
/*
			int iProc = -1;
			for (int i=0; i<multiThreadCore.threadNum; i++)
			{
				if (multiThreadCore.threadTask[i].pSubEnv == connection->pClientConnectionEnv)
				{
					for (int j=0; j<MAX_BATCH_CLIENT_NUM; j++)
					{
						if (NULL == multiThreadCore.threadTask[i].pClientDisconnectPtr[j])
						{
							multiThreadCore.threadTask[i].pClientDisconnectPtr[j] = connection;
							multiThreadCore.threadTask[i].clientDisconnectNum ++;
							iProc = 0;
							break;
						}
					}
					break;
				}
			}

			if (iProc < 0)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"#######[ERROR][%s] No free clientPtr.\n", connection->pClientConnectionEnv->GetEnvirName());
			}

			//connection->SetLockFlag(0);
			//connection->ResetConnectionTask();
			//connection->handleRequestBytes(-1, connection->pClientConnectionEnv);
*/
			break;
		}
	}
	delete iter;
	UnlockClientConnection();
//#endif
  //=========================================================

	if (socketIsExist)
	{
		closeSocket(clientSocket);
		return;
	}

  ignoreSigPipeOnSocket(clientSocket); // so that clients on the same host that are killed don't also kill us
  makeSocketNonBlocking(clientSocket);
  //increaseSendBufferTo(envir(), clientSocket, 50*1024);
  increaseSendBufferTo(envir(), clientSocket, 512*1024);
  
#ifdef DEBUG
  envir() << "accept()ed connection from " << AddressString(clientAddr.saddr4.sin_addr).val() << "\n";
#endif
  
#ifdef LIVE_MULTI_THREAD_ENABLE

  //LockClientConnection();

  // Create a new object for handling this connection:
  (void)createNewClientConnection(clientSocket, clientAddr, ipVer);

  //UnlockClientConnection();
#else
  // Create a new object for handling this connection:
  (void)createNewClientConnection(clientSocket, clientAddr);
#endif
}

UsageEnvironment *GenericMediaServer::GetEnvBySuffix(UsageEnvironment *pMainThreadEnv, const char *urlSuffix, void *pClientConnection, 
										LIVE_THREAD_TASK_T **pThreadTask, Boolean bLockServerMediaSession)
{
	GenericMediaServer::ClientConnection	*pClient = (GenericMediaServer::ClientConnection *)pClientConnection;

	int iFreeIdx = -1;
	UsageEnvironment *pEnv = NULL;

	if ( (int)strlen(urlSuffix) < 1)
	{
		return NULL;
	}

	char streamName[512] = {0};

	//LockClientConnection();		//此处无需加锁,调用该函数的位置已经加锁

	int		iProcRet = 0;
	Boolean bRequestTooMany = False;
	if (bLockServerMediaSession)
	{
		LockServerMediaSession(pMainThreadEnv->GetEnvirName(), 
					(char*)"GenericMediaServer::GetEnvBySuffix", 
					(unsigned long long)this);
	}

	do
	{
		for (int i=0; i<multiThreadCore.threadNum; i++)
		{
			if ( (iFreeIdx<0) && (multiThreadCore.threadTask[i].useStatus==0x00) &&  
					(((int)strlen(multiThreadCore.threadTask[i].liveURLSuffix) < 1 )) && 
					(multiThreadCore.threadTask[i].releaseChannel==0x00) ) 
			{
				iFreeIdx = i;
			}
			if ( 0 == strcmp(urlSuffix, multiThreadCore.threadTask[i].liveURLSuffix))
			{
				if (multiThreadCore.threadTask[i].releaseChannel>0x00)
				{
					iProcRet = -1;
					_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] 当前通道正在被删除. 请稍候访问: %s\n", 
									multiThreadCore.threadTask[i].pSubEnv->GetEnvirName(), urlSuffix);
					break;
				}

				if (NULL == multiThreadCore.threadTask[i].pSubEnv)
				{
					iProcRet = -2;
					break;
				}

				if (multiThreadCore.threadTask[i].pSubEnv->GetStreamStatus() == 0x00)	//源未就绪
				{
					iProcRet = -3;
					break;
				}


				multiThreadCore.threadTask[i].pSubEnv->LockEnvir("GenericMediaServer::GetEnvBySuffix", 
													(unsigned long long)this);
				if (multiThreadCore.threadTask[i].pSubEnv->GetLockFlag() != 0x00)
				{
					iProcRet = -4;
					multiThreadCore.threadTask[i].pSubEnv->UnlockEnvir("GenericMediaServer::GetEnvBySuffix", 
													(unsigned long long)this);
					break;
				}
				
				bool assignEnv = false;
				for (int k=0; k<MAX_BATCH_CLIENT_NUM; k++)
				{
					if (NULL == multiThreadCore.threadTask[i].pClientConnectionPtr[k])
					{
						assignEnv = true;
						multiThreadCore.threadTask[i].pClientConnectionPtr[k] = pClient;

						_TRACE(TRACE_LOG_INFO, (char*)"GenericMediaServer::GetEnvBySuffix  [%s] set [%d] to Index[%d]\n", urlSuffix, pClient->fOurSocket, k);

						strcpy(streamName, urlSuffix);

						break;
					}
				}

				if (assignEnv)
				{
					pEnv = multiThreadCore.threadTask[i].pSubEnv;
					//multiThreadCore.threadTask[i].subSocket = pClient->fOurSocket;
					pClient->pClientConnectionEnv = multiThreadCore.threadTask[i].pSubEnv;

					//multiThreadCore.threadTask[i].handleDescribe = 0x01;
					//*handleDescribe = &multiThreadCore.threadTask[i].handleDescribe;
					if (NULL != pThreadTask)	*pThreadTask = &multiThreadCore.threadTask[i];

					multiThreadCore.threadTask[i].clientConnectionNum ++;

					pEnv->IncrementReferenceCount();		//增加引用计数

					iProcRet = 0;

					_TRACE(TRACE_LOG_INFO, (char*)"共用通道GenericMediaServer::GetEnvBySuffix:: Channel already exist. New Connection[%d]   [%s][%s] ClientNum[%d]\n",
									pClient->fOurSocket, pClient->pClientConnectionEnv->GetEnvirName(), urlSuffix, 
									multiThreadCore.threadTask[i].clientConnectionNum);
				}
				else
				{
					//没有找到有效的Env, 说明客户端列表已满

					taskFullThreadId = multiThreadCore.threadTask[i].id;

					iProcRet = -10;
					_TRACE(TRACE_LOG_ERROR, (char*)"GenericMediaServer::GetEnvBySuffix 当前通道客户端已满[%s]\n", urlSuffix);
				}

				multiThreadCore.threadTask[i].pSubEnv->UnlockEnvir("GenericMediaServer::GetEnvBySuffix", (unsigned long long)this);


#ifdef _DEBUG
				{
					int tally = 0;
					for (int i=0; i<multiThreadCore.threadNum; i++)
					{
						if ( 0 == strcmp(urlSuffix, multiThreadCore.threadTask[i].liveURLSuffix))
						{
							tally ++;
							_TRACE(TRACE_LOG_ERROR, (char*)"%s\n", multiThreadCore.threadTask[i].pSubEnv->GetEnvirName());
						}
					}

					if (tally > 1)
					{
						tally = tally;
					}
				}

#endif

				//envir().taskScheduler().disableBackgroundHandling(pClient->fOurSocket);
				//pClient->pEnv->taskScheduler().turnOnBackgroundReadHandling(pClient->fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);

				break;
			}
		}
		if (pEnv)			break;
		if (iFreeIdx<0)		break;

		if (iProcRet < 0)	break;

		multiThreadCore.threadTask[iFreeIdx].useStatus = 0x01;		//使用中

		if (NULL == multiThreadCore.threadTask[iFreeIdx].osThread)
		{
			CreateOSThread( &multiThreadCore.threadTask[iFreeIdx].osThread, __WorkerThread_Proc, (void *)&multiThreadCore.threadTask[iFreeIdx] );
		}

		
		multiThreadCore.threadTask[iFreeIdx].pClientConnectionPtr[0] = pClient;
		//multiThreadCore.threadTask[iFreeIdx].subSocket = pClient->fOurSocket;
		pClient->pClientConnectionEnv = multiThreadCore.threadTask[iFreeIdx].pSubEnv;

#ifdef _DEBUG
		for (int i=0; i<multiThreadCore.threadNum; i++)
		{
			if ( (int)strlen(multiThreadCore.threadTask[i].liveURLSuffix) > 0)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"通道列表[%d:%s]: %s\n", i, multiThreadCore.threadTask[i].pSubEnv->GetEnvirName(),  multiThreadCore.threadTask[i].liveURLSuffix);

				if ( (0 == strcmp(urlSuffix, multiThreadCore.threadTask[i].liveURLSuffix)) )
				{
					 multiThreadCore.threadTask[i].releaseChannel = multiThreadCore.threadTask[i].releaseChannel;
				}
			}
		}
#endif

		pEnv = pClient->pClientConnectionEnv;

		if ( (int)strlen(urlSuffix) < 1)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"####  urlSuffix 为空.  异常..\n");
		}

		strcpy(multiThreadCore.threadTask[iFreeIdx].liveURLSuffix, urlSuffix);

		strcpy(streamName, multiThreadCore.threadTask[iFreeIdx].liveURLSuffix);

		pEnv->IncrementReferenceCount();		//增加引用计数

		//envir().taskScheduler().disableBackgroundHandling(pClient->fOurSocket);
		//pClient->pEnv->taskScheduler().turnOnBackgroundReadHandling(pClient->fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);

		//multiThreadCore.threadTask[iFreeIdx].handleDescribe = 0x01;

		//*handleDescribe = &multiThreadCore.threadTask[iFreeIdx].handleDescribe;
		if (NULL != pThreadTask)	*pThreadTask = &multiThreadCore.threadTask[iFreeIdx];

		multiThreadCore.threadTask[iFreeIdx].clientConnectionNum ++;

		_TRACE(TRACE_LOG_INFO, (char*)"新建通道  GenericMediaServer::GetEnvBySuffix New Connection[%d] [%s][%s] ClientNum[%d]\n",
						pClient->fOurSocket, pClient->pClientConnectionEnv->GetEnvirName(), 
						multiThreadCore.threadTask[iFreeIdx].liveURLSuffix, 
						multiThreadCore.threadTask[iFreeIdx].clientConnectionNum);
	}while (0);

	if (bLockServerMediaSession)		UnlockServerMediaSession(pMainThreadEnv->GetEnvirName(), "GenericMediaServer::GetEnvBySuffix", (unsigned long long)this);

	//UnlockClientConnection();

	if (NULL != pEnv)
	{
		if ( (int)strlen(streamName) < 1)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"#### ERROR\n");
		}
	}

	return pEnv;
}


UsageEnvironment *GenericMediaServer::FindEnvByStreamName(const char *streamName)		//gavin 2019.02.12
{
	UsageEnvironment *pEnv = NULL;

	if (NULL == streamName)					return NULL;
	if ( (int)strlen(streamName) < 1 )		return NULL;

	//LockClientConnection();		//此处无需加锁,调用该函数的位置已经加锁

	do
	{
		for (int i=0; i<multiThreadCore.threadNum; i++)
		{
			if ( 0 == strcmp(streamName, multiThreadCore.threadTask[i].liveURLSuffix) )
			{
				pEnv = multiThreadCore.threadTask[i].pSubEnv;
				break;
			}
		}

	}while (0);

	return pEnv;
}


int	GenericMediaServer::RemoveSocketFromEnv(UsageEnvironment *pEnv, int sockfd)
{

	return 0;
}



////////// GenericMediaServer::ClientConnection implementation //////////

GenericMediaServer::ClientConnection
::ClientConnection(GenericMediaServer& ourServer, int clientSocket, LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr, LIVE_IP_VER_ENUM ipVer)
  : fOurServer(ourServer), fOurSocket(clientSocket), fClientAddr(clientAddr), fOurIPVer(ipVer), pClientConnectionEnv(NULL),initSessionFlag(0), fLivenessConnectionTask(NULL) {
  // Add ourself to our 'client connections' table:

  recvBytes = 0;
  memset(mStreamName, 0x00, sizeof(mStreamName));
  mFlag = LIVE_FLAG;
  lockFlag  =   0x01;
  // Arrange to handle incoming requests:
  resetRequestBuffer();
#ifdef LIVE_MULTI_THREAD_ENABLE1
  //fOurServer.multiThreadCore.threadTask[0].pSubEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket,	
 //	  (TaskScheduler::BackgroundHandlerProc*)&ClientConnection::incomingRequestHandler, this);

  //int threadIdx = 0;
  while (1)
  {
	  //int threadIdx = rand() % fOurServer.multiThreadCore.threadNum;

	  static int threadIdx = 0;
	  if (threadIdx >= fOurServer.multiThreadCore.threadNum)	threadIdx=0;

	  if (fOurServer.multiThreadCore.threadTask[threadIdx].subSocket < 1)
	  {
		  fOurServer.multiThreadCore.threadTask[threadIdx].pClientConnectionPtr = this;
			//fOurServer.multiThreadCore.threadTask[threadIdx].procPtr = (void *)&ClientConnection::incomingRequestHandler;
			fOurServer.multiThreadCore.threadTask[threadIdx].subSocket = fOurSocket;
			pEnv = fOurServer.multiThreadCore.threadTask[threadIdx].pSubEnv;

			::increaseSendBufferTo(*pEnv, clientSocket, 512*1024);


			_TRACE(TRACE_LOG_INFO, (char*)"New Connection[%d]   threadIdx[%d] [%s] \n",fOurSocket, threadIdx, pEnv->GetEnvirName());
			//pEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
			//pEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
			//pEnv->taskScheduler().setBackgroundHandling(fOurSocket, SOCKET_READABLE|SOCKET_EXCEPTION, incomingRequestHandler, this);

			//RTSPServer::
			//fOurServer.multiThreadCore.threadTask[threadIdx].pSubScheduler->setBackgroundHandling(fOurSocket, SOCKET_READABLE|SOCKET_EXCEPTION, GenericMediaServer::ClientConnection::incomingRequestHandler, this);

			threadIdx ++;

			break;
	  }
  }
  
  if (NULL != pEnv)
  {
	  fOurServer.fClientConnections->Add((char const*)this, this);
  }
  
  //fOurServer.multiThreadCore.threadTask[threadIdx].pSubScheduler->setBackgroundHandling(fOurSocket, SOCKET_READABLE|SOCKET_EXCEPTION, incomingRequestHandler, this);

  

	//fOurServer.multiThreadCore.threadTask[threadIdx].pSubEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket,	\
	  //(TaskScheduler::BackgroundHandlerProc*)&ClientConnection::incomingRequestHandler, this);
#else

  char szKey[32+1] = {0};
  snprintf(szKey, sizeof szKey, "%016X", this);

  //fOurServer.fClientConnections->Add(szKey, this);
  fOurServer.fClientConnections->Add((char const*)this, this);

  // Arrange to handle incoming requests:
  //envir().taskScheduler().setBackgroundHandling(fOurSocket, SOCKET_READABLE|SOCKET_EXCEPTION, incomingRequestHandler, this);
  fOurServer.envir().taskScheduler().setBackgroundHandling(fOurSocket, SOCKET_READABLE|SOCKET_EXCEPTION, incomingRequestHandler, this);
#endif
}

GenericMediaServer::ClientConnection::~ClientConnection() {

	if (GetFlag() != LIVE_FLAG)		return;

    if (lockFlag == 0x01)
		fOurServer.LockClientConnection();

	if (NULL != pClientConnectionEnv && fOurSocket>0)
	{
		//pEnv->taskScheduler().turnOffBackgroundReadHandling(fOurSocket);		//gavin
	}
	else if (fOurSocket > 0)
	{
		//envir().taskScheduler().turnOffBackgroundReadHandling(fOurSocket);
	}

	// Remove ourself from the server's 'client connections' hash table before we go:

	char szKey[32+1] = {0};
	snprintf(szKey, sizeof szKey, "%016X", this);
	//fOurServer.fClientConnections->Remove(szKey);
	fOurServer.fClientConnections->Remove((char const*)this);
  
	closeSockets();

	if (NULL != pClientConnectionEnv && initSessionFlag==0x00)
	{
		pClientConnectionEnv->DecrementReferenceCount();
	}

	if (pClientConnectionEnv)
	{
		if (NULL != fLivenessConnectionTask)
		{
			pClientConnectionEnv->taskScheduler().unscheduleDelayedTask(fLivenessConnectionTask);
		}
	}  
	  pClientConnectionEnv = NULL;


  if (lockFlag == 0x01)
        fOurServer.UnlockClientConnection();

  mFlag = 0;
}

void GenericMediaServer::ClientConnection::closeSockets() {
  // Turn off background handling on our socket:

	UsageEnvironment	*pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)
	{
		pTmpEnv = &fOurServer.envir();
	}
	if (NULL != pTmpEnv)		pTmpEnv->taskScheduler().disableBackgroundHandling(fOurSocket);
	fOurServer.envir().taskScheduler().disableBackgroundHandling(fOurSocket);		//2018.03.29

	if (fOurSocket < 0)		return;

	if (pClientConnectionEnv)
	{
		_TRACE(TRACE_LOG_WARNING, (char*)"[%s]GenericMediaServer::ClientConnection::closeSockets(). disableBackgroundHandling  fOurSocket[%d]\n", pTmpEnv->GetEnvirName(), fOurSocket);
	}
	else
	{
		_TRACE(TRACE_LOG_WARNING, (char*)"GenericMediaServer::ClientConnection::closeSockets(). disableBackgroundHandling  fOurSocket[%d] 已被释放. pEnv为空\n", fOurSocket);
	}

  if (fOurSocket>= 0) ::closeSocket(fOurSocket);

  fOurSocket = -1;
}

//2019.09.29	和CloseAllConnections有冲突，所以放在此处
void			GenericMediaServer::ClientConnection::ResetConnectionTask()
{
	if (NULL != pClientConnectionEnv)
	{
		if (NULL != fLivenessConnectionTask)
		{
			pClientConnectionEnv->taskScheduler().unscheduleDelayedTask(fLivenessConnectionTask);
		}
	}
}
void			GenericMediaServer::ClientConnection::noteLivenessConnection()
{
	if (NULL != pClientConnectionEnv)
	{
		fLivenessConnectionTask = pClientConnectionEnv->taskScheduler().scheduleDelayedTask(1000*1000*5,  (TaskFunc*)CloseConnectionTask, this);
	}
}

//gavin 2019.09.18  关闭任务
void GenericMediaServer::ClientConnection::CloseConnectionTask(ClientConnection *clientConnection)
{
	if (NULL == clientConnection)			return;
	if (clientConnection->GetFlag() != LIVE_FLAG)		return;

	clientConnection->handleRequestBytes(-1, clientConnection->pClientConnectionEnv);
}

//关闭client connection
void	GenericMediaServer::ClientConnection::CloseConnection(int lockFlag)
{
	SetLockFlag(lockFlag);
	ResetConnectionTask();
	handleRequestBytes(-1, pClientConnectionEnv);
}


int 	GenericMediaServer::ClientConnection::OnIncomingRequestHandler(UsageEnvironment *pEnv, void *, int)		//gavin
{
	return pEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket, (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
}
void GenericMediaServer::ClientConnection::incomingRequestHandler(void* instance, int /*mask*/) {
  ClientConnection* connection = (ClientConnection*)instance;
  connection->incomingRequestHandler();
}
void	GenericMediaServer::ClientConnection::OnHandleRequestBytes(int newBytesRead, UsageEnvironment *pEnv)
{
	//if (newBytesRead < 1)		return;

	if (GetFlag() != LIVE_FLAG)		return;
	
	handleRequestBytes(newBytesRead, pEnv);
}
void GenericMediaServer::ClientConnection::incomingRequestHandler() 
{
	LIVE_NET_ADDRESS_SOCKADDR_IN dummy; // 'from' address, meaningless in this case
  
	UsageEnvironment *pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)	pTmpEnv = &fOurServer.envir();
	int bytesRead = 0;

	bytesRead = readSocket(*pTmpEnv, fOurSocket, &fRequestBuffer[fRequestBytesAlreadySeen], fRequestBufferBytesLeft, dummy);
	recvBytes = bytesRead;
	handleRequestBytes(bytesRead, pTmpEnv);

  //gavin 2018.03.01    此处代码之后, 不能再访问其内部的任何资源, 因为该ClientConnection可能已经被删除

  //if (bytesRead < 0)
  //{
	  //_TRACE(TRACE_LOG_WARNING, (char*)"Client disconnect.  fOurSocket[%d]\n", fOurSocket);
  //}

  
}

void GenericMediaServer::ClientConnection::resetRequestBuffer() {
  fRequestBytesAlreadySeen = 0;
  fRequestBufferBytesLeft = sizeof fRequestBuffer;
}


////////// GenericMediaServer::ClientSession implementation //////////

GenericMediaServer::ClientSession::ClientSession(GenericMediaServer& ourServer, u_int32_t sessionId, UsageEnvironment	*pEnv)
  : fOurServer(ourServer), fOurSessionId(sessionId), fOurServerMediaSession(NULL),
    fLivenessCheckTask(NULL), pEnvironment(pEnv), lockClientFlag(True), pClientConnection(NULL) {

	if (pEnvironment->GetEnvirId() == MAIN_THREAD_ID)
	{
		_TRACE(TRACE_LOG_DEBUG, (char*)"############### ClientSession::ClientSession  分配错误....ERROR..\n");
	}

	//_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] GenericMediaServer::ClientSession::ClientSession() BEGIN...\n", pEnvironment->GetEnvirName());

	mFlag = LIVE_FLAG;
	noteLiveness();

	//pEnvironment->IncrementReferenceCount();		//增加引用计数

	//_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] GenericMediaServer::ClientSession::ClientSession() END...\n", pEnvironment->GetEnvirName());
}

GenericMediaServer::ClientSession::~ClientSession() {
  // Turn off any liveness checking:

	pEnvironment->DecrementReferenceCount();		//减少引用计数

	_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] GenericMediaServer::ClientSession::~ClientSession() BEGIN...\n", pEnvironment->GetEnvirName());

	pEnvironment->taskScheduler().unscheduleDelayedTask(fLivenessCheckTask);

  // Remove ourself from the server's 'client sessions' hash table before we go:
	char sessionIdStr[16] = {0};
  sprintf(sessionIdStr, "%08X", fOurSessionId);
  fOurServer.LockClientSession();		//Lock
  fOurServer.fClientSessions->Remove(sessionIdStr);
  fOurServer.UnlockClientSession();		//Unlock
  
  if (fOurServerMediaSession != NULL) {
    fOurServerMediaSession->decrementReferenceCount();
    if (fOurServerMediaSession->referenceCount() == 0 ){
	//&& fOurServerMediaSession->deleteWhenUnreferenced()) {

		if (pEnvironment->GetReferenceCount() < 1)		//检查一下，是否有新的客户端访问当前fOurServerMediaSession
		{
			pEnvironment->SetStreamStatus(0);
			//fOurServerMediaSession->SetStreamStatus(0);
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  BEGIN\n", pEnvironment->GetEnvirName(), this);
			fOurServer.removeServerMediaSession(pEnvironment, fOurServerMediaSession, lockClientFlag);
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  END\n", pEnvironment->GetEnvirName(), this);
			fOurServerMediaSession = NULL;
		}
		else
		{
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] *********removeServerMediaSession: 当前通道的clientSession已全部删除. 但又出现新的客户端访问, 故此处不删除serverMediaSession[%s]\n", 
				pEnvironment->GetEnvirName(), fOurServerMediaSession->streamName());
		}

		/*
#ifdef LIVE_MULTI_THREAD_ENABLE
		if (NULL != pEnvironment && assignSink)
		{
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  BEGIN\n", pEnvironment->GetEnvirName(), this);
			fOurServer.removeServerMediaSession(pEnvironment, fOurServerMediaSession, lockClientFlag);
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  END\n", pEnvironment->GetEnvirName(), this);
		}
		else if (! assignSink)
		{
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  BEGIN\n", envir().GetEnvirName(), this);

			fOurServer.removeServerMediaSession(&envir(), fOurServerMediaSession, lockClientFlag);

			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] removeServerMediaSession: 0x%X  BEGIN\n", envir().GetEnvirName(), this);
		}
#else
		{
			fOurServer.removeServerMediaSession(&envir(), fOurServerMediaSession, True);
		}
#endif
		*/
      
    }
  }


  //在此处创建一个延时任务

  ClientConnection *pClientConnection = (ClientConnection *)GetClientConnection();
  if (pClientConnection && pClientConnection->GetFlag()==LIVE_FLAG)
  {
	pClientConnection->noteLivenessConnection();
  }
  //pEnvironment->taskScheduler().scheduleDelayedTask(1000*1000*5,  (TaskFunc*)CloseConnectionTask, GetClientConnection());

  _TRACE(TRACE_LOG_DEBUG, (char*)"[%s] GenericMediaServer::ClientSession::~ClientSession() END...\n", pEnvironment->GetEnvirName());

  mFlag = 0x00;
}


void GenericMediaServer::ClientSession::noteLiveness() {

  if (fOurServerMediaSession != NULL) fOurServerMediaSession->noteLiveness();

  if (fOurServer.fReclamationSeconds > 0) {
		pEnvironment->taskScheduler().rescheduleDelayedTask(fLivenessCheckTask,
							  (fOurServer.fReclamationSeconds+60)*1000000,
							  (TaskFunc*)livenessTimeoutTask, this);
/*
#ifdef LIVE_MULTI_THREAD_ENABLE
	  if (NULL != pEnvironment && assignSink)
	  {
		pEnvironment->taskScheduler().rescheduleDelayedTask(fLivenessCheckTask,
							  fOurServer.fReclamationSeconds*1000000,
							  (TaskFunc*)livenessTimeoutTask, this);
	  }
	  else if (! assignSink)
	  {
		envir().taskScheduler().rescheduleDelayedTask(fLivenessCheckTask,
							  fOurServer.fReclamationSeconds*1000000,
							  (TaskFunc*)livenessTimeoutTask, this);
	  }
#else
	  {
		envir().taskScheduler().rescheduleDelayedTask(fLivenessCheckTask,
							  fOurServer.fReclamationSeconds*1000000,
							  (TaskFunc*)livenessTimeoutTask, this);
	  }
#endif
*/
  }
}

void GenericMediaServer::ClientSession::noteClientLiveness(ClientSession* clientSession) {
  clientSession->noteLiveness();
}


void GenericMediaServer::ClientSession::livenessTimeoutTask(ClientSession* clientSession) {
  // If this gets called, the client session is assumed to have timed out, so delete it:
#ifdef DEBUG
  char const* streamName
    = (clientSession->fOurServerMediaSession == NULL) ? "???" : clientSession->fOurServerMediaSession->streamName();
  fprintf(stderr, "Client session (id \"%08X\", stream name \"%s\") has timed out (due to inactivity)\n",
	  clientSession->fOurSessionId, streamName);
#endif

	if (NULL == clientSession)        return;
  if (clientSession->GetFlag() == LIVE_FLAG)
  {
	clientSession->mFlag = 0x00;
	delete clientSession;
  }
}


#ifdef LIVE_MULTI_THREAD_ENABLE
GenericMediaServer::ClientSession* GenericMediaServer::createNewClientSessionWithId(UsageEnvironment	*_pEnv, char *pSessionIdStr, void *pClientConnection) {
#else
GenericMediaServer::ClientSession* GenericMediaServer::createNewClientSessionWithId() {
#endif
  LockClientSession();		//Lock

  u_int32_t sessionId;
  char sessionIdStr[16] = {0};

  // Choose a random (unused) 32-bit integer for the session id
  // (it will be encoded as a 8-digit hex number).  (We avoid choosing session id 0,
  // because that has a special use by some servers.)
  do {
	  //sessionId = (u_int32_t)our_random32();
		static u_int32_t clientSessionId = 0x00FFFFFF;//100;
		if (clientSessionId < 0x7FFFFFFF)
		{
			sessionId = (clientSessionId++);
		}
		else
		{
			clientSessionId = 100;
			sessionId = (clientSessionId++);
		}

		snprintf(sessionIdStr, sizeof sessionIdStr, "%08X", sessionId);
  } while (sessionId == 0 || lookupClientSession(sessionIdStr) != NULL);

  ClientSession* clientSession = createNewClientSession(sessionId, _pEnv);
  
  if (clientSession != NULL)
  {
	  fClientSessions->Add(sessionIdStr, clientSession);
	  clientSession->SetClientConnection(pClientConnection);
	  if (NULL != pSessionIdStr)	strcpy(pSessionIdStr, sessionIdStr);	//此处返回生成的sessionId, 后续要根据该值找到对应的ClientSession
  }

  UnlockClientSession();		//Unlock

  return clientSession;
}

GenericMediaServer::ClientSession*
GenericMediaServer::lookupClientSession(u_int32_t sessionId) {
	char sessionIdStr[16] = {0};
  snprintf(sessionIdStr, sizeof sessionIdStr, "%08X", sessionId);
  return lookupClientSession(sessionIdStr);
}

GenericMediaServer::ClientSession*
GenericMediaServer::lookupClientSession(char const* sessionIdStr) {
	if (NULL == fClientSessions)		return NULL;		//gavin 2018.03.14
  return (GenericMediaServer::ClientSession*)fClientSessions->Lookup(sessionIdStr);
}


////////// ServerMediaSessionIterator implementation //////////

GenericMediaServer::ServerMediaSessionIterator
::ServerMediaSessionIterator(GenericMediaServer& server)
  : fOurIterator((server.fServerMediaSessions == NULL)
		 ? NULL : HashTable::Iterator::create(*server.fServerMediaSessions)) {
}

GenericMediaServer::ServerMediaSessionIterator::~ServerMediaSessionIterator() {
  delete fOurIterator;
}

ServerMediaSession* GenericMediaServer::ServerMediaSessionIterator::next() {
  if (fOurIterator == NULL) return NULL;

  char const* key; // dummy
  return (ServerMediaSession*)(fOurIterator->next(key));
}


////////// UserAuthenticationDatabase implementation //////////

UserAuthenticationDatabase::UserAuthenticationDatabase(AUTHENTICATION_TYPE_ENUM authType, char const* realm,
						       Boolean passwordsAreMD5)
  : fTable(HashTable::create(STRING_HASH_KEYS)),
    authenticationType(authType),
    fRealm(strDup(realm == NULL ? "LIVE555 Streaming Media" : realm)),
    fPasswordsAreMD5(passwordsAreMD5) {
}

UserAuthenticationDatabase::~UserAuthenticationDatabase() {
	if (NULL != fRealm)
	{
	  delete[] fRealm;
	  fRealm = NULL;
	}

  // Delete the allocated 'password' strings that we stored in the table, and then the table itself:
	if (NULL != fTable)
	{
	  char* password;
	  while ((password = (char*)fTable->RemoveNext()) != NULL) {
		delete[] password;
	  }
	  delete fTable;
	  fTable = NULL;
	}
}

void UserAuthenticationDatabase::addUserRecord(char const* username,
					       char const* password) {
  char* pw = (char*)(fTable->Lookup(username));
  if (NULL != pw)
  {
	  delete[] pw;
  }
  fTable->Add(username, (void*)(strDup(password)));
}

void UserAuthenticationDatabase::updateUserRecord(char const* username, char const *password)
{
	removeUserRecord(username);
	addUserRecord(username, password);
}

void UserAuthenticationDatabase::removeUserRecord(char const* username) {
	if (NULL != fTable)
	{
	  char* password = (char*)(fTable->Lookup(username));
	  fTable->Remove(username);
	  delete[] password;
	}
}

char const* UserAuthenticationDatabase::lookupPassword(char const* username) {
	if (NULL != fTable)
	{
		return (char const*)(fTable->Lookup(username));
	}
	else
	{
		return NULL;
	}
}
