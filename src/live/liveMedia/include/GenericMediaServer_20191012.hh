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
// C++ header

#ifndef _GENERIC_MEDIA_SERVER_HH
#define _GENERIC_MEDIA_SERVER_HH

#ifndef _MEDIA_HH
#include "Media.hh"
#endif
#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif

#include "../../../libRTSPServerAPI.h"
#include "../../../live_ipv4_ipv6.h"
#include <BasicUsageEnvironment.hh>

#ifndef REQUEST_BUFFER_SIZE
#define REQUEST_BUFFER_SIZE 20000 // for incoming requests
#endif
#ifndef RESPONSE_BUFFER_SIZE
#define RESPONSE_BUFFER_SIZE 20000
#endif

#define		LIVE_MULTI_THREAD_ENABLE		0x01

#ifdef LIVE_MULTI_THREAD_ENABLE
#include "../../../osthread.h"
#include "../../../osmutex.h"


#define MAX_BATCH_CLIENT_NUM		5
typedef struct __LIVE_THREAD_TASK_T
{
	int					id;
	TaskScheduler		*pSubScheduler;
	UsageEnvironment	*pSubEnv;
	char				liveURLSuffix[512];
	int					releaseChannel;		//释放标记
	//int					subSocket;
	//int					port;

	int					handleDescribe;

	OSTHREAD_OBJ_T		*osThread;

	int					clientConnectionNum;
	int					clientDisconnectNum;
	void				*pClientConnectionPtr[MAX_BATCH_CLIENT_NUM];
	void				*pClientDisconnectPtr[MAX_BATCH_CLIENT_NUM];
	void				*procPtr;
	void				*extPtr;
}LIVE_THREAD_TASK_T;

#define MAX_EPOLL_WORKER_THREAD_EVENT	(16+2)		
#define MAX_EPOLL_MAIN_THREAD_EVENT		256

#define	MAX_DEFAULT_MULTI_THREAD_NUM	256			//最大支持通道数


typedef struct __MultiThread_CORE_T
{
	int		threadNum;
	LIVE_THREAD_TASK_T	*threadTask;
}MultiThread_CORE_T;
#endif

class GenericMediaServer: public Medium {
public:
  void addServerMediaSession(UsageEnvironment	*pEnv, ServerMediaSession* serverMediaSession);

  virtual ServerMediaSession*
  lookupServerMediaSession(UsageEnvironment	*pEnv, int iType, void *pClientPtr, char const* streamName, Boolean bLockServerMediaSession, Boolean isFirstLookupInSession = True);

  int	removeServerMediaSession(UsageEnvironment *pEnv, ServerMediaSession* serverMediaSession, Boolean bLock, Boolean resetStreamName=True);
      // Removes the "ServerMediaSession" object from our lookup table, so it will no longer be accessible by new clients.
      // (However, any *existing* client sessions that use this "ServerMediaSession" object will continue streaming.
      //  The "ServerMediaSession" object will not get deleted until all of these client sessions have closed.)
      // (To both delete the "ServerMediaSession" object *and* close all client sessions that use it,
      //  call "deleteServerMediaSession(serverMediaSession)" instead.)
  int	removeServerMediaSession(UsageEnvironment *pEnv, char const* streamName, Boolean bLock);
     // ditto

  void closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession, Boolean bLock);
      // Closes (from the server) all client sessions that are currently using this "ServerMediaSession" object.
      // Note, however, that the "ServerMediaSession" object remains accessible by new clients.
  void closeAllClientSessionsForServerMediaSession(char const* streamName, Boolean bLock);
     // ditto

  void deleteServerMediaSession(ServerMediaSession* serverMediaSession);
      // Equivalent to:
      //     "closeAllClientSessionsForServerMediaSession(serverMediaSession); removeServerMediaSession(serverMediaSession);"
  void deleteServerMediaSession(char const* streamName);
      // Equivalent to:
      //     "closeAllClientSessionsForServerMediaSession(streamName); removeServerMediaSession(streamName);

  unsigned numClientSessions() const { return fClientSessions->numEntries(); }
  void	deleteAllWorkerThread(int flag);


  //add by gavin

  void	LockServerMediaSession(char *threadName, char *functionName, unsigned long long ptr);
  void	UnlockServerMediaSession(char *threadName, char *functionName, unsigned long long ptr);
  void	LockClientConnection();
  void	UnlockClientConnection();
  void	LockClientSession();
  void	UnlockClientSession();

  void	CloseAllConnections(char *streamName);
  void	ResetStreamNameInWorkThread(UsageEnvironment *pEnv, Boolean bLock);//, char *streamName);		//2018.09.10	当源返回失败时,需清空当前线程所占用的通道, 进行回收

  UsageEnvironment *GetEnvBySuffix(UsageEnvironment *pEnv, const char *urlSuffix, void *pClientConnection, LIVE_THREAD_TASK_T **pThreadTask, Boolean bLockServerMediaSession);		//gavin 2018.03.13
  UsageEnvironment *FindEnvByStreamName(const char *streamName);		//gavin 2019.02.12
  int	RemoveSocketFromEnv(UsageEnvironment *pEnv, int sockfd);

  void	*GetCallbackPtr()		{	return mCallbackPtr; }
  void	*GetUserPtr()			{	return mUserPtr; }

protected:
  GenericMediaServer(UsageEnvironment& env, int ourSocketV4, int ourSocketV6, Port ourPort,
		     unsigned reclamationSeconds, void *_callback, void *_userptr);
      // If "reclamationSeconds" > 0, then the "ClientSession" state for each client will get
      // reclaimed if no activity from the client is detected in at least "reclamationSeconds".
  // we're an abstract base class
  virtual ~GenericMediaServer();
  void cleanup(); // MUST be called in the destructor of any subclass of us

  static int setUpOurSocket4(UsageEnvironment& env, Port& ourPort);
  static int setUpOurSocket6(UsageEnvironment& env, Port& ourPort);

  static void incomingConnectionHandler4(void*, int /*mask*/);
  void incomingConnectionHandler4();
  static void incomingConnectionHandler6(void*, int /*mask*/);
  void incomingConnectionHandler6();
  void incomingConnectionHandlerOnSocket(int serverSocket, LIVE_IP_VER_ENUM ipVer);

#ifdef LIVE_MULTI_THREAD_ENABLE
  MultiThread_CORE_T	multiThreadCore;
  OSMutex				mutexClientConnection;
  OSMutex				mutexClientSession;
  OSMutex				mutexServerMediaSession;
  OSMutex                               mutexClose;

  void		*mCallbackPtr;
  void		*mUserPtr;  
#endif
public: // should be protected, but some old compilers complain otherwise
  // The state of a TCP connection used by a client:
  class ClientConnection {
  protected:
    ClientConnection(GenericMediaServer& ourServer, int clientSocket, LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr, LIVE_IP_VER_ENUM ipVer);
    virtual ~ClientConnection();

	UsageEnvironment& envir() { return *pClientConnectionEnv;}//fOurServer.envir(); }
    void closeSockets();
    void    SetLockFlag(int _flag)       {lockFlag = _flag;}

    static void incomingRequestHandler(void*, int /*mask*/);
    void incomingRequestHandler();
    virtual void handleRequestBytes(int newBytesRead, UsageEnvironment *pEnv) = 0;
    void resetRequestBuffer();
#ifdef LIVE_MULTI_THREAD_ENABLE
	UsageEnvironment	*pClientConnectionEnv;		//add by gavin
	int		recvBytes;		//add by gavin
	unsigned int	mFlag;
	int		initSessionFlag;		//初始化session标志,如未初始化就断开连接, 用此标志是否需要减少pEnv的引用计数
	int             lockFlag;               //2019.09.25

	void			ResetConnectionTask();
	void			noteLivenessConnection();
	static void CloseConnectionTask(ClientConnection *clientConnection);

#endif
  public:
	int		OnIncomingRequestHandler(UsageEnvironment *pEnv, void *, int);		//add by gavin 2018.03.14
	void	OnHandleRequestBytes(int newBytesRead, UsageEnvironment *pEnv);
	int		GetRecvBytes()	{return recvBytes;}
	
	char	*GetStreamName()	{return mStreamName;}
	unsigned int	GetFlag()	{return mFlag;}
	void	CloseConnection(int lockFlag);
  protected:
    friend class GenericMediaServer;
    friend class ClientSession;
    friend class RTSPServer; // needed to make some broken Windows compilers work; remove this in the future when we end support for Windows
    GenericMediaServer& fOurServer;
    int fOurSocket;
	LIVE_IP_VER_ENUM fOurIPVer;		//add by gavin 2018.03.26
    LIVE_NET_ADDRESS_SOCKADDR_IN fClientAddr;
	TaskToken fLivenessConnectionTask;	//add by gavin 2019.09.29
    char mStreamName[256];
    unsigned char fRequestBuffer[REQUEST_BUFFER_SIZE];
    unsigned char fResponseBuffer[RESPONSE_BUFFER_SIZE];
    unsigned fRequestBytesAlreadySeen, fRequestBufferBytesLeft;
  };

  // The state of an individual client session (using one or more sequential TCP connections) handled by a server:
  class ClientSession {
  protected:
    ClientSession(GenericMediaServer& ourServer, u_int32_t sessionId, UsageEnvironment	*pEnv);
    virtual ~ClientSession();

	UsageEnvironment& envir() { return *pEnvironment;}//fOurServer.envir(); }
    void noteLiveness();
    static void noteClientLiveness(ClientSession* clientSession);
    static void livenessTimeoutTask(ClientSession* clientSession);
	

  public:
	u_int32_t GetSessionId()		{return fOurSessionId;}
	UsageEnvironment *GetEnvir()	{return pEnvironment;}
	unsigned int	GetFlag()	{return mFlag;}

	void				*GetClientConnection()			{return pClientConnection;}
	void				SetClientConnection(void *pConnection)		{pClientConnection = pConnection;}
  protected:
    friend class GenericMediaServer;
    friend class ClientConnection;
    GenericMediaServer& fOurServer;
    u_int32_t fOurSessionId;
    ServerMediaSession* fOurServerMediaSession;
    TaskToken fLivenessCheckTask;

#ifdef LIVE_MULTI_THREAD_ENABLE
	UsageEnvironment	*pEnvironment;		//add by gavin
	Boolean				lockClientFlag;		//gavin 2018.06.11
	unsigned int		mFlag;//add by gavin
	void					*pClientConnection;		//gavin 2019.09.18
#endif
  };

protected:
  virtual ClientConnection* createNewClientConnection(int clientSocket, LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr, LIVE_IP_VER_ENUM ipVer) = 0;
  virtual ClientSession* createNewClientSession(u_int32_t sessionId, UsageEnvironment	*pEnv) = 0;

#ifdef LIVE_MULTI_THREAD_ENABLE
  ClientSession* createNewClientSessionWithId(UsageEnvironment	*pEnv, char *pSessionIdStr, void *pClientConnection);
#else
  ClientSession* createNewClientSessionWithId();
#endif
      // Generates a new (unused) random session id, and calls the "createNewClientSession()"
      // virtual function with this session id as parameter.

  // Lookup a "ClientSession" object by sessionId (integer, and string):
  ClientSession* lookupClientSession(u_int32_t sessionId);
  ClientSession* lookupClientSession(char const* sessionIdStr);

  // An iterator over our "ServerMediaSession" objects:
  class ServerMediaSessionIterator {
  public:
    ServerMediaSessionIterator(GenericMediaServer& server);
    virtual ~ServerMediaSessionIterator();
    ServerMediaSession* next();
  private:
    HashTable::Iterator* fOurIterator;
  };

protected:
  friend class ClientConnection;
  friend class ClientSession;	
  friend class ServerMediaSessionIterator;
  int fServerSocket4;
  int fServerSocket6;
  Port fServerPort;
  unsigned fReclamationSeconds;


  HashTable* GetClientSessions()		{return fClientSessions;}

private:
  HashTable* fServerMediaSessions; // maps 'stream name' strings to "ServerMediaSession" objects
  HashTable* fClientConnections; // the "ClientConnection" objects that we're using
  HashTable* fClientSessions; // maps 'session id' strings to "ClientSession" objects
};

// A data structure used for optional user/password authentication:

class UserAuthenticationDatabase {
public:
  UserAuthenticationDatabase(AUTHENTICATION_TYPE_ENUM authType, char const* realm = NULL, 
			     Boolean passwordsAreMD5 = False);
    // If "passwordsAreMD5" is True, then each password stored into, or removed from,
    // the database is actually the value computed
    // by md5(<username>:<realm>:<actual-password>)
  virtual ~UserAuthenticationDatabase();

  virtual void addUserRecord(char const* username, char const* password);
  virtual void updateUserRecord(char const* username, char const *password);
  virtual void removeUserRecord(char const* username);

  virtual char const* lookupPassword(char const* username);
      // returns NULL if the user name was not present

  char const* realm() { return fRealm; }
  Boolean passwordsAreMD5() { return fPasswordsAreMD5; }


  AUTHENTICATION_TYPE_ENUM authenticationType;
protected:
  HashTable* fTable;
  char* fRealm;
  Boolean fPasswordsAreMD5;
};

#endif
